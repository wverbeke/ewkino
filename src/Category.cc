#include "../interface/Category.h"

Category::Category(const std::vector < std::vector <std::string> >& catVec){
    if(catVec.empty()){                          //make sure there is always one element
        cat.push_back("");                     
        ranges.push_back(1);
    } else{
        for(auto it = catVec.cbegin(); it  != catVec.cend(); ++it){
            pos catRange = 0;
            std::vector<std::string> tempV;
            if(cat.empty()){
                cat = *it;
                catRange = it->size();
            } else{
                for(auto jt = it->cbegin(); jt != it->cend(); ++jt){
                    for(auto catI = cat.cbegin(); catI != cat.cend(); ++catI){
                        tempV.push_back(*catI + "_" + *jt);
                    }
                    ++catRange;
                }
                cat = tempV;
            }
            ranges.push_back(catRange);
        }
    }
}

size_t Category::getIndex(const std::vector<pos>& indices){
    if(indices.size() != ranges.size()){
        std::cerr << "Given vector of indices has a wrong dimension, returning index 0!" << std::endl;
        return 0;
    } else{
        pos index = 0;
        unsigned multiplier = 1;
        for(pos i = 0; i < indices.size(); ++i){
            index += indices[i]*multiplier;
            multiplier*=ranges[i];
        }
        return index;
    }
}
