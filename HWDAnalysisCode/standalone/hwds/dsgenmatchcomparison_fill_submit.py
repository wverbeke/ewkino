##############################################
# submitter for dsgenmatchcomparison_fill.py #
##############################################

import sys
import os
sys.path.append('../tools')
import optiontools as opt
import condorTools as ct
import qsubTools as qt

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputdir', vtype='path') )
    options.append( opt.Option('treename', default='blackJackAndHookers/blackJackAndHookersTree') )
    options.append( opt.Option('variables', vtype='path') )
    options.append( opt.Option('nprocess', vtype='int', default=-1) )
    options.append( opt.Option('outputdir', default=None) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    # load input files
    if not os.path.exists(options.inputdir):
	raise Exception('ERROR: input directory {} does not exist'.format(options.inputdir))
    rootfiles = ([os.path.join(options.inputdir, f) 
		  for f in os.listdir(options.inputdir) if f[-5:]=='.root'])

    # make output dir
    if not os.path.exists(options.outputdir):
	os.makedirs(options.outputdir)
    
    # loop over files and submit jobs
    for f in rootfiles:
	outputfilename = os.path.basename(f).replace('.root','_histograms.root')
	cmd = 'python dsgenmatchcomparison_fill.py'
	cmd += ' --inputfile {}'.format(f)
	cmd += ' --outputfile {}'.format(os.path.join(options.outputdir, outputfilename))
	cmd += ' --treename {}'.format(options.treename)
	cmd += ' --variables {}'.format(options.variables)
	cmd += ' --nprocess {}'.format(options.nprocess)

	qt.submitCommandsAsQsubJob( [cmd], 'qsub_dsgenmatchcomparison_fill.sh', 
				wall_time = '24:00:00',
                                num_threads = 1, high_memory = False,
                                docmsenv=True, cmssw_version = 'CMSSW_10_2_25' )
