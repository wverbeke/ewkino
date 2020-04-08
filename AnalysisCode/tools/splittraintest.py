#####################################################################
# python functionality to split a root tree into two separate trees #
#####################################################################
import ROOT
import sys
import os

folder = os.path.abspath('/user/llambrec/Files/signalregion/2016MC_flat')
if len(sys.argv)==2:
    folder = os.path.abspath(sys.argv[1])
else:
    print('### WARNING ###: wrong number of command line args found. Use hard-coded args? (y/n)')
    go = raw_input()
    if not go=='y': sys.exit()

treenames = ['blackJackAndHookers/treeCat1',
		'blackJackAndHookers/treeCat2',
		'blackJackAndHookers/treeCat3']

# remove all _train and _test files in the directory
trainfilelist = [f for f in os.listdir(folder) if f[-10:]=='train.root']
testfilelist = [f for f in os.listdir(folder) if f[-9:]=='test.root']
for f in trainfilelist+testfilelist:
    os.system('rm '+os.path.join(folder,f))
    
# make new train and test files
filelist = [f for f in os.listdir(folder) if f[-5:]=='.root']
for f in filelist:
    try: rfile = ROOT.TFile(os.path.join(folder,f))
    except:
        print('### ERROR ###: file '+f+' could not be opened.')
        continue
    print('found file '+f)
    trainfile = ROOT.TFile(os.path.join(folder,f[:-5]+'_train.root'),"RECREATE")
    testfile = ROOT.TFile(os.path.join(folder,f[:-5]+'_test.root'),"RECREATE")
    for treename in treenames:
	try: tree = rfile.Get(treename); test = tree.GetEntries()
	except: 
	    print('### ERROR ###: tree '+treename+' in file '+f+' could not be opened.')
	    continue
	print('found tree with '+tree.GetEntries()+' entries')
	trainfraction = 0.5
	ntrain = int(trainfraction*tree.GetEntries())
	ntest = tree.GetEntries()-ntrain
	# copy tree to train and test tree and write them to file
	if '/' in treename: treename = treename[treename.rfind('/'):]
	trainfile.cd()
	traintree = tree.CopyTree("","",ntrain,0)
	print(traintree.GetEntries())
	traintree.Write(treename+'_train')
	testfile.cd()
	testtree = tree.CopyTree("","",ntest,ntrain)
	testtree.Write(treename+'_test')

    trainfile.Close()
    testfile.Close()
    rfile.Close()
