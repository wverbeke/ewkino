#include "../interface/IndexFlattener.h"


IndexFlattener::IndexFlattener( const std::vector< size_type >& rangeVector ) : ranges( rangeVector )
{}


//project multidimensional index to 1D index 
IndexFlattener::size_type IndexFlattener::index( const std::vector< size_type >& indexVector ) const{

    if( ranges.size() != indexVector.size() ){
        throw std::invalid_argument( "Vector of indices has size " + std::to_string( indexVector.size() ) + " while size " + std::to_string( ranges.size() ) + " is expected." );
    }

    //the last ranges in the initial vector have the largest impact on the final one-dimensional index 
    size_type retIndex = 0;
    size_type multiplier = 1; 
    for( size_type i = 0; i < indexVector.size(); ++i ){
        
        //make sure the index is not larger than the dimension's range 
        if( indexVector[i] >= ranges[i] ){
            throw std::out_of_range( " index " + std::to_string( i ) + " is " + std::to_string( indexVector[i] ) + " while the range is " + std::to_string( ranges[i] ) );
        }

        retIndex += indexVector[i]*multiplier;
        multiplier *= ranges[i];
    }
    return retIndex;
}


//convert 1D index to multidimensional index 
std::vector< IndexFlattener::size_type > IndexFlattener::indices( size_type index ) const{

    //The first time this function is called, compute the total range of each dimension, including its subdimensions
    static std::vector< size_type > flattenedSizes;
    if( flattenedSizes.empty() ){
        size_type multiplier = 1;
        for( const auto& range : ranges ){  
            flattenedSizes.push_back( multiplier*range );
            multiplier *= range;
        }
    }

    //compute the multidimensional indices 
    std::vector< size_type > indices( ranges.size() );
    for( size_type i = flattenedSizes.size() - 1; i > 0; --i ){
        indices[i] = index / flattenedSizes[ i - 1 ];
        index %= flattenedSizes[ i - 1 ];
    }
    indices[0] = index;

    return indices;
}


IndexFlattener::size_type IndexFlattener::size() const{
    static bool size_is_set = false;
    static size_type _size;
    if( ! size_is_set ){
        _size = 1;
        for( const auto& r : ranges ){
            _size *= r;
        }
        size_is_set = true;
    }
    return _size;
}


IndexFlattener::size_type IndexFlattener::range( const size_type dimensionIndex ) const{
    return ranges[ dimensionIndex ];
}
