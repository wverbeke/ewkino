######################
# Run eventFlattener #
######################

import sys
import os

sys.path.append('../../jobSubmission')
import qsubTools as qt
sys.path.append('../tools/python')
import optiontools as opt

def make_eventflattener_command( options ):
    ### make a command line call to ./eventFlattener
    # input arguments:
    # - options: object of type OptionCollection with correct attributes
    
    cmd = './eventFlattener'
    cmd += ' {}'.format(options.inputfile)
    cmd += ' {}'.format(options.norm)
    cmd += ' {}'.format(options.outputdir)
    cmd += ' {}'.format(options.outputfilename)
    cmd += ' {}'.format(options.eventselection)
    cmd += ' {}'.format(options.selectiontype)
    cmd += ' {}'.format(options.variation)
    cmd += ' {}'.format(options.muonfrmapfile)
    cmd += ' {}'.format(options.electronfrmapfile)
    cmd += ' {}'.format(options.domva)
    cmd += ' {}'.format(options.mvaxmlpath)
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
    options.append( opt.Option('norm', vtype='float', default=1.) )
    options.append( opt.Option('muonfrmapfile', vtype='path') )
    options.append( opt.Option('electronfrmapfile', vtype='path') )
    options.append( opt.Option('domva', vtype='bool', default=False ) )
    options.append( opt.Option('mvaxmlpath', vtype='path') )
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
    cmds.append( make_eventflattener_command(options) )

    # submit the job
    if options.runlocal:
	for cmd in cmds: 
	    print('now running {}...'.format(cmd))
	    os.system(cmd)
    else:
	qt.submitCommandsAsQsubJob( cmds, 'qsub_eventflattener.sh' )
