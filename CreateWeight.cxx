

void CreateWeight() {

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
  
  
  int nSamples = pulseShape.size(); 
  int nParams = 1 + int(doFitBaseline_) + int(doFitTime_);
  
  // Check if nSamples is large enough
  if (nSamples < nPulseSamples_ || (doFitBaseline_ && 
    nSamples < (nPulseSamples_ + nPrePulseSamples_))) {
    std::cout << "ComputeWeights::compute: Error: nSamples = "
    << nSamples << " is too small" << std::endl;
  return false;
    }//check samples
    
    // INITIALIZE WEIGHTS MATRICES
    if (weights_.num_row() != nSamples) {
      weights_ = CLHEP::HepMatrix(nSamples, nSamples, 0); // Fill matrices with zeros
      chi2_ = CLHEP::HepSymMatrix(nSamples, 0);
    } else {
      for (int iColumn = 0; iColumn < nSamples; iColumn++) {
        for (int iRow = 0; iRow < nParams; iRow++)
          weights_[iRow][iColumn] = 0.;
        for (int iRow = 0; iRow < nSamples; iRow++)
          chi2_[iRow][iColumn] = 0.;
      }
    }
    
  
}



