##########################################################
# small utility for getting the lepton MVA scale factors #
##########################################################

# The scale factors for the lepton MVA are calculated by Kirill
# and stored in his public_html folder on T2.
# This script downloads the correct files and renames them for clarity.

import os
import sys

if __name__=='__main__':
    
    # initializations
    fpaths = []
    tpaths = []

    # common settings for electrons and muons
    targetdir = '../leptonSF'

    # settings for electrons
    ebasedir = '/user/kskovpen/public_html/egm_v1/plots/'
    ewps = ([	
		'VLoose',
		'Loose',
		'Medium',
		'Tight'
	    ])
    eyears = ({
		'UL2016APV': '2016PreVFP', 
		'UL2016': '2016PostVFP', 
		'UL2017': '2017',
		'UL2018': '2018'
	    })
    ebasename = 'leptonMVAUL_SF_electrons'

    # consider electrons
    # loop over years and working points
    for yearfolder,yeartag in eyears.items():
	for wp in ewps:
	    fpath = os.path.join(ebasedir,yearfolder,'passingLeptonMva{}'.format(wp),
			'egammaEffi.txt_EGM2D.root')
	    tpath = os.path.join(targetdir,'{}_{}_{}.root'.format(ebasename,wp,yeartag))
	    fpaths.append(fpath)
	    tpaths.append(tpath)

    # settings for muons
    mbasedir = '/user/kskovpen/public_html/muon_v1/muon_TagAndProbe/efficiencies/muon/generalTracks/Z/'
    mwps = ([
                'VLoose',
                'Loose',
                'Medium',
                'Tight'
            ])
    myears = ({
                'Run2016UL_HIPM': '2016PreVFP',
                'Run2016UL': '2016PostVFP',
                'Run2017UL': '2017',
                'Run2018UL': '2018'
            })
    mbasename = 'leptonMVAUL_SF_muons'

    # consider muons
    # loop over years and working points
    for yearfolder,yeartag in myears.items():
        for wp in mwps:
	    fname = 'NUM_LeptonMva{}_DEN_TrackerMuons'.format(wp)
            fpath = os.path.join(mbasedir,yearfolder,fname,fname+'_abseta_pt.root')
            tpath = os.path.join(targetdir,'{}_{}_{}.root'.format(mbasename,wp,yeartag))
	    fpaths.append(fpath)
	    tpaths.append(tpath)

    # do the copying
    for fpath,tpath in zip(fpaths,tpaths):
	cmd = 'cp {} {}'.format(fpath,tpath)
	if not os.path.exists(fpath):
	    print('WARNING: file {} does not exist.'.format(fpath))
	    continue
        print('copying {} to {}'.format(fpath,tpath))
        os.system(cmd)
