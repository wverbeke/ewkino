#include "../../Tools/interface/IndexFlattener.h"

//include c++ library classes 
#include <random>
#include <string> 
#include <vector>


template< typename T > std::string vectorToString( const std::vector< T >& vec ){
    std::string vecString = "{";
    for( const auto& entry : vec ){
        vecString += std::to_string( entry ) + ", ";
    }
    vecString = vecString.substr( 0, vecString.size() - 2 );
    vecString += "}";
    return vecString;
}


int main(){
    const std::vector< unsigned >::size_type numIterations = 100000;
    const std::vector< unsigned >::size_type numDimensions = 10;
    unsigned rangeSize = 9;


    //generate a vector of random indices
    std::random_device seeder;
    std::ranlux48 random_engine( seeder() );
    std::uniform_int_distribution< unsigned > index_distribution( 0 , rangeSize - 1 );

    std::vector< std::vector< size_t > > randomIndices;
    randomIndices.reserve( numIterations );
    for( unsigned i = 0; i < numIterations; ++i ){
        std::vector< size_t > tempRandVector;
        for( unsigned j = 0; j < numDimensions; ++j ){
            tempRandVector.push_back( index_distribution( random_engine ) );
        }
        randomIndices.push_back( tempRandVector );
    }

    //Project the multidimensional indices onto a 1D indices
    IndexFlattener indexFlattener( std::vector< size_t >( numDimensions, rangeSize ) );

    //convert the random indices to a 1D index and back to a multidimensional index and check their equality
    for( unsigned i = 0; i < numIterations; ++i ){
        auto index = indexFlattener.index( randomIndices[i] );
        auto indices = indexFlattener.indices( index );

        //check whether new and old indices are equal
        if( indices != randomIndices[i] ){
            throw std::runtime_error( "original indices are " + vectorToString( randomIndices[i] ) + " while recomputed indices are " + vectorToString( indices ) + "." );
        }
    }

    //convert a set of 3D indices to 1D indices and compare the resulting one-dimensional indices with values we know they should have 
    std::vector< IndexFlattener::size_type > rangeVector( {9, 9, 9 } );
    indexFlattener = IndexFlattener( rangeVector );
    for( IndexFlattener::size_type i = 0; i < rangeVector[0]; ++i ){
        for( IndexFlattener::size_type j = 0; j < rangeVector[1]; ++j ){
            for( IndexFlattener::size_type k = 0; k < rangeVector[2]; ++k ){

                auto index = i + j*rangeVector[0] + k*rangeVector[1]*rangeVector[2];
                if( index != indexFlattener.index( {i, j, k} ) ){
                    throw std::runtime_error( "one-dimensional index is computed to be " + std::to_string( indexFlattener.index( {i, j, k} ) ) + " while it should be " + std::to_string( index ) + ".");
                }
            }
        }
    }

    return 0;
}
