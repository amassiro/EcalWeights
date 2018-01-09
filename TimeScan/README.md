Dump pulse shapes
----

Where:

    /afs/cern.ch/user/a/amassiro/work/ECAL/LocalReco/CMSSW_9_2_7/src/EcalWeights/TimeScan/
    
    
Useful code for pulse shape analysis by E. Di Marco

    https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideCMSPhysicsObjectSchool2017ECAL

Code:

    https://github.com/emanueledimarco/EcalReconstruction



    
    

DAS input

    https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset%3D%2FAlCaPhiSym%2FRun2017*%2FRAW
    https://cmsweb.cern.ch/das/request?input=dataset%3D%2FAlCaPhiSym%2FRun2017F-v1%2FRAW&instance=prod%2Fglobal
    
In particular:

    https://cmsweb.cern.ch/das/request?input=dataset%20run%3D305746&instance=prod/global&idx=0&limit=10
    https://cmsweb.cern.ch/das/request?view=list&instance=prod%2Fglobal&input=file+dataset%3D%2FAlCaPhiSym%2FRun2017F-v1%2FRAW+run%3D305746
    
e.g.

    /store/data/Run2017F/AlCaPhiSym/RAW/v1/000/305/746/00000/8231DCE6-94BA-E711-994F-02163E013859.root
    /store/data/Run2017F/AlCaPhiSym/RAW/v1/000/305/746/00000/765552DB-94BA-E711-BE97-02163E0146A1.root
    
    
Dump:

    cmsRun pulsedump_phisymm_lonebunch.py   

    
    
    root -l templates.root
    pulseDump->cd()
    pulse_tree->Draw("pulse:position","abs(pulse[5])<500 && pulse > 0  && barrel", "colz")
    
    pulse_tree->Draw("pulse:position:lumi","abs(pulse[5])<500 && pulse > 0  && barrel")

    
    root -l templates.all.root
    pulseDump->cd()
    TCanvas cc;
    cc.Divide(2,1);
    cc.cd(1);
    pulse_tree->Draw("pulse:position","abs(pulse[5])<1000 && pulse > 0  && barrel  && lumi < 30", "colz")

    cc.cd(2);
    pulse_tree->Draw("pulse:position","abs(pulse[5])<1000 && pulse > 0  && barrel  && lumi > 30", "colz")


    
Crab
====

See details in:

    https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookCRAB3Tutorial#CRAB_configuration_parameters
    https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile

check if I have writing permissions:

    source /cvmfs/cms.cern.ch/crab3/crab.sh
    voms-proxy-init --voms cms

    crab checkwrite --site=T2_CH_CERN
    crab checkwrite --site=T2_CH_CERN  --lfn=/store/user/amassiro/ECAL/
    crab checkwrite --site=T2_CH_CERN  --lfn=/store/group/dpg_ecal/alca_ecalcalib/amassiro/


multicrab 

    python multicrab.py  samples.py        
    python multicrab.py  crab_projects_9Jan2018        status
    
    
    
    
    
    