#include "../interface/HistCollectionDist.h"

//include c++ library classes
#include <fstream>
#include <set>
/*
HistCollectionDist::HistCollectionDist(const std::shared_ptr < HistInfo >& histInfo, const std::vector < std::shared_ptr < Sample > >& samples, const std::shared_ptr < Category >& category){
    for(auto samIt = samples.cbegin(); samIt != samples.cend(); ++samIt){
        collection.push_back(HistCollectionBase(histInfo, *samIt, category) );
    }
}
*/

HistCollectionDist::HistCollectionDist(const std::string& fileList, const std::shared_ptr < HistInfo >& histInfo, const std::vector < std::shared_ptr < Sample > >& samples, const std::shared_ptr < Category >& category){
    std::vector<std::string> fileNameList = getFileNames(fileList);
    for(auto s = 0; s < samples.size(); ++s){
        bool firstFile = true; //check whether file is first for given sample
        for(auto fileIt = fileNameList.cbegin(); fileIt != fileNameList.cend(); ++fileIt){
            //find the correct sample corresponding to the current file
            if(fileIt->find(samples[s]->getFileName()) != std::string::npos){
                if(firstFile){
                    collection.push_back(HistCollectionBase(*fileIt, histInfo, samples[s], category) );
                    firstFile = false;
                }
                else{
                    collection[s] += HistCollectionBase(*fileIt, histInfo, samples[s], category);
                }
            }
        }        
    }
}


HistCollectionDist::HistCollectionDist(const std::string& fileList, const HistInfo& histInfo, const std::vector< Sample >& samples, const Category& category){
    std::shared_ptr<HistInfo> infoPointer = std::make_shared<HistInfo>(histInfo);
    std::shared_ptr<Category> categoryPointer = std::make_shared<Category>(category);
    std::vector < std::shared_ptr< Sample > > samplePointerList;
    for(auto& sam: samples){
        samplePointerList.push_back(std::make_shared< Sample >( sam ) );        
    }
    *this = HistCollectionDist(fileList, infoPointer, samplePointerList, categoryPointer);
}

HistCollectionDist::HistCollectionDist(const std::string& fileList, const HistInfo& histInfo, const std::vector< Sample >& samples, const std::vector< std::vector < std::string > >& categoryVec): 
    HistCollectionDist(fileList, histInfo, samples, Category(categoryVec) ) {}


std::vector<std::string> HistCollectionDist::getFileNames(const std::string& fileName){
    std::vector<std::string> fileList;
    std::ifstream listStream(fileName);
    std::string line;
    while(std::getline(listStream, line)){
        if(!line.empty() && line.find(".root") != std::string::npos){
            fileList.push_back(line);
        }
    }
    listStream.close();
    return fileList;
}

void HistCollectionDist::negBinsToZero() const{
    for(auto colIt = collection.cbegin(); colIt != collection.cend(); ++colIt){
        colIt->negBinsToZero();
    }
}

void HistCollectionDist::mergeProcesses(){
    //set negative bins to 0 before merging
    negBinsToZero();
    //new merged collection
    std::vector< HistCollectionBase > mergedCollection;
    //keep track of processes that have been used
    std::set< std::string > usedProcesses;
    for(auto colIt = collection.cbegin(); colIt != collection.cend(); ++colIt){
        //check if this process is used 
        if(usedProcesses.find( colIt->sampleProcessName() ) != usedProcesses.end() ) continue;
        //add process to merged collection
        mergedCollection.push_back(*colIt);
        usedProcesses.insert(colIt->sampleProcessName());
        //merge all collections containing this process
        for(auto mergeIt = colIt + 1; mergeIt != collection.cend(); ++mergeIt){
            //check if this entry corresponds to the same process
            if(mergeIt->sampleProcessName() == colIt->sampleProcessName()){
                mergedCollection.back() += (*mergeIt);
            }
        }
    }
    //set old collection equal to new collection
    collection = mergedCollection; 
}

std::string HistCollectionDist::name(const size_t categoryIndex) const{
    return distributionName() + "_" + categoryName(categoryIndex);
}

std::string HistCollectionDist::plotPath(const size_t categoryIndex) const{
    std::string path = categoryName(categoryIndex);
    std::replace(path.begin(), path.end(), '_', '/');
    if(path.back() != '/') path.append("/");
    return path;
}

std::shared_ptr<TH1D> HistCollectionDist::getTotalSideBand(const size_t categoryIndex) const{
    if(!hasSideBand()){
        std::cerr<< "Error: requesting totalSideBand for collection without sideband: returning nullptr" << std::endl;
        return std::shared_ptr<TH1D>(nullptr);
    }
    std::shared_ptr<TH1D> totalSideBand = collection.front().access(categoryIndex);
    for(auto colIt = collection.cbegin() + 1; colIt != collection.cend(); ++colIt){
        totalSideBand->Add( (colIt->access(categoryIndex)).get() );
    }
    //set negative bins to zero
    setNegativeBinsToZero(totalSideBand);
    return totalSideBand;
}

Plot HistCollectionDist::getPlot(const size_t categoryIndex){
    //merge processes if this did not already happen
    if(!merged){
        mergeProcesses();
        merged = true;
    }
    std::shared_ptr<TH1D> obs;                                  //data histogram
    std::map< std::string, std::shared_ptr<TH1D> > bkgMap;      //background histogram with names
    for(auto colIt = collection.cbegin(); colIt != collection.cend(); ++colIt){
        if(colIt->isData()){
            //check whether there was not already a data histogram
            if( obs.use_count() != 0) std::cerr << "Error: multiple data histograms present in collection, not clear how to make plot" << std::endl;
            obs = colIt->access(categoryIndex);
        } else{
            bkgMap[colIt->sampleProcessName()] = colIt->access(categoryIndex);
        }        
    } 
    //add sideband (nonprompt) to background list if needed
    if(hasSideBand()){
        bkgMap["Nonprompt e/#mu"] = getTotalSideBand(categoryIndex);
    }
    //return plot initialized with variables computed above
    return Plot(plotPath(categoryIndex) + name(categoryIndex), obs, bkgMap);
}
