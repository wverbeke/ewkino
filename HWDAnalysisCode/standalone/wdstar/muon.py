##########################################
# preliminary class definition for muons #
##########################################

import sys
import os
import ROOT

class Muon:
    
    def __init__( self, tree, evtidx, muidx ):
	tree.GetEntry(evtidx)
	self.pt = getattr(tree,'_lPt')[muidx]
	self.eta = getattr(tree,'_lEta')[muidx]
	self.istight = getattr(tree, '_lPOGTight')[muidx]
	self.isolation = getattr(tree, '_relIso0p4MuDeltaBeta')[muidx]
	self.innertrackx = getattr(tree, '_lMuonInnerTrackX')[muidx]
	self.innertracky = getattr(tree, '_lMuonInnerTrackY')[muidx]
	self.innertrackz = getattr(tree, '_lMuonInnerTrackZ')[muidx]

    def __str__( self ):
	info = 'Muon\n'
	info += '\tpt: {}\n'.format(self.pt)
	info += '\teta: {}\n'.format(self.eta)
	info += '\tistight: {}\n'.format(self.istight)
	info += '\tisolation: {}\n'.format(self.isolation)
	info += '\tinnertrackx: {}\n'.format(self.innertrackx)
	info += '\tinnertracky: {}\n'.format(self.innertracky)
	info += '\tinnertrackz: {}\n'.format(self.innertrackz)
	return info

class MuonCollection:

    def __init__( self, tree, evtidx ):
	tree.GetEntry(evtidx)
	self.collection = []
	nmu = getattr(tree,'_nMu')
	for i in range(nmu):
	    self.collection.append(Muon(tree, evtidx, i))

    def __getitem__( self, idx ):
	return self.collection[idx]

    def __len__( self ):
	return len(self.collection)

    def __str__( self ):
	info = '--- MuonCollection ---\n'
	for muon in self.collection:
	    info += str(muon)
	info += '---'
	return info
