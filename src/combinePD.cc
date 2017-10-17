//include ROOT classes
#include "TLorentzVector.h"
#include "TH1D.h"
#include "TFile.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "TROOT.h"

//include C++ library classes
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <memory>

//include other parts of the code
#include "../plotting/tdrstyle.h"
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"


void treeReader::combinePD(const std::vector<std::string>& datasets, std::string outputDirectory){
    std::set<std::tuple<long unsigned, long unsigned, long unsigned> > usedEvents;
    //Set output file and tree
    outputDirectory = (outputDirectory == "") ? "~/Work/ntuples_temp/" : outputDirectory;
    const std::string outputFileName = "~/Work/ntuples_ewkino/data_combined_trilepton.root";
    TFile* outputFile = new TFile((const TString&) outputFileName ,"RECREATE");
    outputFile->mkdir("blackJackAndHookers");
    outputFile->cd("blackJackAndHookers"); 
    TTree* outputTree = new TTree("blackJackAndHookersTree","blackJackAndHookersTree");
    setOutputTree(outputTree, true);
    for(std::vector<std::string>::const_iterator it = datasets.cbegin(); it != datasets.cend(); ++it){
        std::cout << *it << std::endl;
        //Read tree	
        TFile* sampleFile = new TFile( (const TString&)"~/Work/ntuples_ewkino/" + *it,"read");	
        //Determine hcounter for cross section scaling
        sampleFile->cd("blackJackAndHookers");	
        TTree* sampleTree = (TTree*) (sampleFile->Get("blackJackAndHookers/blackJackAndHookersTree"));
        Init(sampleTree, true);

        double progress = 0; 	//For printing progress bar
        long nEntries = sampleTree->GetEntries();
        for (long it=0; it <nEntries; ++it){
            if(it%100 == 0 && it != 0){
                progress += (double) (100./ (double) nEntries);
                tools::printProgress(progress);
            } else if(it == nEntries -1){
                progress = 1.;
                tools::printProgress(progress);
            }
            if(usedEvents.find(std::make_tuple(_runNb, _lumiBlock, _eventNb) ) == usedEvents.end()){
                usedEvents.insert(std::make_tuple(_runNb, _lumiBlock, _eventNb) );
            } else{
                continue;
            }
            sampleTree->GetEntry(it);
            outputTree->Fill();
        }
        //sampleFile->Close();
        std::cout << std::endl;
    }
    outputFile->cd("blackJackAndHookers"); 
    outputTree->Write("",  BIT(2));
    outputFile->Close();
}

int main(int argc, char* argv[]){	
    std::vector<std::string> datasets = {"SingleElectron.root", "SingleMuon.root", "DoubleEG.root", "DoubleMuon.root", "MuonEG.root"}; 
    treeReader reader;
    switch(argc){
        case 1:{
                   reader.combinePD(datasets);
                   return 0;
               }
        case 2:{
                   reader.combinePD(datasets, argv[1]);
                   return 0;
               }
        default:{
                    std::cerr << "Error: Wrong number of options given!" << std::endl;
                    return 1;
                }
    }
}

