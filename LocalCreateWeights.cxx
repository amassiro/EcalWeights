//
// Create weights for ECAL local reconstruction from a pulse shape
// Standalone version (TMatrixD instead of CLHEP) of ComputeWeights::compute
// from EcalWeightsTools
//
//   W = (C^T C)^-1 C^T     (no noise correlation, i.e. unit covariance)
//
// with C the (nSamples x nParams) design matrix:
//   column 0               --> pulse shape (amplitude)
//   column 1 (doFitTime)   --> pulse shape derivative (amplitude * dt)
//   last col (doFitBaseline) --> 1 (baseline)
//
// chi2 matrix:  (1 - C W)^T (1 - C W)
//
// Run:
//   root -l -q LocalCreateWeights.cxx
//   root -l -q 'LocalCreateWeights.cxx(true, true, 3, 7, 3)'
//

void LocalCreateWeights(bool doFitBaseline = true, bool doFitTime = false, int nPrePulseSamples = 3, int nPulseSamples = 7, int firstPulseSample = 3) {

  std::cout << " Create weights from pulse shape " << std::endl;

  std::vector <float> pulseShape;

  //---- dummy pulse shape
  pulseShape.push_back(0.0);
  pulseShape.push_back(0.0);
  pulseShape.push_back(0.0);
  pulseShape.push_back(0.1);
  pulseShape.push_back(0.3);
  pulseShape.push_back(1.0);
  pulseShape.push_back(0.8);
  pulseShape.push_back(0.7);
  pulseShape.push_back(0.3);
  pulseShape.push_back(0.2);

  //---- derivative of the pulse shape, in units of 1/clock (25 ns)
  //---- FIXME: crude finite difference, to be replaced by the derivative of the true shape
  std::vector <float> pulseShapeDerivative;
  int nSamples = pulseShape.size();
  for (int iSample = 0; iSample < nSamples; iSample++) {
    float derivative = 0;
    if (iSample == 0)                  derivative = pulseShape[1] - pulseShape[0];
    else if (iSample == nSamples - 1)  derivative = pulseShape[nSamples-1] - pulseShape[nSamples-2];
    else                               derivative = 0.5 * (pulseShape[iSample+1] - pulseShape[iSample-1]);
    pulseShapeDerivative.push_back(derivative);
  }

  int nParams = 1 + int(doFitBaseline) + int(doFitTime);
  int iParamTime     = doFitTime     ? 1           : -1;
  int iParamBaseline = doFitBaseline ? nParams - 1 : -1;

  //---- check if nSamples is large enough
  if (firstPulseSample + nPulseSamples > nSamples || (doFitBaseline && nPrePulseSamples > firstPulseSample)) {
    std::cout << " Error: nSamples = " << nSamples << " is too small" << std::endl;
    return;
  }

  //---- design matrix C (nSamples x nParams), samples not used stay at 0
  TMatrixD coef (nSamples, nParams);
  coef.Zero();

  for (int iSample = firstPulseSample; iSample < firstPulseSample + nPulseSamples; iSample++) {
    coef(iSample, 0) = pulseShape[iSample];
    if (doFitTime)     coef(iSample, iParamTime) = pulseShapeDerivative[iSample];
    if (doFitBaseline) coef(iSample, iParamBaseline) = 1.;
  }

  //---- pre-pulse samples only constrain the baseline
  if (doFitBaseline) {
    for (int iSample = firstPulseSample - nPrePulseSamples; iSample < firstPulseSample; iSample++) {
      coef(iSample, iParamBaseline) = 1.;
    }
  }

  //---- W = (C^T C)^-1 C^T   (nParams x nSamples)
  TMatrixD coefT (TMatrixD::kTransposed, coef);
  TMatrixD normal = coefT * coef;
  double det = 0;
  normal.Invert(&det);
  if (det == 0) {
    std::cout << " Error: singular matrix, check pulse shape and sample ranges" << std::endl;
    return;
  }
  TMatrixD weights = normal * coefT;

  //---- chi2 = (1 - C W)^T (1 - C W)   (nSamples x nSamples)
  TMatrixD identity (nSamples, nSamples);
  identity.UnitMatrix();
  TMatrixD residual = identity - coef * weights;
  TMatrixD residualT (TMatrixD::kTransposed, residual);
  TMatrixD chi2 = residualT * residual;

  //---- print
  std::cout << " nSamples = " << nSamples << " nParams = " << nParams << std::endl;
  std::cout << " parameters: 0 = amplitude";
  if (doFitTime)     std::cout << ", " << iParamTime << " = amplitude * dt [clock]";
  if (doFitBaseline) std::cout << ", " << iParamBaseline << " = baseline";
  std::cout << std::endl;

  std::cout << std::endl << " weights: " << std::endl;
  for (int iParam = 0; iParam < nParams; iParam++) {
    std::cout << "   [" << iParam << "] ";
    for (int iSample = 0; iSample < nSamples; iSample++) {
      std::cout << std::setw(10) << std::setprecision(4) << std::fixed << weights(iParam, iSample) << " ";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl << " chi2 matrix: " << std::endl;
  chi2.Print();

  //---- closure: weights applied to the pulse shape itself
  std::cout << " closure (weights x pulse shape): " << std::endl;
  for (int iParam = 0; iParam < nParams; iParam++) {
    double sum = 0;
    for (int iSample = 0; iSample < nSamples; iSample++) sum += weights(iParam, iSample) * pulseShape[iSample];
    std::cout << "   [" << iParam << "] = " << sum << std::endl;
  }

}
