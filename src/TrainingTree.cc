#include "../interface/TrainingTree.h"

TrainingTree::TrainingTree(const std::shared_ptr< Sample >& sam, const std::shared_ptr< Category >& cat, const std::map < std::string, float >& varMap, const bool isSignal):
    sample(sam), category(cat), variableMap(varMap)
{
    treeFile = TFile::Open( (const TString&) "trainingTrees/bdtTrainingTree" + sam->getFileName() + ".root","RECREATE");
    setBranches(isSignal); 
}


TrainingTree::TrainingTree(const Sample& sam, const Category& cat, const std::map < std::string, float >& varMap, const bool isSignal):
    TrainingTree( std::make_shared< Sample >(sam), std::make_shared< Category >(cat), varMap, isSignal) {}

TrainingTree::TrainingTree(const Sample& sam, const std::vector < std::vector < std::string > >& categoryNames, const std::map < std::string, float >& varMap, const bool isSignal):
    TrainingTree( sam, Category(categoryNames), varMap, isSignal) {} 


//close outputfile that was opened in constructor and write trees to file
TrainingTree::~TrainingTree(){
    treeFile->Write();
    treeFile->Close();
}

std::string TrainingTree::treeName(const size_t categoryIndex, const bool isSignal){
    return (  ( (isSignal) ? "signalTree" : "backgroundTree" ) + category->name(categoryIndex) );
}


//set up tree branches 
void TrainingTree::setBranches(const bool isSignal){
    //set up tree with branches for every category
    for(size_t c = 0; c < category->size(); ++c){
        trainingTrees.push_back( new TTree( (const TString&) treeName(c, isSignal) ,(const TString&) treeName(c, isSignal)  ) );
        for(auto mapIt = variableMap.begin(); mapIt != variableMap.end(); ++mapIt){
            //Warning: do not use "&mapIt->second" for the middle argument since this will give the adress of a temporary!"
            trainingTrees[c]->Branch( (const TString&) mapIt->first, &variableMap[mapIt->first], (const TString&) mapIt->first + "/F");
        }
    }
}

//fill event to tree
void TrainingTree::fill(const size_t categoryIndex, const std::map< std::string, float>& varMap){
    /*
    consider to replace map copy here with vector for efficiency!
    */
    //variableMap = varMap;
    for(auto it = varMap.cbegin(); it != varMap.cend(); ++it){
        variableMap[it->first] = it->second;
    }
    trainingTrees[categoryIndex]->Fill();
}

void TrainingTree::fill(const std::vector< size_t>& categoryIndices, const std::map< std::string, float>& varMap){   
    size_t categoryIndex = category->getIndex(categoryIndices);
    fill(categoryIndex, varMap);
}
/*
void trainingTree::write(const std::string& directory) const{
    TFile treeFile((const TString) formatedDirectoryName(directory) + bdtTrainingTree.root","RECREATE");
*/
