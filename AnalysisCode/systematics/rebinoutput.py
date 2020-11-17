#############################################################################
# script to automatically call the rebinner on the output of runsystematics #
#############################################################################
# after rebinning, envelopes are recalculated where needed and histograms are clipped

import os
import sys
import ROOT
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import submitQsubJob, initializeJobScript
from systematicstools import getfilestomerge
sys.path.append(os.path.abspath('../tools'))
import histtools as histtools

##### main function #####

def rebinoutput(topdir,year,region,npmode,mode,firstbincount,binfactor):

    # step 1: merge all relevant files using hadd
    # (the hadding here is only temporary, needed for rebinning)
    combpath = os.path.join(topdir,year+'combined',region,npmode)
    if os.path.exists(combpath):
        os.system('rm -r '+combpath)
    os.makedirs(combpath)
    filestomerge = getfilestomerge(topdir,year,region,npmode) # implicit printing
    if len(filestomerge)==0: return -1
    outputfile = os.path.join(combpath,'combined.root')
    print('running hadd command...')
    command = 'hadd '+outputfile
    for f in filestomerge: command += ' '+f
    os.system(command)

    ### for testing only ###
    #filestomerge = filestomerge[:3]

    # step 1b: for speed, remove all unnecessary histograms from the file
    # (only need nominal histograms for this specific variable)
    # note that the removing of histograms does not need to be very rigorous here,
    # the correct selection is done anyway in rebinoutput.cc
    histlist = histtools.loadallhistograms(outputfile)
    tempfilename = outputfile[:-5]+'_temp.root'
    if os.path.exists(tempfilename):
        os.system('rm '+tempfilename)
    tempfile = ROOT.TFile.Open(tempfilename,'recreate')
    for hist in histlist:
	if not ('_fineBinnedeventBDT' in hist.GetName() 
		and 'nominal' in hist.GetName()): continue
	hist.Write()
    tempfile.Close()
    os.system('mv {} {}'.format(tempfilename,outputfile))

    # step 2: rebin the discriminating variable
    print('rebinning...')
    if not os.path.exists('./rebinoutput'):
        print('### ERROR ###: rebinoutput executable does not seem to exist')
	return -1
    for f in filestomerge:
	print('now rebinning {}'.format(f))
	tempfilename = f[:-5]+'_temp.root'
	if os.path.exists(tempfilename):
	    os.system('rm '+tempfilename)
	command = './rebinoutput '+outputfile+' '+f+' '+tempfilename+' _fineBinnedeventBDT'
	command += ' '+mode+' '+firstbincount+' '+binfactor
	os.system(command)
	command = 'mv '+tempfilename+' '+f
	os.system(command)

    # step 3: recalculate envelopes and RMS's
    for f in filestomerge:
	print('now recalculating envelopes for {}'.format(f))
	gencommand = './reenvelope '+f+' '+f+' _rebinnedeventBDT'
	commands = []
	commands.append( gencommand + ' qcdScalesShapeVar qcdScalesShapeEnv envelope' )
	commands.append( gencommand + ' pdfShapeVar pdfShapeEnv envelope' )
	commands.append( gencommand + ' pdfShapeVar pdfShapeRMS RMS' )
	for command in commands: os.system(command)

    # step 4: clip histograms (if not fake rate)
    for f in filestomerge:
	if 'fakerate' in f.split('/')[-2]: continue
	print('now clipping {}'.format(f))
	histtools.clipallhistograms(f,mustcontain=['_rebinnedeventBDT'])	

if __name__=='__main__':

    sys.stderr.write('###starting###\n')

    # command line arguments: a top-level directory and the rebinning parameters
    # (not included in command line args: files to run on, see below)
    # a specific year, region and npmode can also be specified
    # (if not, it will run on all years and regions defined below)
    # note: append 'f' to firstbincount to indicate fraction of total events instead of absolute!
    topdir = ''
    mode = ''
    firstbincount = ''
    binfactor = ''
    year = ''
    region = ''
    npmode = ''
    runmultiple = True
    if len(sys.argv)==5:
	topdir = sys.argv[1]
	mode = sys.argv[2]
	firstbincount = sys.argv[3]
	binfactor = sys.argv[4]
    elif len(sys.argv)==8:
	topdir = sys.argv[1]
	mode = sys.argv[2]
        firstbincount = sys.argv[3]
        binfactor = sys.argv[4]
	year = sys.argv[5]
	region = sys.argv[6]
	npmode = sys.argv[7]
	runmultiple = False
    else:
        print('### ERROR ###: rebinoutput.py needs three command line arguments.')
        print('           normal usage: rebinoutput <output_directory> <mode> ')
	print('                         <first_bin_count> <bin_factor>')
        sys.exit()

    # global setting of years to run on
    yearlist = []
    yearlist.append('2016')
    yearlist.append('2017')
    yearlist.append('2018')
    
    # global setting of regions to run on
    regionlist = []
    regionlist.append('signalregion_1')
    for r in ['signalregion_2','signalregion_3']: regionlist.append(r)
    # recommended to not rebin the control regions, it is useless anyway 
    # and can give problems if using signal-based rebinning.
    #regionlist.append('wzcontrolregion')
    #regionlist.append('zzcontrolregion')
    #regionlist.append('zgcontrolregion')
    #for r in ['signalsideband_noossf_1']: regionlist.append(r)
    #for r in ['signalsideband_noz_1']: regionlist.append(r)

    # global setting of nonprompt mode to run on
    # WARNING: do not run on multiple modes simultaneously as the same files 
    # might be affected simultaneuously in a different way!
    # maybe try to fix later, e.g. by copying all files per element in the loop
    # to a different directory (but this comes with other disadvantages for the next steps)
    npmode = 'npfromdata'

    # check preconditions
    if not os.path.exists('./rebinoutput'):
	print('### ERROR ###: rebinoutput executable does not seem to exist')
	sys.exit()
    if not os.path.exists('./reenvelope'):
	print('### ERROR ###: reenvelope executable does not seem to exist')
        sys.exit()
    if mode not in ['signal','background']:
	print('### ERROR ###: mode "'+str(mode)+'" not recognized.')
        sys.exit()

    if runmultiple:
	cwd = os.getcwd()
	for year in yearlist:
	    for region in regionlist:
		script_name = 'rebinoutput.sh'
		with open(script_name,'w') as script:
		    initializeJobScript(script)
		    script.write('cd {}\n'.format(cwd))
		    command = 'python rebinoutput.py {} {} {} {} {} {} {}'.format(
				    topdir, mode, firstbincount, binfactor, year, region, npmode )
		    script.write(command+'\n')
		submitQsubJob(script_name)
		# alternative: run locally
		#os.system('bash '+script_name)

    else:
	rebinoutput(topdir,year,region,npmode,mode,firstbincount,binfactor)

    sys.stderr.write('###done###\n')
