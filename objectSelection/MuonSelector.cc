#include "../objects/interface/MuonSelector.h"

//b-tagging working points
#include "bTagWP.h"


double leptonMVACutMuon(){
    return 0.4;
}


/*
loose muon selection
*/

bool MuonSelector::isLooseBase() const{
    if( muonPtr->uncorrectedPt() <= 5 ) return false;
    if( muonPtr->absEta() >= 2.4 ) return false; 
    if( fabs( muonPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( muonPtr->dz() ) >= 0.1 ) return false;
    if( muonPtr->sip3d() >= 8 ) return false;
    if( muonPtr->miniIso() >= 0.4 ) return false;
    if( !muonPtr->isMediumPOGMuon() ) return false;
    return true;
}


bool MuonSelector::isLoose2016() const{
    return true;
}


bool MuonSelector::isLoose2016PreVFP() const{
    return true;
}


bool MuonSelector::isLoose2016PostVFP() const{
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

//interpolation between two working points of deepFlavor between two pT values
double muonSlidingDeepFlavorThreshold( const double lowPt, const double lowPtWP, 
		    const double highPt, const double highPtWP,
		    const double pt ){
    if( pt < lowPt ){
        return lowPtWP;
    } else if( pt > highPt ){
        return highPtWP;
    } else {
        return ( lowPtWP + ( highPtWP - lowPtWP ) / ( highPt - lowPt ) * ( pt - lowPt ) );
    }
}


bool MuonSelector::isFOBase() const{
    if( !isLoose() ) return false;
    if( muonPtr->uncorrectedPt() <= 10 ) return false;
    return true;
}


bool MuonSelector::isFO2016() const{

    if( muonPtr->leptonMVATOP() <= leptonMVACutMuon() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.02, 40., 0.015,
        muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2016PreVFP() const{

    if( muonPtr->leptonMVATOP() <= leptonMVACutMuon() ){
	if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.02, 40., 0.015, 
	muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2016PostVFP() const{

    if( muonPtr->leptonMVATOP() <= leptonMVACutMuon() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.02, 40., 0.015, 
	muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2017() const{
    
    if( muonPtr->leptonMVATOP() <= leptonMVACutMuon() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.025, 40., 0.015, 
                muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2018() const{
    if( muonPtr->leptonMVATOP() <= leptonMVACutMuon() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.025, 40., 0.015, 
                muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


bool MuonSelector::isFORunTime( double ptRatioCut, double deepFlavorCut, int extraCut ) const{
    // function for FO optimization, use ONLY in MC fake rate grid search
    if( !isLoose() ) return false;
    if( muonPtr->uncorrectedPt() <= 10 ) return false;
    if( muonPtr->leptonMVATOP() <= leptonMVACutMuon() ){
	// dummy condition on extraCut to avoid compilation warnings
	if( extraCut < -9999 ) return false;
        if( muonPtr->ptRatio() <= ptRatioCut ) return false;
	if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    }
    return true;
}
   

/*
tight muon selection
*/

bool MuonSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( muonPtr->leptonMVATOP() <= leptonMVACutMuon() ) return false;
    return true;
}


bool MuonSelector::isTight2016() const{
    return true;
}


bool MuonSelector::isTight2016PreVFP() const{
    return true;
}


bool MuonSelector::isTight2016PostVFP() const{
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
    return ( 0.67 / muonPtr->ptRatio() );
}
