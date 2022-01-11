#include "../objects/interface/JetSelector.h"

//b-tagging working points
#include "bTagWP.h"

/* 
Set global properties for b-tagger
*/

double jetBTagCut(const std::string wp, const std::string year){
    //return bTagWP::getWP("DeepCSV", wp, year);
    return bTagWP::getWP("DeepFlavor", wp, year);
}


double jetBTagValue(const Jet* jetPtr){
    //return jetPtr->deepCSV();
    return jetPtr->deepFlavor();
}


/*
Jet ID selection
*/

bool JetSelector::isGoodBase() const{
    if( jetPtr->pt() < 25 ) return false;
    if( fabs( jetPtr->eta() ) > 2.4 ) return false;
    if( !jetPtr->isTight() ) return false;
    return true;
}


bool JetSelector::isGood2016() const{
    return true;
}


bool JetSelector::isGood2016PreVFP() const{
    return true;
}


bool JetSelector::isGood2016PostVFP() const{
    return true;
}


bool JetSelector::isGood2017() const{
    return true;
}


bool JetSelector::isGood2018() const{
    return true;
}


/*
b-tagging working points
*/

bool JetSelector::inBTagAcceptance() const{
    if( !isGood() ) return false;
    if( jetPtr->pt() < 25 ) return false;
    if( fabs( jetPtr->eta() ) >= 2.4 ) return false;
    return true;
}


bool JetSelector::isBTaggedLoose2016() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("loose", "2016") );
}


bool JetSelector::isBTaggedLoose2016PreVFP() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("loose", "2016PreVFP")  );
}


bool JetSelector::isBTaggedLoose2016PostVFP() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("loose", "2016PostVFP")  );
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


bool JetSelector::isBTaggedMedium2016PreVFP() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("medium", "2016PreVFP") );
}


bool JetSelector::isBTaggedMedium2016PostVFP() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("medium", "2016PostVFP") );
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


bool JetSelector::isBTaggedTight2016PreVFP() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("tight", "2016PreVFP") );
}


bool JetSelector::isBTaggedTight2016PostVFP() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("tight", "2016PostVFP") );
}


bool JetSelector::isBTaggedTight2017() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("tight", "2017") );
}


bool JetSelector::isBTaggedTight2018() const{
    return ( jetBTagValue(jetPtr) > jetBTagCut("tight", "2018") );
}
