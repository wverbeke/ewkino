#include "../objects/interface/MuonSelector.h"

//b-tagging working points
#include "bTagWP.h"


double ttHleptonMVACutMuon(){
    return 0.85;
}

double tZqleptonMVACutMuon(){
    return 0.;
}


/*
-------------------------------------------------------------------
loose muon selection (common to ttH and tZq ID)
-------------------------------------------------------------------
*/

bool MuonSelector::isLooseBase() const{
    if( muonPtr->uncorrectedPt() <= 5 ) return false;
    if( muonPtr->absEta() >= 2.4 ) return false; 
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
---------------------------------------------------------------------------------
FO muon selection for ttH ID
---------------------------------------------------------------------------------
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
    if( muonPtr->uncorrectedPt() <= 10 ) return false;
    if( muonPtr->leptonMVAttH() <= ttHleptonMVACutMuon() ){
        if( muonPtr->ptRatio() <= 0.65 ) return false;
    }
    return true;
}


bool MuonSelector::isFO2016() const{
    if( muonPtr->leptonMVAttH() <= ttHleptonMVACutMuon() ){
        double deepFlavorCut = slidingDeepFlavorThreshold( bTagWP::looseDeepFlavor2016(), bTagWP::mediumDeepFlavor2016(), muonPtr->uncorrectedPt() );
        if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    } else {
        if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2016() ) return false;
    }
    return true;
}


bool MuonSelector::isFO2017() const{
    if( muonPtr->leptonMVAttH() <= ttHleptonMVACutMuon() ){
        double deepFlavorCut = slidingDeepFlavorThreshold( bTagWP::looseDeepFlavor2017(), bTagWP::mediumDeepFlavor2017(), muonPtr->uncorrectedPt() );
        if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    } else {
        if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2017() ) return false;
    }
    return true;
}


bool MuonSelector::isFO2018() const{
    if( muonPtr->leptonMVAttH() <= ttHleptonMVACutMuon() ){
        double deepFlavorCut = slidingDeepFlavorThreshold( bTagWP::looseDeepFlavor2018(), bTagWP::mediumDeepFlavor2018(), muonPtr->uncorrectedPt() );
        if( muonPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    } else {
        if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2018() ) return false;
    }
    return true;
}

/*
------------------------------------------------------------------------------
FO muon selection for tZq ID
------------------------------------------------------------------------------
*/ 

bool MuonSelector::isFOBasetZq() const{
    // function to copy as closely as possible lepton ID of tZq analysis note
    if(!isLoose()) return false;
    if(muonPtr->uncorrectedPt()<10) return false;
    if(!muonPtr->isMediumPOGMuon()) return false;
    if(muonPtr->leptonMVAtZq() < tZqleptonMVACutMuon()){
	if(muonPtr->ptRatio()<0.6) return false;
    }
    return true;
}

bool MuonSelector::isFO2016tZq() const{
    if(muonPtr->leptonMVAtZq() < tZqleptonMVACutMuon()){
        if(muonPtr->closestJetDeepCSV() > 0.3) return false;
    }
    else{
        if(muonPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2016()) return false;
    }
    return true;    
}

bool MuonSelector::isFO2017tZq() const{
    if(muonPtr->leptonMVAtZq() < tZqleptonMVACutMuon()){
        if(muonPtr->closestJetDeepCSV()>0.2) return false;
    }
    else{
        if(muonPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2017()) return false;
    }
    return true;
}

bool MuonSelector::isFO2018tZq() const{
    if(muonPtr->leptonMVAtZq() < tZqleptonMVACutMuon()){
	if(muonPtr->closestJetDeepCSV()>0.2) return false;
    }
    else{
        if(muonPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2018()) return false;
    }
    return true;
}

/*
----------------------------------------------------------------------------
tight muon selection for ttH ID
----------------------------------------------------------------------------
*/

bool MuonSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( !muonPtr->isMediumPOGMuon() ) return false;
    if( muonPtr->leptonMVAttH() <= ttHleptonMVACutMuon() ) return false;
    return true;
}


bool MuonSelector::isTight2016() const{
    if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2016() ) return false;
    return true;
}


bool MuonSelector::isTight2017() const{
    if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2017() ) return false;
    return true;
}


bool MuonSelector::isTight2018() const{
    if( muonPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2018() ) return false;
    return true;
}

/* 
--------------------------------------------------------------------------
tight muon selection for tZq ID
--------------------------------------------------------------------------
*/

bool MuonSelector::isTightBasetZq() const{
    // function to copy as closely as possible lepton ID of tZq analysis note
    if(!isFOtZq()) return false;
    if(muonPtr->leptonMVAtZq() < tZqleptonMVACutMuon()) return false;
    return true;
}

bool MuonSelector::isTight2016tZq() const{
    if(muonPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2016()) return false;
    return true;
}

bool MuonSelector::isTight2017tZq() const{
    if(muonPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2017()) return false;
    return true;
}

bool MuonSelector::isTight2018tZq() const{
    if(muonPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2018()) return false;
    return true;
}


/*
cone correction
*/

double MuonSelector::coneCorrection() const{
    return ( 0.75 / muonPtr->ptRatio() );
}
