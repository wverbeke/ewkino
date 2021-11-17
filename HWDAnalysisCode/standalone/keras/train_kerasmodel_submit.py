#####################################
# submitter for train_kerasmodel.py #
#####################################

import sys
import os
sys.path.append('../tools')
import optiontools as opt
import qsubTools as qt

if __name__=='__main__':

    # read options
    options = []
    options.append( opt.Option('inputdata', vtype='path') )
    options.append( opt.Option('configuration', vtype='path') )
    options.append( opt.Option('outputdir', vtype='path') )
    options.append( opt.Option('savemodel', vtype='bool', default=True) )
    options.append( opt.Option('savehistory', vtype='bool', default=True) )
    options.append( opt.Option('logfile', vtype='path') )
    options.append( opt.Option('dolocaltest', vtype='bool', default=True) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
	print('Found following configuration:')
	print(options)
	print('')	

    cmd = 'python train_kerasmodel.py'
    if options.inputdata is not None: 
	cmd += ' --inputdata {}'.format(options.inputdata)
    if options.configuration is not None: 
	cmd += ' --configuration {}'.format(options.configuration)
    if options.outputdir is not None: 
	cmd += ' --outputdir {}'.format(options.outputdir)
    cmd += ' --savemodel {}'.format(options.savemodel)
    cmd += ' --savehistory {}'.format(options.savehistory)
    if options.logfile is not None: 
	cmd += ' --logfile {}'.format(options.logfile)
    
    # run local test first
    if options.dolocaltest:
	testcmd = cmd + ' --istest True'
	print('running local test with following command:')
	print(testcmd)
	os.system(testcmd)
	print('test completed. Continue with job submission? (y/n)')
	go = raw_input()
	if not go=='y': sys.exit()

    # now submit a job
    qt.submitCommandsAsQsubJob( [cmd], 'qsub_train_kerasmodel.sh' )
