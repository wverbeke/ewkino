/*
Implementation of HistCollectionNuisances class
*/
#include "../interface/HistCollectionSampleNuisances.h"

HistCollectionSampleNuisances::HistCollectionSampleNuisances(const std::vector< std::shared_ptr< HistInfo > >& infoList, const std::shared_ptr< Sample >& sam, const std::shared_ptr< Category >& cat,
    const std::vector< std::string >& nuisanceNames, const bool includeSB) : HistCollectionSample(infoList, sam, cat, includeSB)
{
    for(auto& info : infoList){
        nuisanceCollection.push_back(HistCollectionBaseNuisances(info, sam, cat, nuisanceNames, includeSB) );
    }
    for(size_t n = 0; n < nuisanceNames.size(); ++n){
        nuisanceIndexMap[nuisanceNames[n]] = n;
    }
}

std::shared_ptr<TH1D> HistCollectionSampleNuisances::accessNuisance(const std::string& nuisanceName, const size_t infoIndex, const std::vector<size_t>& categoryIndices, const bool sb){
    size_t nuisanceIndex = nuisanceIndexMap[nuisanceName];
    return nuisanceCollection[infoIndex].accessNuisance(nuisanceIndex, categoryIndices, sb); 
}
