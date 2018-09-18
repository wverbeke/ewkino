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
#include "../interface/Reweighter.h"
#include "../interface/HistInfo.h"
#include "../interface/Efficiency.h"
#include "../interface/EfficiencyUnc.h"
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
    readSamples("sampleLists/samples_dilepCR_2016.txt", "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino/");

    //name      xlabel    nBins,  min, max
    histInfo = {

        //new BDT distribution
        HistInfo("pT", "p_{T} (GeV)", 20, 20., 100.),
        HistInfo("eta", "#eta", 20, -2.4, 2.4),

    };

    std::vector< std::string > uncNames_data = {"nonprompt_norm"};
    std::vector< std::string > uncNames_MC = {"lepton_reco", "muon_id_stat_2016", "electron_id_stat_2016", "lepton_id_syst"};
    const unsigned nDist = histInfo.size(); //number of distributions to plot

    std::vector< EfficiencyUnc > efficiencies_data;
    std::vector< EfficiencyUnc > efficiencies_MC;
    for(unsigned dist = 0; dist < nDist; ++dist){
        efficiencies_data.push_back(EfficiencyUnc("data_eff", histInfo[dist], uncNames_data, true) );
        efficiencies_MC.push_back(EfficiencyUnc("MC_eff", histInfo[dist], uncNames_MC) );
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
            if( !( passDileptonTriggers() || passSingleLeptonTriggers() ) ) continue;
            if( !passMETFilters() ) continue;

            //vector containing good lepton indices
            std::vector<unsigned> ind;

            //select two leptons 
            unsigned nLooseLeptons = 0;
            for(unsigned l = 0; l < _nLight; ++l){
                if( lepIsLoose(l) ){
                    ind.push_back(l);
                    ++nLooseLeptons;
                }
            }
            if(nLooseLeptons != 2) continue;

            //require em events 
            if( _lFlavor[ind[0]] == _lFlavor[ind[1]] ) continue;

            //cuts on probe electron
            bool goodElectron = false;
			unsigned probeIndex = 999;
            for(unsigned l = 0; l < nLooseLeptons; ++l){
                if( isElectron(ind[l]) ){
                    if( ! lepIsTight(ind[l]) ) continue;
                    if( _lPt[ind[l]] < 50. ) continue;  
                    goodElectron = true;
                } else {
					probeIndex = ind[l];
				}
            }
    
            if( !goodElectron ) continue;

            //require 2 jets
			unsigned jetCount = nJets(0, true, false);
			if(jetCount < 2) continue;

			//require 1 b jet
			unsigned bJetCount = nBJets(0, true, true, 1);
			if(bJetCount < 1) continue;

            if( isMC() ){

                //initialize SF weights
                initializeWeights();

			    //apply pu reweighting
                weight *= puWeight();

			    //apply b tag reweighting
                weight *= bTagWeight();

                //apply lepton efficiency weights 
                weight *= leptonWeight();
            }

			//check if probe is prompt or nonprompt and whether it's tight
			bool probeIsPrompt;
            if( isMC() ){
                probeIsPrompt =  _lIsPrompt[probeIndex];
            } else {
                probeIsPrompt = true;
            }
            bool probeIsTight = lepIsTight( probeIndex );

            double fill[nDist] = { _lPt[probeIndex], _lEta[probeIndex] };
            for(unsigned dist = 0; dist < nDist; ++dist){
                
                bool isSideband = !(probeIsPrompt);
                if(isSideband || isData() ){
                    efficiencies_data[dist].fill( fill[dist], weight, probeIsTight, isSideband);

                    double npUnc = 0.;
                    if( isSideband ){
                        npUnc = 0.3;
                    }
                    efficiencies_data[dist].fillVariationDown("nonprompt_norm", fill[dist], weight*( 1 - npUnc), probeIsTight, isSideband );
                    efficiencies_data[dist].fillVariationUp("nonprompt_norm", fill[dist], weight*( 1 + npUnc), probeIsTight, isSideband );
                } else {
                    efficiencies_MC[dist].fill( fill[dist], weight, probeIsTight, isSideband);

                    //reco variation
                    efficiencies_MC[dist].fillVariationDown( "lepton_reco", fill[dist], weight*leptonWeight("recoDown")/leptonWeight(""), probeIsTight, isSideband );
                    efficiencies_MC[dist].fillVariationUp( "lepton_reco", fill[dist], weight*leptonWeight("recoUp")/leptonWeight(""), probeIsTight, isSideband );

                    //muon stat variation
                    efficiencies_MC[dist].fillVariationDown("muon_id_stat_2016", fill[dist], weight*leptonWeight("muon_idStatDown")/leptonWeight(""), probeIsTight, isSideband );
                    efficiencies_MC[dist].fillVariationUp("muon_id_stat_2016", fill[dist], weight*leptonWeight("muon_idStatUp")/leptonWeight(""), probeIsTight, isSideband );

                    //electron stat variation
                    efficiencies_MC[dist].fillVariationDown( "electron_id_stat_2016", fill[dist], weight*leptonWeight("electron_idStatDown")/leptonWeight(""), probeIsTight, isSideband );
                    efficiencies_MC[dist].fillVariationUp( "electron_id_stat_2016", fill[dist], weight*leptonWeight("electron_idStatUp")/leptonWeight(""), probeIsTight, isSideband );

                    //id syst variation
                    efficiencies_MC[dist].fillVariationDown( "lepton_id_syst", fill[dist], weight*leptonWeight("idSystDown")/leptonWeight(""), probeIsTight, isSideband );
                    efficiencies_MC[dist].fillVariationUp( "lepton_id_syst", fill[dist], weight*leptonWeight("idSystUp")/leptonWeight(""), probeIsTight, isSideband );
                }
            }
        }
    }

    for(unsigned dist = 0; dist < nDist; ++dist){
        std::shared_ptr<TH1D> data_efficiency = efficiencies_data[dist].getNumerator();
        data_efficiency->Divide( efficiencies_data[dist].getDenominator().get() );

        std::shared_ptr<TH1D> MC_efficiency = efficiencies_MC[dist].getNumerator();
        MC_efficiency->Divide( efficiencies_MC[dist].getDenominator().get() );

        TH1D* efficiencies[2] = {data_efficiency.get(), MC_efficiency.get()};
        std::string names[2] = {"data efficiency", "MC efficiency"};

        plotHistograms(efficiencies, 2, names, "ttbar_efficiency" + histInfo[dist].name() );
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
