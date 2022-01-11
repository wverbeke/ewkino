#include "../objects/interface/ElectronSelector.h"

//include c++ library classes
#include <cmath>

//include other parts of framework
#include "bTagWP.h"


/*
loose electron selection
*/


double leptonMVACutElectron(){
    return 0.4;
}


bool ElectronSelector::isLooseBase() const{
    if( electronPtr->uncorrectedPt() < 7 ) return false;
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


bool ElectronSelector::isLoose2016PreVFP() const{
    return true;
}


bool ElectronSelector::isLoose2016PostVFP() const{
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
    if( electronPtr->hOverE() >= 0.1 ) return false;
    if( electronPtr->inverseEMinusInverseP() <= -0.04 ) return false;
    if( std::abs(electronPtr->etaSuperCluster()) <= 1.479 ){
        if( electronPtr->sigmaIEtaEta() >= 0.011 ) return false;
    } else {
        if( electronPtr->sigmaIEtaEta() >= 0.030 ) return false;
    }
    if( !electronPtr->passConversionVeto() ) return false;
    if( !electronPtr->passChargeConsistency() ) return false;
    return true;
}


bool ElectronSelector::isFO2016() const{
    if( electronPtr->leptonMVATOP() <= leptonMVACutElectron() ){
        if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2016PreVFP() const{
    if( electronPtr->leptonMVATOP() <= leptonMVACutElectron() ){
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false; 
    }
    return true;
}


bool ElectronSelector::isFO2016PostVFP() const{
    if( electronPtr->leptonMVATOP() <= leptonMVACutElectron() ){
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2017() const{
    if( electronPtr->leptonMVATOP() <= leptonMVACutElectron() ){
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2018() const{
    if( electronPtr->leptonMVATOP() <= leptonMVACutElectron() ){
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFORunTime( double ptRatioCut, double deepFlavorCut, int extraCut ) const{
    // function for FO optimization, use ONLY in MC fake rate grid search!
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
    if( !electronPtr->passChargeConsistency() ) return false;

    if( electronPtr->leptonMVATOP() <= leptonMVACutElectron() ){
	if( extraCut==1 && !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
	if( extraCut==2 && !electronPtr->passElectronMVAFall17NoIsoWP90() ) return false;
	if( extraCut==3 && !electronPtr->passElectronMVAFall17NoIsoWP80() ) return false;
        if( electronPtr->ptRatio() <= ptRatioCut ) return false;
	if( electronPtr->closestJetDeepFlavor() >= deepFlavorCut ) return false;
    }

    return true;   
}


/*
tight electron selection
*/

bool ElectronSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( electronPtr->leptonMVATOP() <= leptonMVACutElectron() ) return false;
    return true;
}


bool ElectronSelector::isTight2016() const{
    return true;
}


bool ElectronSelector::isTight2016PostVFP() const{
    return true;
}


bool ElectronSelector::isTight2017() const{
    return true;
}


bool ElectronSelector::isTight2018() const{
    return true;
}


/*
cone correction
*/

double ElectronSelector::coneCorrection() const{
    return ( 0.67 / electronPtr->ptRatio() );
}
