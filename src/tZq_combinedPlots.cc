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
#include "../interface/PsUncertaintyReader.h"
#include "../interface/PostFitScaler.h"
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
    readSamples2016("sampleLists/samples_nonpromptDataDriven_2016.txt", "../../ntuples_tzq");
    readSamples2017("sampleLists/samples_nonpromptDataDriven_2017.txt", "../../ntuples_tzq");

    //name      xlabel    nBins,  min, max
    histInfo = {

        //new BDT distribution
        HistInfo("bdt", "BDT output", 15, -1, 1),
        HistInfo("bdt_10bins", "BDT output", 10, -1, 1),

        HistInfo("taggedRecoilJetEta", "|#eta| (recoiling jet) (GeV)", 15, 0, 5),
        HistInfo("maxMJetJet", "M_{jet + jet}^{max} (GeV)", 15, 0, 1200),
        HistInfo("asymmetryWLep", "asymmetry (lepton from W)",15, -2.5, 2.5),
        HistInfo("highestDeepCSV", "highest deepCSV (b + bb)", 15, 0, 1),
        HistInfo("LTPlusMET", "L_{T} + E_{T}^{miss} (GeV)", 15, 0, 800),
        HistInfo("maxDeltaPhiJetJet", "max(#Delta#Phi(jet, jet))", 15, 0, 3.15),
        HistInfo("mt", "M_{T} (GeV)", 15, 0, 300),
        HistInfo("mtop", "M_{(W + b)} (GeV)", 15, 0, 400),
        HistInfo("maxpTJetJet", "P_{T}^{max}(jet + jet) (GeV)", 15, 0, 300), 
        HistInfo("minDeltaPhiLeptonBJet", "min(#Delta#Phi(l, bjet))", 15, 0, 3.15),
        HistInfo("maxDeltaPhiLeptonLepton", "max(#Delta#Phi(l, l))", 15, 0, 3.15),
        HistInfo("HT", "H_{T} (GeV)", 15, 0, 800), 
        HistInfo("deltaRTaggedBJetRecoilingJet", "#DeltaR(tagged b-jet, recoiling jet)", 15, 0, 10),
        HistInfo("deltaRWlepTaggedbJet", "#DeltaR(lepton from W, tagged b-jet)", 15, 0, 7),
        HistInfo("m3l", "M_{3l} (GeV)", 15, 0, 600),
        HistInfo("jetEta_highestEta", "|#eta| (most forward jet)", 15, 0, 5),
        HistInfo("nJets", "number of jets", 10, 0, 10),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 15, 25, 200),
        HistInfo("subPt", "P_{T}^{subleading} (GeV)", 15, 15, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 15, 10, 200),
        HistInfo("flavors", "flavors", 4, 0, 4, {"eee", "ee#mu", "e#mu#mu", "#mu#mu#mu"} )
    };

    const unsigned nDist = histInfo.size(); //number of distributions to plot
    const unsigned nCat = 3;                //Several categories enriched in different processes
    const unsigned nMll = 1;                //categories based on dilepton Mass
    const std::string mllNames[nMll] = {"onZ"};
    const std::string catNames[nCat] = {"1bJet23Jets", "1bJet4Jets", "2bJets"};

    //initialize vector holding all histograms
    std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > > hists(nMll);
    for(unsigned m = 0; m < nMll; ++m){
        hists[m] = std::vector< std::vector < std::vector< std::shared_ptr< TH1D > > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                hists[m][cat].push_back(std::vector < std::shared_ptr< TH1D > >() );
                for(size_t sam = 0; sam < samples.size() + 2; ++sam){
                    if(sam < samples.size()){
                        hists[m][cat][dist].push_back(histInfo[dist].makeHist(catNames[cat] + mllNames[m] + samples[sam].getUniqueName()) );
                    } else {
                        if( sam == samples.size() ){
                            //extra histogram for nonprompt prediction
                            hists[m][cat][dist].push_back( histInfo[dist].makeHist(catNames[cat] + mllNames[m] + "nonprompt_2016") ); 
                        } else {
                            hists[m][cat][dist].push_back( histInfo[dist].makeHist(catNames[cat] + mllNames[m] + "nonprompt_2017") );
                        }
                    }
                }
            }
        }
    }

    const std::vector< std::string > uncNames = {"JEC_2016", "JEC_2017", "uncl", "scale", "pileup", "bTag_udsg_2016", "bTag_udsg_2017", "bTag_bc_2016", "bTag_bc_2017", "isr", "fsr", "prefiring", "WZ_extrapolation",
        "lepton_reco", "muon_id_stat_2016", "muon_id_stat_2017", "electron_id_stat_2016", "electron_id_stat_2017", "lepton_id_syst", "pdf", "scaleXsec", "pdfXsec", "lumi_2016", "lumi_2017", "trigger_2016", "trigger_2017"};

    std::map < std::string, std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > >  > uncHistMapDown;
    std::map < std::string, std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > >  > uncHistMapUp;
    for( auto& key : uncNames ){
        uncHistMapDown[key] = std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > >(nMll);
        uncHistMapUp[key] = std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > >(nMll);
        for(unsigned m = 0; m < nMll; ++m){
            uncHistMapDown[key][m] = std::vector< std::vector < std::vector< std::shared_ptr< TH1D > > > >(nCat);
            uncHistMapUp[key][m] = std::vector< std::vector < std::vector< std::shared_ptr< TH1D > > > >(nCat);
            for(unsigned cat = 0; cat < nCat; ++cat){
                uncHistMapUp[key][m][cat] = std::vector < std::vector< std::shared_ptr< TH1D > > >( nDist );
                uncHistMapDown[key][m][cat] = std::vector < std::vector< std::shared_ptr< TH1D > > >( nDist );
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown[key][m][cat][dist] = std::vector< std::shared_ptr< TH1D > >(samples.size() + 2);
                    uncHistMapUp[key][m][cat][dist] = std::vector< std::shared_ptr< TH1D > >(samples.size() + 2);
                    for(size_t sam = 0; sam < samples.size() + 2; ++sam){
                        std::string sampleName;
                        if(sam < samples.size() ){
                            sampleName = samples[sam].getUniqueName();
                        } else {
                            //the nonprompt prediction is also affected by all nuisances through the prompt MC subtraction
                            if( sam == samples.size() ){
                                sampleName = "nonprompt_2016";
                            } else {
                                sampleName = "nonprompt_2017";
                            }
                        }
                        uncHistMapDown[key][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + sampleName + key + "Down");
                        uncHistMapUp[key][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + sampleName + key + "Up");
                    }
                }
            }
        }
    }
    
    std::vector< std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > > > pdfUncHists(100);
    for(unsigned pdf = 0; pdf < 100; ++pdf){
        pdfUncHists[pdf] = std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > >(nMll);
        for(unsigned m = 0; m < nMll; ++m){
            pdfUncHists[pdf][m] = std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > >(nCat); 
            for(unsigned cat = 0; cat < nCat; ++cat){
                pdfUncHists[pdf][m][cat] = std::vector < std::vector< std::shared_ptr< TH1D > > >( nDist );
                for(unsigned dist = 0; dist < nDist; ++dist){
                    pdfUncHists[pdf][m][cat][dist] = std::vector< std::shared_ptr< TH1D > >( samples.size() + 2);
                    for(size_t sam = 0; sam < samples.size() + 2; ++sam){
                        std::string sampleName;
                        if(sam < samples.size() ){
                            sampleName = samples[sam].getUniqueName();
                        } else {
                            if( sam == samples.size() ){
                                sampleName = "nonprompt_2016";
                            } else {
                                sampleName = "nonprompt_2017";
                            }
                        } 
                        pdfUncHists[pdf][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + sampleName + "pdf" + std::to_string(pdf) );
                    }
                }
            }
        }
    }

	//initialize Ps uncertainty reader
    PsUncertaintyReader psUncertaintyReader2016("weights/psUnc_2016.txt");
    PsUncertaintyReader psUncertaintyReader2017("weights/psUnc_2017.txt");

    //tweakable options
    const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){

        initSample();          //2 = combined luminosity
        std::cout<<"Entries in "<< currentSample.getFileName() << " " << nEntries << std::endl;

        //reinitialize BDT for each sample to make sure correct BDT is used 
        std::vector < std::string > bdtVars1bJet23Jets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "mTW", "pTMaxjj", "minDeltaPhilb", "maxDeltaPhill"};
        std::string weights;
        if( is2016() ){
            weights = "1bJet23Jets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
        } else{
            weights = "1bJet23Jets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
        }
        bdtReader1bJet23Jets = std::shared_ptr<BDTReader>( new BDTReader("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars1bJet23Jets) );

        //BDT reader for 1bJet4Jets category
        std::vector < std::string > bdtVars1bJet4Jets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "ht", "mTW", "numberOfJets", "maxDeltaPhill", "minDeltaPhilb",
            "deltaRTaggedbJetRecoilingJet", "deltaRWLeptonTaggedbJet", "m3l", "etaMostForward"};
        if( is2016() ){
            weights = "1bJet4Jets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
        } else{
            weights = "1bJet4Jets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
        } 
        bdtReader1bJet4Jets = std::shared_ptr<BDTReader>( new BDTReader("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars1bJet4Jets) );

        //BDT reader for 2bJets category
        std::vector < std::string > bdtVars2bJets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "ht", "mTW", "numberOfJets", "maxDeltaPhill", "etaMostForward", "m3l"}; 
        if( is2016() ){
            weights = "2bJets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
        } else{
            weights = "2bJets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
        }
        bdtReader2bJets = std::shared_ptr<BDTReader>( new BDTReader("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars2bJets) );

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
            if(lCount != 3) continue;

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
                fillIndex = (is2016() ? samples.size() : samples.size() + 1 ); 

                //apply fake-rate weights 
                weight *= fakeRateWeight(); 
            }

            //remove overlap between samples
            if(photonOverlap(currentSample, false)) continue;

            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);

            //require best Z mass to be onZ
            std::pair<unsigned, unsigned> bestZ = trilep::bestZ(lepV, ind, _lFlavor, _lCharge, lCount);
            double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();
            if( fabs(mll - 91.1876) >= 15) continue;
            unsigned mllCat = 0;

            //compute nominal values for all search variables
            unsigned tzqCat = setSearchVariablestZq("nominal", ind, bestZ);             

            //event reweighting
            if( isMC() ){
                weight *= sfWeight(); 
                if( weight == 0.){
                    continue;
                }
            }

            //keys to use for JEC variation (make sure to only change JEC for specific year)
            std::string JECKeyChanged = std::string("JEC_") + (is2016() ? "2016" : "2017");
            std::string JECKeyUnchanged= std::string("JEC_") + (is2016() ? "2017" : "2016");

            //compute nominal bdt value
            double bdtNominal = 999.;
            if(tzqCat > 2 && tzqCat < 6){
                bdtNominal = bdtOutput( tzqCat );
                double fill[nDist] = {bdtNominal, bdtNominal,
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
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    (double) trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) + 0.5
                    };
                for(unsigned dist = 0; dist < nDist; ++dist){
                    hists[mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }

                //in case of data fakes fill all uncertainties for nonprompt with nominal values
                if( isData() && !passTightCut){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        for(auto& key: uncNames){
                            uncHistMapDown[key][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                            uncHistMapUp[key][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                        }
                        for(unsigned pdf = 0; pdf < 100; ++pdf){
                             pdfUncHists[pdf][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                        }
                    }
                } else if( isMC() ){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        uncHistMapDown[JECKeyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                        uncHistMapUp[JECKeyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                    }
                }
            }

            //no nuisances for data
            if( isData() ) continue;

            //vary JEC Down
            unsigned tzqCatJECDown = setSearchVariablestZq("JECDown", ind, bestZ);
            if(tzqCatJECDown > 2 && tzqCatJECDown < 6){
                double bdtJECDown = bdtOutput( tzqCatJECDown );
                double fill[nDist] = {bdtJECDown, bdtJECDown,
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
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    (double) trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) + 0.5
                    };
 
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown[JECKeyChanged][mllCat][tzqCatJECDown - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary JEC up
            unsigned tzqCatJECUp = setSearchVariablestZq("JECUp", ind, bestZ);
            if(tzqCatJECUp > 2 && tzqCatJECUp < 6){
                double bdtJECUp = bdtOutput( tzqCatJECUp );
                double fill[nDist] = {bdtJECUp, bdtJECUp,
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
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    (double) trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp[JECKeyChanged][mllCat][tzqCatJECUp - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary unclustered down
            unsigned tzqCatUnclDown = setSearchVariablestZq("unclDown", ind, bestZ);
            if(tzqCatUnclDown > 2 && tzqCatUnclDown < 6){
                double bdtUnclDown = bdtOutput( tzqCatUnclDown );
                double fill[nDist] = {bdtUnclDown, bdtUnclDown,
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
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    (double) trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown["uncl"][mllCat][tzqCatUnclDown - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary unclustered up
            unsigned tzqCatUnclUp = setSearchVariablestZq("unclUp", ind, bestZ);
            if(tzqCatUnclUp  > 2 && tzqCatUnclUp < 6){
                double bdtUnclUp = bdtOutput( tzqCatUnclUp );
                double fill[nDist] = {bdtUnclUp, bdtUnclUp,
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
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    (double) trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp["uncl"][mllCat][tzqCatUnclUp - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //now nominal cuts can be safely used
            if(tzqCat < 3 || tzqCat > 5) continue;
            
            //reset nominal values 
            setSearchVariablestZq("nominal", ind, bestZ);
            double fill[nDist] = {bdtNominal, bdtNominal,
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
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    (double) trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) + 0.5
                    };


            //set pdf and scale weight for GluGluToContinToZZ samples which do not include lhe weights
            if( currentSample.getFileName().find("GluGluToContinToZZ") != std::string::npos ){
                for(unsigned lhe = 0; lhe < 110; ++lhe){
                    _lheWeight[lhe] = 1.;
                }
            }
 
            //vary scale down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["scale"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[8]);
            }

            //vary scale up
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["scale"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[4]);
            }

            //vary pu down
            double puDownWeight = puWeight(1)/puWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["pileup"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puDownWeight);
            }

            //vary pu up            
            double puUpWeight = puWeight(2)/puWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["pileup"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puUpWeight);
            }

            //vary b-tag down for udsg
            std::string bTag_udsg_keyChanged = std::string("bTag_udsg_") + ( is2016() ? "2016" : "2017" );
            std::string bTag_udsg_keyUnchanged = std::string("bTag_udsg_") + ( is2016() ? "2017" : "2016" );
            double bTag_udsg_downWeight = bTagWeight_udsg(1)/bTagWeight_udsg(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[bTag_udsg_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_udsg_downWeight);
                uncHistMapDown[bTag_udsg_keyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary b-tag up for udsg
            double bTag_udsg_upWeight = bTagWeight_udsg(2)/bTagWeight_udsg(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp[bTag_udsg_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_udsg_upWeight);
                uncHistMapUp[bTag_udsg_keyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary b-tag down for b and c (correlated)
            std::string bTag_bc_keyChanged = std::string("bTag_bc_") + ( is2016() ? "2016" : "2017" );
            std::string bTag_bc_keyUnchanged = std::string("bTag_bc_") + ( is2016() ? "2017" : "2016" );
            double bTag_bc_downWeight = bTagWeight_c(1)*bTagWeight_b(1)/ (bTagWeight_c(0)*bTagWeight_b(0) );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[bTag_bc_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_bc_downWeight);
                uncHistMapDown[bTag_bc_keyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary b-tag up for b and c (correlated)
            double bTag_bc_upWeight = bTagWeight_c(2)*bTagWeight_b(2)/ (bTagWeight_c(0)*bTagWeight_b(0) );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp[bTag_bc_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_bc_upWeight);
                uncHistMapUp[bTag_bc_keyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

			double isrDownUnc = 0;
            double isrUpUnc = 0;
            double fsrDownUnc = 0;
            double fsrUpUnc = 0;
            if( currentSample.getFileName() == "tZq_ll_4f_13TeV-amcatnlo-pythia8_Summer16.root" || currentSample.getFileName() == "TTZToLLNuNu_M-10_TuneCUETP8M1_13TeV-amcatnlo-pythia8_Summer16.root"){
                isrDownUnc = psUncertaintyReader2016.getISRUncertaintyDown( currentSample.getProcessName(), catNames[tzqCat - 3], uncHistMapDown["isr"][mllCat][tzqCat - 3][0][fillIndex].get(), bdtNominal );
                isrUpUnc = psUncertaintyReader2016.getISRUncertaintyUp( currentSample.getProcessName(), catNames[tzqCat - 3], uncHistMapUp["isr"][mllCat][tzqCat - 3][0][fillIndex].get(), bdtNominal );
                fsrDownUnc = psUncertaintyReader2016.getFSRUncertaintyDown( currentSample.getProcessName(), catNames[tzqCat - 3], uncHistMapDown["fsr"][mllCat][tzqCat - 3][0][fillIndex].get(), bdtNominal );
                fsrUpUnc = psUncertaintyReader2016.getFSRUncertaintyUp( currentSample.getProcessName(), catNames[tzqCat - 3], uncHistMapUp["fsr"][mllCat][tzqCat - 3][0][fillIndex].get(), bdtNominal );

            } else if( currentSample.getFileName() == "tZq_ll_4f_ckm_NLO_TuneCP5_PSweights_13TeV-amcatnlo-pythia8_realistic_v10_Fall17_withPsWeights.root" || currentSample.getFileName() == "TTZToLLNuNu_M-10_TuneCP5_PSweights_13TeV-amcatnlo-pythia8_realistic_v14_Fall17_withPsWeights.root"){
                isrDownUnc = psUncertaintyReader2017.getISRUncertaintyDown( currentSample.getProcessName(), catNames[tzqCat - 3], uncHistMapDown["isr"][mllCat][tzqCat - 3][0][fillIndex].get(), bdtNominal );
                isrUpUnc = psUncertaintyReader2017.getISRUncertaintyUp( currentSample.getProcessName(), catNames[tzqCat - 3], uncHistMapUp["isr"][mllCat][tzqCat - 3][0][fillIndex].get(), bdtNominal );
                fsrDownUnc = psUncertaintyReader2017.getFSRUncertaintyDown( currentSample.getProcessName(), catNames[tzqCat - 3], uncHistMapDown["fsr"][mllCat][tzqCat - 3][0][fillIndex].get(), bdtNominal );
                fsrUpUnc = psUncertaintyReader2017.getFSRUncertaintyUp( currentSample.getProcessName(), catNames[tzqCat - 3], uncHistMapUp["fsr"][mllCat][tzqCat - 3][0][fillIndex].get(), bdtNominal );
            }	

            for(unsigned dist = 0; dist < nDist; ++dist){
                //vary isr down
                uncHistMapDown["isr"][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( 1 + isrDownUnc) );

                //vary isr up
                uncHistMapUp["isr"][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ),  weight*( 1 + isrUpUnc) );

                //vary fsr down
                uncHistMapDown["fsr"][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( 1 + fsrDownUnc) );

                //vary fsr up
                uncHistMapUp["fsr"][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( 1 + fsrUpUnc) );
            }


            //vary jet prefiring probabilities down
            double prefiringDownWeight = jetPrefiringWeight(1)/jetPrefiringWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["prefiring"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*prefiringDownWeight );
            }

            //vary jet prefiring probabilities up
            double prefiringUpWeight = jetPrefiringWeight(2)/jetPrefiringWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["prefiring"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*prefiringUpWeight );
            }

            //extrapolation uncertainty for WZ from CR to SR
            double WZExtrapolationUnc;
            if( (currentSample.getProcessName() == "WZ") && ( bdtVariableMap["numberOfbJets"] > 0 ) ){
                WZExtrapolationUnc = 0.08;
            } else {
                WZExtrapolationUnc = 0.;
            }
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["WZ_extrapolation"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( 1. - WZExtrapolationUnc) );
                uncHistMapUp["WZ_extrapolation"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( 1. + WZExtrapolationUnc) );
            }

            //vary lepton reco SF down
            double leptonRecoDownWeight = leptonWeight("recoDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["lepton_reco"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonRecoDownWeight );
            }

            //vary lepton reco SF up
            double leptonRecoUpWeight = leptonWeight("recoUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["lepton_reco"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonRecoUpWeight);
            }

            //vary muon stat down
            std::string muon_id_stat_keyChanged = std::string("muon_id_stat_") + ( is2016() ? "2016" : "2017" );
            std::string muon_id_stat_keyUnchanged = std::string("muon_id_stat_") + ( is2016() ? "2017" : "2016" );
            double muonStatDownWeight = leptonWeight("muon_idStatDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[muon_id_stat_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*muonStatDownWeight);
                uncHistMapDown[muon_id_stat_keyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary muon stat up
            double muonStatUpWeight = leptonWeight("muon_idStatUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp[muon_id_stat_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*muonStatUpWeight);
                uncHistMapUp[muon_id_stat_keyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary electron stat down
            std::string electron_id_stat_keyChanged = std::string("electron_id_stat_") + ( is2016() ? "2016" : "2017" );
            std::string electron_id_stat_keyUnchanged = std::string("electron_id_stat_") + ( is2016() ? "2017" : "2016" );
            double electronStatDownWeight = leptonWeight("electron_idStatDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[electron_id_stat_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*electronStatDownWeight);
                uncHistMapDown[electron_id_stat_keyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary electron stat up
            double electronStatUpWeight = leptonWeight("electron_idStatUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp[electron_id_stat_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*electronStatUpWeight);
                uncHistMapUp[electron_id_stat_keyUnchanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary lepton syst down
            double leptonIdSystDownWeight = leptonWeight("idSystDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["lepton_id_syst"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonIdSystDownWeight );
            }
            
            //vary lepton syst up
            double leptonIdSystUpWeight = leptonWeight("idSystUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["lepton_id_syst"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonIdSystUpWeight);
            }

            //lumi uncertainty, fill it here to properly address the fact that it is uncorrelated between 2016 and 2017
            double lumiUnc = ( is2016() ? 0.025 : 0.023 );
            std::string lumi_keyChanged = std::string("lumi_") + (is2016() ? "2016" : "2017" );
            std::string lumi_keyUnChanged = std::string("lumi_") + (is2016() ? "2017" : "2016" );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[lumi_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*(1. - lumiUnc) );
                uncHistMapUp[lumi_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*(1. + lumiUnc) );
                uncHistMapDown[lumi_keyUnChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight );
                uncHistMapUp[lumi_keyUnChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight );
            } 

            double triggerUnc = 0.02;
            std::string trigger_keyChanged = std::string("trigger_") + (is2016() ? "2016" : "2017" );
            std::string trigger_keyUnChanged = std::string("trigger_") + (is2016() ? "2017" : "2016" );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[trigger_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*(1. - triggerUnc) );
                uncHistMapUp[trigger_keyChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*(1. + triggerUnc) );
                uncHistMapDown[trigger_keyUnChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight );
                uncHistMapUp[trigger_keyUnChanged][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight );
            } 

            //100 pdf variations
            for(unsigned pdf = 0; pdf < 100; ++pdf){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    pdfUncHists[pdf][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[pdf + 9]);
                }
            }
        }
        //set histograms to 0 if negative
        for(unsigned m = 0; m < nMll; ++m){
            for(unsigned cat = 0; cat < nCat; ++cat){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    analysisTools::setNegativeBinsToZero( hists[m][cat][dist][sam].get() );
                    for(auto & key : uncNames){
                        analysisTools::setNegativeBinsToZero(uncHistMapDown[key][m][cat][dist][sam].get() );
                        analysisTools::setNegativeBinsToZero(uncHistMapUp[key][m][cat][dist][sam].get() );
                    } 
                    for(unsigned pdf = 0; pdf < 100; ++pdf){    
                        analysisTools::setNegativeBinsToZero(pdfUncHists[pdf][m][cat][dist][sam].get() );
                    }
                }	
            }
        }
    }

    std::cout << "crash 1" << std::endl;
    //set nonprompt bins to 0 if negative
    for(unsigned m = 0; m < nMll; ++m){
        for( unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                
                //loop here to set both 2016 and 2017 nonprompt to zero
                for(unsigned i = 0; i < 2; ++i){
                    unsigned nonpromptIndex = samples.size() + i;
                    analysisTools::setNegativeBinsToZero( hists[m][cat][dist][nonpromptIndex].get() );
                    for(auto & key : uncNames){
                        analysisTools::setNegativeBinsToZero(uncHistMapDown[key][m][cat][dist][nonpromptIndex].get() );
                        analysisTools::setNegativeBinsToZero(uncHistMapUp[key][m][cat][dist][nonpromptIndex].get() );
                    } 
                    for(unsigned pdf = 0; pdf < 100; ++pdf){    
                        analysisTools::setNegativeBinsToZero(pdfUncHists[pdf][m][cat][dist][nonpromptIndex].get() );
                    }
                }
            }
        }
    }

    std::cout << "crash 2" << std::endl;
    //compute lhe cross section ratio for every sample and variation
    std::vector< std::vector< double> > crossSectionRatio(samples.size(), std::vector<double>(110, 1.) );
    for(unsigned sam = 1; sam < samples.size(); ++sam){

        if(samples[sam].isData() ) continue;
        
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

    std::cout << "crash 2" << std::endl;
    //compute final pdf uncertainties
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                for(unsigned sam = 0; sam < samples.size() + 2; ++sam){

                    if(sam < samples.size() ){
                        if( samples[sam].isData() ) continue; 
                    }

                    for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["pdf"][m][cat][dist][sam]->GetNbinsX() + 1; ++bin){
                        double pdfVarRms = 0.;
                        for(unsigned pdf = 0; pdf < 100; ++pdf){

                            double variedBin = pdfUncHists[pdf][m][cat][dist][sam]->GetBinContent(bin);
                            if( !(sam >= samples.size() ) ){
                                variedBin *= crossSectionRatio[sam][pdf + 9];
                            }
                            double diff = (  variedBin - hists[m][cat][dist][sam]->GetBinContent(bin) );
                            pdfVarRms += diff * diff;
                        }
                        pdfVarRms = sqrt( 0.01 * pdfVarRms );
                        uncHistMapDown["pdf"][m][cat][dist][sam]->SetBinContent( bin, hists[m][cat][dist][sam]->GetBinContent(bin) - pdfVarRms);
                        uncHistMapUp["pdf"][m][cat][dist][sam]->SetBinContent( bin, hists[m][cat][dist][sam]->GetBinContent(bin) + pdfVarRms);
                    }
                }
            }
        }
    }

    std::cout << "crash 3" << std::endl;
    //add pdf and scale variation uncertainties of the cross-section
    std::vector< double > scaleXsecUncDown = {0.};  //first dummy entry for data
    std::vector< double > scaleXsecUncUp = {0.};
    std::vector< double > pdfXsecUncDown = {0.};
    std::vector< double > pdfXsecUncUp = {0.};

    for( unsigned sam = 1; sam < samples.size(); ++sam){ 

        if( samples[sam].isData() ) continue;

        //scale effect on xSec :
        double scaleDownUnc = (1./crossSectionRatio[sam][8] - 1.); //Warning: nominal sum of weights has to be in the denominator!
        scaleXsecUncDown.push_back( scaleDownUnc );

        double scaleUpUnc = (1./crossSectionRatio[sam][4]- 1.); //Warning: nominal sum of weights has to be in the denominator!
        scaleXsecUncUp.push_back( scaleUpUnc );

        //filter out cross section effects from scale shape uncertainty
        if( samples[sam].getFileName().find("GluGluToContinToZZ") == std::string::npos){
            for(unsigned m = 0; m < nMll; ++m){
                for(unsigned cat = 0; cat < nCat; ++cat){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        uncHistMapDown["scale"][m][cat][dist][sam]->Scale(crossSectionRatio[sam][8]);
                        uncHistMapUp["scale"][m][cat][dist][sam]->Scale(crossSectionRatio[sam][4]);
                    }
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

    std::cout << "crash 4" << std::endl;
    //add pdf and scale effects to shape uncertainties to automatically take into account the fractional effects from every sample that is merged
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                for(unsigned sam = 0; sam < samples.size() + 2; ++sam){
                    if(sam < samples.size() ){
                        for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["scaleXsec"][m][cat][dist][sam]->GetNbinsX() + 1; ++bin){
                            uncHistMapDown["scaleXsec"][m][cat][dist][sam]->SetBinContent(bin, (1 + scaleXsecUncDown[sam])*hists[m][cat][dist][sam]->GetBinContent(bin) );
                            uncHistMapUp["scaleXsec"][m][cat][dist][sam]->SetBinContent(bin, (1 + scaleXsecUncUp[sam])*hists[m][cat][dist][sam]->GetBinContent(bin) );
                            uncHistMapDown["pdfXsec"][m][cat][dist][sam]->SetBinContent(bin, (1 + pdfXsecUncDown[sam])*hists[m][cat][dist][sam]->GetBinContent(bin) );
                            uncHistMapUp["pdfXsec"][m][cat][dist][sam]->SetBinContent(bin, (1 + pdfXsecUncUp[sam])*hists[m][cat][dist][sam]->GetBinContent(bin) );
                        }
                    //WARNING: CURRENTLY THE SCALE AND PDF XSEC UNCERTAINTIES ON DATA-DRIVEN FAKES ARE SET TO ZERO
                    } else {
                        for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["scaleXsec"][m][cat][dist][sam]->GetNbinsX() + 1; ++bin){
                            uncHistMapDown["scaleXsec"][m][cat][dist][sam]->SetBinContent(bin, hists[m][cat][dist][sam]->GetBinContent(bin) );
                            uncHistMapUp["scaleXsec"][m][cat][dist][sam]->SetBinContent(bin, hists[m][cat][dist][sam]->GetBinContent(bin) );
                            uncHistMapDown["pdfXsec"][m][cat][dist][sam]->SetBinContent(bin, hists[m][cat][dist][sam]->GetBinContent(bin) );
                            uncHistMapUp["pdfXsec"][m][cat][dist][sam]->SetBinContent(bin, hists[m][cat][dist][sam]->GetBinContent(bin) );
                        }
                    }
                }
            }
        }
    }

    std::cout << "crash 5" << std::endl;
    //merge histograms with the same physical background
    std::vector<std::string> proc = {"Data", "tZq", "WZ", "Multiboson", "t#bar{t} + Z", "t#bar{t}/t + X", "X + #gamma", "ZZ/H", "Nonprompt e/#mu"};
    std::vector< std::vector< std::vector< std::vector< TH1D* > > > > mergedHists(nMll);
    for(unsigned mll = 0; mll < nMll; ++mll){
        mergedHists[mll] = std::vector< std::vector < std::vector < TH1D* > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                mergedHists[mll][cat].push_back(std::vector<TH1D*>(proc.size() ) );

                //cut off loop before nonprompt contribution
                for(size_t m = 0, sam = 0; m < proc.size() - 1; ++m){
                    mergedHists[mll][cat][dist][m] = (TH1D*) hists[mll][cat][dist][sam]->Clone();
                
                    //look for each sample with the same name 
                    std::string currentName = samples[sam].getProcessName();

                    //check how far to jump to next process 
                    int nextProcessIndex = -1;
                    for(size_t match = sam + 1; match < samples.size(); ++match){
                        std::string nameToCompare = samples[match].getProcessName();
                        if(currentName == nameToCompare){
                            mergedHists[mll][cat][dist][m]->Add( hists[mll][cat][dist][match].get() );
                        } else if( nextProcessIndex == -1 ){
                            nextProcessIndex = match;
                        }
                    }
                    sam = nextProcessIndex;
                }

                //add nonprompt histogram for 2016
                mergedHists[mll][cat][dist][proc.size() - 1] = (TH1D*) hists[mll][cat][dist][samples.size()]->Clone();

                //add nonprompt histogram for 2017
                mergedHists[mll][cat][dist][proc.size() - 1]->Add( hists[mll][cat][dist][samples.size() + 1].get() );
            }
        }
    }

    std::cout << "crash 6" << std::endl;

    //merging for uncertainties
    std::map< std::string, std::vector< std::vector< std::vector< std::vector< TH1D* > > > > > mergedUncMapDown;
    std::map< std::string, std::vector< std::vector< std::vector< std::vector< TH1D* > > > > > mergedUncMapUp;
    for( auto& key : uncNames ){
        mergedUncMapDown[key] = std::vector< std::vector< std::vector< std::vector< TH1D* > > > >(nMll);
        mergedUncMapUp[key] = std::vector< std::vector< std::vector< std::vector< TH1D* > > > >(nMll);
        for(unsigned mll = 0; mll < nMll; ++mll){
            mergedUncMapDown[key][mll] = std::vector< std::vector < std::vector < TH1D* > > >(nCat);
            mergedUncMapUp[key][mll] = std::vector< std::vector < std::vector < TH1D* > > >(nCat);
            for(unsigned cat = 0; cat < nCat; ++cat){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    mergedUncMapDown[key][mll][cat].push_back(std::vector<TH1D*>(proc.size() ) );
                    mergedUncMapUp[key][mll][cat].push_back(std::vector<TH1D*>(proc.size() ) );

                    //cut off loop before nonprompt contribution
                    for(size_t m = 0, sam = 0; m < proc.size() - 1; ++m){
                        mergedUncMapDown[key][mll][cat][dist][m] = (TH1D*) uncHistMapDown[key][mll][cat][dist][sam]->Clone();
                        mergedUncMapUp[key][mll][cat][dist][m] = (TH1D*) uncHistMapUp[key][mll][cat][dist][sam]->Clone();

                        //look for each sample with the same name 
                        std::string currentName = samples[sam].getProcessName();

                        //check how far to jump to next process 
                        int nextProcessIndex = -1;
                        for(size_t match = sam + 1; match < samples.size(); ++match){
                            std::string nameToCompare = samples[match].getProcessName();
                            if(currentName == nameToCompare){
                                mergedUncMapDown[key][mll][cat][dist][m]->Add( uncHistMapDown[key][mll][cat][dist][match].get() );
                                mergedUncMapUp[key][mll][cat][dist][m]->Add( uncHistMapUp[key][mll][cat][dist][match].get() );
                            } else if( nextProcessIndex == -1 ){
                                nextProcessIndex = match;
                            }
                        }
                        sam = nextProcessIndex;
                    }

                    //add nonprompt histograms for 2016
                    mergedUncMapDown[key][mll][cat][dist][proc.size() - 1] = (TH1D*) uncHistMapDown[key][mll][cat][dist][samples.size()]->Clone();
                    mergedUncMapUp[key][mll][cat][dist][proc.size() - 1] = (TH1D*) uncHistMapUp[key][mll][cat][dist][samples.size()]->Clone();

                    //add nonprompt histograms for 2017
                    mergedUncMapDown[key][mll][cat][dist][proc.size() - 1]->Add( uncHistMapDown[key][mll][cat][dist][samples.size() + 1].get() );
                    mergedUncMapUp[key][mll][cat][dist][proc.size() - 1]->Add( uncHistMapUp[key][mll][cat][dist][samples.size() + 1].get() );
                }
            }
        }
    } 

    std::cout << "crash 7" << std::endl;

    const std::vector< std::string > uncorrelatedBetweenProcesses = {"scale", "pdf", "scaleXsec", "pdfXsec"};

    //print the effect of systematic uncertainty
    //compute minimum and maximum impact on each bin
    for(unsigned bdt = 0; bdt < 2; ++bdt){
        std::cout << "##################################################" << std::endl;
        if(bdt == 0){
            std::cout << "uncetainty ranges for 30 BDT bins: " << std::endl;
        } else{
            std::cout << "uncetainty ranges for 10 BDT bins: " << std::endl;
        }
        for(auto& key : uncNames ){

            //check if nuisance is correlated or uncorrelated among processes 
            bool nuisanceIsUnCorrelated = ( std::find( uncorrelatedBetweenProcesses.cbegin(), uncorrelatedBetweenProcesses.cend(), key ) != uncorrelatedBetweenProcesses.cend() );

            double maxUnc = 0.;
            double minUnc = 999.;
            for(unsigned mll = 0; mll < nMll; ++mll){
                for(unsigned cat = 0; cat < nCat; ++cat){
                    for(unsigned bin = 1; bin < ( unsigned) mergedHists[mll][cat][bdt][0]->GetNbinsX() + 1; ++bin){

                        //for uncorrelated case
                        double binUnc = 0.;

                        //for correlated case
                        double binUncDown = 0.;
                        double binUncUp = 0.;

                        double binContent = 0.;
                        for(unsigned p = 1; p < proc.size(); ++p){
                            binContent += mergedHists[mll][cat][bdt][p]->GetBinContent(bin);
                            if( (key.find("Xsec") != std::string::npos) && !(proc[p] == "Multiboson" || proc[p] == "t#bar{t}/t + X") ){
                                continue;
                            }
                            if( (key == "WZ_extrapolation") && (proc[p] != "WZ") ){
                                continue;
                            }
                            double varDown = fabs( mergedHists[mll][cat][bdt][p]->GetBinContent(bin) - mergedUncMapDown[key][mll][cat][bdt][p]->GetBinContent(bin) );
                            double varUp = fabs( mergedHists[mll][cat][bdt][p]->GetBinContent(bin) - mergedUncMapUp[key][mll][cat][bdt][p]->GetBinContent(bin) );
                            
                            if(nuisanceIsUnCorrelated){
                                double maxVar = std::max(varDown, varUp);
                                binUnc += maxVar*maxVar;
                            } else {
                                binUncDown += varDown;
                                binUncUp += varUp;
                            }
                        }
                        if( nuisanceIsUnCorrelated ){
                            binUnc = sqrt(binUnc);
                        } else {
                            binUnc = std::max( binUncDown, binUncUp );
                        }

                        double fractionalUnc = binUnc/binContent;
                        if( fractionalUnc > maxUnc){
                            maxUnc = fractionalUnc;
                        }
                        if( fractionalUnc < minUnc){
                            minUnc = fractionalUnc;
                        }
                    }
                }
            }
            std::cout << "Uncertainty : " << key << "\t" << std::setprecision(2) << minUnc*100 << "% - " << maxUnc*100 << "%" << std::endl;
        }
    }

    std::cout << "crash 8" << std::endl;

    //make final uncertainty histogram for plots 
    //std::vector<double> flatUnc = {1.02}; //trigger
    std::vector<double> flatUnc = {} ; //all uncorrelated 
    std::map< std::string, double > backgroundSpecificUnc =        //map of background specific nuisances that can be indexed with the name of the process 
        {
            {"Nonprompt e/#mu", 1.3},
            {"WZ", 1.1},
            {"X + #gamma", 1.1},
            {"ZZ/H", 1.1},
            {"t#bar{t} + Z", 1.15} 
        };

    std::vector< std::string > ignoreTheoryUncInPlot = {"tZq", "WZ", "X + #gamma", "ZZ/H", "t#bar{t} + Z"};
    
    std::vector< std::vector< std::vector< TH1D* > > > totalSystUnc(nMll); //copy pointers to fix dimensionality of vector

    for( unsigned mll = 0; mll < nMll; ++mll){
        totalSystUnc[mll] = std::vector< std::vector< TH1D* > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            totalSystUnc[mll][cat] = std::vector< TH1D* >(nDist);
            for(unsigned dist = 0; dist < nDist; ++dist){
                totalSystUnc[mll][cat][dist] = (TH1D*) mergedHists[mll][cat][dist][0]->Clone();

                for(unsigned bin = 1; bin < (unsigned) totalSystUnc[mll][cat][dist]->GetNbinsX() + 1; ++bin){
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

                            double nominalContent = mergedHists[mll][cat][dist][p]->GetBinContent(bin);
                            double downVariedContent = mergedUncMapDown[key][mll][cat][dist][p]->GetBinContent(bin);
                            double upVariedContent = mergedUncMapUp[key][mll][cat][dist][p]->GetBinContent(bin);
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
							double binContent = mergedHists[mll][cat][dist][p]->GetBinContent(bin);
							double variation = binContent*(unc - 1.);
							var += variation;
						}
						binUnc += var*var;	
					}
                    
                    //add background specific uncertainties (uncorrelated between processes)
					for(auto& uncPair : backgroundSpecificUnc){
                        for(unsigned p = 1; p < proc.size(); ++p){
                            if(proc[p] == uncPair.first){
                                double var = mergedHists[mll][cat][dist][p]->GetBinContent(bin)*(uncPair.second - 1.);
                                binUnc += var*var;
                            }
                        }
					}

					//square root of quadratic sum is total uncertainty
                    totalSystUnc[mll][cat][dist]->SetBinContent(bin, sqrt(binUnc) );
                }
            }
        }
    }
    std::cout << "crash 9" << std::endl;
                    
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

    std::cout << "crash 10" << std::endl;

    const std::string plotCatLabels[3] = {"2-3 jets, 1 b-tagged", "#geq 4 jets, 1 b-tagged", "#geq 2 b jets"};

    //plot all distributions
    const bool isSMSignal[(const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/combined/final/" + catNames[cat] + "/" + histInfo[dist].name() + "_" + catNames[cat] + "_" + mllNames[m] + "_combined", "tzq", false, false, "77.4 fb^{-1} (13 TeV)", totalSystUnc[m][cat][dist], isSMSignal, nullptr, nullptr, 0, true, plotCatLabels[cat]);             //linear plots

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/combined/final/" + catNames[cat] + "/" + histInfo[dist].name() + "_"  + catNames[cat] + "_" + mllNames[m] + "_combined" + "_log", "tzq", true, false, "77.4 fb^{-1} (13 TeV)", totalSystUnc[m][cat][dist], isSMSignal, nullptr, nullptr, 0, true, plotCatLabels[cat]);    //log plots
            }
        }
    }



    //remerge histograms (separately for 2016 and 2017) for postfit scaling
    std::vector< std::vector< std::vector< std::vector< TH1D* > > > > mergedHists2016(nMll);
    std::vector< std::vector< std::vector< std::vector< TH1D* > > > > mergedHists2017(nMll);

    for(unsigned mll = 0; mll < nMll; ++mll){
        mergedHists2016[mll] = std::vector< std::vector < std::vector < TH1D* > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                mergedHists2016[mll][cat].push_back(std::vector<TH1D*>(proc.size() ) );

                //cut off loop before nonprompt contribution
                for(size_t m = 0, sam = 0; m < proc.size() - 1; ++m){
                    mergedHists2016[mll][cat][dist][m] = (TH1D*) hists[mll][cat][dist][sam]->Clone();

                    //look for each sample with the same name 
                    std::string currentName = samples[sam].getProcessName();

                    //check how far to jump to next process 
                    int nextProcessIndex = -1;
                    for(size_t match = sam + 1; match < samples2016.size(); ++match){
                        std::string nameToCompare = samples[match].getProcessName();
                        if(currentName == nameToCompare){
                            mergedHists2016[mll][cat][dist][m]->Add( hists[mll][cat][dist][match].get() );
                        } else if( nextProcessIndex == -1 ){
                            nextProcessIndex = match;
                        }
                    }
                    sam = nextProcessIndex;
                }

                //add 2016 nonprompt histogram
                mergedHists2016[mll][cat][dist][proc.size() - 1] = (TH1D*) hists[mll][cat][dist][samples.size()]->Clone();
            }
        }
    }

    for(unsigned mll = 0; mll < nMll; ++mll){
        mergedHists2017[mll] = std::vector< std::vector < std::vector < TH1D* > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                mergedHists2017[mll][cat].push_back(std::vector<TH1D*>(proc.size() ) );

                //cut off loop before nonprompt contribution
                for(size_t m = 0, sam = samples2016.size(); m < proc.size() - 1; ++m){
                    mergedHists2017[mll][cat][dist][m] = (TH1D*) hists[mll][cat][dist][sam]->Clone();

                    //look for each sample with the same name 
                    std::string currentName = samples[sam].getProcessName();

                    //check how far to jump to next process 
                    int nextProcessIndex = -1;
                    for(size_t match = sam + 1; match < samples2016.size() + samples2017.size(); ++match){
                        std::string nameToCompare = samples[match].getProcessName();
                        if(currentName == nameToCompare){
                            mergedHists2017[mll][cat][dist][m]->Add( hists[mll][cat][dist][match].get() );
                        } else if( nextProcessIndex == -1 ){
                            nextProcessIndex = match;
                        }
                    }
                    sam = nextProcessIndex;
                }

                //add 2017 nonprompt histogram
                mergedHists2017[mll][cat][dist][proc.size() - 1] = (TH1D*) hists[mll][cat][dist][samples.size() + 1]->Clone();
            }
        }
    }

    //initialize postFitScaler
    PostFitScaler postFitScaler("total_postFit_yields.txt");
    for(unsigned mll = 0; mll < nMll; ++mll){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                for(size_t m = 0; m < proc.size(); ++m){
                    if(proc[m] != "Data"){
                        mergedHists2016[mll][cat][dist][m]->Scale( postFitScaler.postFitScaling( mergedHists2016[mll][cat][dist][m]->GetSumOfWeights() ) );
                        mergedHists2017[mll][cat][dist][m]->Scale( postFitScaler.postFitScaling( mergedHists2017[mll][cat][dist][m]->GetSumOfWeights() ) );
                    }
                    delete mergedHists[mll][cat][dist][m];
                    mergedHists[mll][cat][dist][m] = (TH1D*) mergedHists2016[mll][cat][dist][m]->Clone();
                    mergedHists[mll][cat][dist][m]->Add( mergedHists2017[mll][cat][dist][m] );
                }
            }
        }
    }

    //plot all distributions
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/combined/final/" + catNames[cat] + "/" + histInfo[dist].name() + "_" + catNames[cat] + "_" + mllNames[m] + "_combined_postFit", "tzq", false, false, "77.4 fb^{-1} (13 TeV)", totalSystUnc[m][cat][dist], isSMSignal, nullptr, nullptr, 0, true, plotCatLabels[cat]);             //linear plots

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/combined/final/" + catNames[cat] + "/" + histInfo[dist].name() + "_"  + catNames[cat] + "_" + mllNames[m] + "_combined_postFit" + "_log", "tzq", true, false, "77.4 fb^{-1} (13 TeV)", totalSystUnc[m][cat][dist], isSMSignal, nullptr, nullptr, 0, true, plotCatLabels[cat]);    //log plots
            }
        }
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
