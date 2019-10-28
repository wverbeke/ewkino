#include "../objects/interface/ElectronSelector.h"

//include c++ library classes
#include <cmath>

//include other parts of framework
#include "bTagWP.h"


/*
loose electron selection
*/


double rawElectronMVA( const double electronMVA ){
	return -0.5 * ( std::log( ( 1. - electronMVA ) / ( 1. + electronMVA ) ) );
}


unsigned electronMVACategory( const Electron& electron ){
    double absEtaSC = fabs( electron.etaSuperCluster() );
    if( electron.uncorrectedPt() < 10 ){
        if( absEtaSC < 0.8 ) return 0;
        else if( absEtaSC < 1.479 ) return 1;
        else return 2;
    } else {
        if( absEtaSC < 0.8 ) return 3;
        else if( absEtaSC < 1.479 ) return 4;
        else return 5;
    }
}


double looseMVACut( const Electron& electron ){
	unsigned category = electronMVACategory( electron );
	if( category == 0 ) return 0.700642584415;
    else if( category == 1 ) return 0.739335420875;
    else if( category == 2 ) return 1.45390456109;
    else if( category == 3 ) return -0.146270871164;
    else if( category == 4 ) return -0.0315850882679;
    else return -0.0321841194737; 
}


double WP80MVACut( const Electron& electron ){
	unsigned category = electronMVACategory( electron );
	if( category == 0 ) return 3.53495358797 - std::exp( -electron.uncorrectedPt() / 3.07272325141 ) * 9.94262764352;
	else if( category == 1 ) return 3.06015605623 - std::exp( -electron.uncorrectedPt() / 1.95572234114 ) * 14.3091184421;
	else if( category == 2 ) return 3.02052519639 - std::exp( -electron.uncorrectedPt() / 1.59784164742 ) * 28.719380105;
	else if( category == 3 ) return 7.35752275071 - std::exp( -electron.uncorrectedPt() / 15.87907864 ) * 7.61288809226;
	else if( category == 4 ) return 6.41811074032 - std::exp( -electron.uncorrectedPt() / 14.730562874 ) * 6.96387331587;
	else return 5.64936312428 - std::exp( -electron.uncorrectedPt() / 16.3664949747 ) * 7.19607610311;
}


double WP90MVACut( const Electron& electron ){
    unsigned category = electronMVACategory( electron );
    if( category == 0 ) return 2.84704783417 - std::exp( -electron.uncorrectedPt() / 3.32529515837 ) * 9.38050947827;
    else if( category == 1 ) return 2.03833922005 - std::exp( -electron.uncorrectedPt() / 1.93288758682 ) * 15.364588247;
    else if( category == 2 ) return 1.82704158461 - std::exp( -electron.uncorrectedPt() / 1.89796754399 ) * 19.1236071158;
    else if( category == 3 ) return 6.12931925263 - std::exp( -electron.uncorrectedPt() / 13.281753835 ) * 8.71138432196;
    else if( category == 4 ) return 5.26289004857 - std::exp( -electron.uncorrectedPt() / 13.2154971491 ) * 8.0997882835;
    else return 4.37338792902 - std::exp( -electron.uncorrectedPt() / 14.0776094696 ) * 8.48513324496;
}


bool passMVALoose( const Electron& electron ){
	double rawMVA = rawElectronMVA( electron.electronMVAFall17NoIso() );
    return rawMVA > looseMVACut( electron );
}


bool passMVAWP80( const Electron& electron ){
	double rawMVA = rawElectronMVA( electron.electronMVAFall17NoIso() );
    return rawMVA > WP80MVACut( electron );
}


bool passMVAWP90( const Electron& electron ){
    double rawMVA = rawElectronMVA( electron.electronMVAFall17NoIso() );
    return rawMVA > WP90MVACut( electron );
}


double leptonMVACutElectron(){
    return 0.8;
}


bool ElectronSelector::isLooseBase() const{
    if( electronPtr->uncorrectedPt() < 7 ) return false;
    if( fabs( electronPtr->eta() ) >= 2.5 ) return false;
    if( fabs( electronPtr->dxy() ) >= 0.05 ) return false;
    if( fabs( electronPtr->dz() ) >= 0.1 ) return false;
    if( electronPtr->sip3d() >= 8 ) return false;
    if( electronPtr->numberOfMissingHits() >= 2 ) return false;
    if( electronPtr->miniIso() >= 0.4 ) return false;
    if( !passMVALoose( *electronPtr ) ) return false;
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
    if( electronPtr->pt() <= 10 ) return false;
    if( electronPtr->numberOfMissingHits() > 0 ) return false;
    if( !electronPtr->passDoubleEGEmulation() ) return false;
    if( electronPtr->leptonMVAttH() <= leptonMVACutElectron() ){
        if( !passMVAWP80( *electronPtr ) ) return false;
    } else {
        if( !passMVALoose( *electronPtr ) ) return false;
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


/*
cone correction
*/

double ElectronSelector::coneCorrection() const{
    return ( 0.9 / electronPtr->ptRatio() );
}
