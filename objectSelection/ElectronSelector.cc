#include "../objects/interface/ElectronSelector.h"


/*
loose electron selection
*/

bool ElectronSelector::isLooseBase() const{
    if( electronPtr->pt() < 10 ) return false;
    if( fabs( electronPtr->eta() ) >= 2.5 ) return false;
    if( ! electronPtr->passDoubleEGEmulation() ) return false;
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
    if( isLoose() ) return false;
    return true;
}


bool ElectronSelector::isFO2016() const{
    return true;
}


bool ElectronSelector::isFO2017() const{
    return true;
}


bool ElectronSelector::isFO2018() const{
    return true;
}


/*
tight electron selection
*/

bool ElectronSelector::isTightBase() const{
    if( isFO() ) return false;
    if( !electronPtr->isTightPOGElectron() ) return false;
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
    return 1;
}
