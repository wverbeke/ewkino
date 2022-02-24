################################################################
# convert a sample list format from Liam-style to ewkino-style #
################################################################

import sys
import os


def convert_line(line, year=None, mode=None):
    ### convert a single line in a sample list
    # input args:
    # - line: a string represening a valid line in Liam-style sample list
    # returns:
    # a string representing the same line in ewkino-style sample list
    els = line.split(' ')
    # remove empty strings, tabs and newlines
    els = ([el for el in els 
	    if (el!='' and el!='\t' and el!='\n') ])
    # format the sample name
    sname = els[1]
    sname = sname.split('/Reco/')[-1]
    sname = sname.replace('/pnfs/iihe/cms/store/user/lwezenbe/heavyNeutrino/','')
    # remove spcecific samples
    if( 'QCD_Pt' in sname ): return ''
    if( 'HeavyNeutrino_trilepton' in sname ): return ''
    if( year=='2016PreVFP' ):
	if( 'ST_tW' in sname and 'inclusiveDecays' in sname ): return ''
	if( 'TTJets' in sname ): return ''
	if( 'ZZ_TuneCP5_13TeV-pythia8' in sname ): return ''
	if( 'WJetsToLNu_0J' in sname 
	    or 'WJetsToLNu_1J' in sname
	    or 'WJetsToLNu_2J' in sname ): return ''
	if( 'WGToLNuG_01J' in sname ): return ''
	if( 'WW_TuneCP5_13TeV-pythia8' in sname ): return ''
	if( 'WZ_TuneCP5_13TeV-pythia8' in sname ): return ''
    if( year=='2016PostVFP' ):
	if( 'DYJetsToLL_M-50_TuneCP5_13TeV-madgraphMLM-pythia8' in sname ): return ''
	if( 'ST_tW' in sname and 'inclusiveDecays' in sname ): return ''
	if( 'TTGJets' in sname ): return ''
	if( 'TTJets' in sname ): return ''
	if( 'WW_TuneCP5_13TeV-pythia8' in sname ): return ''
	if( 'WZ_TuneCP5_13TeV-pythia8' in sname ): return ''
	if( 'ZZTo2L2Nu_TuneCP5_13TeV_powheg_pythia8' in sname ): return ''
    if( year=='2017' ):
	if( 'ST_tW' in sname and 'inclusiveDecays' in sname ): return ''
	if( 'TTGamma' in sname ): return ''
        if( 'TTJets' in sname ): return ''
	if( 'WW_TuneCP5_13TeV-pythia8' in sname ): return ''
	if( 'WZ_TuneCP5_13TeV-pythia8' in sname ): return ''
	if( 'ZZ_TuneCP5_13TeV-pythia8' in sname ): return ''
    if( year=='2018' ):
	if( 'ST_tW' in sname and 'inclusiveDecays' in sname ): return ''
        if( 'TTGamma' in sname ): return ''
        if( 'TTJets' in sname ): return '' 
	if( 'W1JetsToLNu' in sname
	    or 'W2JetsToLNu' in sname
	    or 'W3JetsToLNu' in sname
	    or 'W4JetsToLNu' in sname ): return ''
	if( 'WGToLNuG_01J' in sname ): return ''
	if( 'WW_TuneCP5_13TeV-pythia8' in sname ): return ''
        if( 'WZ_TuneCP5_13TeV-pythia8' in sname ): return ''
        if( 'ZZ_TuneCP5_13TeV-pythia8' in sname ): return ''
    # remove double-counting by faulty extentions
    lpname = els[0]
    if( '-ext' in lpname): return ''
    # format the process name
    pname = els[2]
    if( 'DYJets' in sname ): pname = 'DY'
    if( 'GluGluH' in sname): pname = 'ZZH'
    if( pname=='ZZ-H' ): pname = 'ZZH'
    if( pname=='TT-T+X' ): pname = 'TTX'
    if( pname=='ZH-H' ): pname = 'ZZH'
    if( pname=='Data' ): pname = 'data'
    if( pname=='triboson' ): pname = 'Triboson'
    # format cross-section
    xsec = els[4]
    xsec = xsec.replace('\n','')
    if 'data' in xsec: 
	xsec = ''
	if( mode=='noskim' ): return '' 
	# ( skip data for noskim sample lists )
    if( '*' in xsec ):
	xsecparts = xsec.split('*')
	xsec = 1
	for xsecpart in xsecparts: xsec*=float(xsecpart)
    # make the new line
    newline = '{} {} {}'.format(pname, sname, xsec)
    return newline
    

def convert_sample_list(inputfile, outputfile, year=None, mode=None):
    ### convert a Liam-style sample list to ewkino-style sample list
    # input args:
    # - inputfile: Liam-style sample list
    # returns:
    # the outputfile is written
    newlines = []
    # read the lines from the input file
    with open(inputfile) as f:
	lines = f.readlines()
    for line in lines:
	# skip empty lines and commented lines
	if( len(line)<=1 ): continue
	if( line[0]=='%' ): continue
	# convert the line
	newline = convert_line(line, year=year, mode=mode)
	if len(newline)!=0: newlines.append(newline)
    # write the output file
    with open(outputfile,'w') as f:
	for line in newlines: f.write(line+'\n')	


if __name__=='__main__':

    # list input files
    inputdir = 'raw'
    years = ['2016PreVFP','2016PostVFP','2017','2018']
    modes = ['skim','noskim']
    inputfiles = []
    inputyears = []
    inputmodes = []
    for year in years:
	for mode in modes:
	    modestr = '' if mode=='skim' else '_'+mode
	    inputfile = os.path.join(inputdir,'samples_{}{}.txt'.format(year,modestr))
	    print('adding input file {}'.format(inputfile))
	    if not os.path.exists(inputfile):
		print('WARNING: input file {} does not exist'.format(inputfile))
		continue
	    inputfiles.append(inputfile)
	    inputyears.append(year)
	    inputmodes.append(mode)

    # loop over input files
    for year,mode,inputfile in zip(inputyears,inputmodes,inputfiles):
	
	# define output file (same name as input file but in this directory)
	outputfile = os.path.split(inputfile)[1]

	# do the conversion
	convert_sample_list(inputfile, outputfile, year=year, mode=mode)
