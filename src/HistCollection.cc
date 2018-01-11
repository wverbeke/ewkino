#include "../interface/HistCollection.h"

HistCollection::HistCollection(const std::vector<HistInfo>& infoList, const std::vector < std::vector < std::string > >& categorization, const std::string& sample): cat(categorization), sampleName(sample){
    for(auto infoIt = infoList.cbegin(); infoIt != infoList.cend(); ++infoIt){
        collection.push_back(std::vector< std::shared_ptr<TH1D> >() );
        size_t counter = 0;
        for(auto catIt = cat.getCat().cbegin(); catIt != cat.getCat().cend(); ++catIt){
            collection[counter].push_back(infoIt->makeHist(*catIt + sampleName) );
            ++counter;
        }
    }
}

std::shared_ptr<TH1D> HistCollection::access(size_t infoIndex, const std::vector<size_t>& catIndices){
    size_t catIndex = cat.getIndex(catIndices);
    return collection[infoIndex][catIndex];
}
