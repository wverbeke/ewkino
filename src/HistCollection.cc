#include "../interface/HistCollection.h"

//include c++ library classes
#include <iostream>
#include <set>

#include <chrono>


HistCollectionSample::HistCollectionSample(std::shared_ptr< std::vector< HistInfo> > infoList, std::shared_ptr<Sample> sam, std::shared_ptr< Category > categorization, bool includeSB):
    histInfo(infoList), sample(sam), cat(categorization) {
    size_t counter = 0;
    for(auto infoIt = infoList->cbegin(); infoIt != infoList->cend(); ++infoIt){
        collection.push_back(std::vector< std::shared_ptr<TH1D> >() );
        if(includeSB) sideBand.push_back(std::vector< std::shared_ptr<TH1D> >() );
        for(auto catIt = cat->getCat().cbegin(); catIt != cat->getCat().cend(); ++catIt){
            collection[counter].push_back(infoIt->makeHist(*catIt + sample->getFileName() ) );
            if(includeSB) sideBand[counter].push_back(infoIt->makeHist(*catIt + sample->getFileName() + "_sideband") );  
        }
        ++counter;
    }
}

HistCollectionSample::HistCollectionSample(const std::vector<HistInfo>& infoList, std::shared_ptr<Sample> sam, std::shared_ptr< Category > categorization, bool includeSB):
    HistCollectionSample(std::make_shared< std::vector< HistInfo > >(infoList), sam, categorization, includeSB) {}

HistCollectionSample::HistCollectionSample(std::shared_ptr< std::vector<HistInfo> > infoList, std::shared_ptr<Sample> sam, const std::vector < std::vector < std::string > >& categorization, bool includeSB):
    HistCollectionSample(infoList, sam, std::make_shared<Category>(categorization), includeSB) {}

HistCollectionSample::HistCollectionSample(const std::vector<HistInfo>& infoList, std::shared_ptr<Sample> sam, const std::vector < std::vector < std::string > >& categorization, bool includeSB):
    HistCollectionSample(std::make_shared< std::vector< HistInfo > >(infoList), sam, std::make_shared<Category>(categorization), includeSB) {}

std::shared_ptr<TH1D> HistCollectionSample::access(size_t infoIndex, const std::vector<size_t>& catIndices, bool sb) const{
    size_t catIndex = cat->getIndex(catIndices);
    if(!sb){
        return collection[infoIndex][catIndex];
    } else{
        if(sideBand.empty()){
            std::cerr << "empty sideband, returning index 0" << std::endl;
            return 0;
        }
        return sideBand[infoIndex][catIndex];
    }
}

std::string HistCollectionSample::name(size_t infoIndex, const std::vector<size_t>& catIndices, bool sb) const{
    return sample->getFileName() + (*histInfo)[infoIndex].name() + cat->name(catIndices) + (sb ? "sideband" : "");
}

void HistCollectionSample::setNegZero(){
    //only set negative bins in collection to 0, sideband can be negative
    for(auto dIt = collection.begin(); dIt != collection.cend(); ++dIt){
        for(auto cIt = dIt->cbegin(); cIt != dIt->cend(); ++cIt){
            for(unsigned b = 1; b < (*cIt)->GetNbinsX() + 1; ++b){
                if((*cIt)->GetBinContent(b) < 0.) (*cIt)->SetBinContent(b, 0.);
            }
        }
    }
}

HistCollectionSample& HistCollectionSample::operator+=(const HistCollectionSample& rhs){
    if(collection.size() != rhs.collection.size() || cat->getCat().size() != rhs.cat->getCat().size()){
        std::cerr << "HistCollection of incompatible dimensions can not be added: returning left hand side!" << std::endl;
    } else{
        auto start = std::chrono::high_resolution_clock::now();
        for(size_t dist = 0; dist < collection.size(); ++dist){
            for(size_t c = 0; c < collection[dist].size(); ++c){
                collection[dist][c]->Add(rhs.collection[dist][c].get());
                if(!sideBand.empty()){
                    if(rhs.sideBand.empty()) std::cerr << "Right-hand side has no sideband, can not add sidebands!" << std::endl;
                    else sideBand[dist][c]->Add(rhs.sideBand[dist][c].get());
                }
            }
        }
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "merging HistCollections" << procName() << " and " << rhs.procName() << " took:" << elapsed.count() << "s" << std::endl;
    }
    return *this;
};

HistCollectionSample operator+(const HistCollectionSample& lhs, const HistCollectionSample& rhs){
    HistCollectionSample ret(lhs);
    ret += rhs;
    return ret;
}

HistCollection::HistCollection(std::shared_ptr< std::vector < HistInfo > > infoList, const std::vector<Sample>& samList, std::shared_ptr< Category > categorization, bool includeSB){
    for(auto samIt = samList.cbegin(); samIt != samList.cend(); ++samIt){
        fullCollection.push_back(HistCollectionSample(infoList, std::make_shared<Sample>(Sample(*samIt) ), categorization, includeSB) );
    }
}

HistCollection::HistCollection(const std::vector<HistInfo>& infoList, const std::vector<Sample>& samList, std::shared_ptr< Category > categorization, bool includeSB):
    HistCollection(std::make_shared< std::vector < HistInfo > >(infoList), samList, categorization, includeSB) {}

HistCollection::HistCollection(const std::vector<HistInfo>& infoList, const std::vector<Sample>& samList, const std::vector < std::vector < std::string > >& categorization, bool includeSB):
    HistCollection(infoList, samList, std::make_shared<Category>(Category(categorization)), includeSB) {}

std::shared_ptr<TH1D> HistCollection::access(size_t samIndex, size_t infoIndex, const std::vector<size_t>& catIndices, bool sb) const{
    return fullCollection[samIndex].access(infoIndex, catIndices, sb);
}

std::string HistCollection::name(size_t samIndex, size_t infoIndex, const std::vector<size_t>& catIndices, bool sb) const{
    return fullCollection[samIndex].name(infoIndex, catIndices, sb);
}

void HistCollection::setNegZero(){
    for(HistCollectionSample& samCol : fullCollection){
        samCol.setNegZero();
    }
}

HistCollection HistCollection::mergeProcesses(){
    setNegZero();
    HistCollection tempCol;
    std::set<std::string> usedProcesses;
    for(auto it = fullCollection.cbegin(); it != fullCollection.cend(); ++it){
        if(usedProcesses.find(it->sample->getProc()) == usedProcesses.end()){
            usedProcesses.insert(it->sample->getProc());
            HistCollectionSample tempSam = *it;
            for(auto jt = it + 1; jt != fullCollection.cend(); ++jt){
                if(it->sample->getProc() == jt->sample->getProc()){
                    tempSam += *jt;
                }
            }
            tempCol.fullCollection.push_back(tempSam);
        }
    }
    //*this = tempCol;
    return tempCol;
}

Plot HistCollection::getPlot(size_t infoIndex, size_t catIndex) const{
    std::shared_ptr<TH1D> obs;
    std::map< std::string, std::shared_ptr<TH1D> > bkg;
    for(auto& samCol : fullCollection){
        if(samCol.isData()){
            if(obs.use_count() == 0) obs = samCol.access(infoIndex, catIndex);
            else obs->Add( samCol.access(infoIndex, catIndex).get() );
        }
        else bkg[samCol.procName()] = samCol.access(infoIndex, catIndex);
    }
    std::cout << fullCollection[0].infoName(infoIndex) + "_" + fullCollection[1].catName(catIndex) << std::endl;
    return Plot(fullCollection[0].infoName(infoIndex) + "_" + fullCollection[1].catName(catIndex), obs, bkg);
}

Plot HistCollection::getPlot(size_t infoIndex, const std::vector<size_t>& catIndices) const{
    return getPlot(infoIndex, getIndex(catIndices));
}
