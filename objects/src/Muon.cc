#include "../interface/Muon.h"

//include other parts of framework
#include "../interface/MuonSelector.h"


Muon::Muon( const TreeReader& treeReader, const unsigned leptonIndex ):
    LightLepton( treeReader, leptonIndex, new MuonSelector( this ) ),
    _segmentCompatibility( treeReader._lMuonSegComp[leptonIndex] ),
    _trackPt( treeReader._lMuonTrackPt[leptonIndex] ),
    _trackPtError( treeReader._lMuonTrackPtErr[leptonIndex] ),
    _isLoosePOGMuon( treeReader._lPOGLoose[leptonIndex] ),
    _isMediumPOGMuon( treeReader._lPOGMedium[leptonIndex] ),
    _isTightPOGMuon( treeReader._lPOGTight[leptonIndex] )
    {}
