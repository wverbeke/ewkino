#include "../objects/interface/ElectronSelector.h"

//include c++ library classes
#include <cmath>

//include other parts of framework
#include "bTagWP.h"

double ttHleptonMVACutElectron(){
    return 0.8;
}

double tZqleptonMVACutElectron(){
    return 0.;
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
    if( electronPtr->leptonMVAttH() <= ttHleptonMVACutElectron() ){
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
    if(electronPtr->leptonMVAtZq() < tZqleptonMVACutElectron()){
	if(electronPtr->ptRatio()<0.6) return false;
    }
    return true;
}

bool ElectronSelector::isFO2016tZq() const{
    if(electronPtr->leptonMVAtZq() < tZqleptonMVACutElectron()){
        if(electronPtr->closestJetDeepCSV()>0.3) return false;
        if(electronPtr->absEta()<1.479){ if(electronPtr->electronMVASummer16GP()<0.4) return false; }
        else{ if(electronPtr->electronMVASummer16GP()<0.7) return false; }
    }
    else{ if(electronPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2016()) return false; }
    return true;
}

bool ElectronSelector::isFO2017tZq() const{
    if(electronPtr->leptonMVAtZq() < tZqleptonMVACutElectron()){
        if(electronPtr->closestJetDeepCSV()>0.2) return false;
        if(electronPtr->absEta()<1.479){ if(electronPtr->electronMVAFall17NoIso()<0.3) return false; }
        else{ if(electronPtr->electronMVAFall17NoIso()<0.6) return false; }
    }
    else{ if(electronPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2017()) return false; }
    return true;
}

bool ElectronSelector::isFO2018tZq() const{
    if(electronPtr->leptonMVAtZq() < tZqleptonMVACutElectron()){
        if(electronPtr->closestJetDeepCSV()>0.2) return false;
        if(electronPtr->absEta()<1.479){ if(electronPtr->electronMVAFall17NoIso()<0.3) return false; }
        else{ if(electronPtr->electronMVAFall17NoIso()<0.6) return false; }
    }
    else{ if(electronPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2018()) return false; }
    return true;
}


/*
-------------------------------------------------------------------------
tight electron selection for ttH ID
-------------------------------------------------------------------------
*/

bool ElectronSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( electronPtr->leptonMVAttH() <= ttHleptonMVACutElectron() ) return false;
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
    if(electronPtr->leptonMVAtZq() < tZqleptonMVACutElectron()) return false;
    return true;
}

bool ElectronSelector::isTight2016tZq() const{
    if(electronPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2016()) return false;
    return true;
}

bool ElectronSelector::isTight2017tZq() const{
    if(electronPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2017()) return false;
    return true;
}

bool ElectronSelector::isTight2018tZq() const{
    if(electronPtr->closestJetDeepCSV() > bTagWP::tightDeepCSV2018()) return false;
    return true;
}

/*
cone correction
*/

double ElectronSelector::coneCorrection() const{
    return ( 0.9 / electronPtr->ptRatio() );
}
