############################################
# a submitter for ReweighterBTagShape_test #
############################################

import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct

if __name__=='__main__':

    inputdir = os.path.abspath(sys.argv[1])
    inputfilename = sys.argv[2]
    nevents = sys.argv[3]
    outputfile = os.path.abspath(sys.argv[4])
    exe = './fourtopsfakerateReweighter_test'

    # case 1: sample list
    if(inputfilename[-4:]=='.txt'):
	inputfile = os.path.abspath(inputfilename)
    # case 2: root file
    elif(inputfilename[-5:]=='.root'):
	inputfile = os.path.join(inputdir,inputfilename)

    if not os.path.exists(inputfilename):
	raise Exception('ERROR: input file does not exist.')
    if not os.path.exists(exe):
	raise Exception('ERROR: executable does not exist.')
    if not os.path.exists(os.path.split(outputfile)[0]):
	os.makedirs(os.path.split(outputfile)[0])

    command = '{} {} {} {} {}'.format(exe, inputdir, inputfilename, nevents, outputfile)
    ct.submitCommandAsCondorJob( 'cjob_fourtopsfakerateReweighter_test', command )
