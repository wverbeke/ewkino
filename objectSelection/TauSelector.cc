#include "../objects/interface/TauSelector.h"

/*
loose tau selection
*/

bool TauSelector::isLooseBase() const{
    if( tauPtr->pt() < 20 ) return false;
    if( fabs( tauPtr->eta() ) >= 2.3 ) return false;
    if( ! tauPtr->passDecayModeFinding() ) return false;
    if( ! tauPtr->passVLooseMVAOld2017() ) return false;
    return true;
}


bool TauSelector::isLoose2016PreVFP() const{
    return true;
}


bool TauSelector::isLoose2016PostVFP() const{
    return true;
}


bool TauSelector::isLoose2017() const{
    return true;
}


bool TauSelector::isLoose2018() const{
    return true;
}


/*
FO tau selection
*/

bool TauSelector::isFOBase() const{
    if( !isLoose() ) return false;
    return true;
}


bool TauSelector::isFO2016PreVFP() const{
    return true;
}


bool TauSelector::isFO2016PostVFP() const{
    return true;
}


bool TauSelector::isFO2017() const{
    return true;
}


bool TauSelector::isFO2018() const{
    return true;
}


/*
tight tau selection
*/

bool TauSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( !tauPtr->passTightMVAOld2017() ) return false;
    return true;
}


bool TauSelector::isTight2016PreVFP() const{
    return true;
}


bool TauSelector::isTight2016PostVFP() const{
    return true;
}


bool TauSelector::isTight2017() const{
    return true;
}


bool TauSelector::isTight2018() const{
    return true;
}


/*
cone correction
*/


double TauSelector::coneCorrection() const{
    return 1;
}
