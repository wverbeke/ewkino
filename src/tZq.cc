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
#include "../interface/Reweighter.h"
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
    readSamples("sampleLists/samples.txt");
    //info on kinematic distributions to plot
    std::vector< std::tuple < std::string, std::string, unsigned, double , double > > histInfo;
    //name      xlabel    nBins,  min, max
    histInfo = {
        //new BDT distribution
        std::make_tuple("bdtG", "BDTG output", 10, -1, 1),
        std::make_tuple("bdtG_10bins", "BDTG output", 10, -1, 1),
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

    Float_t asymmetryWlep, topMass, etaMostForward, mTW, highestDeepCSV, numberOfJets, numberOfBJets;
    Float_t pTLeadingLepton, pTLeadingBJet, pTMostForwardJet, mAllJets, maxDeltaPhijj, maxDeltaRjj, maxMlb, maxMjj, pTMaxlb, pTMax2l;
    Float_t minMlb, maxmTbmet, maxpTlmet, pT3l, ht, m3l, mZ, deltaRWLeptonTaggedbJet, etaZ, maxDeltaPhibmet, minDeltaPhibmet;
    Float_t minDeltaPhilb, pTmin2l, minmTbmet, minmTlmet, missingEt, maxmTjmet;
    Float_t eventWeight;
    //MVA reader 
    TMVA::Reader *mvaReader[nMll][nCat]; //one BDT for every category
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            if(cat < 3) continue;
            mvaReader[m][cat] = new TMVA::Reader( "!Color:!Silent" );
            mvaReader[m][cat]->AddVariable("asymmetryWlep", &asymmetryWlep);
            if(catNames[cat] != "0bJets_01Jets" && catNames[cat] != "0bJets_2Jets") mvaReader[m][cat]->AddVariable("deltaRWLeptonTaggedbJet", &deltaRWLeptonTaggedbJet);
            mvaReader[m][cat]->AddVariable("etaZ", &etaZ);
            if(catNames[cat] != "1bJet_01jets" && catNames[cat] != "0bJets_01Jets" && catNames[cat]  != "1bJet_4Jets" && catNames[cat] != "2bJets") mvaReader[m][cat]->AddVariable("pTLeadingBJet", &pTLeadingBJet);
            mvaReader[m][cat]->AddVariable("pTMostForwardJet", &pTMostForwardJet);
            mvaReader[m][cat]->AddVariable("highestDeepCSV", &highestDeepCSV);
            mvaReader[m][cat]->AddVariable("etaMostForward", &etaMostForward);
            mvaReader[m][cat]->AddVariable("pTLeadingLepton", &pTLeadingLepton);
            mvaReader[m][cat]->AddVariable("m3l", &m3l);
            if(catNames[cat] != "1bJet_01jets" && catNames[cat] != "0bJets_01Jets" && catNames[cat]  != "1bJet_4Jets" && catNames[cat] != "2bJets") mvaReader[m][cat]->AddVariable("maxDeltaPhijj", &maxDeltaPhijj);
            if(catNames[cat] != "1bJet_01jets" && catNames[cat] != "0bJets_01Jets")  mvaReader[m][cat]->AddVariable("maxMjj", &maxMjj);
            if(catNames[cat] != "0bJets_01Jets" && catNames[cat] != "0bJets_2Jets") mvaReader[m][cat]->AddVariable("minDeltaPhibmet", &minDeltaPhibmet);
            if(catNames[cat] != "0bJets_01Jets" && catNames[cat] != "0bJets_2Jets") mvaReader[m][cat]->AddVariable("minDeltaPhilb", &minDeltaPhilb);
            if(catNames[cat] == "1bJet_4Jets" || catNames[cat] == "2bJets")  mvaReader[m][cat]->AddVariable("minMlb", &minMlb);
            if(catNames[cat] != "0bJets_2Jets" && catNames[cat] != "0bJets_01Jets")  mvaReader[m][cat]->AddVariable("maxMlb", &maxMlb);
            if(catNames[cat] == "1bJet_4Jets" || catNames[cat] == "2bJets") mvaReader[m][cat]->AddVariable("ht", &ht);
            if(catNames[cat] == "1bJet_4Jets" || catNames[cat] == "2bJets") mvaReader[m][cat]->AddVariable("numberOfJets", &numberOfJets);
            if(catNames[cat] == "2bJets") mvaReader[m][cat]->AddVariable("numberOfBJets", &numberOfBJets);
            if(catNames[cat] != "0bJets_01Jets" && catNames[cat] != "0bJets_2Jets") mvaReader[m][cat]->AddVariable("maxmTbmet", &maxmTbmet);
            mvaReader[m][cat]->AddVariable("pTmin2l", &pTmin2l);
            mvaReader[m][cat]->AddVariable("mTW", &mTW);
            mvaReader[m][cat]->AddVariable("topMass", &topMass);
            //mvaReader[m][cat]->BookMVA("BDTG method", "bdtTraining/bdtWeights/" + catNames[cat] + "_" + mllNames[m] + "_BDTG_m1Cuts_Depth4_baggedGrad_6000trees_shrinkage0p05.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTG method", "bdtTraining/bdtWeights/" + catNames[cat] + "_" + mllNames[m] + "_BDTG_m1Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml");
        }
    }
    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        if(sam == 0){                   //skip data for now
            ++currentSample;
            continue;
        }
        initSample(0);          //2 = combined luminosity
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
            //compute best Z mass
            double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();
            //veto events with mll below 30 GeV, corresponding to the tZq sample
            if(mll < 30) continue; 
            //apply event weight
            weight*=sfWeight();
            //determine mll category
            unsigned mllCat = 1;                      //offZ by default
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

            //lepton Jet 
            double minMLeptonJet = 99999.;
            double maxMLeptonJet = 0.;
            double minDeltaRLeptonJet = 99999.;
            double maxDeltaRLeptonJet = 0.;
            double minDeltaPhiLeptonJet = 99999.;
            double maxDeltaPhiLeptonJet = 0.;
            double minpTLeptonJet = 99999.;
            double maxpTLeptonJet = 0.;
            for(unsigned l = 0; l < lCount; ++l){
                for(unsigned j = 0; j < jetCount; ++j){
                    if( (lepV[l] + jetV[jetInd[j]]).M() < minMLeptonJet) minMLeptonJet = (lepV[l] + jetV[jetInd[j]]).M();
                    if( (lepV[l] + jetV[jetInd[j]]).M() > maxMLeptonJet) maxMLeptonJet = (lepV[l] + jetV[jetInd[j]]).M();
                    if( lepV[l].DeltaR(jetV[jetInd[j]]) < minDeltaRLeptonJet) minDeltaRLeptonJet = lepV[l].DeltaR(jetV[jetInd[j]]);
                    if( lepV[l].DeltaR(jetV[jetInd[j]]) > maxDeltaRLeptonJet) maxDeltaRLeptonJet = lepV[l].DeltaR(jetV[jetInd[j]]);
                    if( fabs(lepV[l].DeltaPhi(jetV[jetInd[j]]) ) < minDeltaPhiLeptonJet) minDeltaPhiLeptonJet = fabs(lepV[l].DeltaPhi(jetV[jetInd[j]]));
                    if( fabs(lepV[l].DeltaPhi(jetV[jetInd[j]]) ) > maxDeltaPhiLeptonJet) maxDeltaPhiLeptonJet = fabs(lepV[l].DeltaPhi(jetV[jetInd[j]]));
                    if( (lepV[l] + jetV[jetInd[j]]).Pt()  < minpTLeptonJet) minpTLeptonJet = (lepV[l] + jetV[jetInd[j]]).Pt();
                    if( (lepV[l] + jetV[jetInd[j]]).Pt()  > maxpTLeptonJet) maxpTLeptonJet = (lepV[l] + jetV[jetInd[j]]).Pt();
                }
            }
            if(minDeltaRLeptonJet == 99999.) minDeltaRLeptonJet = 0.;
            if(minDeltaPhiLeptonJet == 99999.) minDeltaPhiLeptonJet = 0.;
            if(minMLeptonJet == 99999.) minMLeptonJet = 0.;
            if(minpTLeptonJet == 99999.) minpTLeptonJet = 0.;
            //lepton bjet
            double minMLeptonbJet = 99999.;
            double maxMLeptonbJet = 0.;
            double minDeltaRLeptonbJet = 99999.;
            double maxDeltaRLeptonbJet = 0.;
            double minDeltaPhiLeptonbJet = 99999.;
            double maxDeltaPhiLeptonbJet = 0.;
            double minpTLeptonbJet = 99999.;
            double maxpTLeptonbJet = 0.;
            for(unsigned l = 0; l < lCount; ++l){
                for(unsigned j = 0; j < bJetCount; ++j){
                    if( (lepV[l] + jetV[bJetInd[j]]).M() < minMLeptonbJet) minMLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).M();
                    if( (lepV[l] + jetV[bJetInd[j]]).M() > maxMLeptonbJet) maxMLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).M();
                    if( lepV[l].DeltaR(jetV[bJetInd[j]]) < minDeltaRLeptonbJet) minDeltaRLeptonbJet = lepV[l].DeltaR(jetV[bJetInd[j]]);
                    if( lepV[l].DeltaR(jetV[bJetInd[j]]) > maxDeltaRLeptonbJet) maxDeltaRLeptonbJet = lepV[l].DeltaR(jetV[bJetInd[j]]);
                    if( fabs(lepV[l].DeltaPhi(jetV[bJetInd[j]]) ) < minDeltaPhiLeptonbJet) minDeltaPhiLeptonbJet = fabs(lepV[l].DeltaPhi(jetV[bJetInd[j]]));
                    if( fabs(lepV[l].DeltaPhi(jetV[bJetInd[j]]) ) > maxDeltaPhiLeptonbJet) maxDeltaPhiLeptonbJet = fabs(lepV[l].DeltaPhi(jetV[bJetInd[j]]));
                    if( (lepV[l] + jetV[bJetInd[j]]).Pt() < minpTLeptonbJet ) minpTLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).Pt();
                    if( (lepV[l] + jetV[bJetInd[j]]).Pt() > maxpTLeptonbJet ) maxpTLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).Pt();
                }
            }
            if(minDeltaRLeptonbJet == 99999.) minDeltaRLeptonbJet = 0.;
            if(minDeltaPhiLeptonbJet == 99999.) minDeltaPhiLeptonbJet = 0.;
            if(minMLeptonbJet == 99999.) minMLeptonbJet = 0.;
            if(minpTLeptonbJet ==  99999.) minpTLeptonbJet = 0.;

            double minMJetJet = 99999.;
            double maxMJetJet = 0.;
            double minDeltaRJetJet = 99999.;
            double maxDeltaRJetJet = 0.;
            double minDeltaPhiJetJet = 99999.;
            double maxDeltaPhiJetJet = 0.;
            double minpTJetJet = 99999.;
            double maxpTJetJet = 0.;
            if(jetCount != 0){
                for(unsigned l = 0; l < jetCount - 1; ++l){
                    for(unsigned j = l + 1; j < jetCount; ++j){
                        if( (jetV[jetInd[l]] + jetV[jetInd[j]]).M() < minMJetJet) minMJetJet = (jetV[jetInd[l]] + jetV[jetInd[j]]).M();
                        if( (jetV[jetInd[l]] + jetV[jetInd[j]]).M() > maxMJetJet) maxMJetJet = (jetV[jetInd[l]] + jetV[jetInd[j]]).M();
                        if( jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]) < minDeltaRJetJet) minDeltaRJetJet = jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]);
                        if( jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]) > maxDeltaRJetJet) maxDeltaRJetJet = jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]);
                        if( fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]) ) < minDeltaPhiJetJet) minDeltaPhiJetJet = fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]));
                        if( fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]) ) > maxDeltaPhiJetJet) maxDeltaPhiJetJet = fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]));
                        if( (jetV[jetInd[l]] + jetV[jetInd[j]]).Pt() < minpTJetJet ) minpTJetJet = (jetV[jetInd[l]] + jetV[jetInd[j]]).Pt();
                        if( (jetV[jetInd[l]] + jetV[jetInd[j]]).Pt() > maxpTJetJet ) maxpTJetJet = (jetV[jetInd[l]] + jetV[jetInd[j]]).Pt();
                    }
                }
            }
            if(minDeltaRJetJet == 99999.) minDeltaRJetJet = 0.;
            if(minDeltaPhiJetJet == 99999.) minDeltaPhiJetJet = 0.;
            if(minMJetJet == 99999.) minMJetJet = 0.;
            if(minpTJetJet == 99999.) minpTJetJet = 0.;
            //lepton lepton 
            double minMLeptonLepton = 99999.;
            double maxMLeptonLepton = 0.;
            double minDeltaRLeptonLepton = 99999.;
            double maxDeltaRLeptonLepton = 0.;
            double minDeltaPhiLeptonLepton = 99999.;
            double maxDeltaPhiLeptonLepton = 0.;
            double minpTLeptonLepton = 99999.;
            double maxpTLeptonLepton = 0.;
            for(unsigned l = 0; l < lCount - 1; ++l){
                for(unsigned j = l + 1; j < lCount; ++j){
                    if( (lepV[l] + lepV[j]).M() < minMLeptonLepton) minMLeptonLepton = (lepV[l] + lepV[j]).M();
                    if( (lepV[l] + lepV[j]).M() > maxMLeptonLepton) maxMLeptonLepton = (lepV[l] + lepV[j]).M();
                    if( lepV[l].DeltaR(lepV[j]) < minDeltaRLeptonLepton) minDeltaRLeptonLepton = lepV[l].DeltaR(lepV[j]);
                    if( lepV[l].DeltaR(lepV[j]) > maxDeltaRLeptonLepton) maxDeltaRLeptonLepton = lepV[l].DeltaR(lepV[j]);
                    if( fabs(lepV[l].DeltaPhi(lepV[j])) < minDeltaPhiLeptonLepton) minDeltaPhiLeptonLepton = fabs(lepV[l].DeltaPhi(lepV[j]));
                    if( fabs(lepV[l].DeltaPhi(lepV[j])) > maxDeltaPhiLeptonLepton) maxDeltaPhiLeptonLepton = fabs(lepV[l].DeltaPhi(lepV[j]));
                    if( (lepV[l] + lepV[j]).Pt() < minpTLeptonLepton) minpTLeptonLepton = (lepV[l] + lepV[j]).Pt();
                    if( (lepV[l] + lepV[j]).Pt() > maxpTLeptonLepton) maxpTLeptonLepton = (lepV[l] + lepV[j]).Pt();
                }
            }
            if(minDeltaRLeptonLepton == 99999.) minDeltaRLeptonLepton = 0.;
            if(minDeltaPhiLeptonLepton == 99999.) minDeltaPhiLeptonLepton = 0.;
            if(minMLeptonLepton == 99999.) minMLeptonLepton = 0.;
            if(minpTLeptonLepton == 99999.) minpTLeptonLepton = 0.;

            //lepton + MET 
            double minDeltaPhiLeptonMET = 99999.;
            double maxDeltaPhiLeptonMET = 0.;
            double minmTLeptonMET = 99999.;
            double maxmTLeptonMET = 0.;
            double minpTLeptonMET = 99999.;
            double maxpTLeptonMET = 0.;
            for(unsigned l = 0; l < lCount; ++l){
                if(fabs(lepV[l].DeltaPhi(met)) < minDeltaPhiLeptonMET) minDeltaPhiLeptonMET = fabs(lepV[l].DeltaPhi(met));
                if(fabs(lepV[l].DeltaPhi(met)) > maxDeltaPhiLeptonMET) maxDeltaPhiLeptonMET = fabs(lepV[l].DeltaPhi(met));
                if(tools::mt(lepV[l], met) < minmTLeptonMET) minmTLeptonMET = tools::mt(lepV[l], met);
                if(tools::mt(lepV[l], met) > maxmTLeptonMET) maxmTLeptonMET = tools::mt(lepV[l], met);
                if((lepV[l] + met).Pt() < minpTLeptonMET) minpTLeptonMET = (lepV[l] + met).Pt();
                if((lepV[l] + met).Pt() > maxpTLeptonMET) maxpTLeptonMET = (lepV[l] + met).Pt();
            }
            if(minDeltaPhiLeptonMET == 99999.) minDeltaPhiLeptonMET = 0;
            if(minmTLeptonMET == 99999.) minmTLeptonMET = 0.;
            if(minpTLeptonMET == 99999.) minpTLeptonMET = 0.;
            //jet + MET
            double minDeltaPhiJetMET = 99999.;
            double maxDeltaPhiJetMET = 0.;
            double minmTJetMET = 99999.;
            double maxmTJetMET = 0.;
            double minpTJetMET = 99999.;
            double maxpTJetMET = 0.;
            for(unsigned j = 0; j < jetCount; ++j){
                if(fabs(jetV[jetInd[j]].DeltaPhi(met)) < minDeltaPhiJetMET) minDeltaPhiJetMET = fabs(jetV[jetInd[j]].DeltaPhi(met));
                if(fabs(jetV[jetInd[j]].DeltaPhi(met)) > maxDeltaPhiJetMET) maxDeltaPhiJetMET = fabs(jetV[jetInd[j]].DeltaPhi(met));
                if(tools::mt(jetV[jetInd[j]], met) < minmTJetMET) minmTJetMET = tools::mt(jetV[jetInd[j]], met);
                if(tools::mt(jetV[jetInd[j]], met) > maxmTJetMET) maxmTJetMET = tools::mt(jetV[jetInd[j]], met);
                if((jetV[jetInd[j]] + met).Pt() < minpTJetMET) minpTJetMET = (jetV[jetInd[j]] + met).Pt();
                if((jetV[jetInd[j]] + met).Pt() > maxpTJetMET) maxpTJetMET = (jetV[jetInd[j]] + met).Pt();
            }
            if(minDeltaPhiJetMET == 99999.) minDeltaPhiJetMET = 0;
            if(minmTJetMET == 99999.) minmTJetMET = 0.;
            if(minpTLeptonMET == 99999.) minpTLeptonMET = 0.;
            //bjet + MET 
            double minDeltaPhiBJetMET = 99999.;
            double maxDeltaPhiBJetMET = 0.;
            double minmTBJetMET = 99999.;
            double maxmTBJetMET = 0.;
            double minpTBJetMET = 99999.;
            double maxpTBJetMET = 0.;
            for(unsigned j = 0; j < bJetCount; ++j){
                if(fabs(jetV[bJetInd[j]].DeltaPhi(met)) < minDeltaPhiBJetMET) minDeltaPhiBJetMET = fabs(jetV[bJetInd[j]].DeltaPhi(met));
                if(fabs(jetV[bJetInd[j]].DeltaPhi(met)) > maxDeltaPhiBJetMET) maxDeltaPhiBJetMET = fabs(jetV[bJetInd[j]].DeltaPhi(met));
                if(tools::mt(jetV[bJetInd[j]], met) < minmTBJetMET) minmTBJetMET = tools::mt(jetV[bJetInd[j]], met);
                if(tools::mt(jetV[bJetInd[j]], met) > maxmTBJetMET) maxmTBJetMET = tools::mt(jetV[bJetInd[j]], met);
                if((jetV[bJetInd[j]] + met).Pt() < minpTBJetMET) minpTBJetMET = (jetV[bJetInd[j]] + met).Pt();
                if((jetV[bJetInd[j]] + met).Pt() > maxpTBJetMET) maxpTBJetMET = (jetV[bJetInd[j]] + met).Pt();
            }
            if(minDeltaPhiBJetMET == 99999.) minDeltaPhiBJetMET = 0;
            if(minmTBJetMET == 99999.) minmTBJetMET = 0.;
            if(minpTBJetMET == 99999.) minpTBJetMET = 0.;

            //compute HT
            double HT = 0;
            for(unsigned j = 0; j < jetCount; ++j){
                HT += _jetPt[jetInd[j]];
            }

            asymmetryWlep = _lEta[ind[lw]]*_lCharge[ind[lw]];
            topMass = std::max(topV.M(), 0.);
            etaMostForward = fabs(highestEtaJet.Eta());
            mTW = tools::mt(lepV[lw], met);
            highestDeepCSV = (jetCount == 0) ? 0. : _jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI];
            numberOfJets = jetCount;
            numberOfBJets = bJetCount;
            pTLeadingLepton = _lPt[ind[0]];
            pTLeadingBJet = leadingBJet.Pt();
            pTMostForwardJet = highestEtaJet.Pt();
            maxDeltaPhijj = maxDeltaPhiJetJet;
            maxMjj = std::max(maxMJetJet, 0.);
            maxMlb = std::max(maxMLeptonbJet, 0.);
            minMlb = std::max(minMLeptonbJet, 0.);
            pTMaxlb = maxpTLeptonbJet;
            maxmTbmet = maxmTBJetMET;
            maxpTlmet = maxpTLeptonMET;
            pTMax2l = maxpTLeptonLepton;
            m3l = (lepV[0] + lepV[1] + lepV[2]).M();
            pT3l = (lepV[0] + lepV[1] + lepV[2]).Pt();
            ht = HT;
            mZ = mll;
            deltaRWLeptonTaggedbJet = lepV[lw].DeltaR(taggedBJet);
            etaZ = fabs((lepV[bestZ.first] + lepV[bestZ.second]).Eta());
            maxDeltaPhibmet = maxDeltaPhiBJetMET;
            minDeltaPhibmet = minDeltaPhiBJetMET;
            minDeltaPhilb = minDeltaPhiLeptonbJet;
            pTmin2l = minpTLeptonLepton;
            minmTbmet =  minmTBJetMET;
            minmTlmet = minmTLeptonMET;
            missingEt = _met;
            maxmTjmet = maxmTJetMET;
            eventWeight = weight;
            double bdt = 0;
            if(tzqCat > 2){
                bdt = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTG method");
            }
            double fill[nDist] = {bdt, bdt};

            for(unsigned dist = 0; dist < nDist; ++dist){
                hists[mllCat][tzqCat][dist][sam]->Fill(std::min(fill[dist], maxBin[dist]), weight);
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
    std::vector<std::string> proc = {"total bkg.", "tZq", "DY", "TT + Jets", "WZ", "multiboson", "TT + Z", "TT/T + X", "X + #gamma", "ZZ/H"};
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
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/final/36fb/" + std::get<0>(histInfo[dist]) + "_" + catNames[cat] + "_" + mllNames[m], "tzq", false, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);             //linear plots

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/final/36fb/" + std::get<0>(histInfo[dist]) + "_"  + catNames[cat] + "_" + mllNames[m] + "_log", "tzq", true, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);    //log plots
            }
        }
    }
    
    //make shape datacards for each category
    const unsigned nBkg = proc.size() - 2;  //number of background processes
    const std::string bkgNames[nBkg] = {"DY", "TTJets", "WZ", "multiboson", "TTZ", "TTX", "Xgamma", "ZZH"}; //rewrite bkg names not to confuse combine

    const unsigned nSyst = 11 + 1 + 2*nBkg; //11 general uncertainties + stat signal + stat bkg + extra unc per bkg
    std::string systNames[nSyst] = {"lumi", "pdfXsec", "scaleXsec", "JEC", "metUncl", "scale", "pdf", "pu", "btagSF", "id_eff", "trigeff"};
    std::vector<std::vector<double>> systUnc(nSyst, std::vector<double>(nBkg + 1, 0)); //2D array containing all systematics
    //initialize flat and shape systematics
    for(unsigned p = 0; p < nBkg + 1; ++p){ //signal and bkg
        systUnc[0][p] = 1.025;   //lumi
        systUnc[1][p] = 0;       //pdfXsec
        systUnc[2][p] = 0;       //   
        systUnc[3][p] = 1.05;    //JEC
        systUnc[4][p] = 1.05;    //PU 
        systUnc[5][p] = 1.05;    //MET unclustered 
        systUnc[6][p] = 1.1;     //b-tag SF
        systUnc[7][p] = 1.05;    //scale shape
        systUnc[8][p] = 1.02;    //pdf shape
        systUnc[9][p] = 1.06;    //id eff
        systUnc[10][p] = 1.05;   //trig eff  
    }
    //scale and pdf xsection on tzq
    systUnc[1][0] = 1.1;
    systUnc[2][0] = 1.1;
    //set statistical uncertainties
    for(unsigned p = 0; p < nBkg + 1; ++p){
        systUnc[11 + p][p] = 1.00;
    }

    std::string systDist[nSyst]; //probability distribution of nuisances
    for(unsigned syst = 0; syst < nSyst; ++syst){
        if(syst < 11  + 1 || syst  > 11 + nBkg) systDist[syst] = "lnN"; //no shapes at the moment
        else systDist[syst] = "shape";                                     //statistical shape of BDT
    }
    const double extraUnc[nBkg] = {1.3, 1.3, 1.15, 1.5, 1.15, 1.15, 1.15, 1.15}; //extra flat uncertainties assigned to each background
    for(unsigned syst = 12 + nBkg; syst < nSyst; ++syst){//loop over last nBkg uncertainties, being the exta uncertainties for each bkg
        unsigned bkg = syst - 12 - nBkg;//index of the background corresponding to the uncertainty index
        systNames[syst] = "extra" + bkgNames[bkg];
        systUnc[syst][bkg + 1] = extraUnc[bkg];
    }
    TH1D* bdtShape[nMll][nCat][(const size_t) proc.size()] ; //shape histograms of bdt
    TH1D* bdtShapeStatUp[nMll][nCat][(const size_t) proc.size()][(const size_t) proc.size()]; //statistical shape uncertainty on bdt
    TH1D* bdtShapeStatDown[nMll][nCat][(const size_t) proc.size()][(const size_t) proc.size()]; //statistical shape uncertainty on bdt

    //set up background yield array

    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            //Set bkg yields 
            double bkgYields[(const size_t) proc.size() - 2];
            for(unsigned bkg = 0; bkg < proc.size() - 2; ++bkg) bkgYields[bkg] = mergedHists[m][cat][0][2 + bkg]->GetSumOfWeights();
            //Set names of statistical systematics
            for(unsigned p = 1; p < proc.size(); ++p){
                if(p == 1)  systNames[11 + p - 1] = "stattZq" + mllNames[m] + catNames[cat];
                else        systNames[11 + p - 1] = "stat" + bkgNames[p -2] + mllNames[m] + catNames[cat];
            }
            //set BDT shape histogram
            TFile* shapeFile = new TFile((const TString&) "./datacards/shapes/shapeFile_"  + catNames[cat] + mllNames[m] +  ".root", "recreate");
            for(unsigned p = 0; p < proc.size(); ++p){
                bdtShape[m][cat][p] = (TH1D*) mergedHists[m][cat][0][p]->Clone();
                if(p == 0) bdtShape[m][cat][p]->Write("data_obs");
                else if (p == 1) bdtShape[m][cat][p]->Write("tZq");
                else bdtShape[m][cat][p]->Write((const TString&) bkgNames[p -2]);
                if(p != 0){     //set statistical uncertainty as shape
                    for(unsigned k = 1; k < proc.size(); ++k){
                        bdtShapeStatUp[m][cat][p][k] = (TH1D*) mergedHists[m][cat][0][p]->Clone();
                        bdtShapeStatDown[m][cat][p][k] = (TH1D*) mergedHists[m][cat][0][p]->Clone();
                        if(k == p){
                            for(unsigned bin = 1; bin < mergedHists[m][cat][0][p]->GetNbinsX() + 1; ++bin){
                                bdtShapeStatUp[m][cat][p][k]->SetBinContent(bin, std::max( bdtShapeStatUp[m][cat][p][k]->GetBinContent(bin) + bdtShapeStatUp[m][cat][p][k]->GetBinError(bin),  std::numeric_limits< double >::min() ));
                                bdtShapeStatDown[m][cat][p][k]->SetBinContent(bin, std::max( bdtShapeStatDown[m][cat][p][k]->GetBinContent(bin) - bdtShapeStatDown[m][cat][p][k]->GetBinError(bin), std::numeric_limits< double >::min() ));
                            }
                            if (p == 1){
                                bdtShapeStatUp[m][cat][p][k]->Write((const TString&) "tZq_" + systNames[11 + k - 1] + "Up"); 
                                bdtShapeStatDown[m][cat][p][k]->Write((const TString&) "tZq_" + systNames[11 + k - 1] + "Down");
                            } else{
                                bdtShapeStatUp[m][cat][p][k]->Write((const TString&) bkgNames[p -2] + "_" + systNames[11 + k - 1] + "Up"); 
                                bdtShapeStatDown[m][cat][p][k]->Write((const TString&) bkgNames[p -2] + "_" + systNames[11 + k - 1] + "Down");
                            }
                        }
                    }
                }
            }
            shapeFile->Close();
            tools::printDataCard( mergedHists[m][cat][0][0]->GetSumOfWeights(), mergedHists[m][cat][0][1]->GetSumOfWeights(), "tZq", bkgYields, proc.size() - 2, bkgNames, systUnc, nSyst, systNames, systDist, "datacards/datacard_" + mllNames[m] + "_" + catNames[cat], true, "shapes/shapeFile_"  + catNames[cat] + mllNames[m]);
        }
    }

}
int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
