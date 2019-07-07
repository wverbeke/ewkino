#include "../interface/JetSelector.h"

//include other parts of framework
#include "../interface/Jet.h"

bool JetSelector::isGood() const{
    if( jetPtr->pt() < 20 ) return false;
    if( fabs( jetPtr->eta() ) > 5 ) return false;
    if( !jetPtr->isLoose() ) return false;
    return true;
}


bool JetSelector::isBTaggedLoose() const{
    return ( jetPtr->deepCSV() > 0.5426 );
}
        

bool JetSelector::isBTaggedMedium() const{
    return ( jetPtr->deepCSV() > 0.8484 );
}


bool JetSelector::isBTaggedTight() const{
    return ( jetPtr->deepCSV() > 0.9535 );
}
