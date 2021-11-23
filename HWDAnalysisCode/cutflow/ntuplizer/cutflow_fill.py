###############
# Run cutFlow #
###############

import sys
import os

sys.path.append('../../../jobSubmission')
import qsubTools as qt
sys.path.append('../../tools/python')
import optiontools as opt

def make_cutflow_command( options ):
    ### make a command line call to ./cutFlow
    # input arguments:
    # - options: object of type OptionCollection with correct attributes

    cmd = './cutFlow'
    cmd += ' {}'.format(options.inputfile)
    cmd += ' {}'.format(options.cutflowvar)
    cmd += ' {}'.format(options.outputfile)
    cmd += ' {}'.format(options.nevents)
    cmd += ' {}'.format(options.maxcutflowvalue)
    return cmd


if __name__=='__main__':

    options = []
    # options passed down
    options.append( opt.Option('inputfile', vtype='path', required=True) )
    options.append( opt.Option('cutflowvar', required=True) )
    options.append( opt.Option('outputfile', vtype='path', required=True) )
    options.append( opt.Option('nevents', vtype='int', default=-1) )
    options.append( opt.Option('maxcutflowvalue', vtype='int', default=10) )
    # meta options
    options.append( opt.Option('runlocal', vtype='bool', default=False) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    # parse input arguments
    if os.path.splitext(options.outputfile)[1]!='.root':
	print('WARNING: adding .root to output file name {}'.format(options.outputfile))
	options.outputfile = os.path.splitext(options.outputfile)[0]+'.root'
    outputdir = os.path.dirname(options.outputfile)
    if not os.path.exists(outputdir):
	os.makedirs(outputdir)

    # make the commands
    cmds = []
    cmds.append( make_cutflow_command(options) )

    # submit the job
    if options.runlocal:
	for cmd in cmds: 
	    print('now running {}...'.format(cmd))
	    os.system(cmd)
    else:
	qt.submitCommandsAsQsubJob( cmds, 'qsub_cutflow.sh' )
