import sys
import os
sys.path.append('../../jobSubmission')
import condorTools as ct

#inputfile = '/pnfs/iihe/cms/store/user/kskovpen/heavyNeutrinoEOY20/tZq_ll_4f_PSweights_13TeV-amcatnlo-pythia8/crab_MiniAOD2016v3-v1_noskim_EOY20_2016/201211_065610/0000/noskim_10.root'
inputfile = '/user/llambrec/Files/trileptonskim/2016splitjec/tZq_ll_4f_PSweights_13TeV-amcatnlo-pythia8_Summer16.root'
outputdir = '/user/llambrec/Files/tzqid_splitjec'
outputfile = 'tZq_ll_4f_PSweights_13TeV-amcatnlo-pythia8_Summer16.root'
eventselection = 'signalregion'
selectiontype = '3tight'
variation= 'all'

command = './eventselector {} {} {} {} {} {}'.format( inputfile, outputdir,
		outputfile, eventselection, selectiontype, variation )
ct.submitCommandAsCondorJob('temp_cjob',command)
