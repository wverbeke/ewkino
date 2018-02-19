#include "../interface/TrainingTree.h"

TrainingTree::TrainingTree(const std::shared_ptr< Sample >& sam, const std::shared_ptr< Category >& cat, const std::map < std::string, double >& varMap):
    sample(sam), category(cat), variableMap(varMap)
{
    treeFile = TFile::Open( (const TString&) "trainingTrees/bdtTrainingTree" + sam->getFileName() + ".root","RECREATE");
    setBranches(); 
}


TrainingTree::TrainingTree(const Sample& sam, const Category& cat, const std::map < std::string, double >& varMap):
    TrainingTree( std::make_shared< Sample >(sam), std::make_shared< Category >(cat), varMap) {}

TrainingTree::TrainingTree(const Sample& sam, const std::vector < std::vector < std::string > >& categoryNames, const std::map < std::string, double >& varMap):
    TrainingTree( sam, Category(categoryNames), varMap) {} 


//close outputfile that was opened in constructor and write trees to file
TrainingTree::~TrainingTree(){
    treeFile->Write();
    treeFile->Close();
}


//set up tree branches 
void TrainingTree::setBranches(){
    //set up tree with branches for every category
    for(size_t c = 0; c < category->size(); ++c){
        backgroundTrees.push_back( new TTree( (const TString&) "backgroundTree" + category->name(c),(const TString&) "backgroundTree" + category->name(c) ) );
        signalTrees.push_back( new TTree( (const TString&) "signalTree" + category->name(c),(const TString&) "signalTree" + category->name(c) ) ); 
        for(auto mapIt = variableMap.begin(); mapIt != variableMap.end(); ++mapIt){
            backgroundTrees[c]->Branch( (const TString&) mapIt->first, &mapIt->second, (const TString&) mapIt->first + "/F");
            signalTrees[c]->Branch( (const TString&) mapIt->first, &mapIt->second, (const TString&) mapIt->first + "/F");
        }
    }
}

//fill event to tree
void TrainingTree::fill(const size_t categoryIndex, const bool isSignal, const std::map< std::string, double>& varMap){
    /*
    consider to replace map copy here with vector for efficiency!
    */
    variableMap = varMap;
    if(isSignal){
        signalTrees[categoryIndex]->Fill();
    } else{
        backgroundTrees[categoryIndex]->Fill();
    }
}
/*
void trainingTree::write(const std::string& directory) const{
    TFile treeFile((const TString) formatedDirectoryName(directory) + bdtTrainingTree.root","RECREATE");
*/
