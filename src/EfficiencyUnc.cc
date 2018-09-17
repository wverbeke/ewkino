#include "../interface/EfficiencyUnc.h"

//include c++ library classes
#include <iostream>


EfficiencyUnc::EfficiencyUnc(const std::string& name, const HistInfo& info, const std::vector<std::string>& uncNames, const bool sideband):
    Efficiency(name, info, sideband)
{
    for(auto& key: uncNames){
        efficiencyVariationsDown.insert( {key, Efficiency(name + "_" + key + "Down", info, sideband) } );
        efficiencyVariationsUp.insert( {key, Efficiency(name + "_" + key + "Up", info, sideband) } );
    }
}

void EfficiencyUnc::fillVariation(std::map< std::string, Efficiency>& variationMap, const std::string& variation, const double entry, const double weight, const bool isNumerator, const bool isSideband){
    auto it = variationMap.find( variation );
    if( it == variationMap.cend() ){
        std::cerr << "Error in EfficiencyUnc::fillVariation : " << variation << " does not correspond to any known uncertainty source! Returning control." << std::endl;
    }
    (it->second).fill(entry, weight, isNumerator, isSideband);
}


void EfficiencyUnc::fillVariationDown(const std::string& variation, const double entry, const double weight, const bool isNumerator, const bool isSideband){
    fillVariation( efficiencyVariationsDown, variation, entry, weight, isNumerator, isSideband);
}   

void EfficiencyUnc::fillVariationUp(const std::string& variation, const double entry, const double weight, const bool isNumerator, const bool isSideband){
    fillVariation( efficiencyVariationsUp, variation, entry, weight, isNumerator, isSideband);
}
