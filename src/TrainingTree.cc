#include "../interface/TrainingTree.h"

TrainingTree::TrainingTree(const std::shared_ptr<Sampe>& sam, const std::shared_ptr<Category>& cat, const std::map < std::string, double >& varMap):
    sample(sam), category(cat), variableMap(varMap)
{
    setBranches(); 
}


//set up tree branches 
void TrainingTree::setBranches(){
    //set up tree with branches for every category
    for(size_t c = 0; c < category->size(); ++c){
        backgroundTrees.push_back(std::make_shared<TTree>( TTree( (const TString&) "backgroundTree" + category->name(c),(const TString&) "backgroundTree" + category->name(c) ) ) ); 
        signalTrees.push_back(std::make_shared<TTree>( TTree( (const TString&) "signalTree" + category->name(c),(const TString&) "signalTree" + category->name(c) ) ) ); 
        for(mapIt = variableMap.cbegin(); mapIt != variableMap.cend(); ++mapIt){
            backgroundTrees[c]->Branch(mapIt->first, &mapIt->second(), (const TString&) mapIt->first + "/F");
            signalTrees[c]->Branch(mapIt->first, &mapIt->second(), (const TString&) mapIt->first + "/F");
        }
    }
}

//fill event to tree
void trainingTree::fill(const size_t categoryIndex, const bool isSignal, const std::map< std::string, double>& varMap){
    /*
    consider to replace map copy here with vector for efficiency!
    */
    variableMap = varMap;
    if(isSignal){
        signalTrees[c]->Fill();
    } else{
        backgroundTrees[c]->Fill();
    }
}

void trainingTree::write(const std::string& directory) const{
    TFile treeFile((const TString) formatedDirectoryName(directory) + bdtTrainingTree.root","RECREATE");
