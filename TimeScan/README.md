Dump pulse shapes
----

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

    
    