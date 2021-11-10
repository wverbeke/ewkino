#################################
# Run dsGenMatchComparison_fill #
#################################

import sys
import os

sys.path.append('../../jobSubmission')
import qsubTools as qt
sys.path.append('../tools/python')
import variabletools as vt
import optiontools as opt

if __name__=='__main__':

    options = []
    # options passed down
    options.append( opt.Option('inputfile', vtype='path', required=True) )
    options.append( opt.Option('outputfile', vtype='path', required=True) )
    options.append( opt.Option('variables', vtype='path', required=True) )
    options.append( opt.Option('nevents', vtype='int', default=-1) )
    options.append( opt.Option('doselection', vtype='bool', default=False) )
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

    # parse options
    if not os.path.exists(options.inputfile):
	raise Exception('ERROR: input file does not exist.')
    if os.path.splitext(options.outputfile)[1]!='.root':
        print('WARNING: adding .root to output file name {}'.format(options.outputfile))
        options.outputfile = os.path.splitext(options.outputfile)[0]+'.root'
    if not os.path.exists(os.path.dirname(options.outputfile)):
        os.makedirs(os.path.dirname(options.outputfile))
    if not os.path.exists(options.variables):
	raise Exception('ERROR: varialbe json file does not exist.')
    if options.variables[-5:]!='.json':
	raise Exception('ERROR: variable json file must have extension ".json"')

    # read the variables from json format and write to plain txt format
    vardir,varfile = os.path.split( options.variables )
    variables = vt.readvariables( options.variables )
    txtvarfile = os.path.join(vardir,varfile.replace('.json','_tmp.txt'))
    vt.writevariablestxt( variables, txtvarfile )

    # make the commands
    cmd = './fillDsGenMatchComparison'
    cmd += ' {}'.format(options.inputfile)
    cmd += ' {}'.format(options.outputfile)
    cmd += ' {}'.format(txtvarfile)
    cmd += ' {}'.format(options.nevents)
    cmd += ' {}'.format(options.doselection)

    # submit the job
    if options.runlocal:
        print('now running {}...'.format(cmd))
        os.system(cmd)
    else: qt.submitCommandsAsQsubJob( [cmd], 'qsub_filldsgenmatchcomparison.sh' )
