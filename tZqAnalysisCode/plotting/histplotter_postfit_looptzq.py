#############################################################################
# call hisplotter_postfit.py specifically for tZq analysis folder structure #
#############################################################################

import sys
import os
import json
sys.path.append('../../jobSubmission')
import condorTools as ct
sys.path.append(os.path.abspath('../../skimmer'))
from jobSubmission import initializeJobScript, submitQsubJob

def getlabel( tag ):
    ### customize additional labels to display on the plot

    extrainfos = []
    #if('signalregion' in tag): extrainfos.append('tZq signal region')
    if('wzcontrolregion' in tag): extrainfos.append('WZ enriched')
    if('zzcontrolregion' in tag): extrainfos.append('ZZ enriched')
    if('zgcontrolregion' in tag): extrainfos.append(r'Z#gamma enriched')
    if('ttzcontrolregion' in tag): extrainfos.append(r't#bar{t}Z enriched')
    if('signalsideband' in tag): extrainfos.append('Nonprompt enriched')

    if('signalregion' in tag and '_cat1_' in tag):
        extrainfos.append('1 b jet, 2-3 jets')
    if('signalregion' in tag and '_cat2_' in tag):
        extrainfos.append(r'1 b jet, #geq 4 jets')
    if('signalregion' in tag and '_cat3_' in tag):
        extrainfos.append(r'#geq 2 b jets')
    if('signalregion' in tag and '_cat123_' in tag):
	extrainfos.append(r'#geq 1 b jet, #geq 2 jets')
    if('signalregion' in tag and '_ch0_' in tag):
        extrainfos.append('channel eee')
    if('signalregion' in tag and '_ch1_' in tag):
        extrainfos.append('channel eem')
    if('signalregion' in tag and '_ch2_' in tag):
        extrainfos.append('channel emm')
    if('signalregion' in tag and '_ch3_' in tag):
        extrainfos.append('channel mmm')

    #extrainfos.append('BDT score > 0.5')

    if len(extrainfos)==0: return None
    label = '\n'.join(extrainfos)
    return label

def getyaxrange( tag ):
    ### customize y axis range
    
    # default: automatic ranges
    return None

    # for paper: fix ranges (equal pre- and postfit)
    if('signalregion' in tag and '_cat1_' in tag):
        return (0,430)
    if('signalregion' in tag and '_cat2_' in tag):
        return (0,260)
    if('signalregion' in tag and '_cat3_' in tag):
        return (0,260)
    return None

def getp2yaxrange( tag ):
    ### customize y axis range of bottom pad

    # default: default ranges (by passing None)
    #return None

    # for some plots in paper: smaller ranges
    if('signalsideband' in tag): return (0.4, 1.599)
    if('zgcontrolregion' in tag): return (0.601, 1.399)
    return None

def getextracmstext():
    ### customize whether or not to show 'Preliminary' or similar next to CMS logo

    return None
    #return 'Preliminary'


if __name__=='__main__':

    ### command line arguments: 
    # - top level input directory
    # - fitDiagnostics fit result file (ignored if prefit)
    # - mode: either 'prefit' or 'postfit'
    # - output directory
    # - run options 
    # - other options, see histplotter_postfit.py for details
    
    topdir = os.path.abspath(sys.argv[1])
    fitresultfile = os.path.abspath(sys.argv[2])
    mode = sys.argv[3]
    outputdir = os.path.abspath(sys.argv[4])
    otherargs = sys.argv[5:]
    otherargs_copy = otherargs[:] 

    runlocal=False
    runqsub=False
    for arg in otherargs_copy:
	if arg=='runlocal':
            runlocal = True
            print('found option runlocal')
            otherargs.remove(arg)
        if arg=='runqsub':
            runqsub = True
            print('found option runqsub')
            otherargs.remove(arg)

    # check arguments
    if not os.path.exists(topdir):
	raise Exception('ERROR: top level input directory does not seem to exis') 
    if not mode in ['prefit','postfit']:
	raise Exception('ERROR: mode must be "prefit" or "postfit"') 
    if mode=='postfit' and not os.path.exists(fitresultfile):
	raise Exception('ERROR: fit result file does not seem to exist')
    if not os.path.exists(outputdir):
	os.makedirs(outputdir)

    # define what histograms to read (regions, years, npdatatype)
    npfromdata = True
    regions = []
    #regions.append('all')
    #regions.append('signalregion_cat1')
    #regions.append('signalregion_cat2')
    #regions.append('signalregion_cat3')
    #regions.append('signalregion_cat123')
    #regions.append('wzcontrolregion')
    #regions.append('zzcontrolregion')
    regions.append('zgcontrolregion')
    #regions.append('ttzcontrolregion')
    #regions.append('signalsideband_noossf_cat1')
    #regions.append('signalsideband_noossf_cat123')
    #regions.append('signalsideband_noz_cat1')
    #regions.append('signalsideband_noz_cat123')
    #regions.append('signalsideband_cat1')
    #regions.append('signalsideband_cat123')
    #regions.append('signalregion_cat1_top')
    #regions.append('signalregion_cat2_top')
    #regions.append('signalregion_cat3_top')
    #regions.append('signalregion_cat123_top')
    #regions.append('signalregion_cat1_antitop')
    #regions.append('signalregion_cat2_antitop')
    #regions.append('signalregion_cat3_antitop')
    #regions.append('signalregion_cat123_antitop')
    years = ['2016','2017','2018']
    #years = ['2017','2018'] # for testing

    # define what processes to consider as signal
    signals = ['tZq']

    # automatically determine what regions to consider if requested
    regionsdict = {}
    if( len(regions)==1 and regions[0]=='all' ):
        regionsdict[years[0]] = os.listdir(os.path.join(topdir,years[0]+'combined'))
        regionsdict['yearscombined'] = regionsdict[years[0]][:]
        for year in years[1:]:
            regionsdict[year] = os.listdir(os.path.join(topdir,year+'combined'))
            for r in regionsdict['yearscombined']:
                if r not in regionsdict[year]: regionsdict['yearscombined'].remove(r)
    else:
        for year in years: regionsdict[year] = regions[:]
        regionsdict['yearscombined'] = regions[:]

    # make a list of channel info dicts for easy looping
    channels = []
    suffix = 'npfromdata' if npfromdata else 'npfromsim'
    for year in years:
        for region in regionsdict[year]:
	    name = region+'_'+year
            channels.append( {'name':name,
                'region':region,
                'year':year,
                'npfromdata':npfromdata,
		'label':getlabel(name),
		'yaxrange':getyaxrange(name),
		'p2yaxrange':getp2yaxrange(name),
		'extracmstext':getextracmstext(),
                'path':os.path.join(topdir,year+'combined',region,suffix,'combined.root')})

    commands = []
    # make plots of individual channels
    for c in channels:
        print('running on channel '+c['name'])
        channelcombo = ({'name':c['name'],'channels':[c],'signals':signals})
        channelcombostr = json.dumps(channelcombo).replace(' ','')
        cmd = "python histplotter_postfit.py '{}' {} {} {}".format(
                        channelcombostr,fitresultfile,mode,outputdir)
        for arg in otherargs: cmd += ' '+arg
	if c['label'] is not None: cmd += ' label=\''+str(c['label'])+'\''
	if c['yaxrange'] is not None: 
	    cmd += ' ymin='+str(c['yaxrange'][0])
	    cmd += ' ymax='+str(c['yaxrange'][1])
	if c['p2yaxrange'] is not None:
	    cmd += ' p2ymin='+str(c['p2yaxrange'][0])
            cmd += ' p2ymax='+str(c['p2yaxrange'][1])
	if c['extracmstext'] is not None:
	    cmd += ' extracmstext=\''+str(c['extracmstext'])+'\''
        if runlocal: os.system(cmd)
        #commands.append(cmd)

    # make plots for full run II
    if sorted(years)==['2016','2017','2018']:
        for region in regionsdict['yearscombined']:
            print('running full run II for region '+region)
            thischannels = [c for c in channels if c['region']==region]
            thisname = region+'_yearscombined'
            channelcombo = ({'name':thisname,'channels':thischannels,'signals':signals})
            channelcombostr = json.dumps(channelcombo).replace(' ','')
            cmd = "python histplotter_postfit.py '{}' {} {} {}".format(
                        channelcombostr,fitresultfile,mode,outputdir)
            for arg in otherargs: cmd += ' '+arg
	    if thischannels[0]['label'] is not None: 
		cmd += ' label=\''+str(thischannels[0]['label'])+'\''
	    if thischannels[0]['yaxrange'] is not None:
		cmd += ' ymin='+str(thischannels[0]['yaxrange'][0])
		cmd += ' ymax='+str(thischannels[0]['yaxrange'][1])
	    if thischannels[0]['p2yaxrange'] is not None:
		cmd += ' p2ymin='+str(thischannels[0]['p2yaxrange'][0])
		cmd += ' p2ymax='+str(thischannels[0]['p2yaxrange'][1])
	    if c['extracmstext'] is not None:
		cmd += ' extracmstext=\''+str(c['extracmstext'])+'\''
            if runlocal: os.system(cmd)
            commands.append(cmd)
        
    # submit jobs
    # case 1: qsub
    if runqsub:
        for command in commands:
            script_name = 'qsub_histplotter_postfit.sh'
            with open( script_name, 'w') as script:
                initializeJobScript( script )
                script.write( command+'\n' )
            submitQsubJob( script_name )
    # case 2: condor
    if not (runqsub or runlocal):
        # need to escape double quotes by repeating them (not for running locally!)
        commands = [cmd.replace('"','""') for cmd in commands]
        condortools.submitCommandsAsCondorCluster( 'cjob_histplotter_postfit', commands )

