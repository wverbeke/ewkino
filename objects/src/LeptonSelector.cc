#include "../interface/LeptonSelector.h"

// define lepton ID
std::string LeptonSelector::leptonID(){
   return "v2";
}


bool LeptonSelector::isLoose() const{
    if( leptonID()=="v1" ) return isLoose_v1();
    else if( leptonID()=="v2" ) return isLoose_v2();
    else if( leptonID()=="v1||v2" ) return (isLoose_v1() || isLoose_v2());
    else{
	std::string msg = "ERROR: undefined lepton ID in LeptonSelector::isLoose():";
	msg.append( " "+leptonID() );
	throw std::runtime_error(msg);
    }
}


bool LeptonSelector::isFO() const{
    if( leptonID()=="v1" ) return isFO_v1();
    else if( leptonID()=="v2" ) return isFO_v2();
    else{ 
        std::string msg = "ERROR: undefined lepton ID in LeptonSelector::isFO():";
        msg.append( " "+leptonID() ); 
        throw std::runtime_error(msg);
    }
}


bool LeptonSelector::isTight() const{
    if( leptonID()=="v1" ) return isTight_v1();
    else if( leptonID()=="v2" ) return isTight_v2();
    else{ 
        std::string msg = "ERROR: undefined lepton ID in LeptonSelector::isTight():";
        msg.append( " "+leptonID() ); 
        throw std::runtime_error(msg);
    }
}


bool LeptonSelector::isLoose_v1() const{
    if( !isLooseBase_v1() ) return false;
    if( is2016PreVFP() ){ return isLoose2016PreVFP_v1(); }
    else if( is2016PostVFP() ){ return isLoose2016PostVFP_v1(); }
    else if( is2016() ){ return isLoose2016_v1(); } 
    else if( is2017() ){ return isLoose2017_v1(); } 
    else if( is2018() ){ return isLoose2018_v1(); }
    return false;
}


bool LeptonSelector::isLoose_v2() const{
    if( !isLooseBase_v2() ) return false;
    if( is2016PreVFP() ){ return isLoose2016PreVFP_v2(); }
    else if( is2016PostVFP() ){ return isLoose2016PostVFP_v2(); }
    else if( is2016() ){ return isLoose2016_v2(); }
    else if( is2017() ){ return isLoose2017_v2(); }
    else if( is2018() ){ return isLoose2018_v2(); }
    return false;
}


bool LeptonSelector::isFO_v1() const{
    if( !isFOBase_v1() ) return false;
    if( is2016PreVFP() ){ return isFO2016PreVFP_v1(); }
    else if( is2016PostVFP() ){ return isFO2016PostVFP_v1(); }
    else if( is2016() ){ return isFO2016_v1(); } 
    else if( is2017() ){ return isFO2017_v1(); } 
    else if( is2018() ){ return isFO2018_v1(); }
    return false;
}


bool LeptonSelector::isFO_v2() const{
    if( !isFOBase_v2() ) return false;
    if( is2016PreVFP() ){ return isFO2016PreVFP_v2(); }
    else if( is2016PostVFP() ){ return isFO2016PostVFP_v2(); }
    else if( is2016() ){ return isFO2016_v2(); }
    else if( is2017() ){ return isFO2017_v2(); }
    else if( is2018() ){ return isFO2018_v2(); }
    return false;
}


bool LeptonSelector::isTight_v1() const{
    if( !isTightBase_v1() ) return false;
    if( is2016PreVFP() ){ return isTight2016PreVFP_v1(); }
    else if( is2016PostVFP() ){ return isTight2016PostVFP_v1(); }
    else if( is2016() ){ return isTight2016_v1(); } 
    else if( is2017() ){ return isTight2017_v1(); } 
    else if( is2018() ){ return isTight2018_v1(); }
    return false;
}


bool LeptonSelector::isTight_v2() const{
    if( !isTightBase_v2() ) return false;
    if( is2016PreVFP() ){ return isTight2016PreVFP_v2(); }
    else if( is2016PostVFP() ){ return isTight2016PostVFP_v2(); }
    else if( is2016() ){ return isTight2016_v2(); }
    else if( is2017() ){ return isTight2017_v2(); }
    else if( is2018() ){ return isTight2018_v2(); }
    return false;
}
