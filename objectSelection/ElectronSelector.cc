#include "../objects/interface/ElectronSelector.h"

//include c++ library classes
#include <cmath>

//include other parts of framework
#include "bTagWP.h"

// define here what mva threshold to use in tZq ID's listed below
double electronMVACut(){
    return 0.8; // for leptonMVAttH
    //return 0.8; // for leptonMVAtZq
    //return -1.; // for leptonMVATOP
}

// define here what mva value to use in tZq ID's listed below
double electronMVAValue(const Electron* electronPtr){
    return electronPtr->leptonMVAttH();
    //return electronPtr->leptonMVAtZq();
    //return electronPtr->leptonMVATOP();
}

// define here what b-tagger threshold to use in all tZq ID's listed below
// warning: deepFlavor thresholds are hard-coded in ttH ID!
double electronJetBTagCut(const std::string wp, const std::string year){
    return bTagWP::getWP("DeepCSV", wp, year);
    //return bTagWP::getWP("DeepFlavor", wp, year);    
}

// define here what b-tagger to use in all tZq ID's listed below
// warning: deepFlavor is hard-coded in ttH ID!
double electronJetBTagValue(const Electron* electronPtr){
    return electronPtr->closestJetDeepCSV();
    //return electronPtr->closestJetDeepFlavor();
}

/*
----------------------------------------------------------------
loose electron selection (common for ttH and tZq ID)
----------------------------------------------------------------
*/
bool ElectronSelector::isLooseBase() const{
    if( electronPtr->uncorrectedPt() < 7 ) return false;
    if( electronPtr->absEta() >= 2.5 ) return false;
    if( fabs( electronPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( electronPtr->dz() ) >= 0.1 ) return false;
    if( electronPtr->sip3d() >= 8 ) return false;
    if( electronPtr->numberOfMissingHits() >= 2 ) return false;
    if( electronPtr->miniIso() >= 0.4 ) return false;
    //if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
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
-------------------------------------------------------------------
FO electron selection for ttH ID
-------------------------------------------------------------------
*/

bool ElectronSelector::isFOBase() const{
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


bool ElectronSelector::isFO2016() const{
    if( electronPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2016() ) return false;
    return true;
}

bool ElectronSelector::isFO2017() const{
    if( electronPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2017() ) return false;
    return true;
}

bool ElectronSelector::isFO2018() const{
    if( electronPtr->closestJetDeepFlavor() >= bTagWP::mediumDeepFlavor2018() ) return false;
    return true;
}

/*
-------------------------------------------------------------------
FO electron selection for tZq ID
-------------------------------------------------------------------
*/

bool ElectronSelector::isFOBasetZq() const{
    // function to copy as closely as possible the lepton ID of the analysis note of tZq discovery
    if(!isLoose()) return false;
    if(electronPtr->uncorrectedPt()<10) return false;
    if(!electronPtr->passDoubleEGEmulation()) return false;
    if(electronMVAValue(electronPtr) < electronMVACut()){
	if(electronPtr->ptRatio()<0.6) return false;
    }
    return true;
}

bool ElectronSelector::isFO2016tZq() const{
    if(electronMVAValue(electronPtr) < electronMVACut()){
        if(electronJetBTagValue(electronPtr) > 0.3) return false;
        if(electronPtr->absEta()<1.479){ if(electronPtr->electronMVASummer16GP()<0.4) return false; }
        else{ if(electronPtr->electronMVASummer16GP()<0.7) return false; }
    }
    else{ if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2016")) return false; }
    return true;
}

bool ElectronSelector::isFO2017tZq() const{
    if(electronMVAValue(electronPtr) < electronMVACut()){
        if(electronJetBTagValue(electronPtr) > 0.2) return false;
        if(electronPtr->absEta()<1.479){ if(electronPtr->electronMVAFall17NoIso()<0.3) return false; }
        else{ if(electronPtr->electronMVAFall17NoIso()<0.6) return false; }
    }
    else{ if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2017")) return false; }
    return true;
}

bool ElectronSelector::isFO2018tZq() const{
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
tight electron selection for ttH ID
-------------------------------------------------------------------------
*/

bool ElectronSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( electronMVAValue(electronPtr) <= electronMVACut() ) return false;
    return true;
}


bool ElectronSelector::isTight2016() const{
    return true;
}


bool ElectronSelector::isTight2017() const{
    return true;
}


bool ElectronSelector::isTight2018() const{
    return true;
}

/*
----------------------------------------------------------------------
tight electron selection for tZq ID
----------------------------------------------------------------------
*/

bool ElectronSelector::isTightBasetZq() const{
    if(!isFOtZq()) return false;
    if(electronMVAValue(electronPtr) < electronMVACut()) return false;
    return true;
}

bool ElectronSelector::isTight2016tZq() const{
    if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2016")) return false;
    return true;
}

bool ElectronSelector::isTight2017tZq() const{
    if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2017")) return false;
    return true;
}

bool ElectronSelector::isTight2018tZq() const{
    if(electronJetBTagValue(electronPtr) > electronJetBTagCut("tight", "2018")) return false;
    return true;
}

/*
cone correction
*/

double ElectronSelector::coneCorrection() const{
    return ( 0.9 / electronPtr->ptRatio() );
}
