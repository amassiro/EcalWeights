# EcalWeights

Standalone code to transform a shape into weights for ECAL local reconstruction.
To be then migrated to CMSSW.

Original source: 

    http://theofil.web.cern.ch/theofil/ecal/EcalWeightsTools_CMSSW350_pre.tar.bz2





    
Useful code for pulse shape analysis by E. Di Marco

    https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideCMSPhysicsObjectSchool2017ECAL

Code:

    https://github.com/emanueledimarco/EcalReconstruction


Dump the time-scan using the aforementioned code

    TimeScan/
    


Phase-2
====

ECAL phase-2 barrel: 16 samples at 160 MHz (6.25 ns), gains x10 and x1 (DataFormats/EcalDigi/interface/EcalConstants.h).
Weights used in CMSSW: RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitPhase2WeightsProducer.cc (ampWeights, timeWeights, hardcoded defaults).

Dump the simulation pulse shape from the conditions DB (via Frontier, no CMSSW needed):

    python3 DumpSimPulseShape.py -t EcalSimPulseShapes_TB2025_v1 -o tmp/shape_TB2025_v1.txt

Create the weights (16 samples, amplitude, amplitude*dt, baseline):

    root -l -q 'LocalCreateWeightsPhase2.cxx("tmp/shape_TB2025_v1.txt", "tmp/weights_TB2025_v1.txt")'
    root -l -q 'LocalCreateWeightsPhase2.cxx("tmp/shape_TB2025_v1.txt", "tmp/weights_TB2025_v1.txt", 32.25, true, true, true)'

Where:

    tPeak = time [ns] of the pulse maximum w.r.t. sample 0 (default 32.25 ns)
    last argument = use noise correlation G10 from 2023 TB (SimCalorimetry/EcalSimProducers/python/ecalDigiParameters_Ph2_cff.py)

NB: the CMSSW default ampWeights are NOT reproduced with the TB2025 shape (max difference ~0.06 with
uncorrelated noise, best tPeak = 32.25 ns; ~0.14 with the 2023 TB noise correlation):
they were derived from a different shape and/or procedure.
NB: in EcalUncalibRecHitPhase2WeightsProducer t0 = sum(timeWeights * adc) is not divided by the amplitude,
while here the time parameter is amplitude * dt.

Dump the samples from a phase-2 sample and apply the weights: see EcalPulseDumper (SamplesTreeProducer, test/ApplyWeights.cxx).

