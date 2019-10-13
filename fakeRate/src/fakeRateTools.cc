#include "../interface/fakeRateTools.h"


//include c++ library classes 
#include <stdexcept>


void fakeRate::checkFlavorString( const std::string& flavorString ){
    if( !( flavorString == "muon" || flavorString == "electron" ) ){
        throw std::invalid_argument( "Flavor string is '" + flavorString + "' while it must be either 'muon' or 'electron'" );
    }
}


void fakeRate::checkYearString( const std::string& yearString ){
    if( !( yearString == "2016" || yearString == "2017" || yearString == "2018" ) ){
        throw std::invalid_argument( "Year string is '" + yearString + "' while it must be either '2016', '2017' or '2018'" );
    }
}
