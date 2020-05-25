##################################################################################
# A Python wrapper for the computeBTagEfficienciesMC.cc program in the ewkino code #
##################################################################################

import sys
import os
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob

sampleDirectory = '/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino_fakerate'
years = ['2016','2017','2018']
cleanings = ['looseLeptons','uncleaned']

# set executable directory
btag_directory = os.path.abspath('../../weights/bTagSFCode')

# check if executable exists
if not os.path.exists(os.path.join(btag_directory,'computeBTagEfficienciesMC')):
    print('### ERROR ###: computeBTagEfficienciesMC executable does not seem to exist.')
    print('Go to ewkino/weights/bTagSFCode and run make -f makeComputeBTagEfficienciesMC.')
    sys.exit()

# make job scripts
for year in years:
    for cleaning in cleanings:
	script_name = 'btagefficiency.sh'
	with open( script_name, 'w') as script:
	    initializeJobScript( script )
	    script.write('cd '+btag_directory+'\n')
	    command = './computeBTagEfficienciesMC {} {} {}'.format(sampleDirectory,year,cleaning)
	    script.write( command )

	# submit job and catch errors 
	submitQsubJob( script_name, wall_time='168:00:00' )
	# alternative: run locally (for testing and debugging)
	#os.system('bash '+script_name)

