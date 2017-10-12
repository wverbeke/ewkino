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
#include <tuple>

//include other parts of the code
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"


void treeReader::Analyze(){
    //Set CMS plotting style
    setTDRStyle();
    //gROOT->SetBatch(kTRUE);
    //read samples and cross sections from txt file
    std::ifstream file("samples.txt");
    std::string line;
    std::vector<std::tuple<std::string, std::string, double> > samples;
    while(std::getline(file, line)){	
        samples.push_back(tools::readSampleLine(line));
    }
    //Always close file after usage
    file.close();
    for( auto it = samples.cbegin(); it != samples.cend(); ++it){
        std::cout << std::get<0>(*it) << "     " << std::get<1>(*it) << "      " << std::get<2>(*it) << std::endl;
    }	
    //info on kinematic distributions to plot
    const unsigned nDist = 1;
    std::vector< std::tuple < std::string, std::string, unsigned, double , double > > histInfo;
    histInfo = {
        std::make_tuple("flavors", "", 3, 0, 2)
    };
    //initialize vector holding all histograms
    std::vector< std::vector < TH1D* > > hists(nDist, std::vector<TH1D*>(samples.size()) );
    for(unsigned dist = 0; dist < nDist; ++dist){
        for(size_t sam = 0; sam < samples.size(); ++sam){
            hists[dist][sam] = new TH1D( (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) ), (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist])  + ";" + std::get<1>(histInfo[dist]) + ";Events" ),  std::get<2>(histInfo[dist]), std::get<3>(histInfo[dist]), std::get<4>(histInfo[dist]) );
        }
    }

    //tweakable options
    const double DataLuminosity = 35.867; //in units of 1/fb
    const TString extra = ""; //for plot names

    //Loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        //Read info from tree
        std::shared_ptr<TFile> sampleFile= std::make_shared<TFile>("../../ntuples_ttg/"+ (const TString&) std::get<1>(samples[sam]),"read");
        sampleFile->cd("blackJackAndHookers");
        //Determine hcounter for cross section scaling
        double hCounter;
        if(sam != 0){
            TH1D* _hCounter = new TH1D("hCounter", "Events counter", 5,0,5);
            _hCounter->Read("hCounter");
            hCounter = _hCounter->GetBinContent(1);
            delete _hCounter;
        }
        std::shared_ptr<TTree> sampleTree = std::shared_ptr<TTree> ( (TTree*) (sampleFile->Get("blackJackAndHookers/blackJackAndHookersTree")) );
        Init(sampleTree.get(), sam == 0); //don't store generator info for data ( first entry )

        double scale;
        if(sam != 0) scale = std::get<2>(samples[sam])*DataLuminosity*1000/(hCounter);
        //find number of entries in sample
        long unsigned nEntries = sampleTree->GetEntries();
        std::cout<<"Entries in "<< std::get<1>(samples[sam]) << " " << nEntries << std::endl;
        double progress = 0; 	//For printing progress bar
        for(long unsigned it = 0; it < nEntries/10; ++it){
            //Print progress bar	
            if(it%100 == 0 && it != 0){
                progress += (double) (100./nEntries);
                tools::printProgress(progress);
            } else if(it == nEntries -1){
                progress = 1.;
                tools::printProgress(progress);
            }
            sampleTree->GetEntry(it);
            double weight;
            if(sam == 0) weight = 1;
            else weight = scale*_weight;	
            //vector containing good lepton indices
            std::vector<unsigned> ind;
            //select leptons
            unsigned lCount = selectLep(ind);
            if(lCount != 2) continue;
            //Select photons
            unsigned phCount = selectPhot();
            if(phCount < 1) continue;			
            //Select jets
            if(nJets() < 2) continue;
            //Select b jets
            if(nBJets() < 1) continue;
            //Fill histograms
            double fill[nDist] = {static_cast<double> (ttgCat(ind))};
            for(unsigned dist = 0; dist < nDist; ++dist){
                hists[dist][sam]->Fill(fill[dist], weight);
            }
        }
        //Set histograms to 0 if negative
        for(unsigned dist = 0; dist < nDist; ++dist){
            tools::setNegativeZero(hists[dist][sam]);
        }	
    }
    //Merge histograms with the same physical background
    std::vector<std::string> proc = {"obs.", "TT + #gamma", "TT + jets", "TT/T + X", "X + #gamma", "multiboson", "Drell-Yan"};
    std::vector< std::vector <TH1D*> > mergedHists;
    for(unsigned dist = 0; dist < nDist; ++dist){
        mergedHists.push_back(std::vector<TH1D*>());
        for(size_t m = 0, sam = 0; m < proc.size(); ++m){
            mergedHists[dist].push_back((TH1D*) hists[dist][sam]->Clone());
            while(sam < samples.size() - 1 && std::get<0>(samples[sam]) == std::get<0>(samples[sam + 1]) ){
                mergedHists[dist][m]->Add(hists[dist][sam + 1]);
                ++sam;
            }
            ++sam;
        }
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
