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

def getavailableregions(topdir,year,npmode):
    ### get a list of available regions in a certain directory for a given year and npmode
    # depends strongly on naming and folder structuring convention!!!
    
    # list all directories in data
    if not os.path.exists( os.path.join(topdir,year+'data') ): return []
    datadirs = os.listdir(os.path.join(topdir,year+'data'))
    # extract potential regions from data dirs
    regions = []
    for datadir in datadirs:
	region = datadir
	region = region.replace('_3prompt','')
	region = region.replace('_3tight','')
	region = region.replace('_fakerate','')
	if not region in regions: regions.append(region)
    # check if necessary folders exist
    validregions = []
    for region in regions:
	checkfolders = checkfolderstomerge(topdir,year,region,npmode)
	if checkfolders: validregions.append(region)
    return validregions

def printfilestomerge(mcfiles,datafiles):
    ### for debugging or checking

    print('--- merging files: ---')
    print('  - MC files -')
    for f in mcfiles:
        print('    '+f)
    print('  - data files -')
    for f in datafiles:
        print('    '+f)

def checkfolderstomerge(topdir,year,region,npmode):

    mcpath = os.path.join(topdir,year+'MC',region)
    datapath = os.path.join(topdir,year+'data',region)
    # check if the necessary paths exist
    if npmode=='npfromsim':
        if( not (os.path.exists(mcpath+'_3tight')
                and os.path.exists(datapath+'_3tight') )):
            print('### ERROR ###: necessary folders do not exist for '+year+' '+region+' '+npmode)
            return False
	return True
    elif npmode=='npfromdata':
        if( not (os.path.exists(mcpath+'_3prompt')
                and os.path.exists(mcpath+'_fakerate')
                and os.path.exists(datapath+'_3tight')
                and os.path.exists(datapath+'_fakerate') )):
            print('### ERROR ###: necessary folders do not exist for '+year+' '+region+' '+npmode)
            return False
	return True
    else:
        print('### ERROR ###: npmode "'+npmode+'" not recognized')
        return False

def getfilestomerge(topdir,year,region,npmode,samplelistdir):
    ### get a list of files to merge for this specific year, region and nonprompt mode
    # depends strongly on naming and folder structuring convention!!!

    # first check if correct folders exist
    foldersexist = checkfolderstomerge(topdir,year,region,npmode)
    if not foldersexist: return []

    # set sample lists
    mcpath = os.path.join(topdir,year+'MC',region)
    mcsamplelist = os.path.abspath(os.path.join(samplelistdir,'samplelist_tzq_'+year+'_MC.txt'))
    mcsamplelistsys = os.path.abspath(os.path.join(samplelistdir,
					'samplelist_tzq_'+year+'_MC_sys.txt'))
    datapath = os.path.join(topdir,year+'data',region)
    datasamplelist = os.path.abspath(os.path.join(samplelistdir,'samplelist_tzq_'+year+'_data.txt'))
    if not os.path.exists(mcsamplelist):
	raise Exception('ERROR in getfilestomerge: samplelist {}'.format(mcsamplelist)
			+' does not seem to exist...')
    if not os.path.exists(datasamplelist):
        raise Exception('ERROR in getfilestomerge: samplelist {}'.format(datasamplelist)
                        +' does not seem to exist...')
    mcsamplelists = [mcsamplelist]
    if os.path.exists(mcsamplelistsys):
	mcsamplelists.append(mcsamplelistsys)
    else:
	print('WARNING in getfilestomerge: did not find sample list'
		+' with systematically varied samples;'
		+' taking only nominal sample list')
    
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
        # use all files with regular 3-tight selection
        allmcfiles = extendsamplelist(mcsamplelists,mcpath+'_3tight')
        selmcfiles = tls.subselect_inputfiles(allmcfiles,'3tight')
        selmcfiles = [f['file'] for f in selmcfiles]
    elif npmode=='npfromdata':
        # use prompt files with 3-prompt selection
        allmcfiles = extendsamplelist(mcsamplelists,mcpath+'_3prompt')
        selmcfiles = tls.subselect_inputfiles(allmcfiles,'3prompt')
        selmcfiles = [f['file'] for f in selmcfiles]
        allnpdatafiles = [os.path.join(datapath+'_fakerate',f)
                            for f in os.listdir(datapath+'_fakerate')
                            if f[-5:]=='.root']
        tokeep = ['data_combined']
        selnpdatafiles = tls.subselect(allnpdatafiles,tagstokeep=tokeep)
        allpcorrectionfiles = extendsamplelist(mcsamplelists,mcpath+'_fakerate')
        selpcorrectionfiles = tls.subselect_inputfiles(allpcorrectionfiles,'fakerate')
        selpcorrectionfiles = [f['file'] for f in selpcorrectionfiles]

    printfilestomerge(selmcfiles+selpcorrectionfiles,seldatafiles+selnpdatafiles)
    return selmcfiles+selpcorrectionfiles+seldatafiles+selnpdatafiles
