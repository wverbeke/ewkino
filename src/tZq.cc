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
#include "../interface/tZqTools.h"
#include "../interface/trilepTools.h"
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
        std::make_tuple("met", "E_{T}^{miss} (GeV)", 30, 0, 300),
        std::make_tuple("mll", "M_{ll} (GeV) (GeV)", 60, 12, 200),
        std::make_tuple("leadPt", "P_{T}^{leading} (GeV)", 30, 25, 200),
        std::make_tuple("subPt", "P_{T}^{subleading} (GeV)", 30, 15, 200),
        std::make_tuple("trailPt", "P_{T}^{trailing} (GeV)", 30, 10, 200),
        std::make_tuple("nJets", "number of jets", 10, 0, 10),
        std::make_tuple("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
        std::make_tuple("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8),
        std::make_tuple("jetEta_highestEta", "|#eta| (most forward jet)", 30, 0, 5),
        std::make_tuple("jetEta_leading", "|#eta| (leading jet)", 30, 0, 5),
        std::make_tuple("jetLeadPt", "P_{T} (leading jet) (GeV)", 30, 0, 300),
        std::make_tuple("jetHighestEtaPt", "P_{T} (most forward jet) (GeV)", 30, 0, 300),
        std::make_tuple("mtop", "M_{W + b} (GeV)", 30, 0, 400),
        std::make_tuple("taggedBJetPt", "P_{T} (b-jet from top) (GeV)", 30, 0, 300),
        std::make_tuple("taggedBJetEta", "|#eta| (b-jet from top) (GeV)", 30, 0, 2.5),
        std::make_tuple("taggedRecoilJetPt", "P_{T} (recoiling jet) (GeV)", 30, 0, 300), 
        std::make_tuple("taggedRecoilJetEta", "|#eta| (recoiling jet) (GeV)", 30, 0, 2.5)
    };

    const unsigned nDist = histInfo.size(); //number of distributions to plot
    const unsigned nCat = 7;                //Several categories enriched in different processes
    const std::string catNames[nCat] = {"inclusive", "0bJets_01Jets", "0bJets_2Jets", "1bJet_01jets", "1bJet_23Jets", "1bJet_3Jets", "2bJets"};
    //initialize vector holding all histograms
    std::vector< std::vector < std::vector< TH1D* > > > hists(nCat);
    for(unsigned cat = 0; cat < nCat; ++cat){
        for(unsigned dist = 0; dist < nDist; ++dist){
            hists[cat].push_back(std::vector < TH1D* >() );
            for(size_t sam = 0; sam < samples.size(); ++sam){
                hists[cat][dist].push_back(nullptr);
                hists[cat][dist][sam] = new TH1D( (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) + catNames[cat]), (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) + catNames[cat] + ";" + std::get<1>(histInfo[dist]) + ";Events" ),  std::get<2>(histInfo[dist]), std::get<3>(histInfo[dist]), std::get<4>(histInfo[dist]) );
            }
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
        initSample(2);          //Use combined 2016 + 2017 luminosity
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
            //require pt cuts (25, 15, 10) to be passed
            if(tightLepCount(ind) != 3) continue; //require 3 tight leptons
            if(!passPtCuts(ind)) continue;
            //require presence of OSSF pair
            if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 
            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
            //require best Z mass to be onZ
            std::pair<unsigned, unsigned> bestZ = trilep::bestZ(lepV, lCount);
            double mll = (lepV[bestZ.first] - lepV[bestZ.second]).M();
            if( fabs(mll - 91.1876) < 15) continue;
            //make ordered jet and bjet collections
            std::vector<unsigned> jetInd, bJetInd;
            unsigned jetCount = nJets(jetInd);
            unsigned bJetCount = nBJets(bJetInd);
            //find highest eta jet
            unsigned highestEtaJ = jetInd[0];
            for(unsigned j = 1; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) > fabs(_jetEta[highestEtaJ]) ) highestEtaJ = jetInd[j];
            }
            //Determine tZq analysis category
            unsigned tzqCat = tzq::cat(jetCount, bJetCount);
            //make LorentzVector for all jets 
            TLorentzVector jetV[(const unsigned) _nJets];
            for(unsigned j = 0; j < _nJets; ++j){
                jetV[j].SetPtEtaPhiE(_jetPt[j], _jetEta[j], _jetPhi[j], _jetE[j]);
            }
            //find W lepton 
            unsigned lw;
            for(unsigned l = 0; l < lCount; ++l){
                if( l != bestZ.first && l != bestZ.second ) lw = l;
            }
            //make met vector 
            TLorentzVector met;
            met.SetPtEtaPhiE(_met, _metPhi, 0, _met);
            //reconstruct top mass and tag jets
            std::vector<unsigned> taggedJetI; //0 -> b jet from tZq, 1 -> forward recoiling jet
            double mTop = tzq::findMTop(lepV[lw], met, taggedJetI, jetInd, bJetInd, jetV);
            
            //distributions to plot
            double fill[nDist] = {_met, mll, _lPt[ind[0]], _lPt[ind[1]], _lPt[ind[2]], (double) nJets(), (double) nBJets(), (double) nBJets(0, false), fabs(_jetEta[highestEtaJ]), fabs(_jetEta[jetInd[0]]), _jetPt[jetInd[0]], _jetPt[highestEtaJ], mTop, _jetPt[taggedJetI[0]], fabs(_jetEta[taggedJetI[0]]), _jetPt[taggedJetI[1]], fabs(_jetEta[taggedJetI[1]])};
            for(unsigned cat = 0; cat < nCat; ++cat){
                if(cat == 0 || cat == (tzqCat + 1) ){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        hists[cat][dist][sam]->Fill(std::min(fill[dist], maxBin[dist]), weight);
                    }
                }
            }
        }
        //set histograms to 0 if negative
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                tools::setNegativeZero(hists[cat][dist][sam]);
            }	
        }
    }
    //merge histograms with the same physical background
    std::vector<std::string> proc = {"obs.", "tZq", "DY", "TT + Jets", "WJets", "VV", "TT + X", "T + X"};
    std::vector< std::vector< std::vector< TH1D* > > > mergedHists(nCat);
    for(unsigned cat = 0; cat < nCat; ++cat){
        for(unsigned dist = 0; dist < nDist; ++dist){
            mergedHists[cat].push_back(std::vector<TH1D*>(proc.size() ) );
            for(size_t m = 0, sam = 0; m < proc.size(); ++m){
                mergedHists[cat][dist][m] = (TH1D*) hists[cat][dist][sam]->Clone();
                while(sam < samples.size() - 1 && std::get<0>(samples[sam]) == std::get<0>(samples[sam + 1]) ){
                    mergedHists[cat][dist][m]->Add(hists[cat][dist][sam + 1]);
                    ++sam;
                }
                ++sam;
            }
        }
    }
    const bool isSMSignal[ (const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
    //plot all distributions
    for(unsigned cat = 0; cat < nCat; ++cat){
        for(unsigned dist = 0; dist < nDist; ++dist){
            plotDataVSMC(mergedHists[cat][dist][0], &mergedHists[cat][dist][1], &proc[0], mergedHists[cat][dist].size() - 1, "tZq/" + std::get<0>(histInfo[dist]), "ewkino", false, false, "", nullptr, isSMSignal);             //linear plots
            plotDataVSMC(mergedHists[cat][dist][0], &mergedHists[cat][dist][1], &proc[0], mergedHists[cat][dist].size() - 1, "tZq/" + std::get<0>(histInfo[dist])  + "_log", "ewkino", true, false, "", nullptr, isSMSignal);    //log plots
        }
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}