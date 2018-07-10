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

    const std::vector< std::string > uncNames = {"JECDown", "JECUp", "unclDown", "unclUp", "scaleDown", "scaleUp", "pileupDown", "pileupUp", "bTag_udsg_Down", "bTag_udsg_Up", "bTag_c_Down", "bTag_c_Up", "bTag_b_Down", "bTag_b_Up"};
    std::map < std::string, std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > >  > uncHistMap;
    for( auto& key : uncNames ){
        uncHistMap[key] = std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > >(nMll);
        for(unsigned m = 0; m < nMll; ++m){
            uncHistMap[key][m] = std::vector< std::vector < std::vector< std::shared_ptr< TH1D > > > >(nCat);
            for(unsigned cat = 0; cat < nCat; ++cat){
                uncHistMap[key][m][cat] = std::vector < std::vector< std::shared_ptr< TH1D > > >( nDist );
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMap[key][m][cat][dist] = std::vector< std::shared_ptr< TH1D > >(samples.size());
                    for(size_t sam = 0; sam < samples.size(); ++sam){
                        uncHistMap[key][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + samples[sam].getUniqueName() + key);
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
                    pdfUncHists[pdf][m][cat][dist] = std::vector< std::shared_ptr< TH1D > >( samples.size() );
                    for(size_t sam = 0; sam < samples.size(); ++sam){
                       pdfUncHists[pdf][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + samples[sam].getUniqueName() + "pdf" + std::to_string(pdf) );
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
            const unsigned lCount = selectLep(ind);
            if(lCount != 3) continue;
            if(tightLepCount(ind, lCount) != 3) continue; //require 3 tight leptons

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
                double fill[nDist] = {bdtNominal, bdtNominal};
                for(unsigned dist = 0; dist < nDist; ++dist){
                    hists[mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //no nuisances for data
            if( isData() ) continue;

            //vary JEC Down
            unsigned tzqCatJECDown = setSearchVariablestZq("JECDown", ind, bestZ);
            if(tzqCatJECDown > 2 && tzqCatJECDown < 6){
                double bdtJECDown = bdtOutput( tzqCatJECDown );
                double fill[nDist] = {bdtJECDown, bdtJECDown};
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMap["JECDown"][mllCat][tzqCatJECDown - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
                
            }

            //vary JEC up
            unsigned tzqCatJECUp = setSearchVariablestZq("JECUp", ind, bestZ);
            if(tzqCatJECUp > 2 && tzqCatJECUp < 6){
                double bdtJECUp = bdtOutput( tzqCatJECUp );
                double fill[nDist] = {bdtJECUp, bdtJECUp};
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMap["JECUp"][mllCat][tzqCatJECUp - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary unclustered down
            unsigned tzqCatUnclDown = setSearchVariablestZq("unclDown", ind, bestZ);
            if(tzqCatUnclDown > 2 && tzqCatUnclDown < 6){
                double bdtUnclDown = bdtOutput( tzqCatUnclDown );
                double fill[nDist] = {bdtUnclDown, bdtUnclDown};
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMap["unclDown"][mllCat][tzqCatUnclDown - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary unclustered up
            unsigned tzqCatUnclUp = setSearchVariablestZq("unclUp", ind, bestZ);
            if(tzqCatUnclUp  > 2 && tzqCatUnclUp < 6){
                double bdtUnclUp = bdtOutput( tzqCatUnclUp );
                double fill[nDist] = {bdtUnclUp, bdtUnclUp};
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMap["unclUp"][mllCat][tzqCatUnclUp - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //now nominal cuts can be safely used
            if(tzqCat < 3 || tzqCat > 5) continue;
            double fill[nDist] = {bdtNominal, bdtNominal};

            //vary scale down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["scaleDown"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[8]);
            }

            //vary scale up
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["scaleUp"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[4]);
            }

            //vary pu down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["pileupDown"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puWeight(1)/puWeight(0));
            }

            //vary pu up            
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["pileupUp"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puWeight(2)/puWeight(0));
            }

            //vary b-tag down for udsg
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["bTag_udsg_Down"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puWeight(1)/puWeight(0));
            }

            //vary b-tag up for udsg
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["bTag_udsg_Up"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_udsg(2)/bTagWeight_udsg(0));
            }

            //vary b-tag down for c 
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["bTag_c_Down"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_c(1)/bTagWeight_c(0));
            }

            //vary b-tag up for c
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["bTag_c_Up"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_c(2)/bTagWeight_c(0));
            }

            //vary b-tag down for b
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["bTag_b_Up"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_b(1)/bTagWeight_b(0));
            }

            //vary b-tag up for b
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMap["bTag_b_Up"][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_b(2)/bTagWeight_b(0));
            }

            //100 pdf variations
            for(unsigned pdf = 0; pdf < 100; ++pdf){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    pdfUncHists[pdf][mllCat][tzqCat - 3][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
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
    
    //compute final pdf uncertainties
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                for(unsigned sam = 0; sam < samples.size(); ++sam){
                    uncHistMap["pdfDown"][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + samples[sam].getUniqueName() + "pdfDown");
                    uncHistMap["pdfUp"][m][cat][dist][sam] = histInfo[dist].makeHist(catNames[cat] + mllNames[m] + samples[sam].getUniqueName() + "pdfUp");
                    for(unsigned bin = 1; bin < (unsigned) uncHistMap["pdf"][m][cat][dist][sam]->GetNbinsX() + 1; ++bin){
                        double pdfVarRms = 0.;
                        for(unsigned pdf = 0; pdf < 100; ++pdf){
                            double diff = (  pdfUncHists[pdf][m][cat][dist][sam]->GetBinContent(bin) - hists[m][cat][dist][sam]->GetBinContent(bin) );
                            pdfVarRms += diff * diff;
                        }
                        pdfVarRms = sqrt( 0.01 * pdfVarRms );
                        uncHistMap["pdfDown"][m][cat][dist][sam]->SetBinContent( bin, hists[m][cat][dist][sam]->GetBinContent(bin) - pdfVarRms);
                        uncHistMap["pdfUp"][m][cat][dist][sam]->SetBinContent( bin, hists[m][cat][dist][sam]->GetBinContent(bin) + pdfVarRms);
                    }
                }
            }
        }
    }

    //add pdf and scale variation uncertainties of the cross-section

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

    //merging for uncertainties
    std::map< std::string, std::vector< std::vector< std::vector< std::vector< TH1D* > > > > > mergedUncMap;
    for( auto& key : uncNames ){
        mergedUncMap[key] = std::vector< std::vector< std::vector< std::vector< TH1D* > > > >(nMll);
        for(unsigned mll = 0; mll < nMll; ++mll){
            mergedUncMap[key][mll] = std::vector< std::vector < std::vector < TH1D* > > >(nCat);
            for(unsigned cat = 0; cat < nCat; ++cat){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    mergedUncMap[key][mll][cat].push_back(std::vector<TH1D*>(proc.size() ) );
                    for(size_t m = 0, sam = 0; m < proc.size(); ++m){
                        mergedUncMap[key][mll][cat][dist][m] = (TH1D*) uncHistMap[key][mll][cat][dist][sam]->Clone();
                        while(sam < samples.size() - 1 && samples[sam].getProcessName() == samples[sam + 1].getProcessName() ){
                            mergedUncMap[key][mll][cat][dist][m]->Add( uncHistMap[key][mll][cat][dist][sam + 1].get());
                            ++sam;
                        }
                        ++sam;
                    }
                }
            }
        }
    } 

    //make final uncertainty histogram for plots 
    const std::vector<double> flatHist = {1.025, 1.06, 1.05}; //lumi, leptonID, trigger

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
