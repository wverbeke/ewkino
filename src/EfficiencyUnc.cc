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


std::map< std::string, Efficiency >::iterator EfficiencyUnc::findEntry( std::map< std::string, Efficiency>& variationMap, const std::string& variation ) {
    auto it = variationMap.find( variation );
    if( it == variationMap.cend() ){
        std::cerr << "Error in EfficiencyUnc::findEntry : " << variation << " does not correspond to any known uncertainty source! Returning off-the-end iterator." << std::endl;
    }
    return it;    
}


std::map< std::string, Efficiency >::iterator EfficiencyUnc::findEntryDown( const std::string& variation ) {
    return findEntry( efficiencyVariationsDown, variation );
}


std::map< std::string, Efficiency >::iterator EfficiencyUnc::findEntryUp( const std::string& variation ) {
    return findEntry( efficiencyVariationsUp, variation );
}


void EfficiencyUnc::fillVariation(std::map< std::string, Efficiency>& variationMap, const std::string& variation, const double entry, const double weight, const bool isNumerator, const bool isSideband){
    auto it = variationMap.find( variation );
    if( it == variationMap.cend() ){
        std::cerr << "Error in EfficiencyUnc::fillVariation : " << variation << " does not correspond to any known uncertainty source! Returning control." << std::endl;
        return;
    }
    (it->second).fill(entry, weight, isNumerator, isSideband);
}


void EfficiencyUnc::fillVariationDown(const std::string& variation, const double entry, const double weight, const bool isNumerator, const bool isSideband){
    fillVariation( efficiencyVariationsDown, variation, entry, weight, isNumerator, isSideband);
}   


void EfficiencyUnc::fillVariationUp(const std::string& variation, const double entry, const double weight, const bool isNumerator, const bool isSideband){
    fillVariation( efficiencyVariationsUp, variation, entry, weight, isNumerator, isSideband);
}


std::shared_ptr<TH1D> EfficiencyUnc::getNumeratorUnc(){
    std::shared_ptr< TH1D > numerator_unc = getNumerator();

    for(int bin = 1; bin < numerator_unc->GetNbinsX() + 1; ++bin){
        double binUnc = 0;
        for(auto& pair : efficiencyVariationsDown ){
            auto& key = pair.first;
            double binContent = numerator_unc->GetBinContent( bin );
    
            std::shared_ptr< TH1D > numerator_down = ( findEntryDown(key)->second ).getNumerator();
            double varDown = numerator_down->GetBinContent( bin );

            std::shared_ptr< TH1D > numerator_up = ( findEntryUp(key)->second ).getNumerator();
            double varUp = numerator_up->GetBinContent( bin );

            double maxVar = std::max( fabs(binContent - varDown), fabs(binContent - varUp) );
            binUnc += (maxVar * maxVar);
        }
        numerator_unc->SetBinContent(bin, sqrt( binUnc ) );
        numerator_unc->SetBinError(bin, 0.);
    }
    return numerator_unc;
}
