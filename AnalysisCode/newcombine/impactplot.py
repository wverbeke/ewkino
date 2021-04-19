##########################################
# make impact plot for a given workspace #
##########################################
import os
import sys
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct

def makeimpactplot(workspace,usedata,expectsignal):
    ### run the commands to produce the impact plots for a given combine workspace (.root)
    
    workspace = os.path.abspath(workspace) # absolute path to workspace
    workspace_name = workspace.split('/')[-1].replace('.root','') # simple name of workspace
    appendix = ''
    if usedata: appendix += '_obs'
    #appendix += '_nocmin' # temp when disabling cminDefaultMinimizerStrategy
    #if not expectsignal: appendix += '_bkg'
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
    command += ' --redefineSignalPOIs r_ratio' # temp for ratio measurement
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
	for wspace in wspaces:
	    wspace = os.path.join(workspace,wspace)
	    cmd = 'python impactplot.py {}'.format(wspace)
	    for arg in sys.argv[2:]: cmd += ' '+arg
	    print(cmd)
	    os.system(cmd)
	sys.exit()

    # set other args
    for arg in sys.argv[2:]:
	[argkey,argval] = arg.split('=',1)
	if argkey=='usedata': usedata = (argval=='True' or argval=='true')
	if argkey=='expectsignal': expectsignal = (argval=='True' or argval=='true')

    makeimpactplot(workspace,usedata,expectsignal)
