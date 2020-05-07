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


bool TauSelector::isLoose2016() const{
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
    if( !tauPtr->passMediumMVAOld2017() ) return false;
    return true;
}


bool TauSelector::isFO2016() const{
    return true;
}


bool TauSelector::isFO2017() const{
    return true;
}


bool TauSelector::isFO2018() const{
    return true;
}

bool TauSelector::isFOBasetZq() const{
    // to avoid compilation errors this function must be defined here as well
    return false; // since taus are not considered in the analysis
}
bool TauSelector::isFO2016tZq() const{return false;}
bool TauSelector::isFO2017tZq() const{return false;}
bool TauSelector::isFO2018tZq() const{return false;}

/*
tight tau selection
*/

bool TauSelector::isTightBase() const{
    if( !isFO() ) return false;
    if( !tauPtr->passVTightMVAOld2017() ) return false;
    return true;
}


bool TauSelector::isTight2016() const{
    return true;
}


bool TauSelector::isTight2017() const{
    return true;
}


bool TauSelector::isTight2018() const{
    return true;
}

bool TauSelector::isTightBasetZq() const{
    // to avoid compilation errors this function must be defined here as well
    return false; // since taus are not considered in the analysis
}
bool TauSelector::isTight2016tZq() const{return false;}
bool TauSelector::isTight2017tZq() const{return false;}
bool TauSelector::isTight2018tZq() const{return false;}

/*
cone correction
*/


double TauSelector::coneCorrection() const{
    return 1;
}
