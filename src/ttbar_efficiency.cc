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
//#include "../interface/trilepTools.h"
#include "../interface/Reweighter.h"
#include "../interface/HistInfo.h"
//#include "../interface/kinematicTools.h"
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

    const unsigned nDist = histInfo.size(); //number of distributions to plot

    //initialize vector holding all histograms
    std::vector< std::vector< std::shared_ptr< TH1D > > > numerator_prompt( 2, std::vector< std::shared_ptr< TH1D > >(nDist) );
    std::vector< std::vector< std::shared_ptr< TH1D > > > numerator_nonprompt( 2, std::vector< std::shared_ptr< TH1D > >(nDist) );
    std::vector< std::vector< std::shared_ptr< TH1D > > > denominator_prompt( 2, std::vector< std::shared_ptr< TH1D > >(nDist) );
    std::vector< std::vector< std::shared_ptr< TH1D > > > denominator_nonprompt( 2, std::vector< std::shared_ptr< TH1D > >(nDist) );

    const std::string processNames[2] = {"data", "MC"};
    
    for(unsigned p = 0; p < 2; ++p){
        for(unsigned dist = 0; dist < nDist; ++dist){
            numerator_prompt[p][dist] = histInfo[dist].makeHist("numerator_prompt_" + processNames[p] );
            numerator_prompt[p][dist]->Sumw2();
            numerator_nonprompt[p][dist] = histInfo[dist].makeHist("numerator_nonprompt_" + processNames[p] );
            numerator_nonprompt[p][dist]->Sumw2();
            denominator_prompt[p][dist] = histInfo[dist].makeHist("denominator_prompt_" + processNames[p] );
            denominator_prompt[p][dist]->Sumw2();
            denominator_nonprompt[p][dist] = histInfo[dist].makeHist("denominator_nonprompt_" + processNames[p] );
            denominator_nonprompt[p][dist]->Sumw2();
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
                    if( _lPt[ind[l]] < 25. ) continue;  
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

            //fill different histograms for data and MC
            unsigned dataIndex = 0;
            if( isMC() ){
                dataIndex = 1;
            }

            double fill[nDist] = { _lPt[probeIndex], _lEta[probeIndex] };
            for(unsigned dist = 0; dist < nDist; ++dist){
                if(probeIsPrompt){
                    if(probeIsTight){
                        numerator_prompt[dataIndex][dist]->Fill(fill[dist], weight);
                    }
                    denominator_prompt[dataIndex][dist]->Fill(fill[dist], weight);
                } else {
                    if(probeIsTight){
                        numerator_nonprompt[dataIndex][dist]->Fill(fill[dist], weight);
                    }
                    denominator_nonprompt[dataIndex][dist]->Fill(fill[dist], weight);
                }    
            }
        }
    }

    //make efficiency histograms for data and mc 
    std::vector< std::shared_ptr < TH1D > > data_efficiencies( nDist );
    std::vector< std::shared_ptr < TH1D > > MC_efficiencies( nDist );

    for(unsigned dist = 0; dist < nDist; ++dist){
        data_efficiencies[dist] = std::shared_ptr<TH1D>( (TH1D*) numerator_prompt[0][dist]->Clone() );
        data_efficiencies[dist]->Add( numerator_nonprompt[1][dist].get(), -1. );
        
        std::shared_ptr<TH1D> data_denominator = std::shared_ptr<TH1D>( (TH1D*) denominator_prompt[0][dist]->Clone() );
        data_denominator->Add( denominator_nonprompt[1][dist].get(), -1. );

        data_efficiencies[dist]->Divide(data_denominator.get());

        MC_efficiencies[dist] = std::shared_ptr<TH1D>( (TH1D*) numerator_prompt[1][dist]->Clone() );
        MC_efficiencies[dist]->Divide( denominator_prompt[1][dist].get() );

        TH1D* efficiencies[2] = {data_efficiencies[dist].get(),  MC_efficiencies[dist].get()};
        std::string names[2] = {"data efficiency", "MC efficiency"};

        plotHistograms(efficiencies, 2, names, "ttbar_efficiency" + histInfo[dist].name() );
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
