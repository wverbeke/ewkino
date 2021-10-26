#include "../objects/interface/DMesonSelector.h"

/*
DMeson selection
*/

bool DMesonSelector::isGoodBase() const{
    // note: current implementation is based on a study from 20/10/2021,
    //       see notes for details!
    if( dmesonPtr->pt() < 12. ) return false;
    if( dmesonPtr->isolation() < 0.65 ) return false;
    if( dmesonPtr->dR()<0.02 || dmesonPtr->dR()>0.17 ) return false;
    if( dmesonPtr->intResMass()<1. || dmesonPtr->intResMass()>1.04 ) return false;
    if( dmesonPtr->intResMassDiff()<0.9 || dmesonPtr->intResMassDiff()>1.05 ) return false;
    if( dmesonPtr->firstTrackPtr()->pt()<5. ) return false;
    if( dmesonPtr->secondTrackPtr()->pt()<2. ) return false;
    if( dmesonPtr->thirdTrackPtr()->pt()<1. ) return false;
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


bool DMesonSelector::passCut( double (DMeson::*property)() const, 
			      double minValue, double maxValue) const{
    // decide whether a DMeson passes a given cut
    // the cut is given at runtime via a DMeson property and min/max values
    double value = (*dmesonPtr.*property)();
    if( value<minValue || value>maxValue ) return false;
    else return true;
}


bool DMesonSelector::passCut( std::tuple< double (DMeson::*)() const, 
					  double, double > cut ) const{
    // same as previous but different cut input format
    double minValue = std::get<1>(cut);
    double maxValue = std::get<2>(cut);
    double (DMeson::*property)() const = std::get<0>(cut);
    return passCut( property, minValue, maxValue);
}


bool DMesonSelector::passCuts( std::vector<std::tuple< double (DMeson::*)() const, 
						       double, double > > cuts ) const{
    // decide whether a DMeson passes a set of given cuts
    // the cuts are given at runtime via a vector of tuples of DMeson properties and min/max values
    for( std::tuple< double (DMeson::*)() const, double, double > cut : cuts ){
	if( !passCut( cut ) ) return false;
    }
    return true;
}
