#include "../interface/ElectronSelector.h"


//include other parts of framework
#include "../interface/Electron.h"


bool ElectronSelector::isLoose() const{
    if( electronPtr->pt() < 10 ) return false;
    if( fabs( electronPtr->eta() ) >= 2.5 ) return false;
    if( ! electronPtr->passDoubleEGEmulation() ) return false;
    return true;
}


bool ElectronSelector::isFO() const{
    if( isLoose() ) return false;
    return true;
}


bool ElectronSelector::isGood() const{
    return isFO();
}


bool ElectronSelector::isTight() const{
    if( isTight() ) return false;
    return true;
}
