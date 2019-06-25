#include "../interface/MuonSelector.h"


//include other parts of framework
#include "../interface/Muon.h"


bool MuonSelector::isLoose() const{
    if( muonPtr->pt() < 5 ) return false;
    if( fabs( muonPtr->eta() ) < 2.4 ) return false; 
    if( !muonPtr->isLoosePOGMuon() ) return false;
    return true;
}


bool MuonSelector::isFO() const{
    if( !isLoose() ) return false;
    if( !muonPtr->isMediumPOGMuon() ) return false;
    return true;
}


bool MuonSelector::isGood() const{
    return isFO();
}


bool MuonSelector::isTight() const{
    if( !isFO() ) return false;
    return true;
}
