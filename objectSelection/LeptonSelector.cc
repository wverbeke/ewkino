#include "../objects/interface/LeptonSelector.h"

// define here what lepton ID to use throughout the whole framework!
std::string LeptonSelector::leptonID(){
    return "tzqmedium0p4"; 
    // choose from "tzqtight", "tzqmedium0p4", "tzqloose", "tth" or "oldtzq"
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
    bool isFO = (leptonID()=="tzqtight") ? isFOtZq() :
		    (leptonID()=="tzqmedium0p4") ? isFOtZqMedium0p4() :
		    (leptonID()=="tzqloose") ? isFOtZqLoose() :
		    (leptonID()=="tth") ? isFOttH() :
                    (leptonID()=="oldtzq") ? isFOOldtZq() :
                    false;
    return isFO;
}

bool LeptonSelector::isTight() const{
    bool isTight = (leptonID()=="tzqtight") ? isTighttZq() :
		    (leptonID()=="tzqmedium0p4") ? isTighttZqMedium0p4() :
		    (leptonID()=="tzqloose") ? isTighttZqLoose() :
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

bool LeptonSelector::isFOtZqMedium0p4() const{
    if(!isFOBasetZqMedium0p4()) return false;
    if( is2016() ){
        return isFO2016tZqMedium0p4();
    } else if( is2017() ){
        return isFO2017tZqMedium0p4();
    } else return isFO2018tZqMedium0p4();
}

bool LeptonSelector::isFOtZqLoose() const{
    if(!isFOBasetZqLoose()) return false;
    if( is2016() ){
	return isFO2016tZqLoose();
    } else if( is2017() ){
	return isFO2017tZqLoose();
    } else return isFO2018tZqLoose();
}

bool LeptonSelector::isTighttZq() const{
    if(!isTightBasetZq()) return false;
    if( is2016() ){
        return isTight2016tZq();
    } else if( is2017() ){
        return isTight2017tZq();
    } else return isTight2018tZq();
}

bool LeptonSelector::isTighttZqMedium0p4() const{
    if(!isTightBasetZqMedium0p4()) return false;
    if( is2016() ){
        return isTight2016tZqMedium0p4();
    } else if( is2017() ){
        return isTight2017tZqMedium0p4();
    } else return isTight2018tZqMedium0p4();
}

bool LeptonSelector::isTighttZqLoose() const{
    if(!isTightBasetZqLoose()) return false;
    if( is2016() ){
        return isTight2016tZqLoose();
    } else if( is2017() ){
        return isTight2017tZqLoose();
    } else return isTight2018tZqLoose();
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
