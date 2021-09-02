######################################################
# script to perform additional scaling on histograms #
######################################################
# usage: apply to the MC output files of runsystematics
#        to apply an extra global scaling, e.g. updated lumi or xsec values
# important: apply before mergeoutput, else not all scalings can be applied consistently
#            (e.g. prompt subtraction within nonprompt contribution)

import sys
import os
import glob
import ROOT
sys.path.append(os.path.abspath('../tools'))
import histtools as ht
sys.path.append(os.path.abspath('../../skimmer')) # old qsub way
from jobSubmission import submitQsubJob, initializeJobScript # old qsub way

def scale( histlist, samplename, year ):
    ### perform scaling on a list of histograms
    # samplename, year and histogram titles/names can be used to decide which scaling to apply
    
    if( 'SingleMuon' in samplename or 'SingleElectron' in samplename 
	or 'DoubleMuon' in samplename or 'DoubleEG' in samplename 
	or 'MuonEG' in samplename or 'EGamma' in samplename
	or 'data' in samplename ):
	return
	# (don't apply scaling to data)
	# (best to just not call this function for histogram lists from data samples,
	#  but just for extra safety)

    #if( 'tZq_ll_4f' in samplename ):
    #	for hist in histlist:
    #	    hist.Scale(92.2/94.2)

    if year=='2016':
	for hist in histlist:
	    hist.Scale(36.3/35.9)

    return

if __name__=='__main__':

    sys.stderr.write('###starting###\n')

    # command line arguments:
    # - if 2 arguments are given, they correspond to top-level input and output directory
    #   and the program will loop over scaledirs and copydirs (defined below) and submit jobs
    # - if a third argument is given, it corresponds to a specific subdirectory
    #   (must include top-level input directory in its path)
    #   and the program will run locally on this specific directory only
    # - if the third argument ends in .root, it must correspond to the path of a root file
    #   (must include top-level input directory in its path)
    #   and the program will run locally on this file only
    #   (note that in this case the target directory must be created manually first
    #    to avoid problems with running files in parralel submission)
    topindir = os.path.abspath(sys.argv[1])
    topoutdir = os.path.abspath(sys.argv[2])
    if len(sys.argv)==4:
	scaledir = os.path.abspath(sys.argv[3])
	print('scaling and copying {}'.format(scaledir))
	if scaledir[-5:]=='.root':
	    f = scaledir
	    # run on single file
	    sourcedir = os.path.dirname(f)
	    filename = os.path.basename(f)
	    reldir = sourcedir.split(topindir)[-1].strip('/')
            tardir = os.path.join(topoutdir,reldir) # assumed to exist
	    histlist = ht.loadallhistograms(f)
            samplename = filename[:-5]
            year = ''
            if 'Summer16' in samplename: year='2016'
            if 'Fall17' in samplename: year='2017'
            if 'Autumn18' in samplename: year='2018'
            scale(histlist,samplename,year)
            outfile = ROOT.TFile.Open(os.path.join(tardir,filename),'recreate')
            for hist in histlist:
                hist.Write()
            outfile.Close()
	else:
	    reldir = scaledir.split(topindir)[-1].strip('/')
	    tardir = os.path.join(topoutdir,reldir)
	    os.makedirs(tardir)
	    for f in [f for f in os.listdir(scaledir) if f[-5:]=='.root']:
		cmd = 'python scalehistograms.py {} {} {}'.format(topindir,topoutdir,
			    os.path.join(scaledir,f))
		os.system(cmd)

    else:

	scaledirs = '201?MC/*'
	copydirs = '201?data/*'

	if os.path.exists(topoutdir):
	    raise Exception('output directory already exists')

	scaledirs = glob.glob(os.path.join(topindir,scaledirs))
	copydirs = glob.glob(os.path.join(topindir,copydirs))

	for scaledir in scaledirs:
            print('scaling and copying {}'.format(scaledir))
	    jobsplit='perdir'
	    if jobsplit=='perdir':
		# mehtod 1: one job per directory
		cmd = 'python scalehistograms.py {} {} {}'.format(topindir,topoutdir,scaledir)
		script_name = 'qsub_scalehistograms.sh'
		cwd = os.getcwd()
		with open(script_name,'w') as script:
		    initializeJobScript(script)
		    script.write('cd {}\n'.format(cwd))
		    script.write(cmd+'\n')
		submitQsubJob(script_name)
		#os.system('bash '+script_name)
	    if jobsplit=='perfile':
		# method 2: one job per file
		reldir = scaledir.split(topindir)[-1].strip('/')
		tardir = os.path.join(topoutdir,reldir)
		os.makedirs(tardir)
		for f in [f for f in os.listdir(scaledir) if f[-5:]=='.root']:
		    cmd = 'python scalehistograms.py {} {} {}'.format(topindir,topoutdir,
			        os.path.join(scaledir,f))
		    script_name = 'qsub_scalehistograms.sh'
		    cwd = os.getcwd()
		    with open(script_name,'w') as script:
			initializeJobScript(script)
			script.write('cd {}\n'.format(cwd))
			script.write(cmd+'\n')
		    submitQsubJob('bash '+script_name)

	for copydir in copydirs:
	    print('copying {}'.format(copydir))
	    reldir = copydir.split(topindir)[-1].strip('/')
	    tardir = os.path.join(topoutdir,reldir)
	    os.makedirs(tardir)
	    for f in [f for f in os.listdir(copydir) if f[-5:]=='.root']:
		cmd = 'cp {} {}'.format(os.path.join(copydir,f),os.path.join(tardir,f))
		#print('  '+cmd)
		os.system(cmd)
	    
    sys.stderr.write('###done###\n')
 
