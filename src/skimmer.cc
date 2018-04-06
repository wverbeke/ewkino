//include ROOT classes
#include "TH1D.h"
#include "TFile.h"
#include "TString.h"
#include "TTree.h"
#include "TROOT.h"

//include C++ library classes
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <memory>
#include <algorithm>

//include other parts of the code
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"


void treeReader::skimTree(const std::string& fileName, std::string outputDirectory, const bool isData){//std::string outputFileName){
    //Read tree	
    std::shared_ptr<TFile> sampleFile = std::make_shared<TFile>( (const TString& ) fileName,"read");	
    sampleFile->cd("blackJackAndHookers");
    //Determine hcounter and lheCounter for MC cross section scaling and uncertainties
    TH1D* hCounter;
    TH1D* lheCounter;
    TH1D* nTrueInt;
    TH1D* nVertices;
    if(!isData){
        hCounter = (TH1D*) sampleFile->Get("blackJackAndHookers/hCounter");
        lheCounter = (TH1D*) sampleFile->Get("blackJackAndHookers/lheCounter");
        nTrueInt = (TH1D*) sampleFile->Get("blackJackAndHookers/nTrueInteractions");
    }
    nVertices = (TH1D*) sampleFile->Get("blackJackAndHookers/nVertices");
    //Get Tree
    TTree* sampleTree = (TTree*) (sampleFile->Get("blackJackAndHookers/blackJackAndHookersTree"));
    initTree(sampleTree, isData);
    outputDirectory = (outputDirectory == "") ? "~/Work/ntuples_temp/" : outputDirectory;
    std::string outputFileName = fileName;
    /*
    auto it = outputFileName.find_last_of("/");
    outputFileName.erase(0, it + 1);
    */
    outputFileName.erase(std::remove(outputFileName.begin(), outputFileName.end(), '/'), outputFileName.end());
    outputFileName.insert(0, outputDirectory);
    auto it = outputFileName.find(".root");
    outputFileName.erase(it, outputFileName.size());
    outputFileName.append("_dilepSkim.root");
    std::cout << "output file : " << outputFileName << std::endl;
    
    std::shared_ptr<TFile> outputFile = std::make_shared<TFile>((const TString&) outputFileName ,"RECREATE");
    outputFile->mkdir("blackJackAndHookers");
    outputFile->cd("blackJackAndHookers"); 
    TTree* outputTree = new TTree("blackJackAndHookersTree","blackJackAndHookersTree");
    setOutputTree(outputTree, isData);

    double progress = 0; 	//For printing progress bar
    long nEntries = sampleTree->GetEntries();
    for (long it=0; it <nEntries; ++it) {
        if(it%100 == 0 && it != 0){
            progress += (double) (100./ (double) nEntries);
            tools::printProgress(progress);
        } else if(it == nEntries -1){
            progress = 1.;
            tools::printProgress(progress);
        }
        sampleTree->GetEntry(it);

        std::vector<unsigned> ind;
        unsigned lCount = selectLep(ind);
        if(lCount < 2) continue;

        outputTree->Fill();
    }   
    std::cout << std::endl;
    if(!isData){
        hCounter->Write();
        lheCounter->Write();
        nTrueInt->Write();
    }
    nVertices->Write();
    outputTree->Write("",  BIT(2));
    outputFile->Close(); 
}

int main(int argc, char* argv[]){
    treeReader reader;
    bool isData = false;
    if(argc != 0){
        std::vector<std::string> datasets = {"SingleElectron", "SingleMuon", "DoubleEG", "DoubleMuon", "MuonEG"}; 
        for(auto it = datasets.cbegin(); it != datasets.cend(); ++it){
            std::string name(argv[1]);
            auto pos = name.find(*it);
            if(pos < name.size()) isData = true;
        }
    }
    switch(argc){
        case 2:{
                   reader.skimTree(argv[1], "", isData);
                   return 0;
               }
        case 3:{
                   reader.skimTree(argv[1], argv[2], isData);
                   return 0;
               }
        default:{
                    std::cerr << "Error: Wrong number of options given!" << std::endl;
                    return 1;
                }
    }
}



