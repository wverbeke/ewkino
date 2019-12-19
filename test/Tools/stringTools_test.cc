#include "../../Tools/interface/stringTools.h"


//include c++ library classes 
#include <string>
#include <iostream>
#include <vector>

std::string vectorToString( const std::vector< std::string >& vec ){
    std::string ret = "{";
    for( const auto& entry : vec ){
        ret += "'" + entry + "', ";
    }
    ret = ret.substr( 0, ret.size() - 2 );
    ret += "}";
    return ret;
}


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


    std::string split_test = "__oslo___kristiansand_stavanger_";
    std::vector< std::string > split_string = stringTools::split( split_test, "_" );
    if( split_string != std::vector< std::string >( { "", "oslo", "kristiansand", "stavanger", "" } ) ){
        throw std::runtime_error( "result is " + vectorToString( split_string ) + " while it should be { 'oslo', 'stavanger', 'kristiansand', '' }. ");
    }

    split_test = "_x_y__xoslo__xkristiansand_x__y_stavanger_x_y___x";
    split_string = stringTools::split( split_test, {"_", "_x", "_y"} );
    if( split_string != std::vector< std::string >( { "", "oslo", "kristiansand", "stavanger", "" } ) ){
        throw std::runtime_error( "result is " + vectorToString( split_string ) + " while it should be { 'oslo', 'stavanger', 'kristiansand', '' }. ");
    }

    split_test = "trondheim";
    split_string = stringTools::split( split_test, "oslo" );
    if( split_string != std::vector< std::string >( { "trondheim" } ) ){
        throw std::runtime_error( "result is " + vectorToString( split_string ) + " while it should be { 'trondheim' }." );
    }

    return 0;
}
