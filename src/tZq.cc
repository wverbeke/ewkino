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
        std::make_tuple("mll", "M_{ll} (GeV)", 60, 12, 200),
        std::make_tuple("mt", "M_{T} (GeV)", 30, 0, 300),
        std::make_tuple("leadPt", "P_{T}^{leading} (GeV)", 30, 25, 200),
        std::make_tuple("subPt", "P_{T}^{subleading} (GeV)", 30, 15, 200),
        std::make_tuple("trailPt", "P_{T}^{trailing} (GeV)", 30, 10, 200),
        std::make_tuple("nJets", "number of jets", 10, 0, 10),
        std::make_tuple("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8),
        std::make_tuple("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
        std::make_tuple("jetEta_highestEta", "|#eta| (most forward jet)", 30, 0, 5),
        std::make_tuple("jetEta_leading", "|#eta| (leading jet)", 30, 0, 5),
        std::make_tuple("jetLeadPt", "P_{T} (leading jet) (GeV)", 30, 0, 300),
        std::make_tuple("trailingJetPt", "P_{T} (trailing jet) (GeV)", 30, 0, 200), 
        std::make_tuple("leadinBJetPt", "P_{T} (leading b-jet) (GeV)", 30, 0, 200),
        std::make_tuple("trailingBJetPt", "P_{T} (trailing b-jet) (GeV)", 30, 0, 200),
        std::make_tuple("jetHighestEtaPt", "P_{T} (most forward jet) (GeV)", 30, 0, 300),
        std::make_tuple("mtop", "M_{(W + b)} (GeV)", 30, 0, 400),
        std::make_tuple("taggedBJetPt", "P_{T} (b-jet from top) (GeV)", 30, 0, 300),
        std::make_tuple("taggedBJetEta", "|#eta| (b-jet from top) (GeV)", 30, 0, 2.5),
        std::make_tuple("taggedRecoilJetPt", "P_{T} (recoiling jet) (GeV)", 30, 0, 300), 
        std::make_tuple("taggedRecoilJetEta", "|#eta| (recoiling jet) (GeV)", 30, 0, 5),
        std::make_tuple("m_highestEta_leadingB_W", "M_{(most forward jet + leading b-jet + W)} (GeV)", 30, 0, 600),
        std::make_tuple("m_highestEta_leadingB_Wlep", "M_{(most forward jet + leading b-jet + lepton)} (GeV)", 30, 0, 600),
        std::make_tuple("m_highestEta_leadingB_WZ", "M_{(most forward jet + leading b-jet + WZ)} (GeV)", 30, 0, 600),
        std::make_tuple("m_highestEta_leadingB_WlepZ", "M_{(most forward jet + leading b-jet + lepton + Z)} (GeV)", 30, 0, 600),
        std::make_tuple("m_taggedRecoil_taggedB_W", "M_{(recoiling jet + tagged b-jet + W)} (GeV)", 30, 0, 600),
        std::make_tuple("m_taggedRecoil_taggedB_Wlep", "M_{(recoiling jet + tagged b-jet + lepton)} (GeV)", 30, 0, 600),
        std::make_tuple("m_taggedRecoil_taggedB_WZ", "M_{(recoiling jet + tagged b-jet + WZ)} (GeV)", 30, 0, 600),
        std::make_tuple("m_taggedRecoil_taggedB_WlepZ", "M_{(recoiling jet + tagged b-jet + lepton + Z)} (GeV)", 30, 0, 600),
        std::make_tuple("m_forwardJets_leadingB_W", "M_{(forward jets + leading b-jet + W)} (GeV)", 30, 0, 600),
        std::make_tuple("m_forwardJets_leadingB_Wlep", "M_{(forward jets + leading b-jet + lepton)} (GeV)", 30, 0, 600),
        std::make_tuple("m_forwardJets_leadingB_WZ", "M_{(forward jets + leading b-jet + WZ)} (GeV)", 30, 0, 600),
        std::make_tuple("m_forwardJets_leadingB_WlepZ", "M_{(forward jets + leading b-jet + lepton + Z)} (GeV)", 30, 0, 600),
        std::make_tuple("m_forwardJets", "M_{(|#eta| > 2.4 jets)} (GeV)", 30, 0, 600),
        std::make_tuple("m_notSoForwardJets", "M_{(|#eta| > 0.8 jets)} (GeV)", 30, 0, 600),
        std::make_tuple("m_superForwardJets", "M_{(|#eta| > 3 jets)} (GeV)", 30, 0, 600),

        std::make_tuple("pT_highestEta_leadingB_W", "P_{T}^{most forward jet + leading b-jet + W} (GeV)", 30, 0, 600),
        std::make_tuple("pT_highestEta_leadingB_Wlep", "P_{T}^{most forward jet + leading b-jet + lepton} (GeV)", 30, 0, 600),
        std::make_tuple("pT_highestEta_leadingB_WZ", "P_{T}^{most forward jet + leading b-jet + WZ} (GeV)", 30, 0, 600),
        std::make_tuple("pT_highestEta_leadingB_WlepZ", "P_{T}^{most forward jet + leading b-jet + lepton + Z} (GeV)", 30, 0, 600),
        std::make_tuple("pT_taggedRecoil_taggedB_W", "P_{T}^{recoiling jet + tagged b-jet + W} (GeV)", 30, 0, 600),
        std::make_tuple("pT_taggedRecoil_taggedB_Wlep", "P_{T}^{recoiling jet + tagged b-jet + lepton} (GeV)", 30, 0, 600),
        std::make_tuple("pT_taggedRecoil_taggedB_WZ", "P_{T}^{recoiling jet + tagged b-jet + WZ} (GeV)", 30, 0, 600),
        std::make_tuple("pT_taggedRecoil_taggedB_WlepZ", "P_{T}^{recoiling jet + tagged b-jet + lepton + Z} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets_leadingB_W", "P_{T}^{forward jets + leading b-jet + W} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets_leadingB_Wlep", "P_{T}^{forward jets + leading b-jet + lepton} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets_leadingB_WZ", "P_{T}^{forward jets + leading b-jet + WZ} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets_leadingB_WlepZ", "P_{T}^{forward jets + leading b-jet + lepton + Z} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets", "P_{T}^{|#eta| > 2.4 jets} (GeV)", 30, 0, 600),
        std::make_tuple("pT_notSoForwardJets", "P_{T}^{|#eta| > 0.8 jets} (GeV)", 30, 0, 600),
        std::make_tuple("pT_superForwardJets", "P_{T}^{|#eta| > 3 jets} (GeV)", 30, 0, 600),

        std::make_tuple("eta_forwardJets", "|#eta|^{|#eta| > 2.4 jets}", 30, 0, 5),
        std::make_tuple("eta_notSoForwardJets", "|#eta|^{|#eta| > 0.8 jets}", 30, 0, 5),
        std::make_tuple("eta_superForwardJets", "|#eta|^{|#eta| > 3 jets}", 30, 0, 5),

        std::make_tuple("highestDeepCSV", "highest deepCSV", 30, 0, 1), 
        std::make_tuple("highestCSVv2", "highest CSVv2", 30, 0, 1),
        std::make_tuple("highestDeepCSVJetPt", "P_{T}(highest DeepCSV jet) (GeV)", 30, 0, 300),
        std::make_tuple("deltaRTaggedBJetRecoilingJet", "#DeltaR(tagged b-jet, recoiling jet)", 30, 0, 10),
        std::make_tuple("deltaRleadingBJet_highestEtaJet", "#DeltaR(leading b-jet, most forward jet)", 30, 0, 10),
        std::make_tuple("deltaRhighestDeepCSVJet_highestEtaJet", "#DeltaR(highest DeepCSV jet, most forward jet)", 30, 0, 10),
    };

    const unsigned nDist = histInfo.size(); //number of distributions to plot
    const unsigned nCat = 7;                //Several categories enriched in different processes
    const unsigned nMll = 3;                //categories based on dilepton Mass
    const std::string mllNames[nMll] = {"mllInclusive", "onZ", "offZ"};
    const std::string catNames[nCat] = {"nJetsInclusive", "0bJets_01Jets", "0bJets_2Jets", "1bJet_01jets", "1bJet_23Jets", "1bJet_4Jets", "2bJets"};
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

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        if(sam == 0){                   //skip data for now
            ++currentSample;
            continue;
        }
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
            if(tightLepCount(ind, lCount) != 3) continue; //require 3 tight leptons
            if(!passPtCuts(ind)) continue;
            //require presence of OSSF pair
            if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 
            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
            //require best Z mass to be onZ
            std::pair<unsigned, unsigned> bestZ = trilep::bestZ(lepV, lCount);
            //make ordered jet and bjet collections
            std::vector<unsigned> jetInd, bJetInd;
            unsigned jetCount = nJets(jetInd);
            unsigned bJetCount = nBJets(bJetInd);
            if(nBJets() != nBJets(bJetInd) ) std::cout << "bug in number of bjets!" << std::endl;
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
            double mTop = tzq::findMTop(lepV[lw], met, taggedJetI, jetInd, bJetInd, jetV);

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
            //super forward jet sum
            TLorentzVector superForwardJets(0,0,0,0);
            for(unsigned j = 0; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) >= 3.0){
                    superForwardJets += jetV[jetInd[j]];
                }
            }
            
            //find jets with highest DeepCSV and CSVv2 values
            unsigned highestDeepCSVI = (jetCount == 0) ? 0 : jetInd[0], highestCSVv2I = (jetCount == 0) ? 0 : jetInd[0];
            for(unsigned j = 1; j < jetCount; ++j){
                if( (_jetDeepCsv_b[jetInd[j]] + _jetDeepCsv_bb[jetInd[j]]) > (_jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI]) ) highestDeepCSVI = jetInd[j];
                if( _jetCsvV2[jetInd[j]] > _jetCsvV2[jetInd[j]] ) highestCSVv2I = jetInd[j];
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
            TLorentzVector highestCSVv2Jet(0,0,0,0);
            if(taggedJetI[0] != 99) taggedBJet = jetV[taggedJetI[0]];
            if(taggedJetI[1] != 99) recoilingJet = jetV[taggedJetI[1]];
            if(jetCount != 0){
                leadingJet = jetV[jetInd[0]];
                trailingJet = jetV[jetInd[jetInd.size() - 1]];
                highestEtaJet = jetV[highestEtaJ];
                highestDeepCSVJet = jetV[highestDeepCSVI];
                highestCSVv2Jet = jetV[highestCSVv2I]; 
            }
            if(bJetCount != 0){
                leadingBJet = jetV[bJetInd[0]];
                if(bJetCount > 1) trailingBJet = jetV[bJetInd[bJetInd.size() - 1]];
            } else if(jetCount > 1){
                leadingBJet = jetV[jetInd[1]];
            }

            //distributions to plot
            double fill[nDist] = {_met, mll, tools::mt(lepV[lw], met),  _lPt[ind[0]], _lPt[ind[1]], _lPt[ind[2]], (double) nJets(), (double) nBJets(), 
            (double) nBJets(0, false), fabs(highestEtaJet.Eta()), fabs(leadingJet.Eta()), leadingJet.Pt(), trailingJet.Pt(), leadingBJet.Pt(), trailingBJet.Pt(),
             highestEtaJet.Pt(), mTop, taggedBJet.Pt(), fabs(taggedBJet.Eta()), recoilingJet.Pt(), fabs(recoilingJet.Eta()),

            (highestEtaJet + leadingBJet + lepV[lw] + met).M(),
            (highestEtaJet + leadingBJet + lepV[lw]).M(),
            (highestEtaJet + leadingBJet + lepV[lw] + met + lepV[bestZ.first] + lepV[bestZ.second] ).M(),
            (highestEtaJet + leadingBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).M(),

            (recoilingJet + taggedBJet + lepV[lw] + met).M(),
            (recoilingJet + taggedBJet + lepV[lw]).M(),
            (recoilingJet + taggedBJet + lepV[lw] + met + lepV[bestZ.first] + lepV[bestZ.second]).M(),
            (recoilingJet + taggedBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).M(),

            (forwardJets + leadingBJet + lepV[lw] + met).M(),
            (forwardJets + leadingBJet + lepV[lw]).M(),
            (forwardJets + leadingBJet + lepV[lw] + met + lepV[bestZ.first] + lepV[bestZ.second]).M(),
            (forwardJets + leadingBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).M(),

            forwardJets.M(),
            notSoForwardJets.M(),
            superForwardJets.M(),

            (highestEtaJet + leadingBJet + lepV[lw] + met).Pt(),
            (highestEtaJet + leadingBJet + lepV[lw]).Pt(),
            (highestEtaJet + leadingBJet + lepV[lw] + met + lepV[bestZ.first] + lepV[bestZ.second] ).Pt(),
            (highestEtaJet + leadingBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),

            (recoilingJet + taggedBJet + lepV[lw] + met).Pt(),
            (recoilingJet + taggedBJet + lepV[lw]).Pt(),
            (recoilingJet + taggedBJet + lepV[lw] + met + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),
            (recoilingJet + taggedBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),

            (forwardJets + leadingBJet + lepV[lw] + met).Pt(),
            (forwardJets + leadingBJet + lepV[lw]).Pt(),
            (forwardJets + leadingBJet + lepV[lw] + met + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),
            (forwardJets + leadingBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),

            forwardJets.Pt(),
            notSoForwardJets.Pt(),
            superForwardJets.Pt(),
    
            fabs(forwardJets.Eta()),
            fabs(notSoForwardJets.Eta()),
            fabs(superForwardJets.Eta()),

            _jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_b[highestDeepCSVI],
            _jetCsvV2[highestCSVv2I],
            highestDeepCSVJet.Pt(),
            taggedBJet.DeltaR(recoilingJet),
            leadingBJet.DeltaR(highestEtaJet),
            highestDeepCSVJet.DeltaR(highestEtaJet)
            };
            for(unsigned m = 0; m < nMll; ++m){
                if(m == 0 || m == (mllCat + 1) ){
                    for(unsigned cat = 0; cat < nCat; ++cat){
                        if(cat == 0 || cat == (tzqCat + 1) ){
                            for(unsigned dist = 0; dist < nDist; ++dist){
                                hists[m][cat][dist][sam]->Fill(std::min(fill[dist], maxBin[dist]), weight);
                            }
                        }
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

    const bool isSMSignal[ (const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
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
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_" + catNames[cat] + "_" + mllNames[m], "tzq", false, false, "", nullptr, isSMSignal);             //linear plots
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_" + catNames[cat] + "_" + mllNames[m] + "_withSignal", "tzq", false, false, "", nullptr, isSMSignal, &signal[m][cat][dist], sigNames, 1);             //linear plots with signal

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_"  + catNames[cat] + "_" + mllNames[m] + "_log", "tzq", true, false, "", nullptr, isSMSignal);    //log plots
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_"  + catNames[cat] + "_" + mllNames[m] + "_withSignal_log", "tzq", true, false, "", nullptr, isSMSignal, &signal[m][cat][dist], sigNames, 1);    //log plots with signal
            }
        }
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
