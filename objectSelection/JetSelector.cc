#include "../objects/interface/JetSelector.h"

/*
Jet ID selection
*/

bool JetSelector::isGoodBase() const{
    if( jetPtr->pt() < 25 ) return false;
    if( fabs( jetPtr->eta() ) > 5 ) return false;
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
    return ( jetPtr->deepCSV() > 0.2217 );
}


bool JetSelector::isBTaggedLoose2017() const{
    return ( jetPtr->deepCSV() > 0.5803 ); 
}


bool JetSelector::isBTaggedLoose2018() const{
    return ( jetPtr->deepCSV() > 0.1241  );
}
        

bool JetSelector::isBTaggedMedium2016() const{
    return ( jetPtr->deepCSV() > 0.6321 );
}


bool JetSelector::isBTaggedMedium2017() const{
    return ( jetPtr->deepCSV() > 0.8838 );
}


bool JetSelector::isBTaggedMedium2018() const{
    return ( jetPtr->deepCSV() > 0.4184 );
}


bool JetSelector::isBTaggedTight2016() const{
    return ( jetPtr->deepCSV() > 0.8953 );
}


bool JetSelector::isBTaggedTight2017() const{
    return ( jetPtr->deepCSV() > 0.9693 );
}


bool JetSelector::isBTaggedTight2018() const{
    return ( jetPtr->deepCSV() >  0.7527 );
}
