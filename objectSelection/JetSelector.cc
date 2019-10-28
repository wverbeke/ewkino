#include "../objects/interface/JetSelector.h"

//b-tagging working points
#include "bTagWP.h"

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
