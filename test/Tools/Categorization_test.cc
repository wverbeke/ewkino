
#include "../../Tools/interface/Categorization.h"

//include c++ library classes 
#include <string>
#include <vector>

//include test functions
#include "../copyMoveTest.h"

int main(){

    
    std::vector< std::vector< std::string > > categorizationVector = { { "a", "b", "c", "e", "f", "g"}, { "x", "y", "z", "alpha", "beta", "gamma"}, { "i", "j", "k", "l", "m", "n" } };
    Categorization categ( categorizationVector );

    //compare names returned by Categorization class to what we know they should be in this case
    for( Categorization::size_type i = 0; i < categorizationVector[0].size(); ++i ){
        for( Categorization::size_type j = 0; j < categorizationVector[1].size(); ++j ){
            for( Categorization::size_type k = 0; k < categorizationVector[2].size(); ++k ){
                std::string name = categorizationVector[0][i] + "_" + categorizationVector[1][j] + "_" + categorizationVector[2][k];

                if( name != categ.name( {i, j, k } ) ){
                    throw std::runtime_error( "Name returned by Categorization class is '" + categ.name( {i, j, k } ) + "', while it should be '" + name + "'." );
                }
            }
        }
    }

    //test copy and move behavior for leaks
    copyMoveTest( categ );

    return 0;
}
