#####################
# Run eventSelector #
#####################

import sys
import os

sys.path.append('../../jobSubmission')
import qsubTools as qt
sys.path.append('../tools/python')
import optiontools as opt

def make_eventselector_command( options ):
    ### make a command line call to ./eventSelector
    # input arguments:
    # - options: object of type OptionCollection with correct attributes

    cmd = './eventSelector'
    cmd += ' {}'.format(options.inputfile)
    cmd += ' {}'.format(options.outputdir)
    cmd += ' {}'.format(options.outputfilename)
    cmd += ' {}'.format(options.eventselection)
    cmd += ' {}'.format(options.selectiontype)
    cmd += ' {}'.format(options.variation)
    cmd += ' {}'.format(options.nevents)
    return cmd


if __name__=='__main__':

    options = []
    # options passed down
    options.append( opt.Option('inputfile', vtype='path', required=True) )
    options.append( opt.Option('outputdir', vtype='path', required=True) )
    options.append( opt.Option('outputfilename', required=True) )
    options.append( opt.Option('eventselection', default='signalregion') )
    options.append( opt.Option('selectiontype', default='tight') )
    options.append( opt.Option('variation', default='nominal') )
    options.append( opt.Option('nevents', vtype='int', default=-1) )
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
    if os.path.splitext(options.outputfilename)[1]!='.root':
	print('WARNING: adding .root to output file name {}'.format(options.outputfilename))
	options.outputfilename = os.path.splitext(options.outputfilename)[0]+'.root'
    if not os.path.exists(options.outputdir):
	os.makedirs(options.outputdir)

    # make the commands
    cmds = []
    cmds.append( make_eventselector_command(options_step1) )

    # submit the job
    if options.runlocal:
	for cmd in cmds: 
	    print('now running {}...'.format(cmd))
	    os.system(cmd)
    else:
	qt.submitCommandsAsQsubJob( cmds, 'qsub_eventselector.sh' )
