#######################################
# merge output files from eventbinner #
#######################################
# note: corresponds to new convention with one file per sample
#       (inclusve in event selections and selection types)

import sys
import os


if __name__=='__main__':

    directory = sys.argv[1]
    outputfile = sys.argv[2]
    npmode = sys.argv[3]

    selfiles = [os.path.join(directory,f) for f in os.listdir(directory) if f[-5:]=='.root']

    # printouts
    print('will merge the following files:')
    for f in selfiles: print('  - {}'.format(f))
    print('into {}'.format(outputfile))

    # make output directory
    outputdirname, outputbasename = os.path.split(outputfile)
    if not os.path.exists(outputdirname):
        os.makedirs(outputdirname)

    # do hadd
    cmd = 'hadd -f {}'.format(outputfile)
    for f in selfiles: cmd += ' {}'.format(f)
    os.system(cmd)

    # load the histograms
    histlist = ht.loadallhistograms(outputfile)
    
    # select and change name
    if npmode=='npfromsim':
	histlist = ht.select_histograms(histlist, mustcontainall=['_3tight_'])[1]
	for hist in histlist: hist.SetName( hist.GetName().replace('_3tight_','_') )
    if npmode=='npfromdata':
	histlist = ht.select_histograms(histlist, mustcontainone=['_3prompt_','_fakerate_'])[1]
	for hist in histlist: 
	    hist.SetName( hist.GetName().replace('_3tight_','_').replace('_fakerate_','_') )

    # clip all resulting histograms to minimum zero
    ht.cliphistograms(histlist)

    # re-write output file
    f = ROOT.TFile.Open(outputfile,'recreate')
    for hist in histlist:
	hist.Write()
    f.Close()
