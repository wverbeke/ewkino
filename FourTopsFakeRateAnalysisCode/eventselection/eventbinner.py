######################################################################
# python script to run the eventbinner executable via job submission #
######################################################################

import ROOT
import sys
import os
import glob
# in order to import local functions: append location to sys.path
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
sys.path.append(os.path.abspath('../../jobSubmission'))
import condorTools as ct


def year_from_samplelist( slname ):
    ### small utility function to extract the year from a sample list name.
    # only needed to set the correct fake rate map.
    if( '2016PreVFP' in slname ): return '2016PreVFP'
    elif( '2016PostVFP' in slname ): return '2016PostVFP'
    elif( '2017' in slname ): return '2017'
    elif( '2018' in slname ): return '2018'
    else:
	raise Exception('ERROR: could not retrieve year'
			+' from sample list name "{}"'.format(slname))


if __name__=='__main__':
    
    # read command line args
    if len(sys.argv) != 8:
	print('### ERROR ###: eventbinner.py requires a different number of command-line arguments.')
	print('Normal usage from the command line:')
	print('python eventbinner.py <input_directory> <samplelist> <output_directory>') 
	print('<event_selection> <selection_type> <variation> <path_to_frmaps>')
	sys.exit()

    input_directory = os.path.abspath(sys.argv[1])
    samplelist = os.path.abspath(sys.argv[2])
    output_directory = sys.argv[3]
    output_directory = os.path.abspath(output_directory)
    event_selection = sys.argv[4]
    selection_type = sys.argv[5]
    variation = sys.argv[6]
    frdir = os.path.abspath(sys.argv[7])
    cwd = os.getcwd()

    # argument checks and parsing
    if not os.path.exists(input_directory):
	raise Exception('ERROR: input directory '+input_directory+' does not seem to exist...')
    if not os.path.exists(samplelist):
	raise Exception('ERROR: requested sample list does not seem to exist...')
    if os.path.exists(output_directory):
	print('WARNING: output directory already exists. Clean it? (y/n)')
	go=raw_input()
	if not go=='y': sys.exit()
	os.system('rm -r '+output_directory)
    event_selections = event_selection.split('+')
    for es in event_selections:
	if es not in (['wzcontrolregion','zzcontrolregion','zgcontrolregion',
			'nonprompt_trilepton_noossf','nonprompt_trilepton_noz',
			'nonprompt_trilepton','nonprompt_dilepton']):
	    raise Exception('ERROR: event_selection not in list of recognized event selections')
    selection_types = selection_type.split('+')
    for st in selection_types:
	if st not in ['3tight','3prompt','fakerate']:
	    raise Exception('ERROR: selection_type not in list of recognized types')
    if variation not in ['nominal']:
	raise Exception('ERROR: only nominal variation supported for now')

    # check if executable is present
    if not os.path.exists('./eventbinner'):
	raise Exception('ERROR: eventbinner executable was not found.')

    # make output directory
    os.makedirs(output_directory)

    # set and check fake rate maps
    frmapyear = year_from_samplelist( samplelist )
    muonfrmap = os.path.join(frdir,'fakeRateMap_data_muon_'+frmapyear+'_mT.root')
    electronfrmap = os.path.join(frdir,'fakeRateMap_data_electron_'+frmapyear+'_mT.root')
    if selection_type=='fakerate':
	if not os.path.exists(muonfrmap):
	    raise Exception('ERROR: fake rate map {} does not exist'.format(muonfrmap))
	if not os.path.exists(electronfrmap):
            raise Exception('ERROR: fake rate map {} does not exist'.format(electronfrmap))

    # check number of samples
    nsamples = 0
    with open(samplelist) as sf:
	for sl in sf:
	    if(sl[0] == '#'): continue
	    if(sl[0] == '\n'): continue
	    if(not '.root' in sl): continue
	    nsamples += 1
    print('found '+str(nsamples)+' samples')

    # loop over input files and submit jobs
    commands = []
    for i in range(nsamples):
	# make the command
	command = './eventbinner {} {} {} {} {} {} {} {} {}'.format(
                    input_directory, samplelist, i, output_directory,
                    event_selection, selection_type, variation,
                    muonfrmap, electronfrmap )
	commands.append(command)

    # submit the jobs
    ct.submitCommandsAsCondorCluster('cjob_eventbinner',commands)
