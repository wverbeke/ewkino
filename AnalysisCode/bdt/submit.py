##################################################################
# A python script to submit tmvatrain.py as a job to the cluster #
##################################################################
import sys
import os
import json
import copy
from tmvatrain import tmvatrain
sys.path.append(os.path.abspath('../../skimmer/'))
from jobSubmission import initializeJobScript, submitQsubJob

### Declare a list of configurations
# each configuration is a dict of options to tmvatrain.py
configs = []

### Set output directory and check if already exists
outdir = 'temp2'
if os.path.exists(outdir):
        print('### WARNING ###: requested output folder already exists. Overwrite? (y/n)')
        go = raw_input()
        if go=='y': os.system('rm -r '+outdir)
        else: sys.exit()

### Define configurations
varlist = (['\"_abs_eta_recoil\"','\"_Mjj_max\"','\"_lW_asymmetry\"',
                '\"_deepCSV_max\"','\"_lT\"','\"_MT\"','\"_dPhill_max\"',
                '\"_pTjj_max\"','\"_dRlb_min\"','\"_HT\"','\"_dRlWrecoil\"',
		'\"_dRlWbtagged\"','\"_M3l\"','\"_abs_eta_max\"'])
varlist += (['\"_nJets\"','\"_nBJets\"']) # parametrized learning
indirs = ['\"/user/llambrec/Files/signalregion/2016MC_flat\"']
indirs.append('\"/user/llambrec/Files/signalregion/2017MC_flat\"')
indirs.append('\"/user/llambrec/Files/signalregion/2018MC_flat\"')
treenames = ['\"blackJackAndHookers/treeCat1\"']
treenames.append('\"blackJackAndHookers/treeCat2\"')
treenames.append('\"blackJackAndHookers/treeCat3\"')
default = ({'indirs':json.dumps(indirs,separators=(',',':')),
	    'treenames':json.dumps(treenames,separators=(',',':')),
	    'sigtag':'tZq',
	    'lopts':'SplitMode=Random:NormMode=None',
	    'fopts':'!H:!V:NTrees=800:BoostType=AdaBoost:MinNodeSize=5%:MaxDepth=3:SeparationType=GiniIndex:nCuts=20',
	    'varlist':json.dumps(varlist,separators=(',',':'))
	    })
configs.append(default)

'''for ntrees in ['400','800','1000','1200']:
    for minnodesize in ['1%','5%','10%']:
	    for maxdepth in ['2','3','4']:
		for ncuts in ['50','200']:
		    for alg in ['AdaBoost','Grad']:
			newdict = copy.deepcopy(default)
			newdict['fopts'] = '!H:!V:NTrees='+ntrees+':BoostType='+alg+':MinNodeSize='+minnodesize+':MaxDepth='+maxdepth+':nCuts='+ncuts
			if alg=='Grad':
			    for bag in ['0.5','1.']:
				for shrinkage in ['0.05','0.1','0.5']:
				    newdict = copy.deepcopy(default)
				    newdict['fopts'] = '!H:!V:NTrees='+ntrees+':BoostType='+alg+':MinNodeSize='+minnodesize+':MaxDepth='+maxdepth+':nCuts='+ncuts+':UseBaggedGrad=True:BaggedSampleFraction='+bag+':Shrinkage='+shrinkage
				    configs.append(newdict)
			else: configs.append(newdict)

if len(configs)>10:
    print('### WARNING ###: you are about to submit '+str(len(configs))+' jobs. Continue? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()'''

### Temp: redefine default
default['fopts'] = '!H:!V:NTrees=1200:BoostType=Grad:MinNodeSize=1%:MaxDepth=4:nCuts=200:Shrinkage=0.05'
configs = [default]

### Run over configurations and submit jobs
maindir = os.getcwd()
for jobn,config in enumerate(configs):
    os.chdir(maindir)
    # check if input folders exist
    exist = True
    indirs = [os.path.abspath(str(f)[1:-1]) for f in json.loads(config['indirs'])]
    for indir in indirs: 
	if(not (os.path.exists(indir))):
	    print('### ERROR ###: input folder '+indir+' not found.')
	    exist = False
    if not exist:
	print('               skipping this configuration...')
	continue
    # make a separate output folder for each job
    thisoutdir = os.path.join(outdir,'jobn'+str(jobn))
    os.makedirs(thisoutdir)
    os.chdir(thisoutdir)
    # make tmvatrain command and submit script
    script_name = 'tmvatrain.sh'
    command = 'python '+os.path.join(maindir,'tmvatrain.py')
    with open(script_name,'w') as script:
        initializeJobScript( script )
        script.write('cd {}\n'.format(thisoutdir))
	for key in config:
	    command += ' '+key+'='+config[key]
        script.write(command+'\n')
	script.write('echo ---command---\n')
	script.write('echo {}'.format(command))
    # for testing: run sequentially on m-machine
    #os.system(command)
    # actual submission
    submitQsubJob(script_name)
