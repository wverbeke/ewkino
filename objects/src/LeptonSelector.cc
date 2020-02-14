#include "../interface/LeptonSelector.h"


bool LeptonSelector::isLoose() const{
    if( !isLooseBase() ) return false;

    if( is2016() ){
        return isLoose2016();
    } else if( is2017() ){
        return isLoose2017(); 
    } else {
        return isLoose2018();
    }
}


bool LeptonSelector::isFO() const{
    if( !isFOBase() ) return false;

    if( is2016() ){
        return isFO2016();
    } else if( is2017() ){
        return isFO2017();
    } else {
        return isFO2018();
    }
}


bool LeptonSelector::isTight() const{
    if( !isTightBase() ) return false;
    
    if( is2016() ){
        return isTight2016();
    } else if( is2017() ){
        return isTight2017();
    } else {
        return isTight2018();
    }
}

bool LeptonSelector::isFOtZq() const{
    if(!isFOBasetZq()) return false;
    if( is2016() ){
        return isFO2016tZq();
    } else if( is2017() ){
        return isFO2017tZq();
    } else return false; // no 2018 implementation!
}

bool LeptonSelector::isTighttZq() const{
    if(!isTightBasetZq()) return false;
    if( is2016() ){
        return isTight2016tZq();
    } else if( is2017() ){
        return isTight2017tZq();
    } else return false; // no 2018 implementation!
}
