#include "../objects/interface/ElectronSelector.h"

//include c++ library classes
#include <cmath>

//include other parts of framework
#include "bTagWP.h"


/*
loose electron selection
*/


double leptonMVACutElectron(){
    return 0.8;
}


bool ElectronSelector::isLooseBase() const{
    if( electronPtr->uncorrectedPt() < 7 ) return false;
    if( electronPtr->absEta() >= 2.5 ) return false;
    if( fabs( electronPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( electronPtr->dz() ) >= 0.1 ) return false;
    if( electronPtr->sip3d() >= 8 ) return false;
    if( electronPtr->numberOfMissingHits() >= 2 ) return false;
    if( electronPtr->miniIso() >= 0.4 ) return false;
    if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
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
FO electron selection
*/

bool ElectronSelector::isFOBase() const{
    if( !isLoose() ) return false;
    if( electronPtr->uncorrectedPt() <= 10 ) return false;
    if( electronPtr->numberOfMissingHits() > 0 ) return false;
    if( electronPtr->hOverE() >= 0.1 ) return false;
    if( electronPtr->inverseEMinusInverseP() <= -0.04 ) return false;
    if( electronPtr->etaSuperCluster() <= 1.479 ){
        if( electronPtr->sigmaIEtaEta() >= 0.011 ) return false;
    } else {
        if( electronPtr->sigmaIEtaEta() >= 0.030 ) return false;
    }
    if( electronPtr->leptonMVAttH() <= leptonMVACutElectron() ){
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

bool ElectronSelector::isFOBasetZq() const{
    // function to copy as closely as possible the lepton ID of the analysis note of tZq discovery
    if(electronPtr->absEta()>2.5) return false;
    if(electronPtr->uncorrectedPt()<10) return false;
    if(electronPtr->dxy()>0.05) return false;
    if(electronPtr->dz()>0.1) return false;
    if(electronPtr->sip3d()>8) return false;
    if(electronPtr->miniIso()>0.4) return false;
    if(electronPtr->numberOfMissingHits()>1) return false;
    if(!electronPtr->passDoubleEGEmulation()) return false;
    return true;
}

bool ElectronSelector::isFO2016tZq() const{
    if(electronPtr->leptonMVAtZq()<0.8){
        if(electronPtr->closestJetDeepCSV()>0.3) return false;
        if(electronPtr->ptRatio()<0.6) return false;
        if(electronPtr->absEta()<1.479){
            if(electronPtr->electronMVASummer16GP()<0.4) return false;
        }
        else{
            if(electronPtr->electronMVASummer16GP()<0.7) return false;
        }
    }
    else{
        if(electronPtr->closestJetDeepCSV()>0.8958) return false;
    }
    return true;
}

bool ElectronSelector::isFO2017tZq() const{
    if(electronPtr->leptonMVAtZq()<0.8){
        if(electronPtr->closestJetDeepCSV()>0.2) return false;
        if(electronPtr->ptRatio()<0.6) return false;
        if(electronPtr->absEta()<1.479){
            if(electronPtr->electronMVASummer16GP()<0.3) return false;
        }
        else{
            if(electronPtr->electronMVASummer16GP()<0.6) return false;
        }
    }
    else{
        if(electronPtr->closestJetDeepCSV()>0.8001) return false;
    }
    return true;
}


/*
tight electron selection
*/

bool ElectronSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( electronPtr->leptonMVAttH() <= leptonMVACutElectron() ) return false;
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

bool ElectronSelector::isTightBasetZq() const{
    if(!isFOtZq()) return false;
    if(electronPtr->leptonMVAtZq()<0.8) return false;
    return true;
}

bool ElectronSelector::isTight2016tZq() const{
    if(electronPtr->closestJetDeepCSV()>0.8958) return false;
    return true;
}

bool ElectronSelector::isTight2017tZq() const{
    if(electronPtr->closestJetDeepCSV()>0.8001) return false;
    return true;
}

/*
cone correction
*/

double ElectronSelector::coneCorrection() const{
    return ( 0.9 / electronPtr->ptRatio() );
}
