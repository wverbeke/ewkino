#include "../objects/interface/JetSelector.h"

//b-tagging working points
#include "bTagWP.h"

// define here what b-tagger threshold to use
double jetBTagCut(const std::string wp, const std::string year){
    //return bTagWP::getWP("DeepCSV", wp, year);
    return bTagWP::getWP("DeepFlavor", wp, year);    
}

// define here what b-tagger to use
double jetBTagValue(const Jet* jetPtr){
    //return jetPtr->deepCSV();
    return jetPtr->deepFlavor();
}

// define here what jet ID to use in the entire ewkino framework!
std::string jetID(){
    return "tzq"; // choose from "tzq" or "tth"
}

bool JetSelector::isGood() const{
    bool isGood = (jetID()=="tth") ? isGoodttH() :
		    (jetID()=="tzq") ? isGoodtZq() : 
		    false;
    return isGood;
}

bool JetSelector::isGoodttH() const{
    if( !isGoodBasettH() ) return false;
    if( jetPtr->is2016() ){
        return isGood2016ttH();
    } else if( jetPtr->is2017() ){
        return isGood2017ttH();
    } else {
        return isGood2018ttH();
    }
}

bool JetSelector::isGoodtZq() const{
    if( !isGoodBasetZq() ) return false;
    if( jetPtr->is2016() ){ return isGood2016tZq(); }
    else if(jetPtr->is2017()) {return isGood2017tZq(); }
    else{ return isGood2018tZq(); }
}

/*
--------------------------------------------------------------
Jet ID selection for ttH analysis
--------------------------------------------------------------
*/

bool JetSelector::isGoodBasettH() const{
    if( !jetPtr->isTight() ) return false;
    if( jetPtr->pt() < 25 ) return false;
    if( fabs( jetPtr->eta() ) > 2.4 ) return false;
    return true;
}


bool JetSelector::isGood2016ttH() const{
    return true;
}


bool JetSelector::isGood2017ttH() const{
    return true;
}


bool JetSelector::isGood2018ttH() const{
    return true;
}

/*
----------------------------------------------------------------
Jet ID selection for tZq analysis
----------------------------------------------------------------
*/

bool JetSelector::isGoodBasetZq() const{
    // function to copy closely the tZq analysis note
    if( !jetPtr->isTight() ) return false;
    if( jetPtr->pt() < 25 ) return false;
    if( fabs( jetPtr->eta() ) > 5.0 ) return false;

    // additional pt threshold for specific eta range
    // (test if this reduces bump and data/MC discrepancy)
    if( fabs( jetPtr->eta() )>2.7 
	and fabs( jetPtr->eta() )<3.0
	and jetPtr->pt() < 60 ) return false;
    return true;
}

bool JetSelector::isGood2016tZq() const{ return true; }

bool JetSelector::isGood2017tZq() const{ return true; }

bool JetSelector::isGood2018tZq() const{ return true; }

/*
---------------------------------------------------------------
b-tagging working points (common to ttH and tZq analysis)
---------------------------------------------------------------
*/

bool JetSelector::inBTagAcceptance() const{
    if( !isGood() ) return false;
    // the following criterion is in principle already satisfied if isGood()
    if( jetPtr->pt() < 25 ) return false;
    // note that the b-tagging acceptance is much smaller than general jet selection
    // eta range taken from subpages of https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation
    double etathreshold = (jetPtr->is2016())?2.4:2.5;
    if( fabs( jetPtr->eta() ) >= etathreshold ) return false;
    return true;
}

bool JetSelector::isBTaggedLoose2016() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("loose", "2016") );
}


bool JetSelector::isBTaggedLoose2017() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("loose", "2017") );
}


bool JetSelector::isBTaggedLoose2018() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("loose", "2018") );
}
        

bool JetSelector::isBTaggedMedium2016() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("medium", "2016") );
}


bool JetSelector::isBTaggedMedium2017() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("medium", "2017") );
}


bool JetSelector::isBTaggedMedium2018() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("medium", "2018") );
}


bool JetSelector::isBTaggedTight2016() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("tight", "2016") );
}


bool JetSelector::isBTaggedTight2017() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("tight", "2017") );
}


bool JetSelector::isBTaggedTight2018() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("tight", "2018") );
}
