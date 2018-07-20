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
        HistInfo("bdt", "BDT output", 30, -1, 1),
        HistInfo("bdt_10bins", "BDT output", 10, -1, 1),

        HistInfo("taggedRecoilJetEta", "|#eta| (recoiling jet) (GeV)", 10, 0, 5),
        HistInfo("maxMJetJet", "M_{jet + jet}^{max} (GeV)", 10, 0, 1200),
        HistInfo("asymmetryWLep", "asymmetry (lepton from W)",10, -2.5, 2.5),
        HistInfo("highestDeepCSV", "highest deepCSV (b + bb)", 10, 0, 1),
        HistInfo("LTPlusMET", "L_{T} + E_{T}^{miss} (GeV)", 10, 0, 800),
        HistInfo("maxDeltaPhiJetJet", "max(#Delta#Phi(jet, jet))", 10, 0, 3.15),
        HistInfo("mt", "M_{T} (GeV)", 10, 0, 300),
        HistInfo("mtop", "M_{(W + b)} (GeV)", 10, 0, 400),
        HistInfo("maxpTJetJet", "P_{T}^{max}(jet + jet) (GeV)", 10, 0, 300), 
        HistInfo("minDeltaPhiLeptonBJet", "min(#Delta#Phi(l, bjet))", 10, 0, 3.15),
        HistInfo("maxDeltaPhiLeptonLepton", "max(#Delta#Phi(l, l))", 10, 0, 3.15),
        HistInfo("HT", "H_{T} (GeV)", 10, 0, 800), 
        HistInfo("deltaRTaggedBJetRecoilingJet", "#DeltaR(tagged b-jet, recoiling jet)", 10, 0, 10),
        HistInfo("deltaRWlepTaggedbJet", "#DeltaR(lepton from W, tagged b-jet)", 10, 0, 7),
        HistInfo("m3l", "M_{3l} (GeV)", 10, 0, 600),
        HistInfo("jetEta_highestEta", "|#eta| (most forward jet)", 10, 0, 5),
        HistInfo("nJets", "number of jets", 10, 0, 10),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 10, 25, 200),
        HistInfo("subPt", "P_{T}^{subleading} (GeV)", 10, 15, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 10, 10, 200)
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
                for(size_t sam = 0; sam < samples.size() + 1; ++sam){
                    if(sam < samples.size()){
                        hists[m][cat][dist].push_back(histInfo[dist].makeHist(catNames[cat] + mllNames[m] + samples[sam].getUniqueName()) );
                    } else {
                        //extra histogram for nonprompt prediction
                        hists[m][cat][dist].push_back( histInfo[dist].makeHist(catNames[cat] + mllNames[m] + "nonprompt") ); 
                    }
                }
            }
        }
    }

    const std::vector< std::string > uncNames = {"JEC", "uncl", "scale", "pileup", "bTag_udsg", "bTag_c", "bTag_b", "pdf", "scaleXsec", "pdfXsec"};
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
                    uncHistMapDown[key][m][cat][dist] = std::vector< std::shared_ptr< TH1D > >(samples.size() + 1);
                    uncHistMapUp[key][m][cat][dist] = std::vector< std::shared_ptr< TH1D > >(samples.size() + 1);
                    for(size_t sam = 0; sam < samples.size() + 1; ++sam){
                        std::string sampleName;
                        if(sam < samples.size() ){
                            sampleName = samples[sam].getUniqueName();
                        } else {
                            //the nonprompt prediction is also affected by all nuisances through the prompt MC subtraction
                            sampleName = "nonprompt";
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
                    pdfUncHists[pdf][m][cat][dist] = std::vector< std::shared_ptr< TH1D > >( samples.size() + 1);
                    for(size_t sam = 0; sam < samples.size() + 1; ++sam){
                        std::string sampleName;
                        if(sam < samples.size() ){
                            sampleName = samples[sam].getUniqueName();
                        } else {
                            sampleName = "nonprompt";
                        } 
                        pdfUncHists[pdf][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + sampleName + "pdf" + std::to_string(pdf) );
                    }
                }
            }
        }
    }

    //BDT reader for 1bJet23Jets category
    std::vector < std::string > bdtVars1bJet23Jets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "maxDeltaPhijj", "mTW", "topMass", "pTMaxjj", "minDeltaPhilb", "maxDeltaPhill"};
    std::string weights;
    if( is2016() ){
        weights = "1bJet23Jets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    } else if( is2017() ){
        weights = "1bJet23Jets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    }
    bdtReader1bJet23Jets = std::shared_ptr<BDTReader>( new BDTReader("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars1bJet23Jets) );

    //BDT reader for 1bJet4Jets category
    if( is2016() ){
        weights = "1bJet4Jets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    } else if( is2017() ){
        weights = "1bJet4Jets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    }
    std::vector < std::string > bdtVars1bJet4Jets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "ht", "mTW", "topMass", "numberOfJets", "maxDeltaPhill", "maxDeltaPhijj", "minDeltaPhilb",
        "deltaRTaggedbJetRecoilingJet", "deltaRWLeptonTaggedbJet", "m3l", "etaMostForward"};
    bdtReader1bJet4Jets = std::shared_ptr<BDTReader>( new BDTReader("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars1bJet4Jets) );

    //BDT reader for 2bJets category
    if( is2016() ){
        weights = "2bJets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    } else if( is2017() ){
        weights = "2bJets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    }
    std::vector < std::string > bdtVars2bJets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "ht", "mTW", "topMass", "numberOfJets", "maxDeltaPhill", "maxDeltaPhijj", "etaMostForward", "m3l"}; 
    bdtReader2bJets = std::shared_ptr<BDTReader>( new BDTReader("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars2bJets) );


    //tweakable options
    const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        /*
        if(sam == 0){                   //skip data for now
            ++currentSampleIndex;
            continue;
        }
        */
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
                fillIndex = samples.size();

                //apply fake-rate weights 
                weight *= fakeRateWeight(); 
            }

            //remove overlap between samples
            if(photonOverlap(currentSample)) continue;


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
            }

            //compute nominal bdt value
            double bdtNominal = 999.;
            if(tzqCat > 2 && tzqCat < 6){
                bdtNominal = bdtOutput( tzqCat );
                if(bdtNominal < 0.){
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
                        _lPt[ind[2]]
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
                    }
                }
            }


            //no nuisances for data
            if( isData() ) continue;

            //vary JEC Down
            unsigned tzqCatJECDown = setSearchVariablestZq("JECDown", ind, bestZ);
            if(tzqCatJECDown > 2 && tzqCatJECDown < 6){
                double bdtJECDown = bdtOutput( tzqCatJECDown );
                if(bdtJECDown < 0.){
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
                        _lPt[ind[2]]
                        };
 
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        uncHistMapDown["JEC"][mllCat][tzqCatJECDown - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                    }
                }
            }

            //vary JEC up
            unsigned tzqCatJECUp = setSearchVariablestZq("JECUp", ind, bestZ);
            if(tzqCatJECUp > 2 && tzqCatJECUp < 6){
                double bdtJECUp = bdtOutput( tzqCatJECUp );
                if( bdtJECUp < 0.){
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
                        _lPt[ind[2]]
                        };

                    for(unsigned dist = 0; dist < nDist; ++dist){
                        uncHistMapUp["JEC"][mllCat][tzqCatJECUp - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                    }
                }
            }

            //vary unclustered down
            unsigned tzqCatUnclDown = setSearchVariablestZq("unclDown", ind, bestZ);
            if(tzqCatUnclDown > 2 && tzqCatUnclDown < 6){
                double bdtUnclDown = bdtOutput( tzqCatUnclDown );
                if(bdtUnclDown < 0.){
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
                        _lPt[ind[2]]
                        };

                    for(unsigned dist = 0; dist < nDist; ++dist){
                        uncHistMapDown["uncl"][mllCat][tzqCatUnclDown - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                    }
                }
            }

            //vary unclustered up
            unsigned tzqCatUnclUp = setSearchVariablestZq("unclUp", ind, bestZ);
            if(tzqCatUnclUp  > 2 && tzqCatUnclUp < 6){
                double bdtUnclUp = bdtOutput( tzqCatUnclUp );
                if(bdtUnclUp < 0.){
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
                        _lPt[ind[2]]
                        };

                    for(unsigned dist = 0; dist < nDist; ++dist){
                        uncHistMapUp["uncl"][mllCat][tzqCatUnclUp - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                    }
                }
            }

            //now nominal cuts can be safely used
            if(tzqCat < 3 || tzqCat > 5) continue;
            if(bdtNominal >= 0.) continue;

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
                    _lPt[ind[2]]
                    };


            //vary scale down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["scale"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[8]);
            }

            //vary scale up
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["scale"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[4]);
            }

            //vary pu down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["pileup"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puWeight(1)/puWeight(0));
            }

            //vary pu up            
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["pileup"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puWeight(2)/puWeight(0));
            }

            //vary b-tag down for udsg
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["bTag_udsg"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_udsg(1)/bTagWeight_udsg(0));
            }

            //vary b-tag up for udsg
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["bTag_udsg"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_udsg(2)/bTagWeight_udsg(0));
            }

            //vary b-tag down for c 
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["bTag_c"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_c(1)/bTagWeight_c(0));
            }

            //vary b-tag up for c
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["bTag_c"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_c(2)/bTagWeight_c(0));
            }

            //vary b-tag down for b
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["bTag_b"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_b(1)/bTagWeight_b(0));
            }

            //vary b-tag up for b
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["bTag_b"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_b(2)/bTagWeight_b(0));
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
                    tools::setNegativeZero( hists[m][cat][dist][sam].get() );
                    for(auto & key : uncNames){
                        tools::setNegativeZero(uncHistMapDown[key][m][cat][dist][sam].get() );
                        tools::setNegativeZero(uncHistMapUp[key][m][cat][dist][sam].get() );
                    } 
                    for(unsigned pdf = 0; pdf < 100; ++pdf){    
                        tools::setNegativeZero(pdfUncHists[pdf][m][cat][dist][sam].get() );
                    }
                }	
            }
        }
    }

    //set nonprompt bins to 0 if negative
    for(unsigned m = 0; m < nMll; ++m){
        for( unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                unsigned nonpromptIndex = samples.size();
                tools::setNegativeZero( hists[m][cat][dist][nonpromptIndex].get() );
                for(auto & key : uncNames){
                    tools::setNegativeZero(uncHistMapDown[key][m][cat][dist][nonpromptIndex].get() );
                    tools::setNegativeZero(uncHistMapUp[key][m][cat][dist][nonpromptIndex].get() );
                } 
                for(unsigned pdf = 0; pdf < 100; ++pdf){    
                    tools::setNegativeZero(pdfUncHists[pdf][m][cat][dist][nonpromptIndex].get() );
                }
            }
        }
    }

    //compute final pdf uncertainties
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                for(unsigned sam = 0; sam < samples.size() + 1; ++sam){
                    uncHistMapDown["pdf"][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + samples[sam].getUniqueName() + "pdfDown");
                    uncHistMapUp["pdf"][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + samples[sam].getUniqueName() + "pdfUp");
                    for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["pdf"][m][cat][dist][sam]->GetNbinsX() + 1; ++bin){
                        double pdfVarRms = 0.;
                        for(unsigned pdf = 0; pdf < 100; ++pdf){
                            double diff = (  pdfUncHists[pdf][m][cat][dist][sam]->GetBinContent(bin) - hists[m][cat][dist][sam]->GetBinContent(bin) );
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

    //add pdf and scale variation uncertainties of the cross-section
    std::vector< double > scaleXsecUncDown = {0.};  //first dummy entry for data
    std::vector< double > scaleXsecUncUp = {0.};
    std::vector< double > pdfXsecUncDown = {0.};
    std::vector< double > pdfXsecUncUp = {0.};

    std::cout << "setting up pdf uncertainties" << std::endl;

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
        pdfXsecUncDown.push_back( -rms );
        pdfXsecUncUp.push_back( rms );
    }

    std::cout << "adding pdf and scale effects on the cross section as shape uncertainties" << std::endl;
    //add pdf and scale effects to shape uncertainties to automatically take into account the fractional effects from every sample that is merged
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                for(unsigned sam = 0; sam < samples.size() + 1; ++sam){
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

    std::cout << "merging histograms" << std::endl;
    //merge histograms with the same physical background
    std::vector<std::string> proc = {"total bkg.", "tZq", "WZ", "multiboson", "TT + Z", "TT/T + X", "X + #gamma", "ZZ/H", "Nonprompt e/#mu"};
    std::vector< std::vector< std::vector< std::vector< TH1D* > > > > mergedHists(nMll);
    for(unsigned mll = 0; mll < nMll; ++mll){
        mergedHists[mll] = std::vector< std::vector < std::vector < TH1D* > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                mergedHists[mll][cat].push_back(std::vector<TH1D*>(proc.size() ) );

                //cut off loop before nonprompt contribution
                for(size_t m = 0, sam = 0; m < proc.size() - 1; ++m){
                    mergedHists[mll][cat][dist][m] = (TH1D*) hists[mll][cat][dist][sam]->Clone();
                    while(sam < samples.size() - 1 && samples[sam].getProcessName() == samples[sam + 1].getProcessName() ){
                        mergedHists[mll][cat][dist][m]->Add(hists[mll][cat][dist][sam + 1].get());
                        ++sam;
                    }
                    ++sam;
                }

                //add nonprompt histogram
                mergedHists[mll][cat][dist][proc.size() - 1] = (TH1D*) hists[mll][cat][dist][samples.size()].get()->Clone();
            }
        }
    }

    std::cout << "merging uncertainties" << std::endl;
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
                        while(sam < samples.size() - 1 && samples[sam].getProcessName() == samples[sam + 1].getProcessName() ){
                            mergedUncMapDown[key][mll][cat][dist][m]->Add( uncHistMapDown[key][mll][cat][dist][sam + 1].get());
                            mergedUncMapUp[key][mll][cat][dist][m]->Add( uncHistMapUp[key][mll][cat][dist][sam + 1].get());
                            ++sam;
                        }
                        ++sam;
                    }

                    //add nonprompt histograms 
                    mergedUncMapDown[key][mll][cat][dist][proc.size() - 1] = (TH1D*) uncHistMapDown[key][mll][cat][dist][samples.size()]->Clone();
                    mergedUncMapUp[key][mll][cat][dist][proc.size() - 1] = (TH1D*) uncHistMapUp[key][mll][cat][dist][samples.size()]->Clone();
                }
            }
        }
    } 

    std::cout << "adding uncertainties in quadrature" << std::endl;
    //make final uncertainty histogram for plots 
    std::vector<double> flatUnc = {1.025, 1.06, 1.05, 1.00, 1.00}; //lumi, leptonID, trigger , pdf and scale effects on cross section
    std::map< std::string, double > backgroundSpecificUnc =        //map of background specific nuisances that can be indexed with the name of the process 
        {
            {"Nonprompt e/#mu", 1.3}
        };
    std::vector< std::vector< std::vector< std::vector< TH1D* > > > > totalSystUnc = mergedHists; //copy pointers to fix dimensionality of vector
    for(unsigned mll = 0; mll < nMll; ++mll){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                for(unsigned p = 0; p < proc.size(); ++p){
                    if(p == 0) continue;
                    totalSystUnc[mll][cat][dist][p] = (TH1D*) mergedHists[mll][cat][dist][p]->Clone();
                    for(unsigned bin = 1; bin < (unsigned)  totalSystUnc[mll][cat][dist][p]->GetNbinsX() + 1; ++bin){
                        double binUnc = 0;

                        //add all shape uncertainties 
                        for( auto& key : uncNames ){
                            double down = fabs(mergedUncMapDown[key][mll][cat][dist][p]->GetBinContent(bin) - mergedHists[mll][cat][dist][p]->GetBinContent(bin) );
                            double up = fabs(mergedUncMapUp[key][mll][cat][dist][p]->GetBinContent(bin) - mergedHists[mll][cat][dist][p]->GetBinContent(bin) );
                            double var = std::max(down, up);

                            //consider maximum variation between up and down for plotting
                            binUnc += var*var;
                        }
                        
                        //add flat uncertainties
                        for( double unc : flatUnc ){
                            double binContent = mergedHists[mll][cat][dist][p]->GetBinContent(bin);
                            double var = binContent*(unc - 1.); 
                            binUnc += var*var;
                        }

                        //add background specific uncertainties
                        for(auto& uncPair : backgroundSpecificUnc){
                            if(proc[p] == uncPair.first){
                                double var = mergedHists[mll][cat][dist][p]->GetBinContent(bin)*(uncPair.second - 1.);
                                binUnc += var*var;
                            }
                        }
                        totalSystUnc[mll][cat][dist][p]->SetBinContent(bin, sqrt(binUnc) );
                    }
                }
            }
        }
    }

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

    std::cout << "making plots" << std::endl;
    //plot all distributions
    const bool isSMSignal[(const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/2016/lowBDT/" + histInfo[dist].name() + "_" + catNames[cat] + "_" + mllNames[m] + "_2016", "tzq", false, false, "35.9 fb^{-1} (13 TeV)", &totalSystUnc[m][cat][dist][1], isSMSignal);             //linear plots

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/2016/lowBDT/" + histInfo[dist].name() + "_"  + catNames[cat] + "_" + mllNames[m] + "_2016" + "_log", "tzq", true, false, "35.9 fb^{-1} (13 TeV)", &totalSystUnc[m][cat][dist][1], isSMSignal);    //log plots
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
