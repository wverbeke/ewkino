#include <iostream>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"

void trainMvaMethods(const std::string& jetsCat = "", const std::string& mllCat = "", const std::string& year = ""){

    //check whether the training is for 2016 or 2017
    if( ! (year == "2016" || year == "2017") ){
        std::cerr << "Error: specified training year is not 2016 or 2017, returning" << std::endl;
    } 
    bool is2016 = (year == "2016");

    //output file
    TFile* outputFile = TFile::Open( (const TString&) "trainingOutput" + jetsCat + "_" + mllCat + "_" + year + ".root", "RECREATE" );
    //TMVA::Factory *factory = new TMVA::Factory("TMVAClassification" + jetsCat + "_" + mllCat + "_" + year, outputFile, "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );
    TMVA::Factory *factory = new TMVA::Factory("TMVAClassification" + jetsCat + "_" + mllCat + "_" + year, outputFile, "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification" );

    //names of categories for which to do the training
    TMVA::DataLoader *dataloader = new TMVA::DataLoader("dataset" + jetsCat + "_" + mllCat + "_" + year);
    dataloader->AddVariable("asymmetryWlep", 'F');
    if(jetsCat != "0bJets01Jets" && jetsCat != "0bJets2Jets") dataloader->AddVariable("deltaRWLeptonTaggedbJet", 'F');
    dataloader->AddVariable("etaZ", 'F');
    if(jetsCat != "1bJet01jets" && jetsCat != "0bJets01Jets" && jetsCat != "1bJet4Jets" && jetsCat != "2bJets") dataloader->AddVariable("pTLeadingBJet", 'F');
    dataloader->AddVariable("pTMostForwardJet", 'F');
    dataloader->AddVariable("highestDeepCSV", 'F');
    dataloader->AddVariable("etaMostForward", 'F');
    dataloader->AddVariable("pTLeadingLepton", 'F');
    dataloader->AddVariable("m3l", 'F');
    //if(jetsCat != "0bJets_01Jets" && jetsCat != "0bJets_2Jets") dataloader->AddVariable("maxDeltaPhibmet", 'F');
    if(jetsCat != "0bJets01Jets" && jetsCat != "1bJet01jets" && jetsCat != "1bJet4Jets" && jetsCat != "2bJets") dataloader->AddVariable("maxDeltaPhijj", 'F');
    if(jetsCat != "1bJet01jets" && jetsCat != "0bJets01Jets") dataloader->AddVariable("maxMjj", 'F');
    if(jetsCat != "0bJets01Jets" && jetsCat != "0bJets2Jets") dataloader->AddVariable("minDeltaPhibmet", 'F');
    if(jetsCat != "0bJets01Jets" && jetsCat != "0bJets2Jets") dataloader->AddVariable("minDeltaPhilb", 'F');
    if(jetsCat == "1bJet4Jets" || jetsCat == "2bJets") dataloader->AddVariable("minMlb", 'F');
    if(jetsCat != "0bJets01Jets" && jetsCat != "0bJets2Jets") dataloader->AddVariable("maxMlb", 'F');
    if(jetsCat == "1bJet4Jets" || jetsCat == "2bJets") dataloader->AddVariable("ht", 'F'); 
    if(jetsCat == "1bJet4Jets" || jetsCat == "2bJets") dataloader->AddVariable("numberOfJets", 'F');
    if(jetsCat == "2bJets") dataloader->AddVariable("numberOfBJets", 'F');
    if(jetsCat != "0bJets01Jets" && jetsCat != "0bJets2Jets") dataloader->AddVariable("maxmTbmet", 'F');
    //if(jetsCat != "0bJets_01Jets" && jetsCat != "1bJet_01jets") dataloader->AddVariable("maxDeltaRjj", 'F');
    dataloader->AddVariable("pTmin2l", 'F');
    //dataloader->AddVariable("pT3l", 'F');
    dataloader->AddVariable("mTW", 'F');
    dataloader->AddVariable("topMass", 'F');

    //set correct weights for every event (depending on the process it comes from)
    dataloader->SetSignalWeightExpression("eventWeight");
    dataloader->SetBackgroundWeightExpression("eventWeight");

    //get training tree
    std::string directory;
    if( is2016 ){
        directory = "trainingTrees_tZq2016";
    } else {
        directory = "trainingTrees_tZq2017";
    } 
    std::cout << "directory = " << directory << std::endl;
    TFile* inputFile =  TFile::Open( (const TString&) "../" + directory + "/trainingTree.root");
    TTree* signalTree = (TTree*) (inputFile->Get( (const TString&) "signalTree" + mllCat + "_" + jetsCat));
    TTree* backgroundTree = (TTree*) (inputFile->Get( (const TString&) "backgroundTree" + mllCat + "_" + jetsCat));

    dataloader->AddSignalTree(signalTree, 1.);
    dataloader->AddBackgroundTree(backgroundTree, 1.);


    TCut mycuts = "eventWeight>0"; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
    TCut mycutb = "eventWeight>0"; // for example: TCut mycutb = "abs(var1)<0.5";
    dataloader->PrepareTrainingAndTestTree( mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:NormMode=None:SplitMode=Random:!V" );

    //specify BDT to train
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG_200Cuts_Depth4_baggedGrad_3000trees_shrinkage0p1", "!H:!V:NTrees=3000:MinNodeSize=10%:BoostType=Grad:Shrinkage=0.1:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True");
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1", "!H:!V:NTrees=1000:MinNodeSize=10%:BoostType=Grad:Shrinkage=0.1:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True");
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1_node0p05", "!H:!V:NTrees=1000:MinNodeSize=5%:BoostType=Grad:Shrinkage=0.1:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True");
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1_node0p2", "!H:!V:NTrees=1000:MinNodeSize=20%:BoostType=Grad:Shrinkage=0.1:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True");
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p5", "!H:!V:NTrees=1000:MinNodeSize=10%:BoostType=Grad:Shrinkage=0.5:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True");
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p5_node0p2", "!H:!V:NTrees=1000:MinNodeSize=20%:BoostType=Grad:Shrinkage=0.5:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True");

    //train MVAs using the set of training events
    factory->TrainAllMethods();

    //evaluate all MVAs using the set of test events
    factory->TestAllMethods();

    //evaluate and compare performance of all configured MVAs
    factory->EvaluateAllMethods();

    //save the output
    inputFile->Close();
    outputFile->Close();

    delete factory;
    delete dataloader;
}
int main(int argc, char* argv[]){
    if(argc > 2){
        trainMvaMethods(argv[1], argv[2], argv[3]);
    }

    return 0;
}
