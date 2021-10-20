#include "../objects/interface/DMesonSelector.h"

/*
DMeson selection
*/

bool DMesonSelector::isGoodBase() const{
    // note: just dummy conditions for now,
    //       to be set correctly later!
    if( dmesonPtr->pt() < 25 ) return false;
    if( fabs( dmesonPtr->eta() ) > 2.4 ) return false;
    return true;
}


bool DMesonSelector::isGood2016() const{
    return true;
}


bool DMesonSelector::isGood2017() const{
    return true;
}


bool DMesonSelector::isGood2018() const{
    return true;
}

