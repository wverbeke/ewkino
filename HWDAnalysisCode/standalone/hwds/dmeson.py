############################################
# preliminary class definition for d meson #
############################################

import sys
import os
import ROOT

class DMeson:
    
    def __init__( self, tree, evtidx, didx ):
	tree.GetEntry(evtidx)
	# D meson variables
	self.mass = getattr(tree,'_DInvMass')[didx]
	self.dtype = getattr(tree,'_DType')[didx]
	self.px = getattr(tree,'_DPx')[didx]
	self.py = getattr(tree,'_DPy')[didx]
	self.pz = getattr(tree,'_DPz')[didx]
	self.pt = getattr(tree,'_DPt')[didx]
	self.eta = getattr(tree,'_DEta')[didx]
	self.phi = getattr(tree,'_DPhi')[didx]
	self.isolation = getattr(tree,'_DIsolation')[didx]
	self.dr = getattr(tree,'_DDR')[didx]
	self.hasfastgenmatch = getattr(tree,'_DHasFastGenMatch')[didx]
        self.hasgenmatch = getattr(tree,'_DHasGenMatch')[didx]
	# intermediate resonance variables
	self.intresmass = getattr(tree,'_DIntResMass')[didx]
	self.intresmassdiff = getattr(tree,'_DIntResMassDiff')[didx]
	self.intresx = getattr(tree,'_DIntResX')[didx]
	self.intresy = getattr(tree,'_DIntResY')[didx]
	self.intresz = getattr(tree,'_DIntResZ')[didx]
	self.intresvtxnormchi2 = getattr(tree,'_DIntResVtxNormChi2')[didx]
	# track variables
	self.firsttrackpt = getattr(tree,'_DFirstTrackPt')[didx]
	self.secondtrackpt = getattr(tree,'_DSecondTrackPt')[didx]
	self.thirdtrackpt = getattr(tree,'_DThirdTrackPt')[didx]
	self.firsttrackx = getattr(tree,'_DFirstTrackX')[didx]
	self.firsttracky = getattr(tree,'_DFirstTrackY')[didx]
	self.firsttrackz = getattr(tree,'_DFirstTrackZ')[didx]
	self.secondtrackx = getattr(tree,'_DSecondTrackX')[didx]
        self.secondtracky = getattr(tree,'_DSecondTrackY')[didx]
        self.secondtrackz = getattr(tree,'_DSecondTrackZ')[didx]
	self.thirdtrackx = getattr(tree,'_DThirdTrackX')[didx]
        self.thirdtracky = getattr(tree,'_DThirdTrackY')[didx]
        self.thirdtrackz = getattr(tree,'_DThirdTrackZ')[didx]

    def twotrack_spatial_separation_x(self):
	return abs(self.firsttrackx-self.secondtrackx)

    def twotrack_spatial_separation_y(self):
        return abs(self.firsttracky-self.secondtracky)

    def twotrack_spatial_separation_z(self):
        return abs(self.firsttrackz-self.secondtrackz)

    def restrack_spatial_separation_x(self):
        return abs(self.intresx-self.thirdtrackx)
    
    def restrack_spatial_separation_y(self):
        return abs(self.intresy-self.thirdtracky)

    def restrack_spatial_separation_z(self):
        return abs(self.intresz-self.thirdtrackz)


    def __str__( self ):
	info = 'DMeson\n'
	info += '\tmass: {}\n'.format(self.mass)
	info += '\tdtype: {}\n'.format(self.dtype)
	info += '\tintresmass: {}\n'.format(self.intresmass)
	info += '\tmassdiff: {}\n'.format(self.massdiff)
	info += '\tpt: {}\n'.format(self.pt)
	info += '\tthirdtrackx: {}\n'.format(self.thirdtrackx)
	info += '\tthirdtracky: {}\n'.format(self.thirdtracky)
	info += '\tthirdtrackz: {}\n'.format(self.thirdtrackz)
	return info

class DMesonCollection:

    def __init__( self, tree, evtidx ):
	tree.GetEntry(evtidx)
	self.collection = []
	nds = getattr(tree,'_nDs')
	for i in range(nds):
	    self.collection.append(DMeson(tree, evtidx, i))

    def __getitem__( self, idx ):
	return self.collection[idx]

    def __len__( self ):
	return len(self.collection)

    def __str__( self ):
	info = '--- DMesonCollection ---\n'
	for dmeson in self.collection:
	    info += str(dmeson)
	info += '---'
	return info
