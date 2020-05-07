#include "../objects/interface/JetSelector.h"

//b-tagging working points
#include "bTagWP.h"

/*
--------------------------------------------------------------
Jet ID selection for ttH analysis
--------------------------------------------------------------
*/

bool JetSelector::isGoodBase() const{
    if( !jetPtr->isTight() ) return false;
    if( jetPtr->pt() < 25 ) return false;
    if( fabs( jetPtr->eta() ) > 2.4 ) return false;
    return true;
}


bool JetSelector::isGood2016() const{
    return true;
}


bool JetSelector::isGood2017() const{
    return true;
}


bool JetSelector::isGood2018() const{
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
    return true;
}

bool JetSelector::isGood2016tZq() const{ return isGood2016(); }

bool JetSelector::isGood2017tZq() const{ 
    // additional pt cut for jets with 2.7 < abs(eta) < 3.0
    if( fabs( jetPtr->eta() ) > 2.7
        and fabs( jetPtr->eta()) < 3.0
        and jetPtr->pt() < 60 ) return false;
    return true;
}

bool JetSelector::isGood2018tZq() const{ return isGood2018(); }

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
    if( fabs( jetPtr->eta() ) >= 2.4 ) return false;
    return true;
}


bool JetSelector::isBTaggedLoose2016() const{
    return ( jetPtr->deepCSV() > bTagWP::looseDeepCSV2016()  );
}


bool JetSelector::isBTaggedLoose2017() const{
    return ( jetPtr->deepCSV() > bTagWP::looseDeepCSV2017() ); 
}


bool JetSelector::isBTaggedLoose2018() const{
    return ( jetPtr->deepCSV() > bTagWP::looseDeepCSV2018() );
}
        

bool JetSelector::isBTaggedMedium2016() const{
    return ( jetPtr->deepCSV() > bTagWP::mediumDeepCSV2016() );
}


bool JetSelector::isBTaggedMedium2017() const{
    return ( jetPtr->deepCSV() > bTagWP::mediumDeepCSV2017() );
}


bool JetSelector::isBTaggedMedium2018() const{
    return ( jetPtr->deepCSV() > bTagWP::mediumDeepCSV2018() );
}


bool JetSelector::isBTaggedTight2016() const{
    return ( jetPtr->deepCSV() > bTagWP::tightDeepCSV2016() );
}


bool JetSelector::isBTaggedTight2017() const{
    return ( jetPtr->deepCSV() > bTagWP::tightDeepCSV2017() );
}


bool JetSelector::isBTaggedTight2018() const{
    return ( jetPtr->deepCSV() > bTagWP::tightDeepCSV2018() );
}
