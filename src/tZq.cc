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
#include "../interface/HistInfo.h"
#include "../interface/HistCollectionDist.h"
#include "../interface/HistCollectionSample.h"
#include "../interface/kinematicTools.h"
#include "../interface/TrainingTree.h"
#include "../interface/BDTReader.h"
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
    readSamples("sampleLists/samples2016.txt");
    //name      xlabel    nBins,  min, max
    histInfo = {
        //new BDT distribution
        HistInfo("bdt", "BDT output", 30, -1, 1),
        HistInfo("bdt_10bins", "BDT output", 10, -1, 1)
    };

    const unsigned nDist = histInfo.size(); //number of distributions to plot
    const unsigned nCat = 3;                //Several categories enriched in different processes
    const unsigned nMll = 1;                //categories based on dilepton Mass
    const std::string mllNames[nMll] = {"onZ"};
    const std::string catNames[nCat] = {"1bJet_23Jets", "1bJet_4Jets", "2bJets"};
    //initialize vector holding all histograms
    std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > > hists(nMll);
    for(unsigned m = 0; m < nMll; ++m){
        hists[m] = std::vector< std::vector < std::vector< std::shared_ptr< TH1D > > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                hists[m][cat].push_back(std::vector < std::shared_ptr< TH1D > >() );
                for(size_t sam = 0; sam < samples.size(); ++sam){
                    hists[m][cat][dist].push_back(histInfo[dist].makeHist(catNames[cat] + mllNames[m] + samples[sam].getUniqueName()) );
                    /*
                        hists[m][cat][dist][sam] = new TH1D( (const TString&) (samples[sam].getUniqueName() + histInfo[dist].name() + catNames[cat] + mllNames[m]), (const TString&) ( samples[sam].getUniqueName() + histInfo[dist].name() + catNames[cat] + mllNames[m] + ";" + std::get<1>(histInfo[dist]) + ";Events" ),  histInfoDist, std::get<3>(histInfo[dist]), std::get<4>(histInfo[dist]) );
                    */
                }
            }
        }
    }

    std::map < std::string, float > bdtVariableMap =
        {
            {"etaRecoilingJet", 0.},
            {"maxMjj", 0.},
            {"asymmetryWlep", 0.},
            {"highestDeepCSV", 0.},
            {"ltmet", 0.},
            {"maxDeltaPhijj", 0.},
            {"mTW", 0.},
            {"topMass", 0.}, 
            {"pTMaxjj", 0.}, 
            {"minDeltaPhilb", 0.},
            {"maxDeltaPhill", 0.},
            {"ht", 0.},
            {"deltaRTaggedbJetRecoilingJet", 0.},
            {"deltaRWLeptonTaggedbJet", 0.},
            {"m3l", 0.},
            {"etaMostForward", 0.},
            {"numberOfJets", 0.}
        };

    //BDT reader for 1bJet23Jets category
    std::vector < std::string > bdtVars1bJet23Jets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "maxDeltaPhijj", "mTW", "topMass", "pTMaxjj", "minDeltaPhilb", "maxDeltaPhill"};
    std::string weights;
    if( is2016() ){
        weights = "1bJet23Jets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    } else if( is2017() ){
        weights = "1bJet23Jets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    }
    BDTReader bdtReader1bJet23Jets("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars1bJet23Jets);

    //BDT reader for 1bJet4Jets category
    if( is2016() ){
        weights = "1bJet4Jets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    } else if( is2017() ){
        weights = "1bJet4Jets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    }
    std::vector < std::string > bdtVars1bJet4Jets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "ht", "mTW", "topMass", "numberOfJets", "maxDeltaPhill", "maxDeltaPhijj", "minDeltaPhilb",
        "deltaRTaggedbJetRecoilingJet", "deltaRWLeptonTaggedbJet", "m3l", "etaMostForward"};
    BDTReader bdtReader1bJet4Jets("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars1bJet4Jets);

    //BDT reader for 2bJets category
    if( is2016() ){
        weights = "2bJets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    } else if( is2017() ){
        weights = "2bJets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    }
    std::vector < std::string > bdtVars2bJets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "ht", "mTW", "topMass", "numberOfJets", "maxDeltaPhill", "maxDeltaPhijj", "etaMostForward", "m3l"}; 
    BDTReader bdtReader2bJets("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars2bJets);


    //tweakable options
    const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        if(sam == 0){                   //skip data for now
            ++currentSampleIndex;
            continue;
        }
        initSample();          //2 = combined luminosity
        std::cout<<"Entries in "<< currentSample.getFileName() << " " << nEntries << std::endl;
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

            //apply triggers and MET filters
            if( !passTriggerCocktail() ) continue;
            if( !passMETFilters() ) continue;

            //vector containing good lepton indices
            std::vector<unsigned> ind;

            //select leptons
            //const unsigned lCount = selectLep(ind);
            //if(lCount != 3) continue;
            //if(tightLepCount(ind, lCount) != 3) continue; //require 3 tight leptons

            //WARNING, REMOVE THIS AND TAKE PREVIOUS LINES BACK IN
            //Testing with TOP-16-0-200 selection
            const unsigned lCount = selectLep_TOP16_020(ind);
            if(lCount != 3) continue;

            //require pt cuts (25, 15, 10) to be passed
            if(!passPtCuts(ind)) continue;

            //remove overlap between samples
            if(photonOverlap(currentSample)) continue;

            //require presence of OSSF pair
            if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 

            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);

            //require best Z mass to be onZ
            std::pair<unsigned, unsigned> bestZ = trilep::bestZ(lepV, ind, _lFlavor, _lCharge, lCount);
            double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();
            if( fabs(mll - 91.1876) >= 15) continue;
            unsigned mllCat = 0;

            //make ordered jet and bjet collections
            std::vector<unsigned> jetInd, bJetInd;
            unsigned jetCount = nJets(jetInd);
            unsigned bJetCount = nBJets(bJetInd);

            //Determine tZq analysis category
            unsigned tzqCat = tzq::cat(jetCount, bJetCount);

            //only keep search categories
            if(tzqCat < 3) continue;
         
            //find highest eta jet
            unsigned highestEtaJ = (jetCount == 0) ? 99 : jetInd[0];
            for(unsigned j = 1; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) > fabs(_jetEta[highestEtaJ]) ) highestEtaJ = jetInd[j];
            }
            
            //apply event weight
            //weight*=sfWeight();

            //make LorentzVector for all jets 
            TLorentzVector jetV[(const unsigned) _nJets];
            for(unsigned j = 0; j < _nJets; ++j){
                jetV[j].SetPtEtaPhiE(_jetPt[j], _jetEta[j], _jetPhi[j], _jetE[j]);
            }

            //find W lepton 
            unsigned lw = 99;
            for(unsigned l = 0; l < lCount; ++l){
                if( l != bestZ.first && l != bestZ.second ) lw = l;
            }

            //make met vector 
            TLorentzVector met;
            met.SetPtEtaPhiE(_met, _metPhi, 0, _met);

            //reconstruct top mass and tag jets
            std::vector<unsigned> taggedJetI; //0 -> b jet from tZq, 1 -> forward recoiling jet
            TLorentzVector neutrino = tzq::findBestNeutrinoAndTop(lepV[lw], met, taggedJetI, jetInd, bJetInd, jetV);
            
            //find jets with highest DeepCSV and CSVv2 values
            unsigned highestDeepCSVI = 99;
            unsigned highestCSVv2I = 99;
            unsigned counter = 0;
            for(unsigned j = 0; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) < 2.4) {
                    if( (counter == 0) || ( (_jetDeepCsv_b[jetInd[j]] + _jetDeepCsv_bb[jetInd[j]]) > (_jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI]) ) ) highestDeepCSVI = jetInd[j];
                    if( (counter == 0) || ( _jetCsvV2[jetInd[j]] > _jetCsvV2[highestCSVv2I] ) ) highestCSVv2I = jetInd[j];
                    ++counter;
                }
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
                if(jetCount > 1) trailingJet = jetV[jetInd[jetInd.size() - 1]];
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

            //compute top vector
            TLorentzVector topV = (neutrino + lepV[lw] + taggedBJet); 

            //Compute minimum and maximum masses and separations for several objects
            //initialize lepton indices 
            std::vector<unsigned> lepVecInd;
            for(unsigned l = 0; l < lCount; ++l) lepVecInd.push_back(l);


            //lepton bjet
            double minDeltaPhiLeptonbJet = kinematics::minDeltaR(lepV, lepVecInd, jetV, bJetInd);

            //jet jet
            double maxMJetJet = kinematics::maxMass(jetV, jetInd);
            double maxDeltaPhiJetJet = kinematics::maxDeltaPhi(jetV, jetInd);
            double maxpTJetJet = kinematics::maxPT(jetV, jetInd);

            //lepton lepton 
            double maxDeltaPhiLeptonLepton = kinematics::maxDeltaPhi(lepV, lepVecInd);

            //compute HT
            double HT = 0;
            for(unsigned j = 0; j < jetCount; ++j){
                HT += _jetPt[jetInd[j]];
            }

            double LT = 0.;
            for(unsigned l = 0; l < lCount; ++l){
                LT += _lPt[ind[l]];
            }
            
            bdtVariableMap["etaRecoilingJet"] = fabs(recoilingJet.Eta());
            bdtVariableMap["maxMjj"] = std::max(maxMJetJet, 0.);
            bdtVariableMap["asymmetryWlep"] = fabs(_lEta[ind[lw]])*_lCharge[ind[lw]];
            bdtVariableMap["highestDeepCSV"] = (jetCount == 0) ? 0. : _jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI];
            bdtVariableMap["ltmet"] = LT + _met;
            bdtVariableMap["maxDeltaPhijj"] = maxDeltaPhiJetJet;
            bdtVariableMap["mTW"] = kinematics::mt(lepV[lw], met);
            bdtVariableMap["topMass"] =  std::max(topV.M(), 0.);
            bdtVariableMap["pTMaxjj"] =  maxpTJetJet;
            bdtVariableMap["minDeltaPhilb"] = minDeltaPhiLeptonbJet;
            bdtVariableMap["maxDeltaPhill"] = maxDeltaPhiLeptonLepton;
            bdtVariableMap["ht"] = HT;
            bdtVariableMap["deltaRTaggedbJetRecoilingJet"] = taggedBJet.DeltaR(recoilingJet);
            bdtVariableMap["deltaRWLeptonTaggedbJet"] = lepV[lw].DeltaR(taggedBJet);
            bdtVariableMap["m3l"] = (lepV[0] + lepV[1] + lepV[2]).M();
            bdtVariableMap["etaMostForward"] = fabs(highestEtaJet.Eta());
            bdtVariableMap["numberOfJets"] = jetCount;

            double bdt = 0;
            if(tzqCat == 3){
                bdt = bdtReader1bJet23Jets.computeBDT(bdtVariableMap);
            } else if(tzqCat == 4){
                bdt = bdtReader1bJet4Jets.computeBDT(bdtVariableMap);
            } else if(tzqCat == 5){
                bdt = bdtReader2bJets.computeBDT(bdtVariableMap);
            }

            double fill[nDist] = {bdt, bdt};

            for(unsigned dist = 0; dist < nDist; ++dist){
                hists[mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }
        }
        //set histograms to 0 if negative
        for(unsigned m = 0; m < nMll; ++m){
            for(unsigned cat = 0; cat < nCat; ++cat){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    tools::setNegativeZero( hists[m][cat][dist][sam].get() );
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
                    while(sam < samples.size() - 1 && samples[sam].getProcessName() == samples[sam + 1].getProcessName() ){
                        mergedHists[mll][cat][dist][m]->Add(hists[mll][cat][dist][sam + 1].get());
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
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/final/" + histInfo[dist].name() + "_" + catNames[cat] + "_" + mllNames[m] + "_2016", "tzq", false, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);             //linear plots

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/final/" + histInfo[dist].name() + "_"  + catNames[cat] + "_" + mllNames[m] + "_2016" + "_log", "tzq", true, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);    //log plots
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
                            for(int bin = 1; bin < mergedHists[m][cat][0][p]->GetNbinsX() + 1; ++bin){
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
