#include "../objects/interface/ElectronSelector.h"

//include c++ library classes
#include <cmath>

//include other parts of framework
#include "bTagWP.h"


// define the MVA threshold value depending on the lepton ID
double leptonMVACutElectron(){
    // see AN_2022_016!
    // the working point here corresponds to Medium for both v1 and v2!
    if(LeptonSelector::leptonID()=="v1") return 0.64;
    else if(LeptonSelector::leptonID()=="v2") return 0.90;
    else return 1.0;
}


// define the MVA value depending on the lepton ID
double leptonMVAValueElectron(const Electron* electronPtr){
    if(LeptonSelector::leptonID()=="v1") return electronPtr->leptonMVATOPUL();
    else if(LeptonSelector::leptonID()=="v2") return electronPtr->leptonMVATOPv2UL();
    else return 0.0;
}


// define cone correction factor
double leptonConeCorrectionFactorElectron(){
    // cone correction factor to be determined!
    if(LeptonSelector::leptonID()=="v1") return 0.0;
    else if(LeptonSelector::leptonID()=="v2") return 0.0;
    else return 0.0;
}


/*
Loose electron selections
*/

// loose definition for leptonMVA UL v1
bool ElectronSelector::isLooseBase_v1() const{
    if( electronPtr->uncorrectedPt() < 7 ) return false;
    if( electronPtr->absEta() >= 2.5 ) return false;
    if( fabs( electronPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( electronPtr->dz() ) >= 0.1 ) return false;
    if( electronPtr->sip3d() >= 8 ) return false;
    if( electronPtr->numberOfMissingHits() >= 2 ) return false;
    if( electronPtr->miniIso() >= 0.4 ) return false;
    // addition: remove transition region
    if( std::abs(electronPtr->etaSuperCluster()) > 1.4442 
	&& std::abs(electronPtr->etaSuperCluster()) < 1.566) return false;
    return true;
}


bool ElectronSelector::isLoose2016_v1() const{
    return true;
}


bool ElectronSelector::isLoose2016PreVFP_v1() const{
    return true;
}


bool ElectronSelector::isLoose2016PostVFP_v1() const{
    return true;
}


bool ElectronSelector::isLoose2017_v1() const{
    return true;
}


bool ElectronSelector::isLoose2018_v1() const{
    return true;
}



// loose definition for leptonMVA UL v2
bool ElectronSelector::isLooseBase_v2() const{
    if( electronPtr->uncorrectedPt() < 7 ) return false;
    if( electronPtr->absEta() >= 2.5 ) return false;
    if( electronPtr->sip3d() >= 15 ) return false;
    if( electronPtr->relIso0p4() >= 1.0 ) return false;
    // addition: remove transition region
    if( std::abs(electronPtr->etaSuperCluster()) > 1.4442 
        && std::abs(electronPtr->etaSuperCluster()) < 1.566) return false;
    return true;
}


bool ElectronSelector::isLoose2016_v2() const{
    return true;
}


bool ElectronSelector::isLoose2016PreVFP_v2() const{
    return true;
}


bool ElectronSelector::isLoose2016PostVFP_v2() const{
    return true;
}


bool ElectronSelector::isLoose2017_v2() const{
    return true;
}


bool ElectronSelector::isLoose2018_v2() const{
    return true;
}


/*
FO electron selections
*/

// FO definition for leptonMVA UL v1
bool ElectronSelector::isFOBase_v1() const{
    if( !isLoose() ) return false;
    if( electronPtr->uncorrectedPt() <= 10 ) return false;
    // following cuts are needed (after discussing with Niels)  
    if( !electronPtr->passConversionVeto() ) return false;
    // disable ad-hoc cuts below
    // maybe enable and update to current POG values if needed for good electron closure
    /*if( electronPtr->hOverE() >= 0.1 ) return false;
    if( electronPtr->inverseEMinusInverseP() <= -0.04 ) return false;
    if( std::abs(electronPtr->etaSuperCluster()) <= 1.479 ){
        if( electronPtr->sigmaIEtaEta() >= 0.011 ) return false;
    } else {
        if( electronPtr->sigmaIEtaEta() >= 0.030 ) return false;
    }
    if( !electronPtr->passChargeConsistency() ) return false;*/
    return true;
}


bool ElectronSelector::isFO2016_v1() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// initial guess based on previous iteration!
        if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2016PreVFP_v1() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// initial guess based on previous iteration!
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false; 
    }
    return true;
}


bool ElectronSelector::isFO2016PostVFP_v1() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// initial guess based on previous iteration!
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2017_v1() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// initial guess based on previous iteration!
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


bool ElectronSelector::isFO2018_v1() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// initial guess based on previous iteration!
	if( electronPtr->closestJetDeepFlavor() > 0.5 ) return false;
        if( electronPtr->ptRatio() < 0.5 ) return false;
        if( !electronPtr->passElectronMVAFall17NoIsoLoose() ) return false;
    }
    return true;
}


// FO definition for leptonMVA UL v2
bool ElectronSelector::isFOBase_v2() const{
    if( !isLoose() ) return false;
    if( electronPtr->uncorrectedPt() <= 10 ) return false;
    // following cuts are needed (after discussing with Niels)
    if( electronPtr->numberOfMissingHits() >= 2 ) return false;
    if( !electronPtr->passConversionVeto() ) return false;
    // disable ad-hoc cuts below
    // maybe enable and update to current POG values if needed for good electron closure
    /*if( electronPtr->hOverE() >= 0.1 ) return false;
    if( electronPtr->inverseEMinusInverseP() <= -0.04 ) return false;
    if( std::abs(electronPtr->etaSuperCluster()) <= 1.479 ){
        if( electronPtr->sigmaIEtaEta() >= 0.011 ) return false;
    } else {
        if( electronPtr->sigmaIEtaEta() >= 0.030 ) return false;
    }
    if( !electronPtr->passChargeConsistency() ) return false;*/
    return true;
}


bool ElectronSelector::isFO2016_v2() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// to do
    }
    return true;
}


bool ElectronSelector::isFO2016PreVFP_v2() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// to do
    }
    return true;
}


bool ElectronSelector::isFO2016PostVFP_v2() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// to do
    }
    return true;
}


bool ElectronSelector::isFO2017_v2() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// to do
    }
    return true;
}


bool ElectronSelector::isFO2018_v2() const{
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
	// to do
    }
    return true;
}


// FO definition for FO optimization in grid search
bool ElectronSelector::isFORunTime( double ptRatioCut, double deepFlavorCut, int extraCut ) const{
    // function for FO optimization, use ONLY in MC fake rate grid search!
    if( !isLoose() ) return false;
    if( electronPtr->uncorrectedPt() <= 10 ) return false;
    // following cuts are needed (after discussing with Niels)
    if( electronPtr->numberOfMissingHits() >= 2 ) return false;
    if( !electronPtr->passConversionVeto() ) return false;    
    // disable ad-hoc cuts below
    // maybe enable and update to current POG values if needed for good electron closure
    /*if( electronPtr->hOverE() >= 0.1 ) return false;
    if( electronPtr->inverseEMinusInverseP() <= -0.04 ) return false;
    if( std::abs(electronPtr->etaSuperCluster()) <= 1.479 ){
        if( electronPtr->sigmaIEtaEta() >= 0.011 ) return false;
    } else {
        if( electronPtr->sigmaIEtaEta() >= 0.030 ) return false;
    }
    if( !electronPtr->passConversionVeto() ) return false;
    if( !electronPtr->passChargeConsistency() ) return false;*/

    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ){
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

// tight definition for leptonMVA UL v1
bool ElectronSelector::isTightBase_v1() const{
    if( !isFO() ) return false;
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ) return false;
    return true;
}


bool ElectronSelector::isTight2016_v1() const{
    return true;
}


bool ElectronSelector::isTight2016PreVFP_v1() const{
    return true;
}


bool ElectronSelector::isTight2016PostVFP_v1() const{
    return true;
}


bool ElectronSelector::isTight2017_v1() const{
    return true;
}


bool ElectronSelector::isTight2018_v1() const{
    return true;
}


// tight definition for leptonMVA UL v2
bool ElectronSelector::isTightBase_v2() const{
    if( !isFO() ) return false;
    if( leptonMVAValueElectron(electronPtr) <= leptonMVACutElectron() ) return false;
    return true;
}


bool ElectronSelector::isTight2016_v2() const{
    return true;
}


bool ElectronSelector::isTight2016PreVFP_v2() const{
    return true;
}


bool ElectronSelector::isTight2016PostVFP_v2() const{
    return true;
}


bool ElectronSelector::isTight2017_v2() const{
    return true;
}


bool ElectronSelector::isTight2018_v2() const{
    return true;
}


/*
cone correction
*/

double ElectronSelector::coneCorrection() const{
    return ( leptonConeCorrectionFactorElectron() / electronPtr->ptRatio() );
}
