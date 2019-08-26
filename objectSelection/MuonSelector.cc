#include "../objects/interface/MuonSelector.h"


/*
loose muon selection
*/

bool MuonSelector::isLooseBase() const{
    if( muonPtr->pt() < 5 ) return false;
    if( fabs( muonPtr->eta() ) >= 2.4 ) return false; 
    if( !muonPtr->isLoosePOGMuon() ) return false;
    return true;
}


bool MuonSelector::isLoose2016() const{
    return true;
}


bool MuonSelector::isLoose2017() const{
    return true;
}


bool MuonSelector::isLoose2018() const{
    return true;
}


/*
FO muon selection
*/

bool MuonSelector::isFOBase() const{
    if( !isLoose() ) return false;
    if( !muonPtr->isMediumPOGMuon() ) return false;
    return true;
}


bool MuonSelector::isFO2016() const{
    return true;
}


bool MuonSelector::isFO2017() const{
    return true;
}


bool MuonSelector::isFO2018() const{
    return true;
}


/*
tight muon selection
*/

bool MuonSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( !muonPtr->isTightPOGMuon() ) return false;
    return true;
}


bool MuonSelector::isTight2016() const{
    return true;
}


bool MuonSelector::isTight2017() const{
    return true;
}


bool MuonSelector::isTight2018() const{
    return true;
}


/*
cone correction
*/


double MuonSelector::coneCorrection() const{
    return 1;
}
