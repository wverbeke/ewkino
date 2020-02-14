###########################################################################################
# a little python script to get the sum of weights in a tree and compare it with hCounter #
###########################################################################################
# command line args: <root file> <tree name> <hCounter name>
# for files with no hCounter (e.g. data), use only first two args

import ROOT
import sys

if(len(sys.argv)!=3 and len(sys.argv)!=4):
    print('### ERROR ###: wrong number of command line args')
    sys.exit()
infile = sys.argv[1]
treename = sys.argv[2]
if len(sys.argv)==4:
    hcounter = sys.argv[3]

f = ROOT.TFile.Open(infile)
sumweights=0
try:
    tree = f.Get(treename)
    tree.GetEntry(0)
    testweight = getattr(tree,'_weight')
except:
    print('### ERROR ###: tree not found or has no attribute "_weight"')
    sys.exit()

# old method
'''
for j in range(int(tree.GetEntries())):
    if(j%5000==0):
        percent = j*100/tree.GetEntries()
        sys.stdout.write("\r"+str(percent)+'%')
        sys.stdout.flush()
    tree.GetEntry(j)
    sumweights += getattr(tree,'_weight')
print(sumweights)'''

# new method
h = ROOT.TH1F("h","h",1,1,2)
h.StatOverflows(ROOT.kTRUE) # use overflow bins for getmean and getentries!
tree.Draw("_weight>>h", "", "goff")
sumweights = h.GetMean()*h.GetEntries()

print('number of entries in tree: '+str(tree.GetEntries()))
print('sum of weights in tree: '+str(sumweights))
try:
    hcount = f.Get(hcounter).GetSumOfWeights()
    print('hcounter in tree: '+str(hcount))
    print('sum of weights to hcounter ratio: '+str(sumweights/hcount))
except:
    print('### WARNING ###: hCounter not found')

