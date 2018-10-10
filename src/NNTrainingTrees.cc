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
//#include "TMVA/Tools.h"
//#include "TMVA/Reader.h"

void treeReader::Analyze(){

    //Set CMS plotting style
    setTDRStyle();
    gROOT->SetBatch(kTRUE);

    //read samples and cross sections from txt file
    readSamples("sampleLists/samples_ewkino_2016.txt", "../../ntuples_ewkino");

    //name      xlabel    nBins,  min, max
    histInfo = {
        HistInfo("mll", "M_{ll} (best - Z) (GeV)", 50, 0, 400),
        HistInfo("mllOther", "M_{ll} (non best - Z) (GeV)", 50, 0, 400),
        HistInfo("mt", "M_{T} (GeV)", 50, 0, 600),
        HistInfo("LTPlusMET", "L_{T} + E_{T}^{miss} (GeV)", 50, 0, 1200),
        HistInfo("HT", "H_{T} (GeV)", 50, 0, 800), 
        HistInfo("m3l", "M_{3l} (GeV)", 50, 0, 1200),
        HistInfo("mt3l", "M_{T}(3l + MET) (GeV)", 50, 0, 800),
        HistInfo("nJets", "number of jets", 8, 0, 8),
        HistInfo("met", "MET (GeV)", 50, 0, 800),
        HistInfo("metPhi", "#Phi(MET)", 50, 0, 800),
    };

    const unsigned nDist = histInfo.size(); //number of distributions to plot

    const std::vector<std::string> mllNames = {"mllInclusive", "onZ", "offZ"};
    const size_t nMll = mllNames.size();

    //initialize vector holding all histograms
    std::vector< std::vector< std::vector< std::shared_ptr< TH1D > > > > hists(nMll);
    for(unsigned m = 0; m < nMll; ++m){
        hists[m] = std::vector< std::vector< std::shared_ptr< TH1D > > >(nDist);
        for(unsigned dist = 0; dist < nDist; ++dist){
            hists[m][dist] = std::vector< std::shared_ptr< TH1D > >( samples.size() );
            for(size_t sam = 0; sam < samples.size(); ++sam){
                hists[m][dist][sam] = histInfo[dist].makeHist( mllNames[m] + samples[sam].getUniqueName() );
            }
        }
    }

    //variables to write to training file
    std::map< std::string, float > bdtVariableMap = {
        {"mllBestZ", 0.},
        {"mllOther", 0.},
        {"mt", 0.},
        {"LTPlusMET", 0.},
        {"HT", 0.},
        {"m3l", 0.},
        {"mt3l", 0.},
        {"nJets", 0.},
        {"lepPt1", 0.},
        {"lepEta1", 0.},
        {"lepPhi1", 0.},
        {"lepCharge1", 0.},
        {"lepFlavor1", 0.},
        {"lepPt2", 0.},
        {"lepEta2", 0.},
        {"lepPhi2", 0.},
        {"lepCharge2", 0.},
        {"lepFlavor2", 0.},
        {"lepPt3", 0.},
        {"lepEta3", 0.},
        {"lepPhi3", 0.},
        {"lepCharge3", 0.},
        {"lepFlavor3", 0.},
        {"metPt", 0.},
        {"metPhi", 0.},
        {"jetPt1", 0.},
        {"jetEta1", 0.},
        {"jetPhi1", 0.},
        {"jetM1", 0.},
        {"jetCSV1", 0.},
        {"jetPt2", 0.},
        {"jetEta2", 0.},
        {"jetPhi2", 0.},
        {"jetM2", 0.},
        {"jetCSV2", 0.},
        //{"susyMassSplitting", 0.},
        {"eventWeight", 0.}
    };

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){

        initSample();

        //make NN training tree for current sample
        if( isNewPhysicsSignal() ){
            bdtVariableMap["susyMassSplitting"] = 0.;
        }
        TrainingTree trainingTree("ewkinoNNTrainingTree/", currentSample, { {"mllInclusive", "onZ", "offZ"} }, bdtVariableMap, isNewPhysicsSignal() ); 

        std::cout<<"Entries in "<< currentSample.getFileName() << " " << nEntries << std::endl;

        double progress = 0; 	//for printing progress bar
        for(long unsigned it = 0; it < nEntries/100; ++it){

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
            //if( !passTriggerCocktail() ) continue;
            //if( !passMETFilters() ) continue;

            //vector containing good lepton indices
            std::vector<unsigned> ind;

            //select leptons
            const unsigned lCount = selectLep(ind);
            if(lCount != 3) continue;

            if( !(tightLepCount(ind, lCount) == lCount) ) continue;

            //require pt cuts (25, 15, 10) to be passed
            if(!passPtCuts(ind)) continue;

            //require presence of OSSF pair
            if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 

            //remove overlap between samples
            if(photonOverlap(currentSample)) continue;

            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);

            //find best Z mass and use it to determine the mll category
            std::pair<unsigned, unsigned> bestZ = trilep::bestZ(lepV, ind, _lFlavor, _lCharge, lCount);
            double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();
            unsigned mllCat;
            if( fabs(mll - 91.1876) >= 15){
                mllCat = 1;
            } else {
                mllCat = 0;
            }

            //ewkino event selection
            if( _met < 50 ) continue;
            if( nBJets() != 0 ) continue;
    
            //compute kinematic quantities of the event

            //find lepton from W decay
           	unsigned lw = 99;
    		for(unsigned l = 0; l < lCount; ++l){
        		if( l != bestZ.first && l != bestZ.second ) lw = l;
			} 

            TLorentzVector met;
            met.SetPtEtaPhiE( _met, 0, _metPhi, _met);
            double mt = kinematics::mt(met, lepV[lw]);

            //compute mll of OS pair that does not form best Z
            double mllOther = 0.;
            if( _lCharge[ind[lw]] != _lCharge[ind[bestZ.first]]){
                mllOther = (lepV[lw] + lepV[bestZ.second]).M();
            } else if(  _lCharge[ind[lw]] !=  _lCharge[ind[bestZ.second]] ){
                mllOther = (lepV[lw] + lepV[bestZ.second]).M();
            }

			double LT = 0.;
 			for(unsigned l = 0; l < lCount; ++l){
        		LT += _lPt[ind[l]];
			}	

			//select jets and compute HT
    		double HT = 0;
            std::vector<unsigned> jetInd; 
            unsigned jetCount = nJets(jetInd);
            TLorentzVector jetV[jetCount]; 
            for( unsigned j = 0; j < jetCount; ++j){
                HT += _jetPt[jetInd[j]];    
                jetV[j].SetPtEtaPhiE( _jetPt[jetInd[j]], _jetEta[jetInd[j]], _jetPhi[jetInd[j]], _jetE[jetInd[j]] );
            }

    		for(unsigned j = 0; j < _nJets; ++j){
				if( jetIsGood(j) ){
        			HT += _jetPt[j];
				}
			}

			TLorentzVector lepSyst( lepV[0] );
			for(unsigned l = 1; l < lCount; ++l){
				lepSyst += lepV[l];
			}

			double m3l = lepSyst.M();
			double mt3l = kinematics::mt(lepSyst, met);

			double fill[nDist] = {
				mll,
                mllOther,
				mt,	
				LT + _met,
				HT,
				m3l,
				mt3l,
                (double) jetCount,
				_met,
                _metPhi, 
			};

            //set BDT variable maps
			bdtVariableMap["mllBestZ"] = mll;
			bdtVariableMap["mllOther"] = mllOther;
			bdtVariableMap["mt"] = mt;
			bdtVariableMap["LTPlusMET"] = LT + _met;
			bdtVariableMap["HT"] = HT;
			bdtVariableMap["m3l"] = m3l;
			bdtVariableMap["mt3l"] = mt3l;	
            bdtVariableMap["nJets"] = (double) jetCount;
			bdtVariableMap["metPt"] = _met;
            bdtVariableMap["metPhi"] = _metPhi;
            bdtVariableMap["lepPt1"] = _lPt[ind[0]];
            bdtVariableMap["lepEta1"] = _lEta[ind[0]];
            bdtVariableMap["lepPhi1"] = _lPhi[ind[0]];
            bdtVariableMap["lepCharge1"] = _lCharge[ind[0]];
            bdtVariableMap["lepFlavor1"] = _lFlavor[ind[0]];
            bdtVariableMap["lepPt2"] = _lPt[ind[1]];
            bdtVariableMap["lepEta2"] = _lEta[ind[1]];
            bdtVariableMap["lepPhi2"] = _lPhi[ind[1]];
            bdtVariableMap["lepCharge2"] = _lCharge[ind[1]];
            bdtVariableMap["lepFlavor2"] = _lFlavor[ind[1]];
            bdtVariableMap["lepPt3"] = _lPt[ind[2]];
            bdtVariableMap["lepEta3"] = _lEta[ind[2]];
            bdtVariableMap["lepPhi3"] = _lPhi[ind[2]];
            bdtVariableMap["lepCharge3"] = _lCharge[ind[2]];
            bdtVariableMap["lepFlavor3"] = _lFlavor[ind[2]];
            bdtVariableMap["jetPt1"] = ( (jetCount > 0) ? jetV[0].Pt() : 0. );
            bdtVariableMap["jetEta1"] = ( (jetCount > 0) ? jetV[0].Eta() : 0. );
            bdtVariableMap["jetPhi1"] = ( (jetCount > 0) ? jetV[0].Phi() : 0. );
            bdtVariableMap["jetM1"] = ( (jetCount > 0) ? jetV[0].M() : 0. );
            bdtVariableMap["jetCSV1"] = ( (jetCount > 0) ? std::max( deepCSV( jetInd[0] ), 0.) : 0. );
            bdtVariableMap["jetPt2"] = ( (jetCount > 1) ? jetV[1].Pt() : 0. );
            bdtVariableMap["jetEta2"] = ( (jetCount > 1) ? jetV[1].Eta() : 0. );
            bdtVariableMap["jetPhi2"] = ( (jetCount > 1) ? jetV[1].Phi() : 0. );
            bdtVariableMap["jetM2"] = ( (jetCount > 1) ? jetV[1].M() : 0. );
            bdtVariableMap["jetCSV2"] = ( (jetCount > 1) ? std::max( deepCSV( jetInd[1] ), 0.) : 0. );
            bdtVariableMap["eventWeight"] = weight;
            if( isNewPhysicsSignal() ){
                bdtVariableMap["susyMassSplitting"] = ( _mChi2 - _mChi1 );
            } else{
                //bdtVariableMap["susyMassSplitting"] = 0.;
            }

            //write variables to histograms 
			for(unsigned m = 0; m < nMll; ++m){
				if( !( m == 0 || m == ( mllCat + 1) ) ) continue;
				if( isMC() ){
                    trainingTree.fill( std::vector< size_t >({m}), bdtVariableMap);
                }
            	for(unsigned dist = 0; dist < nDist; ++dist){
            	    hists[m][dist][sam]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            	}
			}
        }

        //set histograms to 0 if negative
        for(unsigned m = 0; m < nMll; ++m){
            for(unsigned dist = 0; dist < nDist; ++dist){
                analysisTools::setNegativeBinsToZero( hists[m][dist][sam].get() );
            }	
        }
    }

    //merge histograms with the same physical background
    std::vector<std::string> proc = {"Total bkg.", "Drell-Yan", "TT", "WZ", "multiboson", "TT/T + X", "X + #gamma", "ZZ/H", "TChiWZ"};
    std::vector< std::vector< std::vector< TH1D* > > > mergedHists(nMll);
    for(unsigned mll = 0; mll < nMll; ++mll){
        mergedHists[mll] = std::vector < std::vector < TH1D* > >(nDist);
        for(unsigned dist = 0; dist < nDist; ++dist){
            mergedHists[mll][dist] = std::vector < TH1D* >( proc.size() );
            for(size_t m = 0, sam = 0; m < proc.size(); ++m){
                mergedHists[mll][dist][m] = (TH1D*) hists[mll][dist][sam]->Clone();
                while( sam < samples.size() - 1 && samples[sam].getProcessName() == samples[sam + 1].getProcessName() ){
                    mergedHists[mll][dist][m]->Add( hists[mll][dist][sam + 1].get() );
                    ++sam;
                }
                ++sam;
            }
        }
    }

    //blind data!
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned dist = 0; dist < nDist; ++dist){
            delete mergedHists[m][dist][0];
            mergedHists[m][dist][0] = (TH1D*) mergedHists[m][dist][1]->Clone();
            for(unsigned p = 2; p < proc.size() - 1 ; ++p){
                mergedHists[m][dist][0]->Add( mergedHists[m][dist][p] );
            }
        }
    }

    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned dist = 0; dist < nDist; ++dist){
        	plotDataVSMC(mergedHists[m][dist][0], &mergedHists[m][dist][1], &proc[0], mergedHists[m][dist].size() - 2, "plots/ewkino/trilep/" + mllNames[m] + "/" + histInfo[dist].name() + "_" + mllNames[m], "ewkino", false, false, "35.9 fb^{-1} (13 TeV)", nullptr, nullptr, &mergedHists[m][dist][proc.size() - 1], &proc[proc.size() - 1], 1);             //linear plots

			plotDataVSMC(mergedHists[m][dist][0], &mergedHists[m][dist][1], &proc[0], mergedHists[m][dist].size() - 2, "plots/ewkino/trilep/" + mllNames[m] + "/" + histInfo[dist].name() + "_" + mllNames[m] + "_log", "ewkino", true, false, "35.9 fb^{-1} (13 TeV)", nullptr, nullptr, &mergedHists[m][dist][proc.size() - 1], &proc[proc.size() - 1], 1); //log plots    
        }
    }
    
    //clean up memory
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned dist = 0; dist < nDist; ++dist){
            for(unsigned p = 0; p < proc.size(); ++p){
                delete mergedHists[m][dist][p];
            }
        }
    }
}


int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
