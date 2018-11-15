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
#include "../interface/PostFitScaler.h"
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
    readSamples("sampleLists/samples_nonpromptDataDriven_2017.txt", "../../ntuples_tzq");
    //name      xlabel    nBins,  min, max
    histInfo = {
        //new BDT distribution
        HistInfo("totalYield", "Total control region yield", 1, 0, 1),
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
        HistInfo("flavors", "flavors", 4, 0, 4, {"eee", "ee#mu", "e#mu#mu", "#mu#mu#mu"} )
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

	const std::vector< std::string > uncNames = {"JEC_2017", "uncl", "scale", "pileup", "bTag_udsg_2017", "bTag_bc_2017", "prefiring", "WZ_extrapolation",
        "lepton_reco", "muon_id_stat_2017", "electron_id_stat_2017", "lepton_id_syst", "pdf", "scaleXsec", "pdfXsec"};

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
                analysisTools::printProgress(progress);
            } else if(it == nEntries -1){
                progress = 1.;
                analysisTools::printProgress(progress);
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
            bool passTightCut = (tightLepCount(ind, lCount) == lCount);
            if(!passTightCut){
                //fill last histogram (nonprompt)
                fillIndex = samples.size();

                //apply fake-rate weights 
                weight *= fakeRateWeight(); 
            }

            //remove overlap between samples
            if(photonOverlap(currentSample, false)) continue;

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
                if( weight == 0.){
                    continue;
                }
            }

            //compute nominal bdt value
            if( !tzq::isWZControlRegion(controlRegion) || (_met > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    0.5,
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
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
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
                    0.5,
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
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

 
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown["JEC_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
                
            }

            //vary JEC up
            setSearchVariablestZq("JECUp", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metJECUp > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    0.5,
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
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp["JEC_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary unclustered down
            setSearchVariablestZq("unclDown", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metUnclDown > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    0.5,
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
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
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
                    0.5,
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
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp["uncl"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //reset nominal values 
            setSearchVariablestZq("nominal", ind, bestZ);

            //now nominal cuts can be safely used
            if( tzq::isWZControlRegion(controlRegion) && (_met < 50 ||  bdtVariableMap["numberOfbJets"] != 0 ) ) continue;
            
            double fill[nDist] = {
                    0.5,
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
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
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
            double puDownWeight = puWeight(1)/puWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["pileup"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puDownWeight);
            }

            //vary pu up            
            double puUpWeight = puWeight(2)/puWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["pileup"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puUpWeight);
            }

            //vary b-tag down for udsg
            double bTag_udsg_downWeight = bTagWeight_udsg(1)/bTagWeight_udsg(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["bTag_udsg_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_udsg_downWeight);
            }

            //vary b-tag up for udsg
            double bTag_udsg_upWeight = bTagWeight_udsg(2)/bTagWeight_udsg(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["bTag_udsg_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_udsg_upWeight);
            }

            //vary b-tag down for b and c (correlated)
            double bTag_bc_downWeight = bTagWeight_c(1)*bTagWeight_b(1)/ (bTagWeight_c(0)*bTagWeight_b(0) );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["bTag_bc_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_bc_downWeight );
            }

            //vary b-tag up for b and c (correlated)
            double bTag_bc_upWeight = bTagWeight_c(2)*bTagWeight_b(2)/ (bTagWeight_c(0)*bTagWeight_b(0) );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["bTag_bc_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_bc_upWeight);
            }
 
			//vary jet prefiring probabilities down
            double prefiringDownWeight = jetPrefiringWeight(1)/jetPrefiringWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["prefiring"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*prefiringDownWeight );
            }

            //vary jet prefiring probabilities up
            double prefiringUpWeight = jetPrefiringWeight(2)/jetPrefiringWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["prefiring"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*prefiringUpWeight );
            }

            //extrapolation uncertainty for WZ from CR to SR
            double WZExtrapolationUnc;
            if( (currentSample.getProcessName() == "WZ") && ( bdtVariableMap["numberOfbJets"] > 0 ) ){
                WZExtrapolationUnc = 0.08;
            } else {
                WZExtrapolationUnc = 0.;
            }
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["WZ_extrapolation"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( 1. - WZExtrapolationUnc) );
                uncHistMapUp["WZ_extrapolation"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( 1. + WZExtrapolationUnc) );
            }

            //vary lepton reco SF down
            double leptonRecoDownWeight = leptonWeight("recoDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["lepton_reco"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonRecoDownWeight );
            }

            //vary lepton reco SF up
            double leptonRecoUpWeight = leptonWeight("recoUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["lepton_reco"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonRecoUpWeight);
            }

            //vary muon stat down
            double muonStatDownWeight = leptonWeight("muon_idStatDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["muon_id_stat_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*muonStatDownWeight);
            }

            //vary muon stat up
            double muonStatUpWeight = leptonWeight("muon_idStatUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["muon_id_stat_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*muonStatUpWeight );
            }

            //vary electron stat down
            double electronStatDownWeight = leptonWeight("electron_idStatDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["electron_id_stat_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*electronStatDownWeight );
            }

            //vary electron stat up
            double electronStatUpWeight = leptonWeight("electron_idStatUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["electron_id_stat_2017"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*electronStatUpWeight );
            }

            //vary lepton syst down
            double leptonIdSystDownWeight = leptonWeight("idSystDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["lepton_id_syst"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonIdSystDownWeight );
            }

            //vary lepton syst up
            double leptonIdSystUpWeight = leptonWeight("idSystUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["lepton_id_syst"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonIdSystUpWeight);
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
                analysisTools::setNegativeBinsToZero( hists[cr][dist][sam].get() );
                for(auto & key : uncNames){
                    analysisTools::setNegativeBinsToZero(uncHistMapDown[key][cr][dist][sam].get() );
                    analysisTools::setNegativeBinsToZero(uncHistMapUp[key][cr][dist][sam].get() );
                } 
                for(unsigned pdf = 0; pdf < 100; ++pdf){    
                    analysisTools::setNegativeBinsToZero(pdfUncHists[pdf][cr][dist][sam].get() );
                }
            }	
        }
    }

    //set nonprompt bins to 0 if negative
    for(unsigned cr = 0; cr < nCr; ++cr){
       for(unsigned dist = 0; dist < nDist; ++dist){
           unsigned nonpromptIndex = samples.size();
           analysisTools::setNegativeBinsToZero( hists[cr][dist][nonpromptIndex].get() );
           for(auto & key : uncNames){
               analysisTools::setNegativeBinsToZero(uncHistMapDown[key][cr][dist][nonpromptIndex].get() );
               analysisTools::setNegativeBinsToZero(uncHistMapUp[key][cr][dist][nonpromptIndex].get() );
           } 
           for(unsigned pdf = 0; pdf < 100; ++pdf){    
               analysisTools::setNegativeBinsToZero(pdfUncHists[pdf][cr][dist][nonpromptIndex].get() );
           }
       }
    
    }

    //compute lhe cross section ratio for every sample and variation
    std::vector< std::vector< double> > crossSectionRatio(samples.size(), std::vector<double>(110, 1.) );
    for(unsigned sam = 1; sam < samples.size(); ++sam){

        //lhe weights are not available for GluGlu->ZZ
        if( samples[sam].getFileName().find("GluGluToContinToZZ") != std::string::npos ) continue;

        std::shared_ptr<TFile> sample = samples[sam].getFile();

        //extract histogram containing nominal sum of weights
        std::shared_ptr<TH1D> hCounter = std::shared_ptr<TH1D>( (TH1D*) sample->Get("blackJackAndHookers/hCounter") );
        double sumOfWeights = hCounter->GetBinContent(1);

        //extract histogram containing sum of weights for all possible pdf variations
        std::shared_ptr<TH1D> lheCounter = std::shared_ptr<TH1D>( (TH1D*) sample->Get("blackJackAndHookers/lheCounter"));
        for(unsigned lhe = 0; lhe < 110; ++lhe){
            double variedSumOfWeights = lheCounter->GetBinContent(lhe + 1);
            crossSectionRatio[sam][lhe] = sumOfWeights/( variedSumOfWeights );
        }
    }

    //compute final pdf uncertainties
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            for(unsigned sam = 0; sam < samples.size() + 1; ++sam){
                for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["pdf"][cr][dist][sam]->GetNbinsX() + 1; ++bin){
                    double pdfVarRms = 0.;
                    for(unsigned pdf = 0; pdf < 100; ++pdf){
                        
                        double variedBin = pdfUncHists[pdf][cr][dist][sam]->GetBinContent(bin);
                        if( sam != samples.size() ){
                            variedBin *= crossSectionRatio[sam][pdf + 9];
                        }
                        double diff = (  variedBin - hists[cr][dist][sam]->GetBinContent(bin) );
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

    for( unsigned sam = 1; sam < samples.size(); ++sam){

        //scale effect on xSec :
        double scaleDownUnc = (1./crossSectionRatio[sam][8] - 1.); //Warning: nominal sum of weights has to be in the denominator!
        scaleXsecUncDown.push_back( scaleDownUnc );

        double scaleUpUnc = (1./crossSectionRatio[sam][4]- 1.); //Warning: nominal sum of weights has to be in the denominator!
        scaleXsecUncUp.push_back( scaleUpUnc );

        //filter out cross section effects from scale shape uncertainty
        if( samples[sam].getFileName().find("GluGluToContinToZZ") == std::string::npos){
            for(unsigned cr = 0; cr < nCr; ++cr){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown["scale"][cr][dist][sam]->Scale(crossSectionRatio[sam][8]);
                    uncHistMapUp["scale"][cr][dist][sam]->Scale(crossSectionRatio[sam][4]);
                } 
            }
            
        }
    
        //pdf effect on xSec:
        double rms = 0.;
        for(unsigned pdf = 0; pdf < 100; ++pdf){
            double pdfUnc = (1./crossSectionRatio[sam][pdf + 9] - 1.);
            rms += pdfUnc*pdfUnc;
        }
        rms = sqrt( 0.01 * rms);

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
    std::vector<std::string> proc = {"Data", "tZq", "WZ", "Multiboson", "t#bar{t} + Z", "t#bar{t}/t + X", "X + #gamma", "ZZ/H", "Nonprompt e/#mu"};
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
    std::vector<double> flatUnc = {1.023, 1.02}; //lumi, trigger
    std::map< std::string, double > backgroundSpecificUnc =        //map of background specific nuisances that can be indexed with the name of the process 
        {
            {"Nonprompt e/#mu", 1.3},
            {"WZ", 1.1},
            {"X + #gamma", 1.1},
            {"ZZ/H", 1.1},
            {"t#bar{t} + Z", 1.15}
        };

    std::vector< std::string > ignoreTheoryUncInPlot = {"tZq", "WZ", "X + #gamma", "ZZ/H", "t#bar{t} + Z"};

    const std::vector< std::string > uncorrelatedBetweenProcesses = {"scale", "pdf", "scaleXsec", "pdfXsec"};
    std::vector< std::vector< TH1D* > > totalSystUnc(nCr); //copy pointers to fix dimensionality of vector

    for( unsigned cr = 0; cr < nCr; ++cr){
        totalSystUnc[cr] = std::vector< TH1D* >(nDist);
        for(unsigned dist = 0; dist < nDist; ++dist){
            totalSystUnc[cr][dist] = (TH1D*) mergedHists[cr][dist][0]->Clone();

            for(unsigned bin = 1; bin < (unsigned) totalSystUnc[cr][dist]->GetNbinsX() + 1; ++bin){
                double binUnc = 0;

                //add all shape uncertainties 
                for(auto& key: uncNames ){

                    bool nuisanceIsUnCorrelated = ( std::find( uncorrelatedBetweenProcesses.cbegin(), uncorrelatedBetweenProcesses.cend(), key ) != uncorrelatedBetweenProcesses.cend() );

                    double var = 0.;

                    //for the correlated case
                    double varDown = 0.;
                    double varUp = 0.;

                    //linearly add the variations for each process 
                    for(unsigned p = 1; p < proc.size(); ++p){

                        //ignore theoretical uncertainties on the normalization of certain processes 
                        if( key.find("Xsec") != std::string::npos ){
                            bool processWithoutTheoryUnc =  ( std::find( ignoreTheoryUncInPlot.cbegin(), ignoreTheoryUncInPlot.cend(), proc[p] ) != ignoreTheoryUncInPlot.cend() );
                            if( processWithoutTheoryUnc){
                                continue;
                            }
                        }

                        double nominalContent = mergedHists[cr][dist][p]->GetBinContent(bin);
                        double downVariedContent = mergedUncMapDown[key][cr][dist][p]->GetBinContent(bin);
                        double upVariedContent = mergedUncMapUp[key][cr][dist][p]->GetBinContent(bin);
                        double down = fabs(downVariedContent - nominalContent);
                        double up = fabs(upVariedContent - nominalContent);

                        //uncorrelated case : 
                        if( nuisanceIsUnCorrelated ){
                            double variation = std::max(down, up);
                            var += variation*variation;

                        //correlated case :     
                        } else {
                            varDown += down;
                            varUp += up;
                        }
                    }

					//correlated case : 
                    if( !nuisanceIsUnCorrelated ){
                        var = std::max( varDown, varUp );
                        var = var*var;
                    }

                    //add (already quadratic) uncertainties 
                    binUnc += var;
                }

                //add general flat uncertainties (considered correlated among all processes)
                for( double unc : flatUnc ){
                    double var = 0;
                    for(unsigned p = 1; p < proc.size(); ++p){
                        if( proc[p] == "Nonprompt e/#mu" ){
                            continue;
                        }
                        double binContent = mergedHists[cr][dist][p]->GetBinContent(bin);
                        double variation = binContent*(unc - 1.);
                        var += variation;
                    }
                    binUnc += var*var;
                }

                //add background specific uncertainties (uncorrelated between processes)
                for(auto& uncPair : backgroundSpecificUnc){
                    for(unsigned p = 1; p < proc.size(); ++p){
                        if(proc[p] == uncPair.first){
                            double var = mergedHists[cr][dist][p]->GetBinContent(bin)*(uncPair.second - 1.);
                            binUnc += var*var;
                        }
                    }
                }

                //square root of quadratic sum is total uncertainty
                totalSystUnc[cr][dist]->SetBinContent(bin, sqrt(binUnc) );
            }
        }
    }





    //plot all distributions
    const bool isSMSignal[(const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
    for(unsigned cr = 0; cr < nCr; ++cr){
       for(unsigned dist = 0; dist < nDist; ++dist){
           plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/2017/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_" + crNames[cr] + "_2017", "tzq", false, false, "41.5 fb^{-1} (13 TeV)", totalSystUnc[cr][dist], isSMSignal);             //linear plots

           plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/2017/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_"  + crNames[cr] + "_2017" + "_log", "tzq", true, false, "41.5 fb^{-1} (13 TeV)", totalSystUnc[cr][dist], isSMSignal);    //log plots
       }
    
    }

    //make shape datacards for each category
    const unsigned nBkg = proc.size() - 2;  //number of background processes
    const std::string bkgNames[nBkg] = {"WZ", "multiboson", "TTZ", "TTX", "Xgamma", "ZZH", "nonprompt"}; //rewrite bkg names not to confuse combine
    std::vector<std::string> processNames = {"tZq", "WZ", "multiboson", "TTZ", "TTX", "Xgamma", "ZZH", "nonprompt"};
    std::vector<std::string> flatSyst = {"lumi_2017", "trigger_2017"};
    std::vector<std::string> shapeSyst = uncNames;
    std::vector<std::string> systNames;

    std::vector<std::string> uncorrelatedBetweenProc = {"pdf", "scale"};
    std::map< std::string, bool> isCorrelatedBetweenProc;
    for(auto& shapeName : shapeSyst ){
        bool correlated = true;
        for( auto& uncorrelatedUnc : uncorrelatedBetweenProc ){
            if( shapeName.find( uncorrelatedUnc ) != std::string::npos ){
                correlated = false;
                break;
            }
        }
        isCorrelatedBetweenProc[shapeName] = correlated;
    }

    for(auto& flatName : flatSyst){
        systNames.push_back( flatName );
    }

    unsigned nShapes = 0;
    for(auto& shapeName : shapeSyst){
        if( isCorrelatedBetweenProc[shapeName] ){
            systNames.push_back( shapeName );
            ++nShapes;
        } else{
            for(auto& process : processNames){
               systNames.push_back( process + "_" + shapeName);
               ++nShapes;
            }
        }
    }


    std::map< std::string, double > bkgSpecificUnc =        //map of background specific nuisances that can be indexed with the name of the process 
        {
            {"nonprompt", 1.3},
            {"WZ", 1.1},
            {"Xgamma", 1.1},
            {"ZZH", 1.1},
            {"TTZ", 1.15}
        };

    const unsigned nBinsFit = mergedHists[0][0][0]->GetNbinsX(); //number of bins used in the final fit
    const unsigned nStatUnc = (1 + nBkg)*nBinsFit;
    const unsigned nFlatSyst = flatSyst.size();
    const unsigned nShapeSyst = nShapes;
    const unsigned nGeneralSystematics = nFlatSyst + nShapeSyst;
    const unsigned nBackGroundSpecificUnc = bkgSpecificUnc.size();
    const unsigned nSyst = nGeneralSystematics + nStatUnc + nBackGroundSpecificUnc; //general uncertainties + stat signal (for every bin) + stat bkg (for every bin) + extra unc per bkg

    std::vector<std::vector<double>> systUnc(nSyst, std::vector<double>(nBkg + 1, 0)); //2D array containing all uncertainty sizes

    //initialize flat systematics
    for(unsigned p = 0; p < nBkg; ++p){ //signal and bkg  but ignore last background which is data-driven
        systUnc[0][p] = 1.023;   //lumi
        systUnc[1][p] = 1.02;   //trig eff  
    }

    //ignore theory xsec uncertainties for some processes
    std::vector<unsigned> ignoreTheoryUncForIndices = {0, 1, 3, 5, 6, 7};

    //initialize general shape uncertainties
    for(unsigned p = 0; p < nBkg + 1; ++p){
        for(unsigned shape = 0; shape < nShapeSyst; ++shape){

            //skip xsec uncertainties for nonprompt
            //skip xsec uncertainties for other processes that are measured directly 
            std::string shapeName = systNames[nFlatSyst + shape];
            bool uncIsXsec = ( shapeName.find("Xsec") != std::string::npos );
            bool processWithoutTheoryUnc = ( std::find(ignoreTheoryUncForIndices.cbegin(), ignoreTheoryUncForIndices.cend(), p) != ignoreTheoryUncForIndices.cend() );
            processWithoutTheoryUnc = processWithoutTheoryUnc || (p == nBkg); //make sure nonprompt is always skipped here
            if( uncIsXsec && processWithoutTheoryUnc) continue;

            //only consider WZ extrapolation uncertainty for WZ 
            if( (processNames[p] != "WZ") && shapeName == "WZ_extrapolation" ) continue;

            if( isCorrelatedBetweenProc[shapeName] ){
                systUnc[nFlatSyst + shape][p] = 1.00;
            } else if( shapeName.find( processNames[p] ) != std::string::npos ){
                systUnc[nFlatSyst + shape][p] = 1.00;
            }
        }
    }

    //set statistical uncertainties
    for(unsigned p = 0; p < nBkg + 1; ++p){

        for(unsigned bin = 0; bin < nBinsFit; ++bin){

            //size 
            systUnc[nGeneralSystematics + p*nBinsFit + bin][p] = 1.00;

            //name 
            systNames.push_back( processNames[p] + "_stat_bin_" + std::to_string(bin + 1) );
        }
    }

    //set background specific uncertainties
    unsigned specCount = 0;
    for(auto& entry : bkgSpecificUnc){
        systNames.push_back( entry.first + "_norm");
        unsigned process = 999;
        for(unsigned bkg = 0; bkg < nBkg; ++bkg){
            if( bkgNames[bkg] == entry.first) process = bkg + 1;
        }
        systUnc[nGeneralSystematics + nStatUnc + specCount][process] = entry.second;
        ++specCount;
    }

    std::string systDist[nSyst]; //probability distribution of nuisances
    for(unsigned syst = 0; syst < nSyst; ++syst){
        if(syst < nFlatSyst || syst  >= nGeneralSystematics + nStatUnc) systDist[syst] = "lnN"; //flat unc 
        else systDist[syst] = "shape";                                                 //stat and all other shapes are shape unc 
    }

    //statistic shape uncertainties 
    TH1D* bdtShape[nCr][(const size_t) proc.size()] ; //shape histograms of bdt
    TH1D* bdtShapeStatDown[nCr][(const size_t) proc.size()][nBinsFit]; //statistical shape uncertainty on bdt
    TH1D* bdtShapeStatUp[nCr][(const size_t) proc.size()][nBinsFit]; //statistical shape uncertainty on bdt

    //set up statistical shape uncertainties 
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned p = 0; p < proc.size(); ++p){
            for(unsigned bin = 0; bin < nBinsFit; ++bin){

                //stat shape nuisances are clones of the nominal histogram, with one bin varied up and down
                double variedBinContent = mergedHists[cr][0][p]->GetBinContent(bin + 1);
                double variedBinUnc = mergedHists[cr][0][p]->GetBinError(bin + 1);

                double binContentStatDown = std::max( variedBinContent - variedBinUnc, std::numeric_limits< double >::min() );
                bdtShapeStatDown[cr][p][bin] = (TH1D*)  mergedHists[cr][0][p]->Clone();
                bdtShapeStatDown[cr][p][bin]->SetBinContent(bin + 1, binContentStatDown );

                double binContentStatUp = std::max( variedBinContent + variedBinUnc, std::numeric_limits< double >::min() );
                bdtShapeStatUp[cr][p][bin] = (TH1D*)  mergedHists[cr][0][p]->Clone();
                bdtShapeStatUp[cr][p][bin]->SetBinContent(bin + 1, binContentStatUp );
            }
        }

    }

    //make datacard for each category
    for(unsigned cr = 0; cr < nCr; ++cr){

        //Set bkg yields 
        double bkgYields[(const size_t) proc.size() - 2];
        for(unsigned bkg = 0; bkg < proc.size() - 2; ++bkg) bkgYields[bkg] = mergedHists[cr][0][2 + bkg]->GetSumOfWeights();

        //set statical shape names ( to be sure they are independent for every category )
        for(unsigned p = 0; p < nBkg + 1; ++p){
            for(unsigned bin = 0; bin < nBinsFit; ++bin){
               systNames[nGeneralSystematics + p*nBinsFit + bin]  = processNames[p] + "_stat_" + crNames[cr] + "_2017_bin_" + std::to_string(bin + 1);
            }
        }

        //set BDT shape histogram
        TFile* shapeFile =TFile::Open((const TString&) "./datacards/shapes/shapeFile_"  + crNames[cr] +  "_2017.root", "recreate");

        //loop over all processes 
        for(unsigned p = 0; p < proc.size(); ++p){

            //write nominal histograms 
            bdtShape[cr][p] = (TH1D*) mergedHists[cr][0][p]->Clone();
            if(p == 0) bdtShape[cr][p]->Write("data_obs");
            else if (p == 1) bdtShape[cr][p]->Write("tZq");
            else bdtShape[cr][p]->Write((const TString&) bkgNames[p -2]);


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
                    if( isCorrelatedBetweenProc[key] ){
                        mergedUncMapDown[key][cr][0][p]->Write( (const TString&) procName + "_" + key + "Down");
                        mergedUncMapUp[key][cr][0][p]->Write( (const TString&) procName + "_" + key + "Up");
                    } else {
                       mergedUncMapDown[key][cr][0][p]->Write( (const TString&) procName + "_" + procName + "_" + key + "Down");
                       mergedUncMapUp[key][cr][0][p]->Write( (const TString&) procName + "_" + procName + "_" + key + "Up");
                    }
                }

                //write statistical shape nuisances 
                for(unsigned bin = 0; bin < nBinsFit; ++bin){
                    bdtShapeStatDown[cr][p][bin]->Write( (const TString&) procName + "_" + systNames[nGeneralSystematics + (p - 1)*nBinsFit + bin] + "Down");
                    bdtShapeStatUp[cr][p][bin]->Write( (const TString&) procName + "_" + systNames[nGeneralSystematics + (p - 1)*nBinsFit + bin] + "Up");
                }
            }
        }
        shapeFile->Close();

        analysisTools::printDataCard( mergedHists[cr][0][0]->GetSumOfWeights(), mergedHists[cr][0][1]->GetSumOfWeights(), "tZq", bkgYields, proc.size() - 2, bkgNames, systUnc, nSyst, &systNames[0], systDist, "datacards/datacard_" + crNames[cr] + "_2017", true, "shapes/shapeFile_"  + crNames[cr] + "_2017");

    }
    std::cout << "Printed all datacards" << std::endl;

   	//initialize postFitScaler
    PostFitScaler postFitScaler("total_postFit_yields.txt");

    //plot all distributions
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){

            //scale histograms to post fit plots
            for(unsigned p = 1; p < proc.size(); ++p){
                mergedHists[cr][dist][p]->Scale( postFitScaler.postFitScaling(  mergedHists[cr][dist][p]->GetSumOfWeights() ) );
            }

            plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/2017/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_" + crNames[cr] + "_2017_postFit", "tzq", false, false, "41.5 fb^{-1} (13 TeV)", totalSystUnc[cr][dist], isSMSignal);             //linear plots

            plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/2017/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_"  + crNames[cr] + "_2017_postFit" + "_log", "tzq", true, false, "41.5 fb^{-1} (13 TeV)", totalSystUnc[cr][dist], isSMSignal);    //log plots

        }
    }
 
}
int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
