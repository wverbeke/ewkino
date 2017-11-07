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
#include "../interface/ewkinoTools.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"

void treeReader::Analyze(){
    //Set CMS plotting style
    setTDRStyle();
    gROOT->SetBatch(kTRUE);
    //read samples and cross sections from txt file
    readSamples("samples.txt");
    //info on kinematic distributions to plot
    std::vector< std::tuple < std::string, std::string, unsigned, double , double > > histInfo;
    //name      xlabel    nBins,  min, max
    histInfo = {
        std::make_tuple("sip3d", "SIP_{3D}", 100, 0, 8),
        std::make_tuple("dxy", "|d_{xy}| (cm)", 100, 0, 0.05),
        std::make_tuple("dz", "|d_{z}| (cm)", 100, 0, 0.1),
        std::make_tuple("miniIso", "miniIso", 100, 0, 0.4),
        std::make_tuple("leptonMva", "lepton MVA value", 100, -1, 1),
        std::make_tuple("ptRel", "P_{T}^{rel} (GeV)", 100, 0, 200),
        std::make_tuple("ptRatio", "P_{T}^{ratio}", 100, 0, 2),
        std::make_tuple("closestJetCsv", "closest jet CSV", 100, 0, 1),
        std::make_tuple("chargedTrackMult", "closest jet track multiplicity", 20, 0, 20),
        std::make_tuple("met", "E_{T}^{miss} (GeV)", 100, 0, 300),
        std::make_tuple("mll", "M_{ll} (GeV)", 200, 12, 200),
        std::make_tuple("leadPt", "P_{T}^{leading} (GeV)", 100, 25, 200),
        std::make_tuple("trailPt", "P_{T}^{trailing} (GeV)", 100, 15, 150),
        std::make_tuple("nVertex", "number of vertices", 100, 0, 100),
        std::make_tuple("nJets", "number of jets", 10, 0, 10),
        std::make_tuple("nBJets_CSVv2", "number of b-jets (CSVv2)", 5, 0, 8),
        std::make_tuple("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 5, 0, 8)
    };

    const unsigned nDist = histInfo.size(); //number of distributions to plot
    //initialize vector holding all histograms
    std::vector< std::vector< TH1D* > > hists;
    for(unsigned dist = 0; dist < nDist; ++dist){
        hists.push_back(std::vector < TH1D* >() );
        for(size_t sam = 0; sam < samples.size(); ++sam){
            hists[dist].push_back(nullptr);
            hists[dist][sam] = new TH1D( (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist])), (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) + ";" + std::get<1>(histInfo[dist]) + ";Events" ),  std::get<2>(histInfo[dist]), std::get<3>(histInfo[dist]), std::get<4>(histInfo[dist]) );
        }
    }

    //store maxima of histograms for overflow bins
    double maxBin[nDist];
    for(unsigned dist = 0; dist < nDist; ++dist){
        maxBin[dist] = std::get<4>(histInfo[dist]) - 0.5*(std::get<4>(histInfo[dist]) - std::get<3>(histInfo[dist]) )/std::get<2>(histInfo[dist]);
    }

    //tweakable options
    const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        initSample();
        std::cout<<"Entries in "<< std::get<1>(samples[sam]) << " " << nEntries << std::endl;
        double progress = 0; 	//for printing progress bar
        for(long unsigned it = 0; it < nEntries; ++it){
            //print progress bar	
            if(it%100 == 0 && it != 0){
                progress += (double) (100./nEntries);
                tools::printProgress(progress);
            } else if(it == nEntries -1){
                progress = 1.;
                tools::printProgress(progress);
            }
            GetEntry(it);
            //vector containing good lepton indices
            std::vector<unsigned> ind;
            //select leptons
            const unsigned lCount = selectLep(ind);
            if(lCount != 3) continue;
            //require pt cuts (25, 20) to be passed
            if(!passPtCuts(ind)) continue;
            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
        }
        //set histograms to 0 if negative
        for(unsigned dist = 0; dist < nDist; ++dist){
            tools::setNegativeZero(hists[dist][sam]);
        }	
    }
    //merge histograms with the same physical background
    std::vector<std::string> proc = {"obs.", "DY", "TT + Jets", "WJets", "VV", "TT + X", "T + X"};
    std::vector< std::vector<TH1D*> > mergedHists;
    for(unsigned dist = 0; dist < nDist; ++dist){
        mergedHists.push_back(std::vector<TH1D*>(proc.size() ) );
        for(size_t m = 0, sam = 0; m < proc.size(); ++m){
            mergedHists[dist][m] = (TH1D*) hists[dist][sam]->Clone();
            while(sam < samples.size() - 1 && std::get<0>(samples[sam]) == std::get<0>(samples[sam + 1]) ){
                mergedHists[dist][m]->Add(hists[dist][sam + 1]);
                ++sam;
            }
            ++sam;
        }
    }
    //plot all distributions
    for(unsigned dist = 0; dist < nDist; ++dist){
        plotDataVSMC(mergedHists[dist][0], &mergedHists[dist][1], &proc[0], mergedHists[dist].size() - 1, std::get<0>(histInfo[dist]), "ewkino", false, true);             //linear plots
        plotDataVSMC(mergedHists[dist][0], &mergedHists[dist][1], &proc[0], mergedHists[dist].size() - 1, std::get<0>(histInfo[dist])  + "_log", "ewkino", true, true);    //log plots
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
