##########################################
# make impact plot for a given workspace #
##########################################
import os
import sys
import json
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

def makeimpactplot(workspace,usedata=False,expectsignal=True,poi='r'):
    ### run the commands to produce the impact plots for a given combine workspace (.root)
    
    workspace = os.path.abspath(workspace) # absolute path to workspace
    workspace_name = workspace.split('/')[-1].replace('.root','') # simple name of workspace
    # define a name appendix depending on the options
    appendix = ''
    if poi!='r': appendix += '_'+str(poi)
    if usedata: appendix += '_obs'
    if not expectsignal: appendix += '_bkg'
    subdir = workspace.replace('.root',appendix) # absolute path to subdirectory for this job
    if os.path.exists(subdir): os.system('rm -r '+subdir)
    os.makedirs(subdir)
    json = workspace_name+appendix+'_impacts.json'
    fig = workspace_name+appendix+'_impacts'
    command = 'combineTool.py -M Impacts -d '+workspace
    command += ' -m 100' # seems to be required argument, put any value?
    command += ' --rMin 0 --rMax 5'
    #command += ' --cminDefaultMinimizerStrategy 0'
    command += ' --robustFit=1'
    if poi!='r': command += ' --redefineSignalPOIs {}'.format(poi)
    if( not usedata and expectsignal ): command += ' -t -1 --expectSignal 1'
    elif( not usedata and not expectsignal ): command += ' -t -1 --expectSignal 0'
    setdir = 'cd {}'.format(subdir)
    initfit = command + ' --doInitialFit'
    impacts = command + ' --doFits --parallel 10'
    output = command + ' --output {}'.format(json)
    plot = 'plotImpacts.py -i {} -o {}'.format(json,fig)
    commands = [setdir,initfit,impacts,output,plot]
    # old qsub way:
    script_name = 'qsub_impactplot.sh'
    with open(script_name,'w') as script:
	initializeJobScript(script)
	script.write(setdir+'\n')
	script.write(initfit+'\n')
	script.write(impacts+'\n')
	script.write(output+'\n')
	script.write(plot+'\n')
    # submit the script    
    submitQsubJob(script_name)
    # alternative: run local
    #os.system('bash {}'.format(script_name))
    # new condor way:
    #ct.submitCommandsAsCondorJob('cjob_impactplot',commands)

if __name__=='__main__':
    
    usedata = False
    expectsignal = True
    pois = ['r']
    force = False

    ### parse arguments
    # first argument is the workspace or folder to run on, other arguments are keyword options

    # set other args
    for arg in sys.argv[2:]:
        [argkey,argval] = arg.split('=',1)
        if argkey=='usedata': usedata = (argval=='True' or argval=='true')
        elif argkey=='expectsignal': expectsignal = (argval=='True' or argval=='true')
        elif argkey=='pois': 
	    print(argval)
	    pois = json.loads(argval)
	elif argkey=='force': force = (argval=='True' or argval=='true')
        else: raise Exception('argument not recognized: {}'.format(arg))

    if not force:
	print('will run with following configuration:')
	print('  usedata: {}'.format(usedata))
	print('  expectsignal: {}'.format(expectsignal))
	print('  pois: {}'.format(pois))
	print('continue? (y/n)')
	go = raw_input()
	if not go=='y':
	    sys.exit()

    # set workspace 
    if len(sys.argv)<2:
	print('### ERROR ###: need at least a workspace provided as command line argument')
	sys.exit()
    workspace = sys.argv[1]
    if not os.path.exists(workspace):
	print('### ERROR ###: requested workspace does not seem to exist...')
	sys.exit()

    # in case workspace is a root file, run on that workspace
    if workspace[-5:]=='.root':
	for poi in pois:
	    makeimpactplot(workspace,usedata=usedata,expectsignal=expectsignal,poi=poi)

    # else it is assumed to be a directory, run on some workspaces in it
    else:
	wspaces = ([
		    'dc_combined_all.root',
		    'dc_combined_2016.root',
		    'dc_combined_2017.root',
		    'dc_combined_2018.root',
		    #'dc_combined_cat1.root',
		    #'dc_combined_cat2.root',
		    #'dc_combined_cat3.root',
		    #'dc_combined_sr_all.root',
                    #'dc_combined_sr_2016.root',
                    #'dc_combined_sr_2017.root',
                    #'dc_combined_sr_2018.root',
                    #'dc_combined_sr_cat1.root',
                    #'dc_combined_sr_cat2.root',
                    #'dc_combined_sr_cat3.root',
		    #'datacard_signalregion_cat1_2016.root',
		    #'datacard_signalregion_cat1_2017.root',
		    #'datacard_signalregion_cat1_2018.root',
		    #'datacard_signalregion_cat2_2016.root', 
                    #'datacard_signalregion_cat2_2017.root',
                    #'datacard_signalregion_cat2_2018.root',
		    #'datacard_signalregion_cat3_2016.root', 
                    #'datacard_signalregion_cat3_2017.root',
                    #'datacard_signalregion_cat3_2018.root',
		])
	for i,wspace in enumerate(wspaces):
	    wspace = os.path.join(workspace,wspace)
	    cmd = 'python impactplot.py {}'.format(wspace)
	    for arg in sys.argv[2:]: cmd += ' '+arg.replace('"',r'\"')
	    cmd += ' force=True'
	    print('executing following command: '+str(cmd))
	    os.system(cmd)
	sys.exit()

