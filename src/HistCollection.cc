#include "../interface/HistCollection.h"


HistCollectionSample::HistCollectionSample(const std::vector<HistInfo>& infoList, std::shared_ptr< Category > categorization, std::shared_ptr<Sample> sam): cat(categorization), sample(sam){
    for(auto infoIt = infoList.cbegin(); infoIt != infoList.cend(); ++infoIt){
        collection.push_back(std::vector< std::shared_ptr<TH1D> >() );
        size_t counter = 0;
        for(auto catIt = cat->getCat().cbegin(); catIt != cat->getCat().cend(); ++catIt){
            collection[counter].push_back(infoIt->makeHist(*catIt + sample->getFileName() ) );
            ++counter;
        }
    }
}

HistCollectionSample::HistCollectionSample(const std::vector<HistInfo>& infoList, const std::vector < std::vector < std::string > >& categorization, std::shared_ptr<Sample> sam):
    HistCollectionSample(infoList, std::make_shared<Category>(categorization), sam) {}

std::shared_ptr<TH1D> HistCollectionSample::access(size_t infoIndex, const std::vector<size_t>& catIndices) const{
    size_t catIndex = cat->getIndex(catIndices);
    return collection[infoIndex][catIndex];
}


HistCollection::HistCollection(const std::vector<HistInfo>& infoList, std::shared_ptr< Category > categorization, const std::vector<Sample>& samList){ 
    for(auto samIt = samList.cbegin(); samIt != samList.cend(); ++samIt){
        fullCollection.push_back(HistCollectionSample(infoList, categorization, std::make_shared<Sample>(Sample(*samIt) ) ) );
    }
}

HistCollection::HistCollection(const std::vector<HistInfo>& infoList, const std::vector < std::vector < std::string > >& categorization, const std::vector<Sample>& samList):
    HistCollection(infoList, std::make_shared<Category>(Category(categorization)), samList) {}

