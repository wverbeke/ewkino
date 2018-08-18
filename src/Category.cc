#include "../interface/Category.h"

//include other parts of code
#include "../interface/stringTools.h"

Category::Category( const std::vector < std::vector <std::string> >& catVec ){

    //make sure there is always one element
    if(catVec.empty()){
        cat.push_back("");                     
        ranges.push_back(1);
    } else{
        
        //build multidimensional categorization tree
        for(const auto& splitLayer : catVec){
            if(cat.empty()){
                cat = splitLayer;
            } else{
                std::vector<std::string> tempV;
                for(const auto& split : splitLayer){
                    for(const auto& category : cat){
                        tempV.push_back( category + "_" + split );
                    }
                }
                cat = tempV;
            }
            
            //store range of splitting dimension that was looped over 
            pos catRange = splitLayer.size(); 
            ranges.push_back(catRange);
        }
    }
}


size_t Category::getIndex(const std::vector<pos>& indices) const{
    if(indices.size() != ranges.size()){
        std::cerr << "Given vector of indices has a wrong dimension, returning index 0!" << std::endl;
        return 0;
    } else{

        /*
        flatten multidimensional indeces into single index
        later categorizations cause greater shifts in the index value
        */ 
        pos index = 0;
        unsigned multiplier = 1;
        for(pos i = 0; i < indices.size(); ++i){
            
            //check that given indices are not out of range 
            if( indices[i] >= ranges[i] ){
                std::cerr << "Error in Category::getIndex : index " << i << " is " << indices[i] << " while the range is " << ranges[i] << ". Returning 0." << std::endl;
                return 0;
            }

            index += indices[i]*multiplier;
            multiplier*=ranges[i];
        }
        return index;
    }
}


std::string Category::name(const std::vector<pos>& indices) const{
    return cat[getIndex(indices)];
}


std::vector< size_t > Category::findCategoriesByName(const std::string& nameToFind) const{
    std::vector< pos > matchingCategories;
    for(pos c = 0; c < size(); ++c){
        if( stringTools::stringContains(name(c), nameToFind) ){
            matchingCategories.push_back(c);
        }
    }
    return matchingCategories;
}
