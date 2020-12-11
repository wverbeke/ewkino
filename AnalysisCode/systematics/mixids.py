###############################################
# python script to mix the output directories #
###############################################
# allows to use e.g. different lepton ids's for different years/signal regions

import sys
import os

outdir = 'output_tzqidmixed'
indirs = ([
	    'output_tzqid/2016combined/signalregion_1/npfromdata',
	    'output_tzqid/2017combined/signalregion_1/npfromdata',
	    'output_tzqid/2018combined/signalregion_1/npfromdata',
	    'output_tzqidmedium0p4/2016combined/signalregion_2/npfromdata',
	    'output_tzqidmedium0p4/2017combined/signalregion_2/npfromdata',
	    'output_tzqidmedium0p4/2018combined/signalregion_2/npfromdata',
	    'output_tzqidmedium0p4/2016combined/signalregion_3/npfromdata',
	    'output_tzqidmedium0p4/2017combined/signalregion_3/npfromdata',
	    'output_tzqidmedium0p4/2018combined/signalregion_3/npfromdata',
	    
	    'output_tzqidmedium0p4/2016combined/wzcontrolregion/npfromdata',
	    'output_tzqidmedium0p4/2017combined/wzcontrolregion/npfromdata',
	    'output_tzqidmedium0p4/2018combined/wzcontrolregion/npfromdata',
	    'output_tzqidmedium0p4/2016combined/zgcontrolregion/npfromdata',
            'output_tzqidmedium0p4/2017combined/zgcontrolregion/npfromdata',
            'output_tzqidmedium0p4/2018combined/zgcontrolregion/npfromdata',
	    'output_tzqidmedium0p4/2016combined/zzcontrolregion/npfromdata',
            'output_tzqidmedium0p4/2017combined/zzcontrolregion/npfromdata',
            'output_tzqidmedium0p4/2018combined/zzcontrolregion/npfromdata',
	])


if os.path.exists(outdir):
    os.system('rm -r '+outdir)
os.makedirs(outdir)

for indir in indirs:
    indir = indir.rstrip('/')
    indirinter = indir[indir.find('/')+1:indir.rfind('/')]
    outsubdir = os.path.join(outdir,indirinter)
    if not os.path.exists(outsubdir):
	os.makedirs(outsubdir)
    os.system('cp -r {} {}'.format(indir,outsubdir))
