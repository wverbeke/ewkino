####################################################
# run the combine executable on the test datacards #
####################################################

import os
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct
sys.path.append(os.path.abspath('../tools'))
import listtools as lt
sys.path.append(os.path.abspath('../newcombine'))
import combinetools as cbt

def cardtochannel( card ):
    return card.replace('datacard_','').replace('dc_combined','').replace('.txt','')

if __name__=="__main__":

    # parse command line args
    if len(sys.argv)<2:
	print('### ERROR ###: need at least one command line arg (input folder)')
	sys.exit()
    datacarddir = os.path.abspath(sys.argv[1])
    usedata = False
    runfast = False
    dostatonly = True
    runlocal = False
    runqsub = False
    for arg in sys.argv[2:]:
	if arg=='usedata': usedata = True
	elif arg=='runfast': runfast = True
	elif arg=='nostat': dostatonly = False
	elif arg=='runlocal': runlocal = True
	elif arg=='runqsub': runqsub = True
    runcondor = True
    if( runqsub or runlocal ): runcondor = False 

    # remove all previous output
    cbt.cleandatacarddir(datacarddir)

    # choose method
    method = 'fitdiagnostics'
    if runfast: method = 'multidimfit'
    #method = 'initimpacts' # temp for testing
 
    cards_all = [f for f in os.listdir(datacarddir) if f[-4:]=='.txt']

    for card in sorted(cards_all):
	print('running combine for '+card)
	commands = cbt.get_default_commands( datacarddir, card, method=method,
						includesignificance=not runfast,
						includestatonly=dostatonly, 
						includedata=usedata )
	if( runcondor ):
	    ct.submitCommandsAsCondorJob( 'cjob_runcombine', commands )
	else:
	    script_name = 'qsub_runcombine.sh'
	    with open( script_name, 'w' ) as script:
		initializeJobScript( script )
		for c in commands: script.write(c+'\n')
	    if runlocal:
		os.system('bash {}'.format(script_name))
	    elif runqsub:
		submitQsubJob( script_name ) 
