############################################
# preliminary class definition for d meson #
############################################

import sys
import os
import ROOT

class DMeson:
    
    def __init__( self, tree, evtidx, didx ):
	tree.GetEntry(evtidx)
	self.mass = getattr(tree,'_DInvMass')[didx]
	self.intresmass = getattr(tree,'_DIntResMass')[didx]
	self.massdiff = getattr(tree,'_DMassDiff')[didx]
	self.pt = getattr(tree,'_DPt')[didx]
	self.thirdtrackx = getattr(tree,'_DThirdTrackX')[didx]
	self.thirdtracky = getattr(tree,'_DThirdTrackY')[didx]
	self.thirdtrackz = getattr(tree,'_DThirdTrackZ')[didx]

    def __str__( self ):
	info = 'DMeson\n'
	info += '\tmass: {}\n'.format(self.mass)
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
