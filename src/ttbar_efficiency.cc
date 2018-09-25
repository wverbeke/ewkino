//include ROOT classes
#include "TLorentzVector.h"
#include "TH1D.h"
#include "TFile.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "TROOT.h"
#include "TGraphAsymmErrors.h"

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
#include "../plotting/drawLumi.h"
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
        HistInfo("pT", "p_{T} (GeV)", 20, 10., 100.),
        HistInfo("eta", "#eta", 20, -2.4, 2.4),

    };

    std::vector< std::string > uncNames_data = {"nonprompt_norm"};
    std::vector< std::string > uncNames_MC = {"muon_id_stat_2016", "lepton_id_syst"};
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
                    if( _lPt[ind[l]] < 60. ) continue;  
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
    
            //TEMPORARY, REMOVE LATER
            //if( !probeIsPrompt ) continue;


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

                    double idDownSF = 1.;
                    double idUpSF = 1.;
                    for(unsigned l = 0; l < _nLight; ++l){
                        if( lepIsTight(l) ){
                            if( isMuon(l) ){
                                idDownSF *= reweighter->muonTightWeight(_lPt[l], _lEta[l], "idSystDown");
                                idUpSF *= reweighter->muonTightWeight(_lPt[l], _lEta[l], "idSystUp");
                            } else if( isElectron(l) ){
                                idDownSF *= reweighter->electronTightWeight(_lPt[l], _lEtaSC[l], "");
                                idUpSF *= reweighter->electronTightWeight(_lPt[l], _lEtaSC[l], "");
                            }
                        } else if( lepIsLoose(l) ){
                            if( isMuon(l) ){
                                idDownSF *= reweighter->muonLooseWeight(_lPt[l], _lEta[l], "idSystDown");
                                idUpSF *= reweighter->muonLooseWeight(_lPt[l], _lEta[l], "idSystUp");
                            } else if( isElectron(l) ){
                                idDownSF *= reweighter->electronLooseWeight(_lPt[l], _lEtaSC[l], "");
                                idUpSF *= reweighter->electronLooseWeight(_lPt[l], _lEtaSC[l], "");
                            }
                        }
                    }

                    //muon stat variation
                    efficiencies_MC[dist].fillVariationDown("muon_id_stat_2016", fill[dist], weight*leptonWeight("muon_idStatDown")/leptonWeight(""), probeIsTight, isSideband );
                    efficiencies_MC[dist].fillVariationUp("muon_id_stat_2016", fill[dist], weight*leptonWeight("muon_idStatUp")/leptonWeight(""), probeIsTight, isSideband );

                    //id syst variation
                    efficiencies_MC[dist].fillVariationDown( "lepton_id_syst", fill[dist], weight*idDownSF/leptonWeight(""), probeIsTight, isSideband );
                    efficiencies_MC[dist].fillVariationUp( "lepton_id_syst", fill[dist], weight*idUpSF/leptonWeight(""), probeIsTight, isSideband );
                }
            }
        }
    }

    for(unsigned dist = 0; dist < nDist; ++dist){
        std::shared_ptr<TH1D> data_efficiency = efficiencies_data[dist].getNumerator();
        data_efficiency->Divide( efficiencies_data[dist].getDenominator().get() );


        std::shared_ptr<TH1D> MC_efficiency = efficiencies_MC[dist].getNumerator();
        MC_efficiency->Divide( efficiencies_MC[dist].getDenominator().get() );


        /*
        for(unsigned bin = 1; bin < data_efficiency->GetNbinsX() + 1; ++bin){
            std::cout << data_efficiency->GetBinContent(bin) << std::endl;
        }
        */

        //plotHistograms(efficiencies, 2, names, "ttbar_efficiency" + histInfo[dist].name() );

        std::shared_ptr<TH1D> numerator_data_var =  efficiencies_data[dist].getNumeratorUnc();
        numerator_data_var->Add(efficiencies_data[dist].getNumerator().get());
        std::shared_ptr<TH1D> denominator_data = efficiencies_data[dist].getDenominator();
        numerator_data_var->Divide(denominator_data.get() );
        numerator_data_var->Add( data_efficiency.get(), -1. );


        std::shared_ptr<TH1D> denominator_data_var = efficiencies_data[dist].getNumerator();
        std::shared_ptr<TH1D> denominator_data2 = efficiencies_data[dist].getDenominatorUnc();
        denominator_data2->Add(efficiencies_data[dist].getDenominator().get());
        denominator_data_var->Divide( denominator_data2.get() );
        denominator_data_var->Add( data_efficiency.get(), -1. );

        std::shared_ptr<TH1D> data_efficiency_clone = std::shared_ptr<TH1D>( (TH1D*) data_efficiency->Clone() );
        for(int bin = 1; bin < data_efficiency_clone->GetNbinsX() + 1; ++bin){
            //double statUnc = data_efficiency_clone->GetBinError(bin);
            double systUncDen = denominator_data_var->GetBinContent(bin);
            double systUncNum = denominator_data_var->GetBinContent(bin);
            //double totalUnc = sqrt( statUnc*statUnc + systUncDen*systUncDen + systUncNum*systUncNum );
            double totalUnc = sqrt( systUncDen*systUncDen + systUncNum*systUncNum );
            data_efficiency_clone->SetBinError(bin, totalUnc);
        }

        /*
        std::cout << "####################" << std::endl;
        for(unsigned bin = 1; bin < numerator_data_var->GetNbinsX() + 1; ++bin){
            std::cout << numerator_data_var->GetBinContent(bin) << std::endl;
            //std::cout << data_efficiency->GetBinContent(bin) << std::endl;
        }
        */

        std::shared_ptr<TH1D> numerator_MC_var =  efficiencies_MC[dist].getNumeratorUnc();
        numerator_MC_var->Add(efficiencies_MC[dist].getNumerator().get());
        std::shared_ptr<TH1D> denominator_MC = efficiencies_MC[dist].getDenominator();
        numerator_MC_var->Divide(denominator_MC.get() );
        numerator_MC_var->Add( MC_efficiency.get(), -1. );

        std::shared_ptr<TH1D> denominator_MC_var =  efficiencies_MC[dist].getNumerator();
        std::shared_ptr<TH1D> denominator_MC2 = efficiencies_MC[dist].getDenominatorUnc();
        denominator_MC2->Add(efficiencies_MC[dist].getDenominator().get());
        denominator_MC_var->Divide( denominator_MC2.get() );
        denominator_MC_var->Add( MC_efficiency.get(), -1. );

        std::shared_ptr<TH1D> MC_efficiency_clone = std::shared_ptr<TH1D>( (TH1D*) MC_efficiency->Clone() );
        for(int bin = 1; bin < MC_efficiency_clone->GetNbinsX() + 1; ++bin){
            //double statUnc = MC_efficiency_clone->GetBinError(bin);
            double systUncDen = denominator_MC_var->GetBinContent(bin);
            double systUncNum = denominator_MC_var->GetBinContent(bin);
            //double totalUnc = sqrt( statUnc*statUnc + systUncDen*systUncDen + systUncNum*systUncNum );
            double totalUnc = sqrt( systUncDen*systUncDen + systUncNum*systUncNum );
            MC_efficiency_clone->SetBinError(bin, totalUnc);
        }

        /*
        std::cout << "####################" << std::endl;
        for(unsigned bin = 1; bin < numerator_MC_var->GetNbinsX() + 1; ++bin){
            std::cout << numerator_MC_var->GetBinContent(bin) << std::endl;
        }
        */

        //TH1D* uncertainties[2] ={numerator_data_var.get(), numerator_MC_var.get()};
        //
        TH1D* efficiencies[2] = {data_efficiency_clone.get(), MC_efficiency_clone.get()};
        std::string names[2] = {"data efficiency", "MC efficiency"};
        //plotHistograms(efficiencies, 2, names, "ttbar_efficiency" + histInfo[dist].name() + "_2017" );
        plotHistograms(efficiencies, 2, names, "ttbar_efficiency" + histInfo[dist].name() );

        TCanvas* c = new TCanvas("", "", 500, 500);
        
        TGraphAsymmErrors* graph_data = new TGraphAsymmErrors( efficiencies_data[dist].getNumerator().get(), efficiencies_data[dist].getDenominator().get() );

        std::cout << "nbins numerator = " << efficiencies_MC[dist].getNumerator().get()->GetNbinsX() << std::endl;
        std::cout << "nbins  = " << efficiencies_MC[dist].getDenominator().get()->GetNbinsX() << std::endl;
        TGraphAsymmErrors* graph_MC = new TGraphAsymmErrors( efficiencies_MC[dist].getNumerator().get(), efficiencies_MC[dist].getDenominator().get() );

        for( int b = 0 ; b < efficiencies[0]->GetNbinsX(); ++b){
            double systUnc = data_efficiency_clone->GetBinError(b + 1);
            double statUncHigh = graph_data->GetErrorYhigh(b);
            double statUncLow = graph_data->GetErrorYlow(b);
            double totalUncLow = sqrt(systUnc*systUnc + statUncLow*statUncLow);
            double diff = fabs(graph_data->GetY()[b] - 1.);
            double totalUncHigh = std::min( sqrt( systUnc*systUnc + statUncHigh*statUncHigh), diff);
            graph_data->SetPointEYlow(b, totalUncLow);
            graph_data->SetPointEYhigh(b, totalUncHigh);
        }

        for( int b = 0 ; b < efficiencies[0]->GetNbinsX(); ++b){
            double systUnc = MC_efficiency_clone->GetBinError(b + 1);
            double statUncHigh = graph_MC->GetErrorYhigh(b);
            double statUncLow = graph_MC->GetErrorYlow(b);
            double totalUncLow = sqrt(systUnc*systUnc + statUncLow*statUncLow);
            double diff = fabs(graph_MC->GetY()[b] - 1.);
            double totalUncHigh = std::min( sqrt( systUnc*systUnc + statUncHigh*statUncHigh), diff);
            graph_MC->SetPointEYlow(b, totalUncLow);
            graph_MC->SetPointEYhigh(b, totalUncHigh);
        }


        for( int b = 0 ; b < efficiencies[0]->GetNbinsX(); ++b){
            std::cout << graph_MC->GetY()[b] << std::endl;
        }

        efficiencies[0]->GetYaxis()->SetRangeUser(0, 1.3);
        efficiencies[0]->SetLineColor(kBlue + 1);
        efficiencies[0]->SetMarkerColor(kBlue + 1);
        //efficiencies[0]->SetFillColor(kBlue + 1);
        efficiencies[0]->SetLineWidth(2); 
        efficiencies[0]->SetFillStyle(0);


        efficiencies[0]->Draw("hist");
        efficiencies[1]->SetLineColor(kRed - 7);
        efficiencies[1]->SetMarkerColor(kRed - 7);
        //efficiencies[1]->SetFillColor(kRed - 7);
        efficiencies[1]->SetLineWidth(2); 
        efficiencies[1]->SetFillStyle(0);
        efficiencies[1]->Draw("histsame");
        graph_data->SetFillStyle(3244);
        graph_data->SetLineColor(kBlue + 1);
        graph_data->SetMarkerColor(kBlue + 1);
        graph_data->SetFillColor(kBlue + 1);
        graph_data->SetLineWidth(2); 
        graph_data->Draw("e2same");
        graph_MC->SetFillStyle(3008);
        graph_MC->SetLineColor(kRed - 7);
        graph_MC->SetMarkerColor(kRed - 7);
        graph_MC->SetFillColor(kRed - 7);
        graph_MC->Draw("e2same");

        TLegend legend = TLegend(0.2,0.8,0.95,0.9,NULL,"brNDC");
        legend.SetNColumns(2);
        legend.SetFillStyle(0); //avoid legend box
        legend.AddEntry(efficiencies[0], "data efficiency");
        legend.AddEntry(efficiencies[1], "MC efficiency");
        legend.AddEntry( graph_data, "data efficiency unc.", "f");
        legend.AddEntry( graph_MC, "MC efficiency unc.", "f");
        legend.Draw("same"); 

        drawLumi(c, "", "(13 TeV)");

        
        c->SaveAs( std::string("ttbar_" + histInfo[dist].name() + "test.pdf").c_str() );


        delete c;

        //names = {"data syst unc", "MC syst unc"};    
        //plotHistograms(uncertainties, 2, names, "ttbar_unc" + histInfo[dist].name() );
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
