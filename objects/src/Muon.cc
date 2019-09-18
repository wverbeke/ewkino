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


Muon::Muon( const Muon& rhs ):
    LightLepton( rhs, new MuonSelector( this ) ),
    _segmentCompatibility( rhs._segmentCompatibility ),
    _trackPt( rhs._trackPt ),
    _trackPtError( rhs._trackPtError ),
    _isLoosePOGMuon( rhs._isLoosePOGMuon ),
    _isMediumPOGMuon( rhs._isMediumPOGMuon ),
    _isTightPOGMuon( rhs._isTightPOGMuon )
    {}    


Muon::Muon( Muon&& rhs ) noexcept:
    LightLepton( std::move( rhs ), new MuonSelector( this ) ),
    _segmentCompatibility( rhs._segmentCompatibility ),
    _trackPt( rhs._trackPt ),
    _trackPtError( rhs._trackPtError ),
    _isLoosePOGMuon( rhs._isLoosePOGMuon ),
    _isMediumPOGMuon( rhs._isMediumPOGMuon ),
    _isTightPOGMuon( rhs._isTightPOGMuon )
    {}    


std::ostream& Muon::print( std::ostream& os ) const{
    os << "Muon : ";
    LightLepton::print( os );
    os << " / segmentCompatibility = " << _segmentCompatibility << " / trackPt = " << _trackPt << " / trackPtError = " << _trackPtError;
    if( _isTightPOGMuon ){
        os << " / tight POG muon";
    } else if( _isMediumPOGMuon ){
        os << " / medium POG muon";
    } else if( _isLoosePOGMuon ){
        os << " / loose POG muon";
    } else {
        os << " / fails POG muon selection";
    }
    return os;
}
