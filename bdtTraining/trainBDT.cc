#include <iostream>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"


void trainMvaMethods(const std::string& jetsCat = "", const std::string& mllCat = ""){
    //output file
    TFile* outputFile = TFile::Open((const TString&) "trainingOutput" + jetsCat + mllCat +".root", "RECREATE" );
    TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile, "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

    //names of categories for which to do the training
    //const std::string mllNames[nMll] = {"mllInclusive", "onZ", "offZ"};
    //const std::string catNames[nCat] = {"nJetsInclusive", "0bJets_01Jets", "0bJets_2Jets", "1bJet_01jets", "1bJet_23Jets", "1bJet_4Jets", "2bJets"};

    TMVA::DataLoader *dataloader=new TMVA::DataLoader("dataset");

    //dataloader->AddVariable("mForwardJets", 'F');
    dataloader->AddVariable("topMass", 'F');
    if(jetsCat != "1bJet_01jets") dataloader->AddVariable("pTForwardJets", 'F');
    dataloader->AddVariable("etaLeading", 'F');
    dataloader->AddVariable("etaMostForward", 'F');
    dataloader->AddVariable("pTLeadingLepton", 'F');
    dataloader->AddVariable("pTLeadingJet", 'F');
    dataloader->AddVariable("pTHighestDeepCSVJet", 'F');
    //dataloader->AddVariable("etaRecoilingJet", 'F');
    //dataloader->AddVariable("pTRecoiling_tagged_wlep", 'F');
    //dataloader->AddVariable("numberOfBJets", 'i');
    //dataloader->AddVariable("numberOfJets", 'i');
    //dataloader->AddVariable("dilepMass", 'F');

    dataloader->SetSignalWeightExpression("eventWeight");
    dataloader->SetBackgroundWeightExpression("eventWeight");

    //get training trees
    TFile* file =  new TFile("../trainingTrees/bdtTrainingTree.root");
    TTree* signalTree = (TTree*) (file->Get((const TString&) "signalTree" + jetsCat + mllCat));
    TTree* backgroundTree = (TTree*) (file->Get((const TString&) "backgroundTree" + jetsCat + mllCat));

    dataloader->AddSignalTree(signalTree);
    dataloader->AddBackgroundTree(backgroundTree);

    //NN
    /*
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator" );
    //multicore CPU Deep NN
    TString cpuOptions = dnnOptions + ":Architecture=CPU";
    factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN CPU", cpuOptions); 
    */
    //BDT 
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG",
    //        "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2" );
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDT",
            "!H:!V:NTrees=1000:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" ); //850 trees
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTB",
    //        "!H:!V:NTrees=400:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20" );
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTD",
    //        "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate" );

    //train MVAs using the set of training events
    factory->TrainAllMethods();

    //evaluate all MVAs using the set of test events
    factory->TestAllMethods();

    //evaluate and compare performance of all configured MVAs
    factory->EvaluateAllMethods();

    //save the output
    outputFile->Close();

    delete factory;
    delete dataloader;
}

int main(int argc, char* argv[]){
    if(argc > 2) trainMvaMethods(argv[1], argv[2]);
    else trainMvaMethods("nJetsInclusive", "mllInclusive");
    return 0;
}
