#include "../interface/Categorization.h"

//include c++ library classes 
#include <stdexcept>

//include other parts of code
#include "../interface/stringTools.h"


Categorization::Categorization( const std::vector < std::vector <std::string> >& catVec ){

    std::vector< size_type > ranges;
    for( const auto& layer : catVec ){
        ranges.push_back( layer.size() );
    }
    flattenerPtr = std::make_shared< IndexFlattener >( ranges );

    for( unsigned i = 0; i < flattenerPtr->size(); ++i ){
        auto indices = flattenerPtr->indices( i );
        std::string categoryName = "";
        for( size_type j = 0; j < indices.size(); ++j ){
            if( j != 0 ){
                categoryName += "_"; 
            }
            categoryName += catVec[j][indices[j]];
        }
        categories.push_back( categoryName );
    }
}


Categorization::size_type Categorization::index( const std::vector< size_type >& indices ) const{
    return flattenerPtr->index( indices );
}


std::string Categorization::name(const std::vector< size_type >& indices) const{
    return categories[ index(indices) ];
}


std::vector< size_t > Categorization::findCategoriesByName(const std::string& nameToFind) const{
    std::vector< size_type > matchingCategories;
    for( size_type c = 0; c < size(); ++c){
        if( stringTools::stringContains(name(c), nameToFind) ){
            matchingCategories.push_back(c);
        }
    }
    return matchingCategories;
}
