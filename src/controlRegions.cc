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
    readSamples("sampleLists/samples_nonpromptDataDriven_2016.txt");
    //name      xlabel    nBins,  min, max
    histInfo = {
        //new BDT distribution
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
        HistInfo("deltaRTaggedBJetRecoilingJet", "#DeltaR(tagged b-jet, recoiling jet)", 20, 0, 10),
        HistInfo("deltaRWlepTaggedbJet", "#DeltaR(lepton from W, tagged b-jet)", 20, 0, 7),
        HistInfo("m3l", "M_{3l} (GeV)", 20, 0, 600),
        HistInfo("jetEta_highestEta", "|#eta| (most forward jet)", 20, 0, 5),
        HistInfo("nJets", "number of jets", 10, 0, 10),
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
            for(size_t sam = 0; sam < samples.size() + 1; ++sam){
                if(sam < samples.size()){
                    hists[cr][dist].push_back(histInfo[dist].makeHist(crNames[cr] + samples[sam].getUniqueName()) );
                } else{
                    hists[cr][dist].push_back(histInfo[dist].makeHist(crNames[cr] + "nonprompt") );
                }
            }
        }
    }

    const std::vector< std::string > uncNames = {"JEC", "uncl", "scale", "pileup", "bTag_udsg", "bTag_bc", "pdf", "scaleXsec", "pdfXsec"};
    std::map < std::string, std::vector< std::vector< std::vector< std::shared_ptr< TH1D > > > > > uncHistMapDown;
    std::map < std::string, std::vector< std::vector< std::vector< std::shared_ptr< TH1D > > > > > uncHistMapUp;
    for( auto& key : uncNames ){
        uncHistMapDown[key] = std::vector< std::vector < std::vector< std::shared_ptr< TH1D > > > >(nCr);
        uncHistMapUp[key] = std::vector< std::vector < std::vector< std::shared_ptr< TH1D > > > >(nCr);
        for(unsigned cr = 0; cr < nCr; ++cr){
            uncHistMapDown[key][cr] = std::vector< std::vector < std::shared_ptr< TH1D > > >(nDist);
            uncHistMapUp[key][cr] = std::vector< std::vector < std::shared_ptr< TH1D > > >(nDist);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[key][cr][dist] = std::vector< std::shared_ptr< TH1D > >(samples.size() + 1);
                uncHistMapUp[key][cr][dist] = std::vector< std::shared_ptr< TH1D > >(samples.size() + 1);
                for(size_t sam = 0; sam < samples.size() + 1; ++sam){
                    std::string sampleName;
                    if(sam < samples.size() ){
                        sampleName = samples[sam].getUniqueName();
                    } else {
                        //the nonprompt prediction is also affected by all nuisances through the prompt MC subtraction
                        sampleName = "nonprompt";
                    }
                    uncHistMapDown[key][cr][dist][sam] = histInfo[dist].makeHist(crNames[cr] + sampleName + key + "Down");
                    uncHistMapUp[key][cr][dist][sam] = histInfo[dist].makeHist(crNames[cr] + sampleName + key + "Up");
                }
            }
        }
    }
    
    std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > > pdfUncHists(100);
    for(unsigned pdf = 0; pdf < 100; ++pdf){
        pdfUncHists[pdf] = std::vector< std::vector< std::vector< std::shared_ptr< TH1D > > > >(nCr);
        for(unsigned cr = 0; cr < nCr; ++cr){
            pdfUncHists[pdf][cr] = std::vector < std::vector< std::shared_ptr< TH1D > > > (nDist); 
            for(unsigned dist = 0; dist < nDist; ++dist){
                pdfUncHists[pdf][cr][dist] = std::vector< std::shared_ptr< TH1D > >( samples.size() + 1);
                for(size_t sam = 0; sam < samples.size() + 1; ++sam){
                    std::string sampleName;
                    if(sam < samples.size() ){
                        sampleName = samples[sam].getUniqueName();
                    } else {
                        sampleName = "nonprompt";
                    } 
                    pdfUncHists[pdf][cr][dist][sam] = histInfo[dist].makeHist(crNames[cr] + sampleName + "pdf" + std::to_string(pdf) );
                }
            }
        }
    }

    //tweakable options
    const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){

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
            const unsigned lCount = selectLepConeCorr(ind);
            if( !(lCount == 3 || lCount == 4) ) continue;

            unsigned lCountTight = tightLepCount(ind, lCount);

            if( !(lCountTight == lCount) ) continue; //require 3 tight leptons
            /*
            //WARNING  : REMOVE AFTER SYNC
            //ask exactly 4 loose leptons for sync
            unsigned looseCount = 0;
            for(unsigned l = 0; l < _nLight; ++l){
                if( closestJetDeepCSV(l) > ( is2016() ? 0.8958 : 0.8001) ) continue;
                if( isMuon(l) && !_lPOGMedium[l]) continue;
                if( lepIsLoose(l) ) ++looseCount;
            }
            if(looseCount != lCount) continue;
            */


            //require pt cuts (25, 15, 10) to be passed
            if(!passPtCuts(ind)) continue;

            //require presence of OSSF pair
            if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 

            //explicitly require selected leptons in MC to be prompt, if one of them is not prompt, reject the event
            if( isMC() && !isSMSignal() ){
                if( !promptLeptons() ) continue;
            } 

            //index to use when filling histogram 
            unsigned fillIndex = sam;
            bool passTightCut = (tightLepCount(ind, lCount) == 3);
            if(!passTightCut){
                //fill last histogram (nonprompt)
                fillIndex = samples.size();

                //apply fake-rate weights 
                weight *= fakeRateWeight(); 
            }

            //remove overlap between samples
            if(photonOverlap(currentSample)) continue;


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

            //control region specific event selection
            if( tzq::isWZControlRegion(controlRegion) ){

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

            //compute nominal values for all search variables
            setSearchVariablestZq("nominal", ind, bestZ, false);             

            //event reweighting
            if( isMC() ){
                weight *= sfWeight(); 
            }

            //compute nominal bdt value
            if( !tzq::isWZControlRegion(controlRegion) || (_met > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _met,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    fabs( _lEta[ind[0]]),
                    fabs( _lEta[ind[1]]),
                    fabs( _lEta[ind[2]]),
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };
                for(unsigned dist = 0; dist < nDist; ++dist){
                    hists[controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
                //in case of data fakes fill all uncertainties for nonprompt with nominal values
                if( isData() && !passTightCut){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        for(auto& key: uncNames){
                            uncHistMapDown[key][controlRegion][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                            uncHistMapUp[key][controlRegion][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                        }
                        for(unsigned pdf = 0; pdf < 100; ++pdf){
                             pdfUncHists[pdf][controlRegion][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                        }
                    }
                }
            }

            //no nuisances for data
            if( isData() ) continue;

            //vary JEC Down
            setSearchVariablestZq("JECDown", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metJECDown > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _metJECDown,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    fabs( _lEta[ind[0]]),
                    fabs( _lEta[ind[1]]),
                    fabs( _lEta[ind[2]]),
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

 
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown["JEC"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
                
            }

            //vary JEC up
            setSearchVariablestZq("JECUp", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metJECUp > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _metJECUp,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    fabs( _lEta[ind[0]]),
                    fabs( _lEta[ind[1]]),
                    fabs( _lEta[ind[2]]),
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp["JEC"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary unclustered down
            setSearchVariablestZq("unclDown", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metUnclDown > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _metUnclDown,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    fabs( _lEta[ind[0]]),
                    fabs( _lEta[ind[1]]),
                    fabs( _lEta[ind[2]]),
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };


                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown["uncl"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary unclustered up
            setSearchVariablestZq("unclUp", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metUnclUp > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _metUnclUp,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    fabs( _lEta[ind[0]]),
                    fabs( _lEta[ind[1]]),
                    fabs( _lEta[ind[2]]),
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp["uncl"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //now nominal cuts can be safely used
            if( tzq::isWZControlRegion(controlRegion) && (_metUnclUp < 50 ||  bdtVariableMap["numberOfbJets"] != 0 ) ) continue;
            
            //reset nominal values 
            setSearchVariablestZq("nominal", ind, bestZ);
            double fill[nDist] = {
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _met,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    fabs( _lEta[ind[0]]),
                    fabs( _lEta[ind[1]]),
                    fabs( _lEta[ind[2]]),
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

            //set pdf and scale weight for GluGluToContinToZZ samples which do not include lhe weights
            if( currentSample.getFileName().find("GluGluToContinToZZ") != std::string::npos ){
                for(unsigned lhe = 0; lhe < 110; ++lhe){
                    _lheWeight[lhe] = 1.;
                }
            }

            //vary scale down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["scale"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[8]);
            }

            //vary scale up
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["scale"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[4]);
            }

            //vary pu down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["pileup"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puWeight(1)/puWeight(0));
            }

            //vary pu up            
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["pileup"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puWeight(2)/puWeight(0));
            }

            //vary b-tag down for udsg
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["bTag_udsg"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_udsg(1)/bTagWeight_udsg(0));
            }

            //vary b-tag up for udsg
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["bTag_udsg"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_udsg(2)/bTagWeight_udsg(0));
            }

            //vary b-tag down for b and c (correlated)
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["bTag_bc"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_c(1)*bTagWeight_b(1)/ (bTagWeight_c(0)*bTagWeight_b(0)) );
            }

            //vary b-tag up for b and c (correlated)
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["bTag_bc"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_c(2)*bTagWeight_b(2)/ (bTagWeight_c(0)*bTagWeight_b(0)) );
            }

            //100 pdf variations
            for(unsigned pdf = 0; pdf < 100; ++pdf){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    pdfUncHists[pdf][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[pdf + 9]);
                }
            }
        }
        //set histograms to 0 if negative
        for(unsigned cr = 0; cr < nCr; ++cr){
            for(unsigned dist = 0; dist < nDist; ++dist){
                tools::setNegativeZero( hists[cr][dist][sam].get() );
                for(auto & key : uncNames){
                    tools::setNegativeZero(uncHistMapDown[key][cr][dist][sam].get() );
                    tools::setNegativeZero(uncHistMapUp[key][cr][dist][sam].get() );
                } 
                for(unsigned pdf = 0; pdf < 100; ++pdf){    
                    tools::setNegativeZero(pdfUncHists[pdf][cr][dist][sam].get() );
                }
            }	
        }
    }

    //set nonprompt bins to 0 if negative
    for(unsigned cr = 0; cr < nCr; ++cr){
       for(unsigned dist = 0; dist < nDist; ++dist){
           unsigned nonpromptIndex = samples.size();
           tools::setNegativeZero( hists[cr][dist][nonpromptIndex].get() );
           for(auto & key : uncNames){
               tools::setNegativeZero(uncHistMapDown[key][cr][dist][nonpromptIndex].get() );
               tools::setNegativeZero(uncHistMapUp[key][cr][dist][nonpromptIndex].get() );
           } 
           for(unsigned pdf = 0; pdf < 100; ++pdf){    
               tools::setNegativeZero(pdfUncHists[pdf][cr][dist][nonpromptIndex].get() );
           }
       }
    
    }

    //compute final pdf uncertainties
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            for(unsigned sam = 0; sam < samples.size() + 1; ++sam){
                uncHistMapDown["pdf"][cr][dist][sam] = histInfo[dist].makeHist(crNames[cr]+ samples[sam].getUniqueName() + "pdfDown");
                uncHistMapUp["pdf"][cr][dist][sam] = histInfo[dist].makeHist(crNames[cr] + samples[sam].getUniqueName() + "pdfUp");
                for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["pdf"][cr][dist][sam]->GetNbinsX() + 1; ++bin){
                    double pdfVarRms = 0.;
                    for(unsigned pdf = 0; pdf < 100; ++pdf){
                        double diff = (  pdfUncHists[pdf][cr][dist][sam]->GetBinContent(bin) - hists[cr][dist][sam]->GetBinContent(bin) );
                        pdfVarRms += diff * diff;
                    }
                    pdfVarRms = sqrt( 0.01 * pdfVarRms );
                    uncHistMapDown["pdf"][cr][dist][sam]->SetBinContent( bin, hists[cr][dist][sam]->GetBinContent(bin) - pdfVarRms);
                    uncHistMapUp["pdf"][cr][dist][sam]->SetBinContent( bin, hists[cr][dist][sam]->GetBinContent(bin) + pdfVarRms);
                }
            }
        }
    }

    //add pdf and scale variation uncertainties of the cross-section
    std::vector< double > scaleXsecUncDown = {0.};  //first dummy entry for data
    std::vector< double > scaleXsecUncUp = {0.};
    std::vector< double > pdfXsecUncDown = {0.};
    std::vector< double > pdfXsecUncUp = {0.};

    //loop over all processes
    for( unsigned sam = 0; sam < samples.size(); ++sam){ 
        
        if(sam == 0){
            continue;
        }
        //get file for sample
        std::shared_ptr<TFile> sample = samples[sam].getFile("../../ntuples_tzq/");

        //extract histogram containing sum of weights for all possible pdf variations
        std::shared_ptr<TH1D> hCounter = std::shared_ptr<TH1D>( (TH1D*) sample->Get("blackJackAndHookers/hCounter") );
        //nominal sum of weights 
        double sumOfWeights = hCounter->GetBinContent(1);
        std::shared_ptr<TH1D> lheCounter = std::shared_ptr<TH1D>( (TH1D*) sample->Get("blackJackAndHookers/lheCounter"));

        //scale effect on xSec :
        double sumOfWeights_scaleDown = lheCounter->GetBinContent(9);
        double scaleDownUnc = (sumOfWeights_scaleDown/sumOfWeights - 1.); //Warning: nominal sum of weights has to be in the denominator!
        scaleXsecUncDown.push_back( scaleDownUnc );

        double sumOfWeights_scaleUp = lheCounter->GetBinContent(5);
        double scaleUpUnc = (sumOfWeights_scaleUp/sumOfWeights - 1.); //Warning: nominal sum of weights has to be in the denominator!
        scaleXsecUncUp.push_back( scaleUpUnc );

        //pdf effect on xSec:
        double rms = 0.;
        for(unsigned pdf = 0; pdf < 100; ++pdf){
            double sumOfWeights_pdf = lheCounter->GetBinContent(pdf + 10) ;
            double pdfUnc = (sumOfWeights_pdf/sumOfWeights - 1.);
            rms += pdfUnc*pdfUnc;
        }
        rms = sqrt( 0.01 * rms);

        //catch CRAZY pdf uncertainty of TTZ-M1to10 in 2016 and set to zero
        if( samples[sam].getFileName() == "TTZToLL_M-1to10_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16.root"){
            rms = 0.;
        }

        pdfXsecUncDown.push_back( -rms );
        pdfXsecUncUp.push_back( rms );

        //set pdf and scale uncertainties to zero for GluGluToContinToZZ samples which do not include the necessary weights
        if( samples[sam].getFileName().find("GluGluToContinToZZ") != std::string::npos){
            scaleXsecUncDown[sam] = 0.;
            scaleXsecUncUp[sam] = 0.;
            pdfXsecUncDown[sam] = 0.;
            pdfXsecUncUp[sam] = 0.;
        }
    }

    //add pdf and scale effects to shape uncertainties to automatically take into account the fractional effects from every sample that is merged
    for(unsigned cr = 0; cr < nCr; ++cr){
       for(unsigned dist = 0; dist < nDist; ++dist){
           for(unsigned sam = 0; sam < samples.size() + 1; ++sam){
               if(sam < samples.size() ){
                   for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["scaleXsec"][cr][dist][sam]->GetNbinsX() + 1; ++bin){
                       uncHistMapDown["scaleXsec"][cr][dist][sam]->SetBinContent(bin, (1 + scaleXsecUncDown[sam])*hists[cr][dist][sam]->GetBinContent(bin) );
                       uncHistMapUp["scaleXsec"][cr][dist][sam]->SetBinContent(bin, (1 + scaleXsecUncUp[sam])*hists[cr][dist][sam]->GetBinContent(bin) );
                       uncHistMapDown["pdfXsec"][cr][dist][sam]->SetBinContent(bin, (1 + pdfXsecUncDown[sam])*hists[cr][dist][sam]->GetBinContent(bin) );
                       uncHistMapUp["pdfXsec"][cr][dist][sam]->SetBinContent(bin, (1 + pdfXsecUncUp[sam])*hists[cr][dist][sam]->GetBinContent(bin) );
                   }
               //WARNING: CURRENTLY THE SCALE AND PDF XSEC UNCERTAINTIES ON DATA-DRIVEN FAKES ARE SET TO ZERO
               } else {
                   for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["scaleXsec"][cr][dist][sam]->GetNbinsX() + 1; ++bin){
                       uncHistMapDown["scaleXsec"][cr][dist][sam]->SetBinContent(bin, hists[cr][dist][sam]->GetBinContent(bin) );
                       uncHistMapUp["scaleXsec"][cr][dist][sam]->SetBinContent(bin, hists[cr][dist][sam]->GetBinContent(bin) );
                       uncHistMapDown["pdfXsec"][cr][dist][sam]->SetBinContent(bin, hists[cr][dist][sam]->GetBinContent(bin) );
                       uncHistMapUp["pdfXsec"][cr][dist][sam]->SetBinContent(bin, hists[cr][dist][sam]->GetBinContent(bin) );
                   }
               }
           }
       }
    
    }

    //merge histograms with the same physical background
    std::vector<std::string> proc = {"total bkg.", "tZq", "WZ", "multiboson", "TT + Z", "TT/T + X", "X + #gamma", "ZZ/H", "Nonprompt e/#mu"};
    std::vector< std::vector< std::vector< TH1D* > > > mergedHists(nCr);
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            mergedHists[cr].push_back(std::vector<TH1D*>(proc.size() ) );

            //cut off loop before nonprompt contribution
            for(size_t m = 0, sam = 0; m < proc.size() - 1; ++m){
                mergedHists[cr][dist][m] = (TH1D*) hists[cr][dist][sam]->Clone();
                while(sam < samples.size() - 1 && samples[sam].getProcessName() == samples[sam + 1].getProcessName() ){
                    mergedHists[cr][dist][m]->Add(hists[cr][dist][sam + 1].get());
                    ++sam;
                }
                ++sam;
            }

            //add nonprompt histogram
            mergedHists[cr][dist][proc.size() - 1] = (TH1D*) hists[cr][dist][samples.size()].get()->Clone();
        }
    }

    //merging for uncertainties
    std::map< std::string, std::vector< std::vector< std::vector< TH1D* > > > > mergedUncMapDown;
    std::map< std::string, std::vector< std::vector< std::vector< TH1D* > > > > mergedUncMapUp;
    for( auto& key : uncNames ){
        mergedUncMapDown[key] = std::vector< std::vector< std::vector< TH1D* > > >(nCr);
        mergedUncMapUp[key] = std::vector< std::vector< std::vector< TH1D* > > >(nCr);
        for(unsigned cr = 0; cr < nCr; ++cr){
           for(unsigned dist = 0; dist < nDist; ++dist){
               mergedUncMapDown[key][cr].push_back(std::vector<TH1D*>(proc.size() ) );
               mergedUncMapUp[key][cr].push_back(std::vector<TH1D*>(proc.size() ) );

               //cut off loop before nonprompt contribution
               for(size_t m = 0, sam = 0; m < proc.size() - 1; ++m){
                   mergedUncMapDown[key][cr][dist][m] = (TH1D*) uncHistMapDown[key][cr][dist][sam]->Clone();
                   mergedUncMapUp[key][cr][dist][m] = (TH1D*) uncHistMapUp[key][cr][dist][sam]->Clone();
                   while(sam < samples.size() - 1 && samples[sam].getProcessName() == samples[sam + 1].getProcessName() ){
                       mergedUncMapDown[key][cr][dist][m]->Add( uncHistMapDown[key][cr][dist][sam + 1].get());
                       mergedUncMapUp[key][cr][dist][m]->Add( uncHistMapUp[key][cr][dist][sam + 1].get());
                       ++sam;
                   }
                   ++sam;
               }

               //add nonprompt histograms 
               mergedUncMapDown[key][cr][dist][proc.size() - 1] = (TH1D*) uncHistMapDown[key][cr][dist][samples.size()]->Clone();
               mergedUncMapUp[key][cr][dist][proc.size() - 1] = (TH1D*) uncHistMapUp[key][cr][dist][samples.size()]->Clone();
           }
        }
    } 

    //make final uncertainty histogram for plots 
    std::vector<double> flatUnc = {1.025, 1.06, 1.05, 1.00, 1.00}; //lumi, leptonID, trigger , pdf and scale effects on cross section
    std::map< std::string, double > backgroundSpecificUnc =        //map of background specific nuisances that can be indexed with the name of the process 
        {
            {"Nonprompt e/#mu", 1.3}
        };
    std::vector< std::vector< std::vector< TH1D* > > > totalSystUnc = mergedHists; //copy pointers to fix dimensionality of vector
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            for(unsigned p = 0; p < proc.size(); ++p){
                if(p == 0) continue;
                totalSystUnc[cr][dist][p] = (TH1D*) mergedHists[cr][dist][p]->Clone();
                for(unsigned bin = 1; bin < (unsigned)  totalSystUnc[cr][dist][p]->GetNbinsX() + 1; ++bin){
                    double binUnc = 0;

                    //add all shape uncertainties 
                    for( auto& key : uncNames ){
                        double down = fabs(mergedUncMapDown[key][cr][dist][p]->GetBinContent(bin) - mergedHists[cr][dist][p]->GetBinContent(bin) );
                        double up = fabs(mergedUncMapUp[key][cr][dist][p]->GetBinContent(bin) - mergedHists[cr][dist][p]->GetBinContent(bin) );
                        double var = std::max(down, up);

                        //consider maximum variation between up and down for plotting
                        binUnc += var*var;
                    }
                    
                    //add flat uncertainties
                    for( double unc : flatUnc ){
                        double binContent = mergedHists[cr][dist][p]->GetBinContent(bin);
                        double var = binContent*(unc - 1.); 
                        binUnc += var*var;
                    }

                    //add background specific uncertainties
                    for(auto& uncPair : backgroundSpecificUnc){
                        if(proc[p] == uncPair.first){
                            double var = mergedHists[cr][dist][p]->GetBinContent(bin)*(uncPair.second - 1.);
                            binUnc += var*var;
                        }
                    }
                    totalSystUnc[cr][dist][p]->SetBinContent(bin, sqrt(binUnc) );
                }
            }
        }
    }

    //plot all distributions
    const bool isSMSignal[(const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
    for(unsigned cr = 0; cr < nCr; ++cr){
       for(unsigned dist = 0; dist < nDist; ++dist){
           plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/2016/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_" + crNames[cr] + "_2016", "tzq", false, false, "35.9 fb^{-1} (13 TeV)", &totalSystUnc[cr][dist][1], isSMSignal);             //linear plots

           plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/2016/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_"  + crNames[cr] + "_2016" + "_log", "tzq", true, false, "35.9 fb^{-1} (13 TeV)", &totalSystUnc[cr][dist][1], isSMSignal);    //log plots
       }
    
    }
    
    /*
    //make shape datacards for each category
    const unsigned nBkg = proc.size() - 2;  //number of background processes
    const std::string bkgNames[nBkg] = {"WZ", "multiboson", "TTZ", "TTX", "Xgamma", "ZZH", "nonprompt"}; //rewrite bkg names not to confuse combine
    std::vector<std::string> flatSyst = {"lumi", "trigger", "id"};
    std::vector<std::string> shapeSyst = uncNames;
    std::vector<std::string> systNames; 
    for(auto& flatName : flatSyst){
        systNames.push_back( flatName );
    }
    for(auto& shapeName : shapeSyst){
        systNames.push_back( shapeName );
    }

    std::map< std::string, double > bkgSpecificUnc =        //map of background specific nuisances that can be indexed with the name of the process 
        {
            {"nonprompt", 1.3}
        };

    const unsigned nBinsFit = mergedHists[0][0][0][0]->GetNbinsX(); //number of bins used in the final fit
    const unsigned nStatUnc = (1 + nBkg)*nBinsFit; 
    const unsigned nFlatSyst = flatSyst.size();
    const unsigned nShapeSyst = uncNames.size();
    const unsigned nGeneralSystematics = nFlatSyst + nShapeSyst;
    const unsigned nBackGroundSpecificUnc = backgroundSpecificUnc.size();
    const unsigned nSyst = nGeneralSystematics + nStatUnc + nBackGroundSpecificUnc; //general uncertainties + stat signal (for every bin) + stat bkg (for every bin) + extra unc per bkg

    std::vector<std::vector<double>> systUnc(nSyst, std::vector<double>(nBkg + 1, 0)); //2D array containing all uncertainty sizes

    //initialize flat systematics
    for(unsigned p = 0; p < nBkg; ++p){ //signal and bkg  but ignore last background which is data-driven
        systUnc[0][p] = 1.025;   //lumi
        systUnc[1][p] = 1.06;    //id eff
        systUnc[2][p] = 1.02;   //trig eff  
    }

    //initialize general shape uncertainties
    for(unsigned p = 0; p < nBkg + 1; ++p){
        for(unsigned shape = 0; shape < nShapeSyst; ++shape){
            if( shape >= nShapeSyst - 2 && p == nBkg) continue; //skip xsec uncertainties for nonprompt
            systUnc[nFlatSyst + shape][p] = 1.00;
        }
    }

    //set statistical uncertainties
    for(unsigned p = 0; p < nBkg + 1; ++p){

        for(unsigned bin = 0; bin < nBinsFit; ++bin){

            //size 
            systUnc[nGeneralSystematics + p*nBinsFit + bin][p] = 1.00;

            //name 
            if(p == 0){
                systNames.push_back( "tZq_stat_bin_" + std::to_string(bin + 1) );
            } else{
                systNames.push_back( bkgNames[p - 1] + "_stat_bin_" + std::to_string(bin + 1) );
            }
        }
    }


    //set background specific uncertainties
    unsigned specCount = 0;
    for(auto& entry : bkgSpecificUnc){
        systNames.push_back( entry.first + "_extra");
        unsigned process = 999;
        for(unsigned bkg = 0; bkg < nBkg; ++bkg){
            if( bkgNames[bkg] == entry.first) process = bkg + 1;
        }
        systUnc[nGeneralSystematics + nStatUnc + specCount][process] = entry.second;
        ++specCount;
    }

    std::string systDist[nSyst]; //probability distribution of nuisances
    for(unsigned syst = 0; syst < nSyst; ++syst){
        if(syst < 3 || syst  >= nGeneralSystematics + nStatUnc) systDist[syst] = "lnN"; //flat unc 
        else systDist[syst] = "shape";                                                 //stat and all other shapes are shape unc 
    }

    //statistic shape uncertainties 
    TH1D* bdtShape[nMll][nCat][(const size_t) proc.size()] ; //shape histograms of bdt
    TH1D* bdtShapeStatDown[nMll][nCat][(const size_t) proc.size()][nBinsFit]; //statistical shape uncertainty on bdt
    TH1D* bdtShapeStatUp[nMll][nCat][(const size_t) proc.size()][nBinsFit]; //statistical shape uncertainty on bdt

    //set up statistical shape uncertainties 
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned p = 0; p < proc.size(); ++p){
                for(unsigned bin = 0; bin < nBinsFit; ++bin){
                    
                    //stat shape nuisances are clones of the nominal histogram, with one bin varied up and down
                    double variedBinContent = mergedHists[m][cat][0][p]->GetBinContent(bin + 1);
                    double variedBinUnc = mergedHists[m][cat][0][p]->GetBinError(bin + 1);

                    double binContentStatDown = std::max( variedBinContent - variedBinUnc, std::numeric_limits< double >::min() );
                    bdtShapeStatDown[m][cat][p][bin] = (TH1D*)  mergedHists[m][cat][0][p]->Clone();
                    bdtShapeStatDown[m][cat][p][bin]->SetBinContent(bin + 1, binContentStatDown );

                    double binContentStatUp = std::max( variedBinContent + variedBinUnc, std::numeric_limits< double >::min() );
                    bdtShapeStatUp[m][cat][p][bin] = (TH1D*)  mergedHists[m][cat][0][p]->Clone();
                    bdtShapeStatUp[m][cat][p][bin]->SetBinContent(bin + 1, binContentStatUp );
                }
            }
        }
    }

    //make datacard for each category
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){

            //Set bkg yields 
            double bkgYields[(const size_t) proc.size() - 2];
            for(unsigned bkg = 0; bkg < proc.size() - 2; ++bkg) bkgYields[bkg] = mergedHists[m][cat][0][2 + bkg]->GetSumOfWeights();

            //set statical shape names ( to be sure they are independent for every category )
            for(unsigned p = 0; p < nBkg + 1; ++p){
                for(unsigned bin = 0; bin < nBinsFit; ++bin){
                    if(p == 0){
                        systNames[nGeneralSystematics + p*nBinsFit + bin] = "tZq_stat_" + catNames[cat] + "_2016_bin_" + std::to_string(bin + 1);
                    } else{
                        systNames[nGeneralSystematics + p*nBinsFit + bin] =  bkgNames[p - 1] + "_stat_" + catNames[cat] + "_2016_bin_" + std::to_string(bin + 1);
                    }
 
                }
            }

            //set BDT shape histogram
            TFile* shapeFile =TFile::Open((const TString&) "./datacards/shapes/shapeFile_"  + catNames[cat] + mllNames[m] +  ".root", "recreate");

            //loop over all processes 
            for(unsigned p = 0; p < proc.size(); ++p){

                //write nominal histograms 
                bdtShape[m][cat][p] = (TH1D*) mergedHists[m][cat][0][p]->Clone();
                if(p == 0) bdtShape[m][cat][p]->Write("data_obs");
                else if (p == 1) bdtShape[m][cat][p]->Write("tZq");
                else bdtShape[m][cat][p]->Write((const TString&) bkgNames[p -2]);

                
                if( p != 0 ){

                    //name of the current process
                    std::string procName;
                    if( p == 1){
                        procName = "tZq";
                    } else {
                        procName = bkgNames[p - 2];
                    }

                    //write general shape nuisances 
                    for( auto& key : uncNames ){
                        mergedUncMapDown[key][m][cat][0][p]->Write( (const TString&) procName + "_" + key + "Down");
                        mergedUncMapUp[key][m][cat][0][p]->Write( (const TString&) procName + "_" + key + "Up");
                    }

                    //write statistical shape nuisances 
                    for(unsigned bin = 0; bin < nBinsFit; ++bin){
                        bdtShapeStatDown[m][cat][p][bin]->Write( (const TString&) procName + "_" + systNames[nGeneralSystematics + (p - 1)*nBinsFit + bin] + "Down");
                        bdtShapeStatUp[m][cat][p][bin]->Write( (const TString&) procName + "_" + systNames[nGeneralSystematics + (p - 1)*nBinsFit + bin] + "Up");
                    }
                }
            }
            shapeFile->Close();

            tools::printDataCard( mergedHists[m][cat][0][0]->GetSumOfWeights(), mergedHists[m][cat][0][1]->GetSumOfWeights(), "tZq", bkgYields, proc.size() - 2, bkgNames, systUnc, nSyst, &systNames[0], systDist, "datacards/datacard_" + mllNames[m] + "_" + catNames[cat], true, "shapes/shapeFile_"  + catNames[cat] + mllNames[m]);
        }
    }
    std::cout << "Printed all datacards" << std::endl;
    */
}
int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
