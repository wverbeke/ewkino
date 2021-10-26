#####################################################
# Run eventSelector and eventFlattener sequentially #
#####################################################

import sys
import os
import copy

from eventSelector import make_eventselector_command
from eventFlattener import make_eventflattener_command
sys.path.append('../../jobSubmission')
import qsubTools as qt
sys.path.append('../tools/python')
import optiontools as opt


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

    # define intermediate output file
    intfilename = options.outputfilename.replace('.root','_tmp.root')
    intfilepath = os.path.join(options.outputdir, intfilename)

    # modify option collection for event selection
    options_step1 = copy.deepcopy( options )
    options_step1.outputfilename = intfilename

    # modify option collection for event flattening
    options_step2 = copy.deepcopy( options )
    options_step2.inputfile = intfilepath

    # make the commands
    cmds = []
    cmds.append( make_eventselector_command(options_step1) )
    cmds.append( make_eventflattener_command(options_step2) )
    cmds.append( 'rm {}'.format(intfilepath) )

    # submit the job
    if options.runlocal:
	for cmd in cmds: 
	    print('now running {}...'.format(cmd))
	    os.system(cmd)
    else:
	qt.submitCommandsAsQsubJob( cmds, 'qsub_eventflow.sh' )
