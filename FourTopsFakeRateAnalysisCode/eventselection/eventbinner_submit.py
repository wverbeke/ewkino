#################################################################################################
# A very simple submitter that runs eventbinner.py for a number of predefined regions and years #
#################################################################################################

import os
import sys

regions = []
for r in ['wzcontrolregion','zzcontrolregion','zgcontrolregion']: regions.append(r)
for r in ['nonprompt_trilepton_noossf','nonprompt_trilepton_noz']: regions.append(r)

years = ['2017']

selection_types = []
selection_types.append('3tight')
selection_types.append('3prompt')
selection_types.append('fakerate')

variation = 'nominal'
frdir = '../../fakeRate/fakeRateMaps' # put dummy path here if not using nonprompt from data

samplelistdir= '../samplelists/copyfromliam'

for year in years:
    # case 1: Liam's trilepton skims
    inputdir = '/pnfs/iihe/cms/store/user/lwezenbe/skimmedTuples/HNL/default/'
    inputyear = year.replace('PreVFP','pre').replace('PostVFP','post')
    inputdir += 'UL{}/Reco'.format(inputyear)
    samplelist = os.path.join(samplelistdir,'samples_{}.txt'.format(year))
    outputdir = 'output_20220224'
    outputdir = os.path.join(outputdir, year)
    regionstr = '+'.join(regions)
    stypestr = '+'.join(selection_types)
    cmd = 'python eventbinner.py '+inputdir+' '+samplelist+' '+outputdir
    cmd += ' '+regionstr+' '+stypestr+' '+variation+' '+frdir
    print('executing '+cmd)
    os.system(cmd)
