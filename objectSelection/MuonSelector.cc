#include "../objects/interface/MuonSelector.h"

//b-tagging working points
#include "bTagWP.h"


// define the MVA threshold value depending on the lepton ID
double leptonMVACutMuon(){
    // dummy implementation for now, 
    // replace with correct ID's and thresholds later!
    if(LeptonSelector::leptonID()=="v1") return 1.0;
    else if(LeptonSelector::leptonID()=="v2") return 1.0;
    else if(LeptonSelector::leptonID()=="v1||v2") return 1.0;
    else return 1.0;
}


// define the MVA value depending on the lepton ID
double leptonMVAValueMuon(const Muon* muonPtr){
    if(LeptonSelector::leptonID()=="v1") return muonPtr->leptonMVATOPUL();
    else if(LeptonSelector::leptonID()=="v2") return muonPtr->leptonMVATOPv2UL();
    else if(LeptonSelector::leptonID()=="v1||v2") return 0.0;
    else return 0.0;
}


// define cone correction factor
double leptonConeCorrectionFactorMuon(){
    // dummy implementation for now,
    // replace with correct ID's and values later!
    if(LeptonSelector::leptonID()=="v1") return 0.0;
    else if(LeptonSelector::leptonID()=="v2") return 0.0;
    else if(LeptonSelector::leptonID()=="v1||v2") return 0.0;
    else return 0.0;
}


/*
loose muon selection
*/

// loose definition for leptonMVA UL v1
bool MuonSelector::isLooseBase_v1() const{
    if( muonPtr->uncorrectedPt() <= 5 ) return false;
    if( muonPtr->absEta() >= 2.4 ) return false; 
    if( fabs( muonPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( muonPtr->dz() ) >= 0.1 ) return false;
    if( muonPtr->sip3d() >= 8 ) return false;
    if( muonPtr->miniIso() >= 0.4 ) return false;
    if( !muonPtr->isMediumPOGMuon() ) return false;
    return true;
}


bool MuonSelector::isLoose2016_v1() const{
    return true;
}


bool MuonSelector::isLoose2016PreVFP_v1() const{
    return true;
}


bool MuonSelector::isLoose2016PostVFP_v1() const{
    return true;
}


bool MuonSelector::isLoose2017_v1() const{
    return true;
}


bool MuonSelector::isLoose2018_v1() const{
    return true;
}


// loose definition for leptonMVA UL v2
bool MuonSelector::isLooseBase_v2() const{
    if( muonPtr->uncorrectedPt() <= 5 ) return false;
    if( muonPtr->absEta() >= 2.4 ) return false;
    if( muonPtr->sip3d() >= 15 ) return false;
    if( muonPtr->relIso0p4() >= 1.0 ) return false;
    return true;
}


bool MuonSelector::isLoose2016_v2() const{
    return true;
}


bool MuonSelector::isLoose2016PreVFP_v2() const{
    return true;
}


bool MuonSelector::isLoose2016PostVFP_v2() const{
    return true;
}


bool MuonSelector::isLoose2017_v2() const{
    return true;
}


bool MuonSelector::isLoose2018_v2() const{
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


// FO definition for leptonMVA UL v1
bool MuonSelector::isFOBase_v1() const{
    if( !isLoose() ) return false;
    if( muonPtr->uncorrectedPt() <= 10 ) return false;
    return true;
}


bool MuonSelector::isFO2016_v1() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.02, 40., 0.015,
        muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2016PreVFP_v1() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
	if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.02, 40., 0.015, 
	muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2016PostVFP_v1() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.02, 40., 0.015, 
	muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2017_v1() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.025, 40., 0.015, 
                muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2018_v1() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
        if( muonPtr->closestJetDeepFlavor() > muonSlidingDeepFlavorThreshold( 20., 0.025, 40., 0.015, 
                muonPtr->uncorrectedPt()) ) return false;
        if( muonPtr->ptRatio() <= 0.45 ) return false;
    }
    return true;
}


// FO definition for leptonMVA UL v2
bool MuonSelector::isFOBase_v2() const{
    if( !isLoose() ) return false;
    if( muonPtr->uncorrectedPt() <= 10 ) return false;
    return true;
}


bool MuonSelector::isFO2016_v2() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
    }
    return true;
}


bool MuonSelector::isFO2016PreVFP_v2() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
    }
    return true;
}


bool MuonSelector::isFO2016PostVFP_v2() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
    }
    return true;
}


bool MuonSelector::isFO2017_v2() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
    }
    return true;
}


bool MuonSelector::isFO2018_v2() const{
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
    }
    return true;
}


// FO definition for FO optimization in grid search
bool MuonSelector::isFORunTime( double ptRatioCut, double deepFlavorCut, int extraCut ) const{
    // function for FO optimization, use ONLY in MC fake rate grid search
    if( !isLoose() ) return false;
    if( muonPtr->uncorrectedPt() <= 10 ) return false;
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ){
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

// tight definition for leptonMVA UL v1
bool MuonSelector::isTightBase_v1() const{
    if( !isFO() ) return false;
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ) return false;
    return true;
}


bool MuonSelector::isTight2016_v1() const{
    return true;
}


bool MuonSelector::isTight2016PreVFP_v1() const{
    return true;
}


bool MuonSelector::isTight2016PostVFP_v1() const{
    return true;
}


bool MuonSelector::isTight2017_v1() const{
    return true;
}


bool MuonSelector::isTight2018_v1() const{
    return true;
}


// tight definition for leptonMVA UL v2
bool MuonSelector::isTightBase_v2() const{
    if( !isFO() ) return false;
    if( leptonMVAValueMuon(muonPtr) <= leptonMVACutMuon() ) return false;
    return true;
}


bool MuonSelector::isTight2016_v2() const{
    return true;
}


bool MuonSelector::isTight2016PreVFP_v2() const{
    return true;
}


bool MuonSelector::isTight2016PostVFP_v2() const{
    return true;
}


bool MuonSelector::isTight2017_v2() const{
    return true;
}


bool MuonSelector::isTight2018_v2() const{
    return true;
}


/*
cone correction
*/


double MuonSelector::coneCorrection() const{
    return ( leptonConeCorrectionFactorMuon() / muonPtr->ptRatio() );
}
