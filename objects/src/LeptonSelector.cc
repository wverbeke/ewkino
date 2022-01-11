#include "../interface/LeptonSelector.h"


bool LeptonSelector::isLoose() const{
    if( !isLooseBase() ) return false;

    if( is2016PreVFP() ){ return isLoose2016PreVFP(); }
    else if( is2016PostVFP() ){ return isLoose2016PostVFP(); }
    else if( is2016() ){ return isLoose2016(); } 
    else if( is2017() ){ return isLoose2017(); } 
    else if( is2018() ){ return isLoose2018(); }
    return false;
}


bool LeptonSelector::isFO() const{
    if( !isFOBase() ) return false;

    if( is2016PreVFP() ){ return isFO2016PreVFP(); }
    else if( is2016PostVFP() ){ return isFO2016PostVFP(); }
    else if( is2016() ){ return isFO2016(); } 
    else if( is2017() ){ return isFO2017(); } 
    else if( is2018() ){ return isFO2018(); }
    return false;
}


bool LeptonSelector::isTight() const{
    if( !isTightBase() ) return false;
    
    if( is2016PreVFP() ){ return isTight2016PreVFP(); }
    else if( is2016PostVFP() ){ return isTight2016PostVFP(); }
    else if( is2016() ){ return isTight2016(); } 
    else if( is2017() ){ return isTight2017(); } 
    else if( is2018() ){ return isTight2018(); }
    return false;
}
