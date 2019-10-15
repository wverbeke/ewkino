#include "../../Tools/interface/stringTools.h"


//include c++ library classes 
#include <string>
#include <iostream>


int main(){
    
    std::string replace_test = "trondheim_oslo_longyearbyen";
    std::string result = stringTools::replace( replace_test, "oslo", "tromso" );
    if( result != "trondheim_tromso_longyearbyen" ){
        throw std::runtime_error( "result is '" + result + "' while it should be 'trondheim_tromso_longyearbyen'.");
    }
    replace_test = "lillehammer_lillehammer_lillehammer";
    result = stringTools::replace( replace_test, "o", "a" );
    if( result != replace_test ){
        throw std::runtime_error( "result is '" + result + "' while it should be '" + replace_test + "'.");
    }

    result = stringTools::replace( replace_test, "lille", "lily" );
    if( result != "lilyhammer_lilyhammer_lilyhammer" ){
        throw std::runtime_error( "result is '" + result + "' while it should be 'lilyhammer_lilyhammer_lilyhammer'.");
    }

    result = stringTools::replace( replace_test, "lillehammer", "bergen" );
    if( result != "bergen_bergen_bergen" ){
        throw std::runtime_error( "result is '" + result + "' while it should be 'bergen_bergen_bergen'.");
    }

    result = stringTools::removeOccurencesOf( replace_test, "lillehammer" );
    if( result != "__" ){
        throw std::runtime_error( "result is '" + result + "' while it should be '__'.");
    }

    return 0;
}
