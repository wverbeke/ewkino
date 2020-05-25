#include "../objects/interface/LeptonSelector.h"

// define here what lepton ID to use throughout the whole framework!
std::string leptonID(){
    return "tzq"; // choose from "tzq" or "tth"
}

bool LeptonSelector::isLoose() const{
    // common to tzq and tth ID!
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
    bool isFO = (leptonID()=="tth") ? isFOttH() :
                    (leptonID()=="tzq") ? isFOtZq() :
                    false;
    return isFO;
}

bool LeptonSelector::isTight() const{
    bool isTight = (leptonID()=="tth") ? isTightttH() :
		    (leptonID()=="tzq") ? isTighttZq() :
		    false;
    return isTight;
}

bool LeptonSelector::isFOttH() const{
    if( !isFOBase() ) return false;

    if( is2016() ){
        return isFO2016();
    } else if( is2017() ){
        return isFO2017();
    } else {
        return isFO2018();
    }
}

bool LeptonSelector::isTightttH() const{
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
    } else return isFO2018tZq();
}

bool LeptonSelector::isTighttZq() const{
    if(!isTightBasetZq()) return false;
    if( is2016() ){
        return isTight2016tZq();
    } else if( is2017() ){
        return isTight2017tZq();
    } else return isTight2018tZq();
}
