###############################################################################
# Submit parallel jobs of train_kerasmodel, scanning the hyperparameter space #
###############################################################################

import sys
import os
import json
import copy
sys.path.append('../tools')
import optiontools as opt
import qsubTools as qt

def read_confspace( jsonfile ):
    with open(jsonfile, 'r') as f:
	jsonobj = json.load(f)
    conflist = []
    keys = list(jsonobj.keys())
    # read first configuration
    thisconf = {}
    for key in keys:
	thisconf[key] = jsonobj[key][0]
    conflist.append(thisconf)
    # read all configurations as incrimental modifications of the first one
    for key in keys[::-1]:
	values = jsonobj[key]
	conflistsnapshot = [copy.deepcopy(conf) for conf in conflist]
	for value in values[1:]:
	    for conf in conflistsnapshot:
		conf[key] = value
		conflist.append(copy.deepcopy(conf))
    return conflist

if __name__=='__main__':

    # read options
    options = []
    options.append( opt.Option('inputdata', vtype='path') )
    options.append( opt.Option('confspace', vtype='path') )
    options.append( opt.Option('outputdir', vtype='path') )
    options.append( opt.Option('dolocaltest', vtype='bool', default=True) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following options:')
        print(options)
        print('')

    # parse options
    if not options.inputdata[-4:]=='.npy':
        raise Exception('ERROR: input file is supposed to be an ndarray in .npy')
    if not os.path.exists(options.inputdata):
        raise Exception('ERROR: input file {} does not seem to exist...'.format(options.inputdata))
    if not options.confspace[-5:]=='.json':
        raise Exception('ERROR: confspace file is supposed to be in .json format')
    if not os.path.exists(options.confspace):
        raise Exception('ERROR: confspace file {} does not seem to exist...'.format(options.confspace))
    if os.path.exists(options.outputdir):
	raise Exception('ERROR: output directory {} exists;'.format(options.outputdir)
			    +' please remove it manually in case you want to overwrite.')
    os.makedirs(options.outputdir)
 
    # read list of configurations
    conflist = read_confspace( options.confspace )
    print('Found following configurations:')
    for conf in conflist:
	print('  - {}'.format(conf))
    print('Found {} configurations.'.format(len(conflist)))
    print('Continue? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()

    # make basic command
    cmd = 'python train_kerasmodel.py'
    cmd += ' --inputdata {}'.format(options.inputdata)
    cmd += ' --savemodel True'
    cmd += ' --savehistory True'

    # run local test first
    if options.dolocaltest:
	thisconffile = os.path.join(options.outputdir,'configuration_localtest.json')
        with open(thisconffile, 'w') as f: json.dump(conflist[0], f)
	testcmd = cmd + ' --configuration {}'.format(thisconffile)
        testcmd += ' --istest True'
        print('running local test with following command:')
        print(testcmd)
        os.system(testcmd)
        print('test completed. Continue with job submission? (y/n)')
        go = raw_input()
        if not go=='y': sys.exit()

    # loop over configurations and submit a job for each
    for i,conf in enumerate(conflist):
	# make a job directory
	thisjobdir = os.path.join(options.outputdir,'job{}'.format(i))
	os.makedirs(thisjobdir)
	# write the configuration
	thisconffile = os.path.join(thisjobdir,'configuration.json')
	with open(thisconffile, 'w') as f:
	    json.dump(conf, f)
	# initialize the log file
	thislogfile = os.path.join(thisjobdir,'log.txt')
	# make the specific command for this job
	thiscmd = cmd + ' --outputdir {}'.format(thisjobdir)
	thiscmd += ' --configuration {}'.format(thisconffile)
	thiscmd += ' &>{}'.format(thislogfile)
	# submit the command
	qt.submitCommandsAsQsubJob( [thiscmd], 'qsub_train_kerasmodel.sh' )
