#include "../interface/TauSelector.h"

//include other parts of framework
#include "../interface/Tau.h"


bool TauSelector::isLoose() const{
    return false;
}


bool TauSelector::isFO() const{
    if( !isLoose() ) return false;
    return true;
}


bool TauSelector::isGood() const{
    return isFO();
}


bool TauSelector::isTight() const{
    if( !isFO() ) return false;
    return true;
}
