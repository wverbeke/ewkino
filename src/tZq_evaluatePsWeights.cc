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
    readSamples("sampleLists/evaluatePsUnc.txt", "../../ntuples_tzq/");
    //name      xlabel    nBins,  min, max
    histInfo = {
        //new BDT distribution
        HistInfo("bdt_10bins", "BDT output", 10, -1, 1),
        HistInfo("bdt_10bins_preFiringWeights", "BDT output", 10, -1, 1),
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

    const std::vector< std::string > uncNames = {"JEC_2017", "uncl", "scale", "pileup", "bTag_udsg_2017", "bTag_bc_2017", "isr", "fsr", "pdf", "scaleXsec", "pdfXsec"};
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
    std::vector < std::string > bdtVars1bJet23Jets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "mTW", "pTMaxjj", "minDeltaPhilb", "maxDeltaPhill"};
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
    std::vector < std::string > bdtVars1bJet4Jets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "ht", "mTW", "numberOfJets", "maxDeltaPhill", "minDeltaPhilb",
        "deltaRTaggedbJetRecoilingJet", "deltaRWLeptonTaggedbJet", "m3l", "etaMostForward"};
    bdtReader1bJet4Jets = std::shared_ptr<BDTReader>( new BDTReader("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars1bJet4Jets) );

    //BDT reader for 2bJets category
    if( is2016() ){
        weights = "2bJets_onZ_2016_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    } else if( is2017() ){
        weights = "2bJets_onZ_2017_BDTG_200Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1.weights.xml";
    }
    std::vector < std::string > bdtVars2bJets = {"etaRecoilingJet", "maxMjj", "asymmetryWlep", "highestDeepCSV", "ltmet", "ht", "mTW", "numberOfJets", "maxDeltaPhill", "etaMostForward", "m3l"}; 
    bdtReader2bJets = std::shared_ptr<BDTReader>( new BDTReader("BDTG", "bdtTraining/bdtWeights/" + weights, bdtVars2bJets) );


    //tweakable options
    const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){

        initSample();          //2 = combined luminosity

        if( currentSample.getProcessName() != "tZq") continue;
        
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

            //jet prefiring weight 
            double preFiringWeight = jetPrefiringWeight();

            //compute nominal bdt value
            double bdtNominal = 999.;
            if(tzqCat > 2 && tzqCat < 6){
                bdtNominal = bdtOutput( tzqCat );
                double fill[nDist] = {bdtNominal, bdtNominal,
                    };
                for(unsigned dist = 0; dist < nDist; ++dist){
                    hists[mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( (dist == 1) ? preFiringWeight : 1. ) );
                }
                //in case of data fakes fill all uncertainties for nonprompt with nominal values
                if( isData() && !passTightCut){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        for(auto& key: uncNames){
                            uncHistMapDown[key][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( (dist == 1) ? preFiringWeight : 1. ) );
                            uncHistMapUp[key][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( (dist == 1) ? preFiringWeight : 1. ));
                        }
                        for(unsigned pdf = 0; pdf < 100; ++pdf){
                             pdfUncHists[pdf][mllCat][tzqCat - 3][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( (dist == 1) ? preFiringWeight : 1. ));
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
                double fill[nDist] = {bdtJECDown, bdtJECDown
                    };
 
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown["JEC_2017"][mllCat][tzqCatJECDown - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( (dist == 1) ? preFiringWeight : 1. ));
                }
                
            }

            //vary JEC up
            unsigned tzqCatJECUp = setSearchVariablestZq("JECUp", ind, bestZ);
            if(tzqCatJECUp > 2 && tzqCatJECUp < 6){
                double bdtJECUp = bdtOutput( tzqCatJECUp );
                double fill[nDist] = {bdtJECUp, bdtJECUp
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp["JEC_2017"][mllCat][tzqCatJECUp - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( (dist == 1) ? preFiringWeight : 1. ));
                }
            }

            //vary unclustered down
            unsigned tzqCatUnclDown = setSearchVariablestZq("unclDown", ind, bestZ);
            if(tzqCatUnclDown > 2 && tzqCatUnclDown < 6){
                double bdtUnclDown = bdtOutput( tzqCatUnclDown );
                double fill[nDist] = {bdtUnclDown, bdtUnclDown
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown["uncl"][mllCat][tzqCatUnclDown - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( (dist == 1) ? preFiringWeight : 1. ));
                }
            }

            //vary unclustered up
            unsigned tzqCatUnclUp = setSearchVariablestZq("unclUp", ind, bestZ);
            if(tzqCatUnclUp  > 2 && tzqCatUnclUp < 6){
                double bdtUnclUp = bdtOutput( tzqCatUnclUp );
                double fill[nDist] = {bdtUnclUp, bdtUnclUp
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp["uncl"][mllCat][tzqCatUnclUp - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( (dist == 1) ? preFiringWeight : 1. ));
                }
            }

            //now nominal cuts can be safely used
            if(tzqCat < 3 || tzqCat > 5) continue;
            
            //reset nominal values 
            setSearchVariablestZq("nominal", ind, bestZ);
            double fill[nDist] = {bdtNominal, bdtNominal
                    };

            //set pdf and scale weight for GluGluToContinToZZ samples which do not include lhe weights
            if( currentSample.getFileName().find("GluGluToContinToZZ") != std::string::npos ){
                for(unsigned lhe = 0; lhe < 110; ++lhe){
                    _lheWeight[lhe] = 1.;
                }
            }

            //vary scale down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["scale"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[8]*( (dist == 1) ? preFiringWeight : 1. ));
            }

            //vary scale up
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["scale"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[4]*( (dist == 1) ? preFiringWeight : 1. ));
            }

            //vary pu down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["pileup"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puWeight(1)/puWeight(0)*( (dist == 1) ? preFiringWeight : 1. ));
            }

            //vary pu up            
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["pileup"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puWeight(2)/puWeight(0)*( (dist == 1) ? preFiringWeight : 1. ));
            }

            //vary b-tag down for udsg
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["bTag_udsg_2017"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_udsg(1)/bTagWeight_udsg(0)*( (dist == 1) ? preFiringWeight : 1. ));
            }

            //vary b-tag up for udsg
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["bTag_udsg_2017"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_udsg(2)/bTagWeight_udsg(0)*( (dist == 1) ? preFiringWeight : 1. ));
            }

            //vary b-tag down for b and c (correlated)
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["bTag_bc_2017"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_c(1)*bTagWeight_b(1)/ (bTagWeight_c(0)*bTagWeight_b(0)*( (dist == 1) ? preFiringWeight : 1. )) );
            }

            //vary b-tag up for b and c (correlated)
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["bTag_bc_2017"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTagWeight_c(2)*bTagWeight_b(2)/ (bTagWeight_c(0)*bTagWeight_b(0)*( (dist == 1) ? preFiringWeight : 1. )) );
            }

            //vary b-tag down for b and c (correlated)
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["isr"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_psWeight[6]*( (dist == 1) ? preFiringWeight : 1. ) );
            }

            //vary b-tag up for b and c (correlated)
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["isr"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_psWeight[8]*( (dist == 1) ? preFiringWeight : 1. ));
            }

            //vary b-tag down for b and c (correlated)
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["fsr"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_psWeight[7]*( (dist == 1) ? preFiringWeight : 1. ) );
            }

            //vary b-tag up for b and c (correlated)
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["fsr"][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_psWeight[9]*( (dist == 1) ? preFiringWeight : 1. ));
            }

            //100 pdf variations
            for(unsigned pdf = 0; pdf < 100; ++pdf){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    pdfUncHists[pdf][mllCat][tzqCat - 3][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[pdf + 9]*( (dist == 1) ? preFiringWeight : 1. ));
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


    //plot bdt before and after prefiring weights 
    TCanvas* c = new TCanvas("", "", 500, 500 );
    TH1D* nominal = (TH1D*) hists[0][0][0][0]->Clone();
    TH1D* prefire = (TH1D*) hists[0][0][1][0]->Clone();
    nominal->SetFillColor(kBlue);
    nominal->SetLineColor(kBlue);
    prefire->SetFillColor(kBlue);
    prefire->SetLineColor(kBlue);
    TLegend legend(0.2,0.8,0.95,0.9,NULL,"brNDC");
    legend.SetNColumns(2);
    legend.SetFillStyle(0); //avoid legend box
    legend.AddEntry( nominal, "nominal", "f");
    legend.AddEntry( prefire, "with prefire weights", "f");
    nominal->Draw("histE");
    prefire->Draw("histEsame");
    legend.Draw("same");
    c->SaveAs("prefiringPlot.pdf");

    //set nonprompt bins to 0 if negative
    for(unsigned m = 0; m < nMll; ++m){
        for( unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                unsigned nonpromptIndex = samples.size();
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

    //compute lhe cross section ratio for every sample and variation
    std::vector< std::vector< double> > crossSectionRatio(samples.size(), std::vector<double>(110, 1.) );
    std::vector< std::vector< double> > psCrossSectionRatio(samples.size(), std::vector<double>(14, 1.) );
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

        std::shared_ptr<TH1D> psCounter = std::shared_ptr<TH1D>( (TH1D*) sample->Get("blackJackAndHookers/psCounter"));
        for(unsigned ps = 0; ps < 14; ++ps){
            double variedSumOfWeights = psCounter->GetBinContent(ps + 1);
            psCrossSectionRatio[sam][ps] = sumOfWeights/( variedSumOfWeights );
        }
    }

    //compute final pdf uncertainties
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                for(unsigned sam = 0; sam < samples.size() + 1; ++sam){
                    if(sam == 0 ) continue;

                    for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["pdf"][m][cat][dist][sam]->GetNbinsX() + 1; ++bin){
                        double pdfVarRms = 0.;
                        for(unsigned pdf = 0; pdf < 100; ++pdf){

                            double variedBin = pdfUncHists[pdf][m][cat][dist][sam]->GetBinContent(bin);
                            if( sam != samples.size() ){
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

        //filter out cross section effects from scale, is and fsr shape uncertainty
        if( samples[sam].getFileName().find("GluGluToContinToZZ") == std::string::npos){
            for(unsigned m = 0; m < nMll; ++m){
                for(unsigned cat = 0; cat < nCat; ++cat){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        uncHistMapDown["scale"][m][cat][dist][sam]->Scale(crossSectionRatio[sam][8]);
                        uncHistMapUp["scale"][m][cat][dist][sam]->Scale(crossSectionRatio[sam][4]);

                        uncHistMapDown["isr"][m][cat][dist][sam]->Scale(psCrossSectionRatio[sam][6]);
                        uncHistMapUp["isr"][m][cat][dist][sam]->Scale(psCrossSectionRatio[sam][8]);

                        uncHistMapDown["fsr"][m][cat][dist][sam]->Scale(psCrossSectionRatio[sam][7]);
                        uncHistMapUp["fsr"][m][cat][dist][sam]->Scale(psCrossSectionRatio[sam][9]);
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

    //merge histograms with the same physical background
    std::vector<std::string> proc = {"Obs.", "tZq", "WZ", "multiboson", "TT + Z", "TT/T + X", "X + #gamma", "ZZ/H", "Nonprompt e/#mu"};
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

    //print the effect of systematic uncertainty
    //compute minimum and maximum impact on each bin
    for(unsigned bdt = 0; bdt < 2; ++bdt){
        if(bdt == 1) continue;
        std::cout << "##################################################" << std::endl;
        if(bdt == 0){
            std::cout << "uncetainty ranges for 30 BDT bins: " << std::endl;
        } else{
            std::cout << "uncetainty ranges for 10 BDT bins: " << std::endl;
        }
        for(auto& key : uncNames ){
            double maxUnc = 0.;
            double minUnc = 999.;
            for(unsigned mll = 0; mll < nMll; ++mll){
                for(unsigned cat = 0; cat < nCat; ++cat){
                    for(unsigned bin = 1; bin < ( unsigned) mergedHists[mll][cat][0][0]->GetNbinsX() + 1; ++bin){
                        double binUnc = 0.;
                        double binContent = 0.;
                        for(unsigned p = 1; p < proc.size(); ++p){
                            if( (key.find("XSec") != std::string::npos) && !(proc[p] == "multiboson" || proc[p] == "TT/T + X") ){
                                continue;
                            }
                            double binUncDown = fabs( mergedHists[mll][cat][bdt][p]->GetBinContent(bin) - mergedUncMapDown[key][mll][cat][bdt][p]->GetBinContent(bin) );
                            double binUncUp = fabs( mergedHists[mll][cat][bdt][p]->GetBinContent(bin) - mergedUncMapUp[key][mll][cat][bdt][p]->GetBinContent(bin) );
                            binUnc += std::max(binUncDown, binUncUp);
                            binContent += mergedHists[mll][cat][bdt][p]->GetBinContent(bin);
                        }
                        double fractionalUnc = binUnc/binContent;
                        if(bdt == 0){
                            std::cout << "fractional uncertainty in " << catNames[cat] << " bin " << bin << "  =  " << fractionalUnc*100 << "%" << std::endl;
                        }
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

    //make final uncertainty histogram for plots 
    std::vector<double> flatUnc = {1.025, 1.06, 1.02}; //lumi, leptonID, trigger , pdf and scale effects on cross section
    std::map< std::string, double > backgroundSpecificUnc =        //map of background specific nuisances that can be indexed with the name of the process 
        {
            {"Nonprompt e/#mu", 1.3},
            {"WZ", 1.1},
            {"X + #gamma", 1.1},
            {"ZZ/H", 1.1},
            {"TTZ", 1.15}
        };

    std::vector< std::string > ignoreTheoryUncInPlot = {"WZ", "X + #gamma", "ZZ/H", "TTZ"};

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

                            //ignore theory uncertainty for certain processes
                            if( key.find("Xsec") != std::string::npos ){
                                bool processWithoutTheoryUnc =  ( std::find( ignoreTheoryUncInPlot.cbegin(), ignoreTheoryUncInPlot.cend(), proc[p] ) != ignoreTheoryUncInPlot.cend() );
                                if( processWithoutTheoryUnc){
                                        continue;
                                }
                            }

                            double down = fabs(mergedUncMapDown[key][mll][cat][dist][p]->GetBinContent(bin) - mergedHists[mll][cat][dist][p]->GetBinContent(bin) );
                            double up = fabs(mergedUncMapUp[key][mll][cat][dist][p]->GetBinContent(bin) - mergedHists[mll][cat][dist][p]->GetBinContent(bin) );
                            double var = std::max(down, up);

                            //consider maximum variation between up and down for plotting
                            binUnc += var*var;
                        }
                        
                        //add flat uncertainties
                        if( proc[p] != "Nonprompt e/#mu"){
                            for( double unc : flatUnc ){
                                double binContent = mergedHists[mll][cat][dist][p]->GetBinContent(bin);
                                double var = binContent*(unc - 1.); 
                                binUnc += var*var;
                            }
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
    /*
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
    */
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
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/2017/final/" + catNames[cat] + "/" + histInfo[dist].name() + "_" + catNames[cat] + "_" + mllNames[m] + "_2017", "tzq", false, false, "41.5 fb^{-1} (13 TeV)", &totalSystUnc[m][cat][dist][1], isSMSignal);             //linear plots

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/2017/final/" + catNames[cat] + "/" + histInfo[dist].name() + "_"  + catNames[cat] + "_" + mllNames[m] + "_2017" + "_log", "tzq", true, false, "41.5 fb^{-1} (13 TeV)", &totalSystUnc[m][cat][dist][1], isSMSignal);    //log plots
            }
        }
    }
 
    //make shape datacards for each category
    const unsigned nBkg = proc.size() - 2;  //number of background processes
    const std::string bkgNames[nBkg] = {"WZ", "multiboson", "TTZ", "TTX", "Xgamma", "ZZH", "nonprompt"}; //rewrite bkg names not to confuse combine
    std::vector<std::string> processNames = {"tZq", "WZ", "multiboson", "TTZ", "TTX", "Xgamma", "ZZH", "nonprompt"};
    std::vector<std::string> flatSyst = {"lumi_2017", "id", "trigger_2017"};
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
            //{"TTZ", 1.3}
        };

    const unsigned nBinsFit = mergedHists[0][0][0][0]->GetNbinsX(); //number of bins used in the final fit
    const unsigned nStatUnc = (1 + nBkg)*nBinsFit;
    const unsigned nFlatSyst = flatSyst.size();
    //const unsigned nShapeSyst = uncNames.size();
    const unsigned nShapeSyst = nShapes;
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
                    systNames[nGeneralSystematics + p*nBinsFit + bin]  = processNames[p] + "_stat_" + catNames[cat] + "_2017_bin_" + std::to_string(bin + 1);
                }
            }

            //set BDT shape histogram
            TFile* shapeFile =TFile::Open((const TString&) "./datacards/shapes/shapeFile_"  + catNames[cat] + mllNames[m] +  "_2017.root", "recreate");

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
                        if( isCorrelatedBetweenProc[key] ){
                            mergedUncMapDown[key][m][cat][0][p]->Write( (const TString&) procName + "_" + key + "Down");
                            mergedUncMapUp[key][m][cat][0][p]->Write( (const TString&) procName + "_" + key + "Up");
                        } else {
                           mergedUncMapDown[key][m][cat][0][p]->Write( (const TString&) procName + "_" + procName + "_" + key + "Down");
                           mergedUncMapUp[key][m][cat][0][p]->Write( (const TString&) procName + "_" + procName + "_" + key + "Up");
                        }
                    }

                    //write statistical shape nuisances 
                    for(unsigned bin = 0; bin < nBinsFit; ++bin){
                        bdtShapeStatDown[m][cat][p][bin]->Write( (const TString&) procName + "_" + systNames[nGeneralSystematics + (p - 1)*nBinsFit + bin] + "Down");
                        bdtShapeStatUp[m][cat][p][bin]->Write( (const TString&) procName + "_" + systNames[nGeneralSystematics + (p - 1)*nBinsFit + bin] + "Up");
                    }
                }
            }
            shapeFile->Close();

            analysisTools::printDataCard( mergedHists[m][cat][0][0]->GetSumOfWeights(), mergedHists[m][cat][0][1]->GetSumOfWeights(), "tZq", bkgYields, proc.size() - 2, bkgNames, systUnc, nSyst, &systNames[0], systDist, "datacards/datacard_" + mllNames[m] + "_" + catNames[cat] + "_2017", true, "shapes/shapeFile_"  + catNames[cat] + mllNames[m] + "_2017");
        }
    }   
    std::cout << "Printed all datacards" << std::endl;

     //initialize postFitScaler
    PostFitScaler postFitScaler("total_postFit_yields.txt");

    //plot all distributions
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                
                //scale histograms to post fit plots
                for(unsigned p = 1; p < proc.size(); ++p){
                    mergedHists[m][cat][dist][p]->Scale( postFitScaler.postFitScaling(  mergedHists[m][cat][dist][p]->GetSumOfWeights() ) );
                }

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/2017/final/" + catNames[cat] + "/" + histInfo[dist].name() + "_" + catNames[cat] + "_" + mllNames[m] + "_2017_postFit", "tzq", false, false, "35.9 fb^{-1} (13 TeV)", &totalSystUnc[m][cat][dist][1], isSMSignal);             //linear plots
      
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "plots/tZq/2017/final/" + catNames[cat] + "/" + histInfo[dist].name() + "_"  + catNames[cat] + "_" + mllNames[m] + "_2017_postFit" + "_log", "tzq", true, false, "35.9 fb^{-1} (13 TeV)", &totalSystUnc[m][cat][dist][1], isSMSignal);    //log plots
            }
        }
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
