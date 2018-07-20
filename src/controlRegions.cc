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
        //HistInfo("bdt", "BDT output", 10, -1, 1)
        HistInfo("taggedRecoilJetEta", "|#eta| (recoiling jet) (GeV)", 20, 0, 5),
        HistInfo("maxMJetJet", "M_{jet + jet}^{max} (GeV)", 20, 0, 1200),
        HistInfo("asymmetryWLep", "asymmetry (lepton from W)",20, -2.5, 2.5),
        HistInfo("highestDeepCSV", "highest deepCSV (b + bb)", 20, 0, 1),
        HistInfo("LTPlusMET", "L_{T} + E_{T}^{miss} (GeV)", 20, 0, 800),
        HistInfo("maxDeltaPhiJetJet", "max(#Delta#Phi(jet, jet))", 20, 0, 3.15),
        HistInfo("mt", "M_{T} (GeV)", 20, 0, 300),
        HistInfo("mtop", "M_{(W + b)} (GeV)", 20, 0, 400),
        HistInfo("maxpTJetJet", "P_{T}^{max}(jet + jet) (GeV)", 20, 0, 300),
        HistInfo("minDeltaPhiLeptonBJet", "min(#Delta#Phi(l, bjet))", 20, 0, 3.15),
        HistInfo("maxDeltaPhiLeptonLepton", "max(#Delta#Phi(l, l))", 20, 0, 3.15),
        HistInfo("HT", "H_{T} (GeV)", 20, 0, 800),
        HistInfo("nJets", "number of jets", 10, 0, 10),
        HistInfo("deltaRTaggedBJetRecoilingJet", "#DeltaR(tagged b-jet, recoiling jet)", 20, 0, 10),
        HistInfo("deltaRWlepTaggedbJet", "#DeltaR(lepton from W, tagged b-jet)", 20, 0, 7),
        HistInfo("m3l", "M_{3l} (GeV)", 20, 0, 600),
        HistInfo("jetEta_highestEta", "|#eta| (most forward jet)", 20, 0, 5),
        HistInfo("nbJets", "number of b-jets", 8, 0, 8),
        HistInfo("met", "E_{T}^{miss} (GeV)", 20, 0, 300),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 20, 25, 200),
        HistInfo("subPt", "P_{T}^{subleading} (GeV)", 20, 15, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 20, 10, 200),
        HistInfo("etaLeading", "|#eta| (leading lepton)", 30, -2.5, 2.5),
        HistInfo("etaSubLeading", "|#eta| (subleading lepton)", 30, -2.5, 2.5),
        HistInfo("etaTrailing", "|#eta| (trailing lepton)", 30, -2.5, 2.5),
        HistInfo("nVertices", "number of vertices", 20, 0, 60),
        HistInfo("flavors", "flavors", 5, 0, 5)
    };

    const unsigned nDist = histInfo.size(); //number of distributions to plot
    const unsigned nCr = 3;                 //Several categories enriched in different processes
    const std::string crNames[nCr] = {"WZ", "Xgamma", "ZZ"};

    //initialize vector holding all histograms
    std::vector< std::vector < std::vector< std::shared_ptr< TH1D > > > > hists(nCr);
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            hists[cr].push_back(std::vector < std::shared_ptr< TH1D > >() );
            for(size_t sam = 0; sam < samples.size(); ++sam){
                hists[cr][dist].push_back(histInfo[dist].makeHist(crNames[cr] + samples[sam].getUniqueName()) );
            }
        }
    }

    //tweakable options
    const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){

        //if( currentSample.getProcessName() != "WZ" ) continue;

        initSample();
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
            const unsigned lCount = selectLep(ind);


            if( !(lCount == 3 || lCount == 4) ) continue;
    
            unsigned lCountTight = tightLepCount(ind, lCount);

            if( !(lCountTight == lCount) ) continue; //require 3 tight leptons

            //WARNING  : REMOVE AFTER SYNC
            //ask exactly 4 loose leptons for sync
            unsigned looseCount = 0;
            for(unsigned l = 0; l < _nLight; ++l){
                if( closestJetDeepCSV(l) > ( is2016() ? 0.8958 : 0.8001) ) continue;
                if( isMuon(l) && !_lPOGMedium[l]) continue;
                if( lepIsLoose(l) ) ++looseCount;
            }
            if(looseCount != lCount) continue;
           
            //require pt cuts (25, 15, 10) to be passed
            if(!passPtCuts(ind)) continue;

            //remove overlap between samples
            if(photonOverlap(currentSample)) continue;

            //require presence of OSSF pair
            if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 
            
            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            TLorentzVector lepSyst(0,0,0,0);    //vector sum of all leptons
            for(unsigned l = 0; l < lCount; ++l){
                lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
                lepSyst += lepV[l];
            }
            double m3l = lepSyst.M();

            //require best Z mass to be onZ
            std::pair<unsigned, unsigned> bestZ = trilep::bestZ(lepV, ind, _lFlavor, _lCharge, lCount);
            double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();

            //control region categorization
            unsigned controlRegion = tzq::controlRegion(lCount, mll, m3l);

            //reject events falling out of the control regions
            if( !tzq::isControlRegion(controlRegion) ) continue;

            //make ordered jet and bjet collections and count jet numbers
            std::vector<unsigned> jetInd, bJetInd;
            unsigned jetCount = nJets(jetInd);
            unsigned bJetCount = nBJets(bJetInd);

            //control region specific event selection
            if( tzq::isWZControlRegion(controlRegion) ){

                //WARNING: remove this cut here when moving to full systematics
                if(_met < 50) continue;
                //WARNING: remove this cut here when moving to full systematics
                if(bJetCount != 0) continue;

            } else if ( tzq::isXgammaControlRegion(controlRegion) ){

                //WARNING: remove this cut here when moving to full systematics 

            } else if ( tzq::isZZControlRegion(controlRegion) ){

                //find leptons not making best Z pair
                std::pair<unsigned, unsigned> secondZ = {99, 99};
                for(unsigned l = 0; l < lCount; ++l){
                    if( (l != bestZ.first) && (l != bestZ.second) ){
                        if(secondZ.first == 99){
                            secondZ.first = l;
                        } else {
                            secondZ.second = l;
                        }
                    }
                } 

                //check presence of second Z in the event
                if( _lCharge[ind[secondZ.first]] == _lCharge[ind[secondZ.second]]) continue;
                if( _lFlavor[ind[secondZ.first]] != _lFlavor[ind[secondZ.second]]) continue;

                double mllSecond = (lepV[secondZ.first] + lepV[secondZ.second]).M();
                if( fabs(mllSecond - 91.1876) > 15) continue;

            } else {
                std::cerr << "Error: controlRegion number does not match any known control region." << std::endl;
            }

            //apply event weight
            if( isMC() ){
                weight*=sfWeight();
            }
        
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
            unsigned counter = 0;
            for(unsigned j = 0; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) < 2.4) {
                    if( (counter == 0) || ( deepCSV( jetInd[j] ) > deepCSV(highestDeepCSVI) ) ) highestDeepCSVI = jetInd[j];
                    ++counter;
                }
            } 

            //find highest eta jet
            unsigned highestEtaJ = (jetCount == 0) ? 99 : jetInd[0];
            for(unsigned j = 1; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) > fabs(_jetEta[highestEtaJ]) ) highestEtaJ = jetInd[j];
            }

            
            //initialize new vectors to make sure everything is defined for 0 jet events!
            TLorentzVector recoilingJet(0,0,0,0);
            TLorentzVector taggedBJet(0,0,0,0);
            TLorentzVector highestDeepCSVJet(0,0,0,0);
            TLorentzVector highestEtaJet(0,0,0,0);
            if(taggedJetI[0] != 99) taggedBJet = jetV[taggedJetI[0]];
            if(taggedJetI[1] != 99) recoilingJet = jetV[taggedJetI[1]];
            if(jetCount != 0){
                if( highestDeepCSVI != 99 ) highestDeepCSVJet = jetV[highestDeepCSVI];
                highestEtaJet = jetV[highestEtaJ];
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
            
            //variables to fill
            double fill[nDist] = { fabs(recoilingJet.Eta()), 
                maxMJetJet,
                fabs(lepV[lw].Eta())*_lCharge[ind[lw]],
                (jetCount == 0 || highestDeepCSVI != 99) ? 0. : deepCSV(highestDeepCSVI),
                LT + _met,
                maxDeltaPhiJetJet,
                kinematics::mt(lepV[lw], met),
                topV.M(),
                maxpTJetJet,
                minDeltaPhiLeptonbJet,
                maxDeltaPhiLeptonLepton,
                HT,
                (double) jetCount,   
                taggedBJet.DeltaR(recoilingJet),
                lepV[lw].DeltaR(taggedBJet),
                m3l,
                fabs(highestEtaJet.Eta()),
                (double) bJetCount,
                _met,
                _lPt[ind[0]],
                _lPt[ind[1]],
                _lPt[ind[2]],
                _lEta[ind[0]],
                _lEta[ind[1]],
                _lEta[ind[2]],
                (double) _nVertex,
                (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5

            };

            //fill histograms
            for(unsigned dist = 0; dist < nDist; ++dist){
                hists[controlRegion][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

        }
        //set histograms to 0 if negative
        for(unsigned cr = 0; cr < nCr; ++cr){
            for(unsigned dist = 0; dist < nDist; ++dist){
                tools::setNegativeZero( hists[cr][dist][sam].get() );
            }	
        }
    }

    //merge histograms with the same physical background
    std::vector<std::string> proc = {"total bkg.", "tZq", "DY", "TT + Jets", "WZ", "multiboson", "TT + Z", "TT/T + X", "X + #gamma", "ZZ/H"};
    std::vector< std::vector< std::vector< TH1D* > > > mergedHists(nCr);
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            mergedHists[cr].push_back(std::vector<TH1D*>(proc.size() ) );
            for(size_t m = 0, sam = 0; m < proc.size(); ++m){
                mergedHists[cr][dist][m] = (TH1D*) hists[cr][dist][sam]->Clone();
                while(sam < samples.size() - 1 && samples[sam].getProcessName() == samples[sam + 1].getProcessName() ){
                    mergedHists[cr][dist][m]->Add(hists[cr][dist][sam + 1].get());
                    ++sam;
                }
                ++sam;
            }
        }
    }

    //plot all distributions
    const bool isSMSignal[(const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/2016/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_" + crNames[cr] + "_2016", "tzq", false, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);             //linear plots

            plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/2016/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_"  + crNames[cr] + "_2016" + "_log", "tzq", true, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);    //log plots
        }
    }
    /*    
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
    */
}
int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
