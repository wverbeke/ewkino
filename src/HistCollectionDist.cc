#include "../interface/HistCollectionDist.h"

//include c++ library classes
#include <fstream>
#include <set>

HistCollectionDist::HistCollectionDist(const std::shared_ptr < HistInfo >& histInfo, const std::vector < std::shared_ptr < Sample > >& samples, const std::shared_ptr < Category >& category){
    for(auto samIt = samples.cbegin(); samIt != samples.cend(); ++samIt){
        collection.push_back(HistCollectionBase(histInfo, *samIt, category) );
    }
}

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
        mergedCollection.push_back(collection[0]);
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


/*
FileList::FileList(const std::string& directory, const std::string& name): fileName(name){
    //list files and pipe to output
    std::system("ls " + directory + " > " + fileName);
    file = std::ifstream(fileName); 
}

FileList::~FileList(){
    file.close();
    std::remove(fileName);
}

bool FileList::getNextFileName(std::string& name){
    return std::getLine(fileName, name);
} 
*/
