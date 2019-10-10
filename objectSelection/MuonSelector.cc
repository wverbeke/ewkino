#include "../objects/interface/MuonSelector.h"

//b-tagging working points
#include "bTagWP.h"


/*
loose muon selection
*/

bool MuonSelector::isLooseBase() const{
    if( muonPtr->pt() <= 5 ) return false;
    if( fabs( muonPtr->eta() ) >= 2.4 ) return false; 
    if( fabs( muonPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( muonPtr->dz() ) >= 0.1 ) return false;
    if( muonPtr->sip3d() >= 8 ) return false;
    if( muonPtr->miniIso() >= 0.4 ) return false;
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

//interpolation between loose and medium working point of deep flavor from 20 to 45 GeV in muon pT as defined in the ttH analysis
double slidingDeepFlavorThreshold( const double looseWP, const double mediumWP, const double pt ){
    static const double minPt = 20.;
    static const double maxPt = 45.;
    if( pt < minPt ){
        return mediumWP;
    } else if( pt > maxPt ){
        return looseWP;
    } else {
        return ( mediumWP - ( mediumWP - looseWP ) / ( maxPt - minPt ) * ( pt - minPt ) );
    }
}


bool MuonSelector::isFOBase() const{
    if( !isLoose() ) return false;
    //currently no medium muon applied in FO, as in ttH AN!
    if( muonPtr->pt() <= 10 ) return false;
    return true;
}


bool MuonSelector::isFO2016() const{
    double deepFlavorCut = slidingDeepFlavorThreshold( bTagWP::looseDeepFlavor2016(), bTagWP::mediumDeepFlavor2016(), muonPtr->pt() );
    if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    return true;
}


bool MuonSelector::isFO2017() const{
    double deepFlavorCut = slidingDeepFlavorThreshold( bTagWP::looseDeepFlavor2017(), bTagWP::mediumDeepFlavor2017(), muonPtr->pt() );
    if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    return true;
}


bool MuonSelector::isFO2018() const{
    double deepFlavorCut = slidingDeepFlavorThreshold( bTagWP::looseDeepFlavor2018(), bTagWP::mediumDeepFlavor2018(), muonPtr->pt() );
    if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    return true;
}


/*
tight muon selection
*/

bool MuonSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( !muonPtr->isMediumPOGMuon() ) return false;
    if( muonPtr->leptonMVAttH() <= 0.85 ) return false;
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
    return ( 0.8 / muonPtr->ptRatio() );
}
