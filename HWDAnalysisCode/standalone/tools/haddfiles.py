#############################################################
# reduce the number of root files in a directory by hadding #
#############################################################

import sys
import os
import optiontools as opt

def partition( allobjects, partsize ):
    ### partition a list of objects in sublists of size partsize
    parts = []
    idx = 0
    while idx+partsize < len(allobjects):
	thispart = []
	for incr in range(0,partsize): thispart.append(allobjects[idx+incr])
	parts.append(thispart)
	idx += partsize
    if idx < len(allobjects):
	thispart = []
	for obj in allobjects[idx:]: thispart.append(obj)
	parts.append(thispart)
    # prints for testing
    print('length of input list: {}'.format(len(allobjects)))
    print('partition structure: {}'.format([len(part) for part in parts]))
    return parts

def partition_rootfiles( directory, partsize ):
    ### apply partition function on all root files in a directory
    allfiles = [os.path.join(directory,f) for f in os.listdir(directory) if f[-5:]=='.root']
    allfiles.sort()
    print('found {} root files in directory {}'.format(len(allfiles),directory))
    return partition( allfiles, partsize )

def hadd( outputfile, inputfiles ):
    cmd = 'hadd'
    cmd += ' {}'.format(outputfile)
    for f in inputfiles: cmd += ' {}'.format(f)
    os.system(cmd)

def haddfiles( directory, partsize, outfiletag='merged', rmorig=False ):
    partset = partition_rootfiles(directory, partsize)
    for i,part in enumerate(partset):
	outputfile = os.path.join(directory,outfiletag+'_{}.root'.format(i))
	# prints for testing
	print('--- {} ---'.format(outputfile))
	for f in part: print('  - {}'.format(f))
	if os.path.exists(outputfile):
	    print('WARNING: output file {} already exists, skipping...'.format(outputfile))
	    continue
	hadd( outputfile, part )
	if rmorig:
	    for f in inputfiles: os.system('rm {}'.format(f))

if __name__=='__main__':

    options = []
    options.append( opt.Option('inputdir', vtype='path') )
    options.append( opt.Option('partsize', vtype='int', default=10) )
    options.append( opt.Option('outfiletag', default='merged' ) )
    options.append( opt.Option('rmorig', vtype='bool', default=False) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    haddfiles( options.inputdir, options.partsize, 
		outfiletag=options.outfiletag, rmorig=options.rmorig)
