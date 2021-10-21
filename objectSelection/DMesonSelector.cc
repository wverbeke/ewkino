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
