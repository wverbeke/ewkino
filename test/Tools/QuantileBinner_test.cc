#include "../../Tools/interface/QuantileBinner.h"

//include c++ library classes
#include <iostream>
#include <memory>

//include ROOT classes
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"

//include other parts of framework

int main(){

    TFile* testFile = TFile::Open( "quantile_test_deltaM_300.root" );

    std::vector< TH1* > histogramVector;
    for( const auto& objectPtr : *( testFile->GetListOfKeys() ) ){
        TH1* histPtr = dynamic_cast< TH1* >( testFile->Get( objectPtr->GetName() ) );
        if( histPtr != nullptr ){
            histogramVector.push_back( histPtr );
        }
    }
    TH1* totalBkg = dynamic_cast< TH1* >( testFile->Get( "neuralNet_totalBackground" ) );
    TH1* totalSignal = dynamic_cast< TH1* >( testFile->Get( "neuralNet_totalSignal" ) );

    //std::vector< double > quantiles = { 0.7, 0.2, 0.09, 0.009, 0.001 };
    std::vector< double > quantiles = { 0.489, 0.256, 0.128, 0.064, 0.032, 0.016, 0.008, 0.004, 0.002, 0.001 };
    QuantileBinner rebinner( totalBkg, quantiles );

    std::shared_ptr< TH1D > rebinnedHist = rebinner.rebinnedHistogram( totalBkg );
    rebinnedHist->SetDirectory( gROOT );
    std::shared_ptr< TH1D > rebinnedHistSignal = rebinner.rebinnedHistogram( totalSignal );
    rebinnedHistSignal->Scale( rebinnedHist->GetSumOfWeights() / rebinnedHistSignal->GetSumOfWeights() );
    rebinnedHistSignal->SetDirectory( gROOT );
    testFile->Close();

    TCanvas* c = new TCanvas( "c", "c", 500, 500 );
    c->SetLogy();
    rebinnedHist->Draw( "histe" );
    rebinnedHistSignal->SetLineColor( kRed );
    rebinnedHistSignal->SetMarkerColor( kRed );
    rebinnedHistSignal->Draw( "histesame" );
    c->SaveAs( "rebinnedHist_test.pdf" );
    delete c; 

}
