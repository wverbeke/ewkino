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

//include TMVA classes
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"

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
        //new BDT distribution
        std::make_tuple("bdtG", "BDTG output", 30, -1, 1),
    };

    const unsigned nDist = histInfo.size(); //number of distributions to plot
    const unsigned nCat = 6;                //Several categories enriched in different processes
    const unsigned nMll = 2;                //categories based on dilepton Mass
    const std::string mllNames[nMll] = {"onZ", "offZ"};
    const std::string catNames[nCat] = {"0bJets_01Jets", "0bJets_2Jets", "1bJet_01jets", "1bJet_23Jets", "1bJet_4Jets", "2bJets"};
    //initialize vector holding all histograms
    std::vector< std::vector < std::vector< std::vector< TH1D* > > > > hists(nMll);
    for(unsigned m = 0; m < nMll; ++m){
        hists[m] = std::vector< std::vector < std::vector< TH1D* > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                hists[m][cat].push_back(std::vector < TH1D* >() );
                for(size_t sam = 0; sam < samples.size(); ++sam){
                    hists[m][cat][dist].push_back(nullptr);
                    hists[m][cat][dist][sam] = new TH1D( (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) + catNames[cat] + mllNames[m]), (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) + catNames[cat] + mllNames[m] + ";" + std::get<1>(histInfo[dist]) + ";Events" ),  std::get<2>(histInfo[dist]), std::get<3>(histInfo[dist]), std::get<4>(histInfo[dist]) );
                }
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

    Float_t topMass, pTForwardJets, etaMostForward, missingET, pTLeadingBJet;
    Float_t numberOfJets, pTLeadingJet, mNotSoForwardJets, pTLeadingLepton, highestDeepCSV;
    Float_t maxMjj, minMlb, asymmetryWlep, etaZ, m3l, maxDeltaPhijj, maxDeltaRjj, maxDeltaPhill;
    Float_t pTMaxlb, pT3l, mForwardJetsLeadinBJetW, ht;

    //MVA reader 
    TMVA::Reader *mvaReader[nMll][nCat]; //one BDT for every category
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            mvaReader[m][cat] = new TMVA::Reader( "!Color:!Silent" );
            mvaReader[m][cat]->AddVariable("topMass", &topMass);
            if(catNames[cat] != "1bJet_01jets") mvaReader[m][cat]->AddVariable("pTForwardJets", &pTForwardJets);
            mvaReader[m][cat]->AddVariable("etaMostForward", &etaMostForward);
            if(catNames[cat]  == "1bJet_4Jets" || catNames[cat] == "2bJets") mvaReader[m][cat]->AddVariable("numberOfJets", &numberOfJets);
            if(catNames[cat]  != "1bJet_4Jets" && catNames[cat] != "2bJets") mvaReader[m][cat]->AddVariable("pTLeadingJet", &pTLeadingJet);
            mvaReader[m][cat]->AddVariable("pTLeadingLepton", &pTLeadingLepton);
            mvaReader[m][cat]->AddVariable("mNotSoForwardJets", &mNotSoForwardJets);
            if(catNames[cat] != "1bJet_01jets" && catNames[cat] != "0bJets_01Jets" && catNames[cat]  != "1bJet_4Jets" && catNames[cat] != "2bJets") mvaReader[m][cat]->AddVariable("pTLeadingBJet", &pTLeadingBJet);
            mvaReader[m][cat]->AddVariable("missingET", &missingET);
            mvaReader[m][cat]->AddVariable("highestDeepCSV", &highestDeepCSV);
            if(catNames[cat] != "1bJet_01jets" && catNames[cat] != "0bJets_01Jets")  mvaReader[m][cat]->AddVariable("maxMjj", &maxMjj);
            if(catNames[cat] != "0bJets_2Jets" && catNames[cat] != "0bJets_01Jets")  mvaReader[m][cat]->AddVariable("minMlb", &minMlb);
            if(catNames[cat] != "1bJet_01jets") mvaReader[m][cat]->AddVariable("asymmetryWlep", &asymmetryWlep);
            mvaReader[m][cat]->AddVariable("etaZ", &etaZ);
            mvaReader[m][cat]->AddVariable("m3l", &m3l);
            if(catNames[cat] != "1bJet_01jets" && catNames[cat] != "0bJets_01Jets" && catNames[cat]  != "1bJet_4Jets" && catNames[cat] != "2bJets") mvaReader[m][cat]->AddVariable("maxDeltaPhijj", &maxDeltaPhijj);
            if(catNames[cat] != "1bJet_01jets" && catNames[cat] != "0bJets_01Jets") mvaReader[m][cat]->AddVariable("maxDeltaRjj", &maxDeltaRjj);
            mvaReader[m][cat]->AddVariable("maxDeltaPhill", &maxDeltaPhill);
            if(catNames[cat] != "0bJets_2Jets" && catNames[cat] != "0bJets_01Jets") mvaReader[m][cat]->AddVariable("pTMaxlb", &pTMaxlb);
            mvaReader[m][cat]->AddVariable("pT3l", &pT3l);
            mvaReader[m][cat]->AddVariable("mForwardJetsLeadinBJetW", &mForwardJetsLeadinBJetW);
            mvaReader[m][cat]->AddVariable("ht", &ht);
            mvaReader[m][cat]->BookMVA("BDTGAlt_20Cuts method", "bdtTraining/bdtWeights/" + catNames[cat] + "_" + mllNames[m] + "_BDTGAlt_20Cuts.weights.xml");
        }
    }
    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        if(sam == 0){                   //skip data for now
            ++currentSample;
            continue;
        }
        initSample(0);          //Use combined 2016 luminosity
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
            if(tightLepCount(ind, lCount) != 3) continue; //require 3 tight leptons
            if(!passPtCuts(ind)) continue;
            //require presence of OSSF pair
            if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 
            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
            //require best Z mass to be onZ
            std::pair<unsigned, unsigned> bestZ = trilep::bestZ(lepV, ind, _lFlavor, _lCharge, lCount);
            //make ordered jet and bjet collections
            std::vector<unsigned> jetInd, bJetInd;
            unsigned jetCount = nJets(jetInd);
            unsigned bJetCount = nBJets(bJetInd);
            //find highest eta jet
            unsigned highestEtaJ = (jetCount == 0) ? 99 : jetInd[0];
            for(unsigned j = 1; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) > fabs(_jetEta[highestEtaJ]) ) highestEtaJ = jetInd[j];
            }
            //Determine tZq analysis category
            unsigned tzqCat = tzq::cat(jetCount, bJetCount);
            //Determine mll category
            unsigned mllCat = 1;                      //offZ by default
            double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();
            if( fabs(mll - 91.1876) < 15) mllCat = 0; //offZ    
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
            TLorentzVector neutrino = tzq::findBestNeutrinoAndTop(lepV[lw], met, taggedJetI, jetInd, bJetInd, jetV);

            //forward jet sum
            TLorentzVector forwardJets(0,0,0,0);
            for(unsigned j = 0; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) >= 2.4){ //very good discriminating power when wrongly using index j!!
                    forwardJets += jetV[jetInd[j]];
                }
            }
            //not so forward jet sum
            TLorentzVector notSoForwardJets(0,0,0,0);
            for(unsigned j = 0; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) >= 0.8){
                    notSoForwardJets += jetV[jetInd[j]];
                }
            }

            //find jets with highest DeepCSV and CSVv2 values
            unsigned highestDeepCSVI = (jetCount == 0) ? 0 : jetInd[0];
            for(unsigned j = 1; j < jetCount; ++j){
                if( (_jetDeepCsv_b[jetInd[j]] + _jetDeepCsv_bb[jetInd[j]]) > (_jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI]) ) highestDeepCSVI = jetInd[j];
            }

            //initialize new vectors to make sure everything is defined for 0 jet events!
            TLorentzVector leadingJet(0,0,0,0);
            TLorentzVector trailingJet(0,0,0,0);
            TLorentzVector highestEtaJet(0,0,0,0);
            TLorentzVector recoilingJet(0,0,0,0);
            TLorentzVector taggedBJet(0,0,0,0);
            TLorentzVector leadingBJet(0,0,0,0);
            TLorentzVector trailingBJet(0,0,0,0);
            TLorentzVector highestDeepCSVJet(0,0,0,0);
            if(taggedJetI[0] != 99) taggedBJet = jetV[taggedJetI[0]];
            if(taggedJetI[1] != 99) recoilingJet = jetV[taggedJetI[1]];
            if(jetCount != 0){
                leadingJet = jetV[jetInd[0]];
                trailingJet = jetV[jetInd[jetInd.size() - 1]];
                highestEtaJet = jetV[highestEtaJ];
                highestDeepCSVJet = jetV[highestDeepCSVI];
            }
            if(bJetCount != 0){
                leadingBJet = jetV[bJetInd[0]];
                if(bJetCount > 1) trailingBJet = jetV[bJetInd[bJetInd.size() - 1]];
            } else if(jetCount > 1){
                leadingBJet = jetV[jetInd[1]];
            }
            //compute top vector
            TLorentzVector topV = (neutrino + lepV[lw] + taggedBJet);

            //Compute minimum and maximum masses and separations for several objects
            //lepton bjet
            double minMLeptonbJet = 99999.;
            double maxpTLeptonbJet = 0.;
            for(unsigned l = 0; l < lCount; ++l){
                for(unsigned j = 0; j < bJetCount; ++j){
                    if( (lepV[l] + jetV[bJetInd[j]]).M() < minMLeptonbJet) minMLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).M();
                    if( (lepV[l] + jetV[bJetInd[j]]).Pt() > maxpTLeptonbJet ) maxpTLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).Pt();
                }
            }
            if(minMLeptonbJet == 99999.) minMLeptonbJet = 0.;
            //jet jet
            double maxMJetJet = 0.;
            double maxDeltaRJetJet = 0.;
            double maxDeltaPhiJetJet = 0.;
            if(jetCount != 0){
                for(unsigned l = 0; l < jetCount - 1; ++l){
                    for(unsigned j = l + 1; j < jetCount; ++j){
                        if( (jetV[jetInd[l]] + jetV[jetInd[j]]).M() > maxMJetJet) maxMJetJet = (jetV[jetInd[l]] + jetV[jetInd[j]]).M();
                        if( jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]) > maxDeltaRJetJet) maxDeltaRJetJet = jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]);
                        if( fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]) ) > maxDeltaPhiJetJet) maxDeltaPhiJetJet = fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]));
                    }
                }
            }
            //lepton lepton 
            double maxDeltaPhiLeptonLepton = 0.;
            for(unsigned l = 0; l < lCount - 1; ++l){
                for(unsigned j = l + 1; j < lCount; ++j){
                    if( fabs(lepV[l].DeltaPhi(lepV[j])) > maxDeltaPhiLeptonLepton) maxDeltaPhiLeptonLepton = fabs(lepV[l].DeltaPhi(lepV[j]));
                }
            }

            //compute HT
            double HT = 0;
            for(unsigned j = 0; j < jetCount; ++j){
                HT += _jetPt[jetInd[j]];
            }

            topMass = std::max(topV.M(), 0.);
            pTForwardJets = forwardJets.Pt();
            etaMostForward = fabs(highestEtaJet.Eta());
            numberOfJets = jetCount;
            pTLeadingJet = leadingJet.Pt();
            pTLeadingLepton = _lPt[ind[0]];
            mNotSoForwardJets = std::max(notSoForwardJets.M(), 0.);
            pTLeadingBJet = leadingBJet.Pt();
            missingET = _met;
            highestDeepCSV = (jetCount == 0) ? 0. : _jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI];
            maxMjj = maxMJetJet;
            minMlb = minMLeptonbJet;
            asymmetryWlep = _lEta[ind[lw]]*_lCharge[ind[lw]];
            etaZ = fabs((lepV[bestZ.first] + lepV[bestZ.second]).Eta());
            m3l = (lepV[0] + lepV[1] + lepV[2]).M();
            maxDeltaPhijj = maxDeltaPhiJetJet;
            maxDeltaRjj = maxDeltaRJetJet;
            maxDeltaPhill = maxDeltaPhiLeptonLepton;
            pTMaxlb = maxpTLeptonbJet;
            pT3l = (lepV[0] + lepV[1] + lepV[2]).Pt();
            mForwardJetsLeadinBJetW = std::max((forwardJets + leadingBJet + lepV[lw] + neutrino).M(), 0.);
            ht = HT;

            double bdtG = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTGAlt_20Cuts method");
            double fill[nDist] = {bdtG};

            for(unsigned m = 0; m < nMll; ++m){
                for(unsigned cat = 0; cat < nCat; ++cat){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        hists[m][cat][dist][sam]->Fill(std::min(fill[dist], maxBin[dist]), weight);
                    }
                }
            }
        }
        //set histograms to 0 if negative
        for(unsigned m = 0; m < nMll; ++m){
            for(unsigned cat = 0; cat < nCat; ++cat){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    tools::setNegativeZero(hists[m][cat][dist][sam]);
                }	
            }
        }
    }
    //merge histograms with the same physical background
    std::vector<std::string> proc = {"total bkg.", "tZq", "DY", "TT + Jets", "WJets", "WZ", "multiboson", "TT + Z", "TT/T + X", "X + #gamma", "ZZ/H"};
    std::vector< std::vector< std::vector< std::vector< TH1D* > > > > mergedHists(nMll);
    for(unsigned mll = 0; mll < nMll; ++mll){
        mergedHists[mll] = std::vector< std::vector < std::vector < TH1D* > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                mergedHists[mll][cat].push_back(std::vector<TH1D*>(proc.size() ) );
                for(size_t m = 0, sam = 0; m < proc.size(); ++m){
                    mergedHists[mll][cat][dist][m] = (TH1D*) hists[mll][cat][dist][sam]->Clone();
                    while(sam < samples.size() - 1 && std::get<0>(samples[sam]) == std::get<0>(samples[sam + 1]) ){
                        mergedHists[mll][cat][dist][m]->Add(hists[mll][cat][dist][sam + 1]);
                        ++sam;
                    }
                    ++sam;
                }
            }
        }
    }
    //TEMPORARY//////
    //replace data with sum of all backgrounds
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                mergedHists[m][cat][dist][0] = (TH1D*) mergedHists[m][cat][dist][1]->Clone(); 
                for(unsigned p = 2; p < proc.size(); ++p){
                    mergedHists[m][cat][dist][0]->Add(mergedHists[m][cat][dist][p]);
                }
            }
        }
    }
    ////////////////
    const std::string sigNames[1] = {"tZq"};
    std::vector< std::vector< std::vector< TH1D* > > >  signal(nMll);
    for(unsigned m = 0; m < nMll; ++m){
        signal[m] = std::vector< std::vector < TH1D* > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            signal[m][cat] = std::vector<TH1D*>(nDist);
            for(unsigned dist = 0; dist < nDist; ++dist){
                signal[m][cat][dist] = (TH1D*) mergedHists[m][cat][dist][1]->Clone();
            }
        }
    }
    //plot all distributions
    const bool isSMSignal[(const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/final/" + std::get<0>(histInfo[dist]) + "_" + catNames[cat] + "_" + mllNames[m], "tzq", false, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);             //linear plots

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/final/" + std::get<0>(histInfo[dist]) + "_"  + catNames[cat] + "_" + mllNames[m] + "_log", "tzq", true, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);    //log plots
            }
        }
    }
}
int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
