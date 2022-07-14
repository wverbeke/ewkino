#include "../objects/interface/TauSelector.h"

/*
loose tau selection
*/

bool TauSelector::isLooseBase_v1() const{
    if( tauPtr->pt() < 20 ) return false;
    if( fabs( tauPtr->eta() ) >= 2.3 ) return false;
    if( ! tauPtr->passDecayModeFinding() ) return false;
    if( ! tauPtr->passVLooseMVAOld2017() ) return false;
    return true;
}


bool TauSelector::isLoose2016_v1() const{
    return true;
}


bool TauSelector::isLoose2016PreVFP_v1() const{
    return true;
}


bool TauSelector::isLoose2016PostVFP_v1() const{
    return true;
}


bool TauSelector::isLoose2017_v1() const{
    return true;
}


bool TauSelector::isLoose2018_v1() const{
    return true;
}


bool TauSelector::isLooseBase_v2() const{
    if( tauPtr->pt() < 20 ) return false;
    if( fabs( tauPtr->eta() ) >= 2.3 ) return false;
    if( ! tauPtr->passDecayModeFinding() ) return false;
    if( ! tauPtr->passVLooseMVAOld2017() ) return false;
    return true;
}


bool TauSelector::isLoose2016_v2() const{
    return true;
}


bool TauSelector::isLoose2016PreVFP_v2() const{
    return true;
}


bool TauSelector::isLoose2016PostVFP_v2() const{
    return true;
}


bool TauSelector::isLoose2017_v2() const{
    return true;
}


bool TauSelector::isLoose2018_v2() const{
    return true;
}


/*
FO tau selection
*/

bool TauSelector::isFOBase_v1() const{
    if( !isLoose() ) return false;
    return true;
}


bool TauSelector::isFO2016_v1() const{
    return true;
}


bool TauSelector::isFO2016PreVFP_v1() const{
    return true;
}


bool TauSelector::isFO2016PostVFP_v1() const{
    return true;
}


bool TauSelector::isFO2017_v1() const{
    return true;
}


bool TauSelector::isFO2018_v1() const{
    return true;
}


bool TauSelector::isFOBase_v2() const{
    if( !isLoose() ) return false;
    return true;
}


bool TauSelector::isFO2016_v2() const{
    return true;
}


bool TauSelector::isFO2016PreVFP_v2() const{
    return true;
}


bool TauSelector::isFO2016PostVFP_v2() const{
    return true;
}


bool TauSelector::isFO2017_v2() const{
    return true;
}


bool TauSelector::isFO2018_v2() const{
    return true;
}

bool TauSelector::isFORunTime( double ptRatioCut, double deepFlavorCut, int extraCut ) const{
    if( !isLoose() ) return false;
    // dummy conditions to avoid compilation warnings
    if( ptRatioCut<0. && deepFlavorCut<0. && extraCut<-9999 ) return true;
    return true;
}


/*
tight tau selection
*/

bool TauSelector::isTightBase_v1() const{
    if( !isFO() ) return false;
    if( !tauPtr->passTightMVAOld2017() ) return false;
    return true;
}


bool TauSelector::isTight2016_v1() const{
    return true;
}


bool TauSelector::isTight2016PreVFP_v1() const{
    return true;
}


bool TauSelector::isTight2016PostVFP_v1() const{
    return true;
}


bool TauSelector::isTight2017_v1() const{
    return true;
}


bool TauSelector::isTight2018_v1() const{
    return true;
}


bool TauSelector::isTightBase_v2() const{
    if( !isFO() ) return false;
    if( !tauPtr->passTightMVAOld2017() ) return false;
    return true;
}


bool TauSelector::isTight2016_v2() const{
    return true;
}


bool TauSelector::isTight2016PreVFP_v2() const{
    return true;
}


bool TauSelector::isTight2016PostVFP_v2() const{
    return true;
}


bool TauSelector::isTight2017_v2() const{
    return true;
}


bool TauSelector::isTight2018_v2() const{
    return true;
}


/*
cone correction
*/


double TauSelector::coneCorrection() const{
    return 1;
}
