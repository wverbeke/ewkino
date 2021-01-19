##########################################
# make impact plot for a given workspace #
##########################################
import os
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

def makeimpactplot(workspace,runblind,expectsignal):
    ### run the commands to produce the impact plots for a given combine workspace (.root)
    
    workspace = os.path.abspath(workspace) # absolute path to workspace
    workspace_name = workspace.split('/')[-1].replace('.root','') # simple name of workspace
    appendix = ''
    #if runblind: appendix += '_blind'
    #if not expectsignal: appendix += '_bkg'
    subdir = workspace.replace('.root',appendix) # absolute path to subdirectory for this job
    if os.path.exists(subdir): os.system('rm -r '+subdir)
    os.makedirs(subdir)
    json = workspace_name+'_impacts.json'
    fig = workspace_name+'_impacts'
    command = 'combineTool.py -M Impacts -d '+workspace
    command += ' -m 100' # seems to be required argument, put any value?
    command += ' --rMin 0 --rMax 5'
    #command += ' --cminDefaultMinimizerStrategy 0'
    #command += ' --robustFit=1'
    if( runblind and expectsignal ): command += ' -t -1 --expectSignal 1'
    elif( runblind and not expectsignal ): command += ' -t -1 --expectSignal 0'
    setdir = 'cd {}'.format(subdir)
    initfit = command + ' --doInitialFit'
    impacts = command + ' --doFits'
    output = command + ' --output {}'.format(json)
    plot = 'plotImpacts.py -i {} -o {}'.format(json,fig)
    commands = [setdir,initfit,impacts,output,plot]
    # old qsub way:
    #script_name = 'impactplot.sh'
    #with open(script_name,'w') as script:
	#initializeJobScript(script)
	#script.write(setdir+'\n')
	#script.write(initfit+'\n')
	#script.write(impacts+'\n')
	#script.write(output+'\n')
	#script.write(plot+'\n')
    # submit the script    
    #submitQsubJob(script_name)
    # alternative: run local
    #os.system('bash {}'.format(script_name))
    # new condor way:
    ct.submitCommandsAsCondorJob('cjob_impactplot',commands)

if __name__=='__main__':
    
    runblind = True
    expectsignal = True

    # set workspace 
    if len(sys.argv)<2:
	print('### ERROR ###: need at least a workspace provided as command line argument')
	sys.exit()
    workspace = sys.argv[1]
    if not os.path.exists(workspace):
	print('### ERROR ###: requested workspace does not seem to exist...')
	sys.exit()
    if not workspace[-5:]=='.root':
	# in case a folder is given instead of a .root workspace, run over some files within it
	wspaces = ([
		    #'dc_combined_all.root',
		    #'dc_combined_2016.root',
		    #'dc_combined_2017.root',
		    #'dc_combined_2018.root',
		    'dc_combined_signalregion_cat1.root',
		    'dc_combined_signalregion_cat2.root',
		    'dc_combined_signalregion_cat3.root'
		])
	for wspace in wspaces:
	    cmd = 'python impactplot.py {}'.format(os.path.join(workspace,wspace))
	    print(cmd)
	    os.system(cmd)
	sys.exit()

    # set other args
    for arg in sys.argv[2:]:
	[argkey,argval] = arg.split('=',1)
	if argkey=='runblind': runblind = not (argval=='False')
	    # (use this instead of runblind = (argval==True) for safety)
	if argkey=='expectsignal': expectsignal = (argval=='True')

    makeimpactplot(workspace,runblind,expectsignal)
