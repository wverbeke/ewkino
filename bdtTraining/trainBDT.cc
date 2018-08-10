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

    TMVA::DataLoader *dataloader = new TMVA::DataLoader("dataset" + jetsCat + "_" + mllCat + "_" + year);
    if(jetsCat == "1bJet23Jets"){
        dataloader->AddVariable("etaRecoilingJet", 'F');
        dataloader->AddVariable("maxMjj", 'F');
        dataloader->AddVariable("asymmetryWlep", 'F');
        dataloader->AddVariable("highestDeepCSV", 'F');
        dataloader->AddVariable("ltmet", 'F');
        //dataloader->AddVariable("maxDeltaPhijj", 'F');
        dataloader->AddVariable("mTW", 'F');
        //dataloader->AddVariable("topMass", 'F');
        dataloader->AddVariable("pTMaxjj", 'F');
        dataloader->AddVariable("minDeltaPhilb", 'F');
        dataloader->AddVariable("maxDeltaPhill", 'F');
    } else if(jetsCat == "1bJet4Jets"){
        dataloader->AddVariable("etaRecoilingJet", 'F');
        dataloader->AddVariable("maxMjj", 'F');
        dataloader->AddVariable("asymmetryWlep", 'F');
        dataloader->AddVariable("highestDeepCSV", 'F');
        dataloader->AddVariable("ltmet", 'F');
        dataloader->AddVariable("ht", 'F');
        dataloader->AddVariable("mTW", 'F');
        //dataloader->AddVariable("topMass", 'F');
        dataloader->AddVariable("numberOfJets", 'F');

        dataloader->AddVariable("maxDeltaPhill", 'F');
        //dataloader->AddVariable("maxDeltaPhijj", 'F');
        dataloader->AddVariable("minDeltaPhilb", 'F');

        dataloader->AddVariable("deltaRTaggedbJetRecoilingJet", 'F');
        dataloader->AddVariable("deltaRWLeptonTaggedbJet", 'F');

        dataloader->AddVariable("m3l", 'F');

        dataloader->AddVariable("etaMostForward", 'F');
    } else if(jetsCat == "2bJets"){
        dataloader->AddVariable("etaRecoilingJet", 'F');
        dataloader->AddVariable("maxMjj", 'F');
        dataloader->AddVariable("asymmetryWlep", 'F');
        dataloader->AddVariable("highestDeepCSV", 'F');
        dataloader->AddVariable("ltmet", 'F');
        dataloader->AddVariable("ht", 'F');
        dataloader->AddVariable("mTW", 'F');
        //dataloader->AddVariable("topMass", 'F');
        dataloader->AddVariable("numberOfJets", 'F');

        dataloader->AddVariable("maxDeltaPhill", 'F');
        //dataloader->AddVariable("maxDeltaPhijj", 'F');

        dataloader->AddVariable("etaMostForward", 'F');

        dataloader->AddVariable("m3l", 'F');

        /*
        dataloader->AddVariable("deltaRTaggedbJetRecoilingJet", 'F');
        dataloader->AddVariable("deltaRWLeptonTaggedbJet", 'F');
        */
        //dataloader->AddVariable("

        //dataloader->AddVariable("numberOfBJets", 'F');

    }
    ////////////////////////////////
    /*
    dataloader->AddVariable("pTLeadingBJet", 'F');
    dataloader->AddVariable("maxMlb", 'F');
    dataloader->AddVariable("pTMaxlb", 'F');
    dataloader->AddVariable("minmTbmet", 'F');
    ////////////////////////////////
    dataloader->AddVariable("etaMostForward", 'F');
    dataloader->AddVariable("deltaRWLeptonTaggedbJet", 'F');
    ////////////////////////////////
    dataloader->AddVariable("deltaRTaggedbJetRecoilingJet", 'F');
    dataloader->AddVariable("deltaRWlepRecoilingJet", 'F');
    */
    /*
    dataloader->AddVariable(
    dataloader->AddVariable(
    */

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
    TFile* inputFile =  TFile::Open( (const TString&) "../" + directory + "/trainingTree.root");
    TTree* signalTree = (TTree*) (inputFile->Get( (const TString&) "signalTree" + mllCat + "_" + jetsCat));
    TTree* backgroundTree = (TTree*) (inputFile->Get( (const TString&) "backgroundTree" + mllCat + "_" + jetsCat));

    //add tree to dataloader
    dataloader->AddSignalTree(signalTree, 1.);
    dataloader->AddBackgroundTree(backgroundTree, 1.);

    TCut mycuts = "eventWeight>0"; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
    TCut mycutb = "eventWeight>0"; // for example: TCut mycutb = "abs(var1)<0.5";

    dataloader->PrepareTrainingAndTestTree( mycuts, mycutb, "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:NormMode=None:SplitMode=Random:!V" );


    //specify BDT to train
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1", "!H:!V:NTrees=1000:MinNodeSize=5%:BoostType=Grad:Shrinkage=0.1:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True");
    //factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1", "!H:!V:NTrees=1000:MinNodeSize=5%:BoostType=Grad:Shrinkage=0.1:nCuts=200:MaxDepth=2:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True");

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
