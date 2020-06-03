#####################################################
# functionality to add a branch to an existing tree #
#####################################################
# for safety, the original tree is not modified in-place, 
# but a new tree is copied and extended from the original one

import os
import sys
import array
import ROOT

def addtmvabranch(infilename,intreename,outfilename,weightfilename):
    # WARNING: make sure outfile does not exist before running this function!
    
    # open input and output file
    infile = ROOT.TFile.Open(infilename)
    intree = infile.Get(intreename)
    outfile = ROOT.TFile(outfilename,'update')
    outtree = intree.CloneTree(0)

    # define mva value reference and make branch
    mvavalue = array.array('f',[0])
    outtree.Branch('_eventbdt',mvavalue,'F')

    # make correct directory structure
    rdirname = ''
    if '/' in intreename:
        rdirname = intreename.rsplit('/',1)[0]
        outfile.mkdir(rdirname)

    # define variables and TReader
    varnames = (['_abs_eta_recoil','_Mjj_max','_lW_asymmetry',
                '_deepCSV_max','_lT','_MT','_dPhill_max',
                '_pTjj_max','_dRlb_min','_HT','_dRlWrecoil','_dRlWbtagged',
                '_M3l','_abs_eta_max'])
    varnames += (['_nJets','_nBJets']) # make sure it corresponds to tmvatrain.py!
    varvalues = []
    treader = ROOT.TMVA.Reader()
    for j,varname in enumerate(varnames): 
	varvalues.append(array.array('f',[0]))
	treader.AddVariable(varname,varvalues[j])
    #weightfilename = os.path.abspath('../bdt/outdata/weights/tmvatrain_BDT.weights.xml')
    treader.BookMVA('BDT',weightfilename)

    # loop over entries
    for i in range(intree.GetEntries()):
	# get entry and variables
	# (note that variables are not needed to copy, only for BDT evaluation)
	intree.GetEntry(i)
	for j,varname in enumerate(varnames):
	    varvalues[j][0] = getattr(intree,varname)
	mvavalue[0] = treader.EvaluateMVA('BDT')
	outtree.Fill()

    outfile.cd(rdirname)
    outtree.Write()
    outfile.Close()
    infile.Close()


if __name__=='__main__':

    inname = os.path.abspath(sys.argv[1]) # either root file or folder name containing root files
    weightfilename = os.path.abspath(sys.argv[2]) # path to TMVA weight file

    if(inname[-5:]=='.root'):
	infilenames = [inname]
    else:
	infilenames = [os.path.join(inname,f) for f in os.listdir(inname) if f[-5:]=='.root']

    for infilename in infilenames:
	outfilename = infilename.rstrip('.root')+'_ext.root'
	# importat: remove extended file if it already exists!
	if os.path.exists(outfilename):
	    os.system('rm '+outfilename)
	addtmvabranch(infilename,'blackJackAndHookers/treeCat1',outfilename,weightfilename)
	addtmvabranch(infilename,'blackJackAndHookers/treeCat2',outfilename,weightfilename)
	addtmvabranch(infilename,'blackJackAndHookers/treeCat3',outfilename,weightfilename)
