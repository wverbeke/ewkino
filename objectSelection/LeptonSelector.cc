#include "../objects/interface/LeptonSelector.h"

// define here what lepton ID to use throughout the whole framework!
std::string leptonID(){
    return "tzq"; // choose from "tzq", "tth" or "oldtzq"
}

bool LeptonSelector::isLoose() const{
    // common to all ID's!
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
    bool isFO = (leptonID()=="tzq") ? isFOtZq() :
		    (leptonID()=="tth") ? isFOttH() :
                    (leptonID()=="oldtzq") ? isFOOldtZq() :
                    false;
    return isFO;
}

bool LeptonSelector::isTight() const{
    bool isTight = (leptonID()=="tzq") ? isTighttZq() :
		    (leptonID()=="tth") ? isTightttH() :
		    (leptonID()=="oldtzq") ? isTightOldtZq() :
		    false;
    return isTight;
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

bool LeptonSelector::isFOttH() const{
    if( !isFOBasettH() ) return false;

    if( is2016() ){
        return isFO2016ttH();
    } else if( is2017() ){
        return isFO2017ttH();
    } else {
        return isFO2018ttH();
    }
}

bool LeptonSelector::isTightttH() const{
    if( !isTightBasettH() ) return false;
    
    if( is2016() ){
        return isTight2016ttH();
    } else if( is2017() ){
        return isTight2017ttH();
    } else {
        return isTight2018ttH();
    }
}

bool LeptonSelector::isFOOldtZq() const{
    if(!isFOBaseOldtZq()) return false;
    if( is2016() ){
        return isFO2016OldtZq();
    } else if( is2017() ){
        return isFO2017OldtZq();
    } else return isFO2018OldtZq();
}

bool LeptonSelector::isTightOldtZq() const{
    if(!isTightBaseOldtZq()) return false;
    if( is2016() ){
        return isTight2016OldtZq();
    } else if( is2017() ){
        return isTight2017OldtZq();
    } else return isTight2018OldtZq();
}
