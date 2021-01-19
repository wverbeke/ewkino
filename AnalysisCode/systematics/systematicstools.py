#########################################################################
# grouping common functions needed by several scripts in this directory #
#########################################################################
import os
import sys
sys.path.append('../tools')
import smalltools as tls
# imports for sample list reading 
sys.path.append(os.path.abspath('../samplelists'))
from readsamplelist import readsamplelist
from extendsamplelist import extendsamplelist

def printfilestomerge(mcfiles,datafiles):
    ### for debugging or checking

    print('--- merging files: ---')
    print('  - MC files -')
    for f in mcfiles:
        print('    '+f)
    print('  - data files -')
    for f in datafiles:
        print('    '+f)

def getfilestomerge(topdir,year,region,npmode,samplelistdir):
    ### get a list of files to merge for this specific year, region and nonprompt mode
    # depends strongly on naming and folder structuring convention!!!

    mcpath = os.path.join(topdir,year+'MC',region)
    mcsamplelist = os.path.abspath(os.path.join(samplelistdir,'samplelist_tzq_'+year+'_MC.txt'))
    datapath = os.path.join(topdir,year+'data',region)
    datasamplelist = os.path.abspath(os.path.join(samplelistdir,'samplelist_tzq_'+year+'_data.txt'))
    # check if the necessary paths exist
    if npmode=='npfromsim':
        if( not (os.path.exists(mcpath+'_3tight')
                and os.path.exists(datapath+'_3tight') )):
            print('### ERROR ###: necessary folders do not exist for '+year+' '+region+' '+npmode)
            return []
    elif npmode=='npfromdata':
        if( not (os.path.exists(mcpath+'_3prompt')
                and os.path.exists(mcpath+'_fakerate')
                and os.path.exists(datapath+'_3tight')
                and os.path.exists(datapath+'_fakerate') )):
            print('### ERROR ###: necessary folders do not exist for '+year+' '+region+' '+npmode)
            return []
    else:
        print('### ERROR ###: npmode "'+npmode+'" not recognized')
        return []
    # intialize list of input files
    selmcfiles = []
    seldatafiles = []
    selnpdatafiles = []
    selpcorrectionfiles = []

    # select data files
    alldatafiles = [os.path.join(datapath+'_3tight',f) for f in os.listdir(datapath+'_3tight')
                        if f[-5:]=='.root']
    tokeep = ['data_combined']
    seldatafiles = tls.subselect(alldatafiles,tagstokeep=tokeep)
    # select mc files
    if npmode=='npfromsim':
        # use all files with regular 3-tight selection (ZGToLL is assumed to be excluded)
        allmcfiles = extendsamplelist(mcsamplelist,mcpath+'_3tight')
        selmcfiles = tls.subselect_inputfiles(allmcfiles,'3tight')
        selmcfiles = [f['file'] for f in selmcfiles]
    elif npmode=='npfromdata':
        # use prompt files with 3-prompt selection
        allmcfiles = extendsamplelist(mcsamplelist,mcpath+'_3prompt')
        selmcfiles = tls.subselect_inputfiles(allmcfiles,'3prompt')
        selmcfiles = [f['file'] for f in selmcfiles]
        allnpdatafiles = [os.path.join(datapath+'_fakerate',f)
                            for f in os.listdir(datapath+'_fakerate')
                            if f[-5:]=='.root']
        tokeep = ['data_combined']
        selnpdatafiles = tls.subselect(allnpdatafiles,tagstokeep=tokeep)
        allpcorrectionfiles = extendsamplelist(mcsamplelist,mcpath+'_fakerate')
        selpcorrectionfiles = tls.subselect_inputfiles(allpcorrectionfiles,'fakerate')
        selpcorrectionfiles = [f['file'] for f in selpcorrectionfiles]

    printfilestomerge(selmcfiles+selpcorrectionfiles,seldatafiles+selnpdatafiles)
    return selmcfiles+selpcorrectionfiles+seldatafiles+selnpdatafiles
