########
# Data
########

#        CRAB task name          DAS name                                 
samples['TimeScan']   = ['/AlCaPhiSym/Run2017F-v1/RAW',  ['outputFile=templates.root'] ]


########
# Alternative global configuration
########

config.Data.splitting = 'LumiBased'    # FileBased
config.Data.unitsPerJob   = 10
config.Data.runRange = '305745-305757'


config.JobType.psetName = 'pulsedump_phisymm_lonebunch.py'
config.JobType.maxMemoryMB = 2500    # 2.5 GB

config.Data.outLFNDirBase = '/store/user/amassiro/ECAL/TimeScan'
config.General.workArea     = 'crab_projects_9Jan2018'
config.Data.allowNonValidInputDataset = True
#config.Data.lumiMask = "/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions15/13TeV/Cert_246908-260627_13TeV_PromptReco_Collisions15_ZeroTesla_25ns_JSON.txt"







