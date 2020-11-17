##########################################
# make impact plot for a given workspace #
##########################################
import os
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob

def makeimpactplot(workspace,runblind):
    ### run the commands to produce the impact plots for a given combine workspace (.root)
    
    workspace = os.path.abspath(workspace) # absolute path to workspace
    workspace_name = workspace.split('/')[-1].replace('.root','') # simple name of workspace
    subdir = workspace.replace('.root','') # absolute path to subdirectory for this job
    if os.path.exists(subdir): os.system('rm -r '+subdir)
    os.makedirs(subdir)
    json = workspace_name+'_impacts.json'
    fig = workspace_name+'_impacts'
    script_name = 'impactplot.sh'
    with open(script_name,'w') as script:
	initializeJobScript(script)
	command = 'combineTool.py -M Impacts -d '+workspace
	command += ' -m 100' # seems to be required argument, put any value?
	command += ' --rMin 0 --rMax 5'
	#command += ' --cminDefaultMinimizerStrategy 0'
	#command += ' --robustFit=1'
	if runblind: command += ' -t -1 --expectSignal=1'
	initfit = command + ' --doInitialFit\n'
	impacts = command + ' --doFits\n'
	output = command + ' --output {}\n'.format(json)
	plot = 'plotImpacts.py -i {} -o {}\n'.format(json,fig)
	script.write('cd {}\n'.format(subdir))
	script.write(initfit)
	script.write(impacts)
	script.write(output)
	script.write(plot)
    # submit job
    submitQsubJob(script_name)
    # alternative: run local
    #os.system('bash {}'.format(script_name))

if __name__=='__main__':
    
    runblind = True

    if len(sys.argv)!=2:
	print('### ERROR ###: no workspace provided as command line argument')
	sys.exit()

    workspace = sys.argv[1]
    if not os.path.exists(workspace):
	print('### ERROR ###: requested workspace does not seem to exist...')
	sys.exit()
    if not workspace[-5:]=='.root':
	print('### ERROR ###: need workspace in .root format')
	sys.exit()

    makeimpactplot(workspace,runblind)
