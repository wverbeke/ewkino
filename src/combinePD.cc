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
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"


void treeReader::combinePD(std::vector<std::string>& datasets, const bool is2017, std::string outputDirectory){

    //Set output file and tree
    outputDirectory = (outputDirectory == "") ? "~/Work/ntuples_temp/" : outputDirectory;
    const std::string outputFileName = "~/Work/ntuples_tzq/data_combined_trilepton.root";
    TFile* outputFile = new TFile((const TString&) outputFileName ,"RECREATE");
    outputFile->mkdir("blackJackAndHookers");
    outputFile->cd("blackJackAndHookers"); 
    TTree* outputTree = new TTree("blackJackAndHookersTree","blackJackAndHookersTree");
    setOutputTree(outputTree, true);

    //set correct input file names depending on year
    std::string suffix;
    if(!is2017){
        suffix = "_Summer16";
    } else{
        suffix = "_Fall17";
    }
    for(auto& dataset : datasets){
        dataset += suffix;
        dataset += ".root";
    }

    //loop over all files and write unique events
    std::set<std::tuple<long unsigned, long unsigned, long unsigned> > usedEvents;
    for(auto& dataset : datasets){
        std::cout << dataset << std::endl;

        //Read tree	
        TFile* sampleFile = TFile::Open( (const TString&)"~/Work/ntuples_tzq/" + dataset,"read");	

        //Determine hcounter for cross section scaling
        sampleFile->cd("blackJackAndHookers");	
        TTree* sampleTree = (TTree*) (sampleFile->Get("blackJackAndHookers/blackJackAndHookersTree"));
        initTree(sampleTree, true);

        double progress = 0; 	//For printing progress bar
        long unsigned nEntries = sampleTree->GetEntries();
        for (long unsigned it=0; it <nEntries; ++it){
            sampleTree->GetEntry(it);
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
            outputTree->Fill();
        }
        sampleFile->Close();
        std::cout << std::endl;
    }
    outputFile->cd("blackJackAndHookers"); 
    outputTree->Write("",  BIT(2));
    outputFile->Close();
}

int main(int argc, char* argv[]){	
    std::vector<std::string> datasets = {"SingleElectron", "SingleMuon", "DoubleEG", "DoubleMuon", "MuonEG"}; 
    treeReader reader;

    //convert arguments to std::string
    std::vector < std::string > argvStr;
    for(unsigned a = 0; a < argc; ++a){
        argvStr.push_back( std::string(argv[a]) );
    }
    if(argc > 1){

        //first argument determines whether we want to merge 2016 of 2017 data
        bool is2017;
        if(argvStr[1] == "2017"){
            is2017 = true;
        } else if(argvStr[1] == "2016"){
            is2017 = false;
        }
        if(argc == 2){
            reader.combinePD(datasets, is2017);
        } else if(argc == 3){
            reader.combinePD(datasets, is2017, argv[1]);
        } else{
            std::cerr << "Error: Wrong number of options given!" << std::endl;
            return -1;
        }

    //if no options are specified run the code for both 2016 and 2017 data 
    } else if(argc == 1){
        for(unsigned i = 0; i < 2; ++i){

            //make submission script
            std::ostream& initScript(std::ostream&);
            std::ofstream script("combinePD.sh");
            script << "./combinePD " << ( (i == 0) ? "2016" : "2017" );
            script.close();

            //submit job
            tools::submitScript("combinePD.sh", "40:00:00");
        }

        //clean up temporary script
        tools::deleteFile("combinePD.sh");
    } else{
        std::cerr << "Error: Wrong number of options given!" << std::endl;
        return -1;
    } 
    return 0;
}
