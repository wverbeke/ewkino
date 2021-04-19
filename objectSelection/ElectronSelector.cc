#include "../objects/interface/ElectronSelector.h"

//include c++ library classes
#include <cmath>

//include other parts of framework
#include "bTagWP.h"

///// general comments /////
// - tzq loose was simply copied from Marek's code, 
//   in order to perform a trigger efficiency measurement;
//   it was not yet tested or used in any other way.

// define here what mva threshold to use in tZq ID's listed below
double electronMVACut(){
    if(LeptonSelector::leptonID()=="tth") return 0.8;
    else if(LeptonSelector::leptonID()=="oldtzq") return 0.8;
    else if(LeptonSelector::leptonID()=="tzqtight") return 0.9;
    else if(LeptonSelector::leptonID()=="tzqmedium0p4") return 0.4;
    else if(LeptonSelector::leptonID()=="tzqloose") return 0.0;
    else return 1;
}

// define here what mva value to use in tZq ID's listed below
double electronMVAValue(const Electron* electronPtr){
    if(LeptonSelector::leptonID()=="tth") return electronPtr->leptonMVAttH();
    else if(LeptonSelector::leptonID()=="oldtzq") return electronPtr->leptonMVAtZq();
    else if(LeptonSelector::leptonID()=="tzqtight") return electronPtr->leptonMVATOP();
    else if(LeptonSelector::leptonID()=="tzqmedium0p4") return electronPtr->leptonMVATOP();
    else if(LeptonSelector::leptonID()=="tzqloose") return electronPtr->leptonMVATOP();
    else return 0;
}

// define here what b-tagger threshold to use in all tZq ID's listed below
// warning: deepFlavor thresholds are hard-coded in ttH ID!
double electronJetBTagCut(const std::string wp, const std::string year){
    //return bTagWP::getWP("DeepCSV", wp, year);
    return bTagWP::getWP("DeepFlavor", wp, year);    
}

// define here what b-tagger to use in all tZq ID's listed below
// warning: deepFlavor is hard-coded in ttH ID!
double electronJetBTagValue(const Electron* electronPtr){
    //return electronPtr->closestJetDeepCSV();
    return electronPtr->closestJetDeepFlavor();
}

// define here what cone correction factor to use
double electronConeCorrectionFactor(){
    if(LeptonSelector::leptonID()=="tth") return 0.9;
    else if(LeptonSelector::leptonID()=="oldtzq") return 0.95;
    else if(LeptonSelector::leptonID()=="tzqtight") return 0.8;
    else if(LeptonSelector::leptonID()=="tzqmedium0p4") return 0.67;
    else if(LeptonSelector::leptonID()=="tzqloose") return 0.6495;
    else return 0;
}

/*
----------------------------------------------------------------
loose electron selection (common for ttH and tZq IDs)
----------------------------------------------------------------
*/
bool ElectronSelector::isLooseBase() const{
    if( electronPtr->uncorrectedPt() < 7 ) return false;
    //if( electronPtr->uncorrectedPt() < 5 ) return false; // for syncing with TT
    if( electronPtr->absEta() >= 2.5 ) return false;
    if( fabs( electronPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( electronPtr->dz() ) >= 0.1 ) return false;
    if( electronPtr->sip3d() >= 8 ) return false;
    if( electronPtr->numberOfMissingHits() >= 2 ) return false;
    if( electronPtr->miniIso() >= 0.4 ) return false;
    return true;
}


bool ElectronSelector::isLoose2016() const{ 
    return true;
}


bool ElectronSelector::isLoose2017() const{
    return true;
}


bool ElectronSelector::isLoose2018() const{
    return true;
}

/*
--------------------------------------------------------------------------------
help function for FO ID's
-------------------------------------------------------------------------------
*/

//interpolation between two working points of deepFlavor between two pT values
double electronSlidingDeepFlavorThreshold( const double lowPt, const double lowPtWP,
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

/*
-------------------------------------------------------------------
FO electron selection for (tight) tZq ID
-------------------------------------------------------------------
*/

bool ElectronSelector::isFOBasetZq() const{
    if( !isLoose() ) return false;
    if( electronPtr->uncorrectedPt() <= 10 ) return false;
    if( electronPtr->hOverE() >= 0.1 ) return false;
    if( electronPtr->inverseEMinusInverseP() <= -0.04 ) return false;
    if( std::abs(electronPtr->etaSuperCluster()) <= 1.479 ){
        if( electronPtr->sigmaIEtaEta() >= 0.011 ) return false;
    } else {
        if( electronPtr->sigmaIEtaEta() >= 0.030 ) return false;
    }
    if( !electronPtr->passConversionVeto() ) return false;
    // put tunable FO cuts below
    //if( electronMVAValue(electronPtr) < electronMVACut() ){
    //} // see different years!
    return true;
}


bool ElectronSelector::isFO2016tZq() const{
    if( electronMVAValue(electronPtr) < electronMVACut() ){
        /*if( electronPtr->closestJetDeepFlavor() > 0.1 ) return false;
	if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoWP80() ) return false;*/	
    
	if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(
	    25,0.1,50,0.05,
            electronPtr->uncorrectedPt()) ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoWP90() ) return false; 
    }
    return true;
}

bool ElectronSelector::isFO2017tZq() const{
    if( electronMVAValue(electronPtr) < electronMVACut() ){
        /*if( electronPtr->closestJetDeepFlavor() > 0.1 ) return false;
	if( electronPtr->ptRatio() < 0.5 ) return false;
	if( !electronPtr->passElectronMVAFall17NoIsoWP80() ) return false;*/

	if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(
	    25,0.1,50,0.05,
            electronPtr->uncorrectedPt()) ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoWP90() ) return false; 
    }
    return true;
}

bool ElectronSelector::isFO2018tZq() const{
    if( electronMVAValue(electronPtr) < electronMVACut() ){
	/*if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(
	    30,0.15,60,0.07,
            electronPtr->uncorrectedPt()) ) return false;
	if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoWP80() ) return false; */

	if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(
	    25,0.1,50,0.05,
            electronPtr->uncorrectedPt()) ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoWP90() ) return false;
    }
    return true;
}

/*
-------------------------------------------------------------------
FO electron selection for medium 0p4 tZq ID
-------------------------------------------------------------------
*/

bool ElectronSelector::isFOBasetZqMedium0p4() const{
    if( !isLoose() ) return false;
    if( electronPtr->uncorrectedPt() <= 10 ) return false;
    if( electronPtr->hOverE() >= 0.1 ) return false;
    if( electronPtr->inverseEMinusInverseP() <= -0.04 ) return false;
    if( std::abs(electronPtr->etaSuperCluster()) <= 1.479 ){
        if( electronPtr->sigmaIEtaEta() >= 0.011 ) return false;
    } else {
        if( electronPtr->sigmaIEtaEta() >= 0.030 ) return false;
    }
    if( !electronPtr->passConversionVeto() ) return false;
    if( !electronPtr->passChargeConsistency() ) return false; // for testing if this fixes closure
    // put tunable FO cuts below
    //if( electronMVAValue(electronPtr) < electronMVACut() ){
    //}
    return true;
}


bool ElectronSelector::isFO2016tZqMedium0p4() const{
    if( electronMVAValue(electronPtr) < electronMVACut() ){
	// modification attempt:
	//if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(25,0.5,50,0.1,
        //    electronPtr->uncorrectedPt()) ) return false;
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;	
	// Tu Thong's original FO ID:
        /*if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(25,0.5,50,0.05,
	    electronPtr->uncorrectedPt()) ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;*/
    }
    return true;
}

bool ElectronSelector::isFO2017tZqMedium0p4() const{
    if( electronMVAValue(electronPtr) < electronMVACut() ){
	// modification attempt:
	//if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(25,0.5,50,0.1,
        //    electronPtr->uncorrectedPt()) ) return false;
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
	// Tu Thong's original FO ID:
	/*if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(25,0.5,50,0.08,
	    electronPtr->uncorrectedPt()) ) return false;
	if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;*/
    }
    return true;
}

bool ElectronSelector::isFO2018tZqMedium0p4() const{
    if( electronMVAValue(electronPtr) < electronMVACut() ){
	// modification attempt:
	//if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(25,0.5,50,0.1,
        //    electronPtr->uncorrectedPt()) ) return false;
        if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
	if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
	// Tu Thong's original FO ID:
        /*if( electronPtr->closestJetDeepFlavor() > electronSlidingDeepFlavorThreshold(25,0.4,50,0.05,
	    electronPtr->uncorrectedPt())) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;*/
    }
    return true;
}


/*
-------------------------------------------------------------------
FO electron selection for loose tZq ID
-------------------------------------------------------------------
*/

bool ElectronSelector::isFOBasetZqLoose() const{
    if( !isLoose() ) return false;
    if( electronPtr->uncorrectedPt() < 10 ) return false;
    if( electronPtr->numberOfMissingHits() > 0 ) return false; // added
    if( electronPtr->hOverE() >= 0.1 ) return false;
    if( electronPtr->inverseEMinusInverseP() <= -0.04 ) return false;
    if( electronPtr->etaSuperCluster() <= 1.479 ){
        if( electronPtr->sigmaIEtaEta() >= 0.011 ) return false;
    } else {
        if( electronPtr->sigmaIEtaEta() >= 0.030 ) return false;
    }
    if( electronMVAValue(electronPtr) <= electronMVACut() ){
        if( electronPtr->ptRatio() <= 0.34 ) return false;   // changed from 0.4
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false; // added, other options WP80, WP90
    }
    if( !electronPtr->passConversionVeto() ) return false; // applied only to 2L selection in ttZ
    return true;
}

bool ElectronSelector::isFO2016tZqLoose() const{
    if( electronMVAValue(electronPtr) <= electronMVACut() ){
        //if( electronPtr->closestJetDeepFlavor() >= slidingDeepFlavorThreshold( 0.3493, 0.4693, 
	//	electronPtr->uncorrectedPt() ) ) return false;
	if( electronPtr->closestJetDeepFlavor() >= electronSlidingDeepFlavorThreshold(
	    20, 0.4693, 45, 0.3493, electronPtr->uncorrectedPt() ) ) return false;
    }
    return true;
}

bool ElectronSelector::isFO2017tZqLoose() const{
    if( electronMVAValue(electronPtr) <= electronMVACut() ){
        //if( electronPtr->closestJetDeepFlavor() >= slidingDeepFlavorThreshold( 0.3833, 0.4433, 
	//	electronPtr->uncorrectedPt() ) ) return false;
	if( electronPtr->closestJetDeepFlavor() >= electronSlidingDeepFlavorThreshold(
            20, 0.4433, 45, 0.3833, electronPtr->uncorrectedPt() ) ) return false;
    }
    return true;
}

bool ElectronSelector::isFO2018tZqLoose() const{
    if( electronMVAValue(electronPtr) <= electronMVACut() ){
        //if( electronPtr->closestJetDeepFlavor() >= slidingDeepFlavorThreshold( 0.4170, 0.3170, 
	//	electronPtr->uncorrectedPt() ) ) return false;
	if( electronPtr->closestJetDeepFlavor() >= electronSlidingDeepFlavorThreshold(
            20, 0.3170, 45, 0.4170, electronPtr->uncorrectedPt() ) ) return false;
    }
    return true;
}
    
/*
-------------------------------------------------------------------
FO electron selection for ttH ID
-------------------------------------------------------------------
*/

bool ElectronSelector::isFOBasettH() const{
    if( !isLoose() ) return false;
    if( electronPtr->uncorrectedPt() <= 10 ) return false;
    if( electronPtr->numberOfMissingHits() > 0 ) return false;
    if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    if( electronPtr->hOverE() >= 0.1 ) return false;
    if( electronPtr->inverseEMinusInverseP() <= -0.04 ) return false;
    if( electronPtr->etaSuperCluster() <= 1.479 ){
        if( electronPtr->sigmaIEtaEta() >= 0.011 ) return false;
    } else {
        if( electronPtr->sigmaIEtaEta() >= 0.030 ) return false;
    }
    if( electronMVAValue(electronPtr) <= electronMVACut() ){
        if( !electronPtr->passElectronMVAFall17NoIsoWP80() ) return false;
    	if( electronPtr->ptRatio() <= 0.7 ) return false;
    }
    if( !electronPtr->passConversionVeto() ) return false;
    return true;
}


bool ElectronSelector::isFO2016ttH() const{
    if( electronPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2016() ) return false;
    return true;
}

bool ElectronSelector::isFO2017ttH() const{
    if( electronPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2017() ) return false;
    return true;
}

bool ElectronSelector::isFO2018ttH() const{
    if( electronPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2018() ) return false;
    return true;
}

/*
-------------------------------------------------------------------
FO electron selection for old tZq ID
-------------------------------------------------------------------
*/

bool ElectronSelector::isFOBaseOldtZq() const{
    // function to copy as closely as possible the lepton ID of the analysis note of tZq discovery
    if(!isLoose()) return false;
    if(electronPtr->uncorrectedPt()<10) return false;
    if(!electronPtr->passDoubleEGEmulation()) return false;
    if(electronMVAValue(electronPtr) < electronMVACut()){
	if(electronPtr->ptRatio()<0.6) return false;
    }
    return true;
}

bool ElectronSelector::isFO2016OldtZq() const{
    if(electronMVAValue(electronPtr) < electronMVACut()){
        if(electronJetBTagValue(electronPtr) > 0.3) return false;
        if(electronPtr->absEta()<1.479){ if(electronPtr->electronMVASummer16GP()<0.4) return false; }
        else{ if(electronPtr->electronMVASummer16GP()<0.7) return false; }
    }
    else{ if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2016")) return false; }
    return true;
}

bool ElectronSelector::isFO2017OldtZq() const{
    if(electronMVAValue(electronPtr) < electronMVACut()){
        if(electronJetBTagValue(electronPtr) > 0.2) return false;
        if(electronPtr->absEta()<1.479){ if(electronPtr->electronMVAFall17NoIso()<0.3) return false; }
        else{ if(electronPtr->electronMVAFall17NoIso()<0.6) return false; }
    }
    else{ if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2017")) return false; }
    return true;
}

bool ElectronSelector::isFO2018OldtZq() const{
    if(electronMVAValue(electronPtr) < electronMVACut()){
        if(electronJetBTagValue(electronPtr) > 0.2) return false;
        if(electronPtr->absEta()<1.479){ if(electronPtr->electronMVAFall17NoIso()<0.3) return false; }
        else{ if(electronPtr->electronMVAFall17NoIso()<0.6) return false; }
    }
    else{ if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2018")) return false; }
    return true;
}

/*
-------------------------------------------------------------------------
tight electron selection for (tight) tZq ID
-------------------------------------------------------------------------
*/

bool ElectronSelector::isTightBasetZq() const{
    if( !isFOtZq() ) return false;
    if( electronMVAValue(electronPtr) <= electronMVACut() ) return false;
    return true;
}


bool ElectronSelector::isTight2016tZq() const{
    return true;
}


bool ElectronSelector::isTight2017tZq() const{
    return true;
}


bool ElectronSelector::isTight2018tZq() const{
    return true;
}


/*
-------------------------------------------------------------------------
tight electron selection for medium 0p4 tZq ID
-------------------------------------------------------------------------
*/

bool ElectronSelector::isTightBasetZqMedium0p4() const{
    if( !isFOtZqMedium0p4() ) return false;
    if( electronMVAValue(electronPtr) <= electronMVACut() ) return false;
    return true;
}


bool ElectronSelector::isTight2016tZqMedium0p4() const{
    return true;
}


bool ElectronSelector::isTight2017tZqMedium0p4() const{
    return true;
}


bool ElectronSelector::isTight2018tZqMedium0p4() const{
    return true;
}


/* 
------------------------------------------------------------------------
tight electron selection for loose tZq ID
------------------------------------------------------------------------
*/

bool ElectronSelector::isTightBasetZqLoose() const{
    if( !isFOtZqLoose() ) return false;
    if( electronMVAValue(electronPtr) <= electronMVACut() ) return false;
    return true;
}


bool ElectronSelector::isTight2016tZqLoose() const{
    return true;
}


bool ElectronSelector::isTight2017tZqLoose() const{
    return true;
}


bool ElectronSelector::isTight2018tZqLoose() const{
    return true;
}


/*
-------------------------------------------------------------------------
tight electron selection for ttH ID
-------------------------------------------------------------------------
*/

bool ElectronSelector::isTightBasettH() const{
    if( !isFOttH() ) return false;
    if( electronMVAValue(electronPtr) <= electronMVACut() ) return false;
    return true;
}


bool ElectronSelector::isTight2016ttH() const{
    return true;
}


bool ElectronSelector::isTight2017ttH() const{
    return true;
}


bool ElectronSelector::isTight2018ttH() const{
    return true;
}

/*
----------------------------------------------------------------------
tight electron selection for old tZq ID
----------------------------------------------------------------------
*/

bool ElectronSelector::isTightBaseOldtZq() const{
    if(!isFOOldtZq()) return false;
    if(electronMVAValue(electronPtr) < electronMVACut()) return false;
    return true;
}

bool ElectronSelector::isTight2016OldtZq() const{
    if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2016")) return false;
    return true;
}

bool ElectronSelector::isTight2017OldtZq() const{
    if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2017")) return false;
    return true;
}

bool ElectronSelector::isTight2018OldtZq() const{
    if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2018")) return false;
    return true;
}

/*
cone correction
*/

double ElectronSelector::coneCorrection() const{
    return ( electronConeCorrectionFactor() / electronPtr->ptRatio() );
}
