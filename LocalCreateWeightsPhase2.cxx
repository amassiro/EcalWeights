//
// Create weights for ECAL phase-2 barrel local reconstruction
//   - 16 samples at 160 MHz (6.25 ns), see DataFormats/EcalDigi/interface/EcalConstants.h
//   - pulse shape from DumpSimPulseShape.py output (fine grid, column "barrel")
//
//   W = (C^T N^-1 C)^-1 C^T N^-1
//
// with N the noise correlation matrix (unit matrix if useNoiseCorrelation = false)
// and C the (nSamples x nParams) design matrix:
//   column 0                 --> pulse shape (amplitude)
//   column 1 (doFitTime)     --> pulse shape derivative (amplitude * dt [ns])
//                                A f(t) + A dt f'(t) ~ A f(t + dt) --> dt > 0 means the pulse is earlier
//   last col (doFitBaseline) --> 1 (baseline)
//
// tPeak = time [ns] of the pulse maximum w.r.t. sample 0 (sample i is at t = i * 6.25 ns)
//
// Output: weights in a text file, one line per parameter (amplitude, [amplitude*dt], [baseline])
//
// Run:
//   root -l -q 'LocalCreateWeightsPhase2.cxx("tmp/shape_TB2025_v1.txt", "weights_TB2025_v1.txt")'
//   root -l -q 'LocalCreateWeightsPhase2.cxx("tmp/shape_TB2025_v1.txt", "weights_TB2025_v1.txt", 32.25, true, true, true)'
//


//---- linear interpolation on the fine grid, flat outside
float interpolate(std::vector<float>& x, std::vector<float>& y, float xx) {
  if (xx <= x.front()) return y.front();
  if (xx >= x.back())  return y.back();
  int i = int( (xx - x.front()) / (x[1] - x[0]) );
  if (i >= int(x.size()) - 1) i = int(x.size()) - 2;
  float f = (xx - x[i]) / (x[i+1] - x[i]);
  return y[i] + f * (y[i+1] - y[i]);
}


void LocalCreateWeightsPhase2(std::string shapeFile = "tmp/shape_TB2025_v1.txt", std::string outputFile = "weights_Phase2.txt", float tPeak = 32.25, bool doFitBaseline = true, bool doFitTime = true, bool useNoiseCorrelation = false) {

  std::cout << " Create phase-2 weights from pulse shape: " << shapeFile << std::endl;

  const int nSamples = 16;
  const float samplingPeriod = 6.25;  // ns

  //---- noise correlation G10 from 2023 TB, as in SimCalorimetry/EcalSimProducers/python/ecalDigiParameters_Ph2_cff.py
  float noiseCorrelation[nSamples] = {1.0000, 0.6986, 0.3782, 0.2452, 0.2900, 0.3532, 0.3911, 0.3896,
                                      0.3892, 0.3842, 0.3796, 0.3711, 0.3763, 0.3791, 0.3867, 0.3820};

  //---- read fine pulse shape: time  barrel  endcap  apd
  std::vector <float> fine_time;
  std::vector <float> fine_shape;
  std::ifstream file (shapeFile.c_str());
  if (!file.is_open()) {
    std::cout << " Error: cannot open " << shapeFile << std::endl;
    return;
  }
  std::string line;
  while (std::getline(file, line)) {
    if (line.size() == 0 || line[0] == '#') continue;
    std::istringstream iss (line);
    float t, barrel;
    iss >> t >> barrel;
    fine_time.push_back(t);
    fine_shape.push_back(barrel);
  }
  file.close();
  std::cout << " read " << fine_time.size() << " points, step = " << fine_time[1] - fine_time[0] << " ns" << std::endl;

  //---- derivative on the fine grid [1/ns]
  std::vector <float> fine_derivative;
  int nFine = fine_time.size();
  for (int i = 0; i < nFine; i++) {
    int iLow  = (i == 0)         ? 0           : i - 1;
    int iHigh = (i == nFine - 1) ? nFine - 1   : i + 1;
    fine_derivative.push_back( (fine_shape[iHigh] - fine_shape[iLow]) / (fine_time[iHigh] - fine_time[iLow]) );
  }

  //---- align the maximum of the shape to tPeak
  int iMax = std::max_element(fine_shape.begin(), fine_shape.end()) - fine_shape.begin();
  float tMaxShape = fine_time[iMax];

  std::vector <float> pulseShape;
  std::vector <float> pulseShapeDerivative;
  for (int iSample = 0; iSample < nSamples; iSample++) {
    float t = iSample * samplingPeriod - tPeak + tMaxShape;
    pulseShape.push_back(interpolate(fine_time, fine_shape, t));
    pulseShapeDerivative.push_back(interpolate(fine_time, fine_derivative, t));
  }

  int nParams = 1 + int(doFitBaseline) + int(doFitTime);
  int iParamTime     = doFitTime     ? 1           : -1;
  int iParamBaseline = doFitBaseline ? nParams - 1 : -1;

  //---- design matrix C (nSamples x nParams)
  TMatrixD coef (nSamples, nParams);
  coef.Zero();
  for (int iSample = 0; iSample < nSamples; iSample++) {
    coef(iSample, 0) = pulseShape[iSample];
    if (doFitTime)     coef(iSample, iParamTime) = pulseShapeDerivative[iSample];
    if (doFitBaseline) coef(iSample, iParamBaseline) = 1.;
  }

  //---- inverse noise matrix
  TMatrixD noiseInverse (nSamples, nSamples);
  for (int iRow = 0; iRow < nSamples; iRow++) {
    for (int iColumn = 0; iColumn < nSamples; iColumn++) {
      if (useNoiseCorrelation) noiseInverse(iRow, iColumn) = noiseCorrelation[abs(iRow - iColumn)];
      else                     noiseInverse(iRow, iColumn) = (iRow == iColumn) ? 1. : 0.;
    }
  }
  double det = 0;
  noiseInverse.Invert(&det);

  //---- W = (C^T N^-1 C)^-1 C^T N^-1   (nParams x nSamples)
  TMatrixD coefT (TMatrixD::kTransposed, coef);
  TMatrixD normal = coefT * noiseInverse * coef;
  normal.Invert(&det);
  if (det == 0) {
    std::cout << " Error: singular matrix, check pulse shape and tPeak" << std::endl;
    return;
  }
  TMatrixD weights = normal * coefT * noiseInverse;

  //---- print
  std::cout << " tPeak = " << tPeak << " ns, noise correlation = " << useNoiseCorrelation << std::endl;
  std::cout << " parameters: 0 = amplitude";
  if (doFitTime)     std::cout << ", " << iParamTime << " = amplitude * dt [ns]";
  if (doFitBaseline) std::cout << ", " << iParamBaseline << " = baseline";
  std::cout << std::endl;

  std::cout << std::endl << " sampled pulse shape: " << std::endl << "   ";
  for (int iSample = 0; iSample < nSamples; iSample++) std::cout << std::setw(8) << std::setprecision(4) << std::fixed << pulseShape[iSample] << " ";
  std::cout << std::endl;

  std::cout << std::endl << " weights: " << std::endl;
  for (int iParam = 0; iParam < nParams; iParam++) {
    std::cout << "   [" << iParam << "] ";
    for (int iSample = 0; iSample < nSamples; iSample++) {
      std::cout << std::setw(9) << std::setprecision(5) << std::fixed << weights(iParam, iSample) << " ";
    }
    std::cout << std::endl;
  }

  //---- closure: weights applied to the sampled pulse shape (+ constant baseline)
  std::cout << std::endl << " closure (weights x (pulse shape + 100)): " << std::endl;
  for (int iParam = 0; iParam < nParams; iParam++) {
    double sum = 0;
    for (int iSample = 0; iSample < nSamples; iSample++) sum += weights(iParam, iSample) * (pulseShape[iSample] + 100.);
    std::cout << "   [" << iParam << "] = " << sum << std::endl;
  }

  //---- cfg format, to be used in EcalUncalibRecHitPhase2WeightsProducer
  //---- NB: in the producer t0 = sum(timeWeights * adc), not divided by the amplitude
  std::cout << std::endl << " ampWeights = cms.vdouble(";
  for (int iSample = 0; iSample < nSamples; iSample++) std::cout << std::setprecision(6) << weights(0, iSample) << (iSample < nSamples - 1 ? ", " : ")");
  std::cout << std::endl;
  if (doFitTime) {
    std::cout << " timeWeights (amplitude * dt [ns]) = cms.vdouble(";
    for (int iSample = 0; iSample < nSamples; iSample++) std::cout << std::setprecision(6) << weights(iParamTime, iSample) << (iSample < nSamples - 1 ? ", " : ")");
    std::cout << std::endl;
  }

  //---- write weights to file
  std::ofstream out (outputFile.c_str());
  out << "# shape " << shapeFile << " tPeak " << tPeak << " noiseCorrelation " << useNoiseCorrelation << std::endl;
  out << "# rows: amplitude";
  if (doFitTime)     out << ", amplitude*dt[ns]";
  if (doFitBaseline) out << ", baseline";
  out << std::endl;
  for (int iParam = 0; iParam < nParams; iParam++) {
    for (int iSample = 0; iSample < nSamples; iSample++) out << std::setprecision(7) << weights(iParam, iSample) << " ";
    out << std::endl;
  }
  out.close();
  std::cout << std::endl << " written: " << outputFile << std::endl;

}
