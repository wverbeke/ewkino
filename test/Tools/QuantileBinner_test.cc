#include "../../Tools/interface/QuantileBinner.h"

//include c++ library classes
#include <memory>
#include <stdexcept>

//include ROOT classes
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"

//include other parts of framework
#include "../../Tools/interface/numericTools.h"

int main(){

    TFile* testFile = TFile::Open( "quantile_test_deltaM_600.root" );

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

    std::vector< double > quantileVector = exponentialBinningQuantiles( 2000, 2, 2, 1 );
    std::vector< double > expectedQuantiles = { 0.489, 0.256, 0.128, 0.064, 0.032, 0.016, 0.008, 0.004, 0.002, 0.001 };

    if( quantileVector.size() != expectedQuantiles.size() ){
        throw std::runtime_error( "Returned vector of quantiles is of size " + std::to_string( quantileVector.size() ) + ", while it is expected to be of size " + std::to_string( expectedQuantiles.size() ) );
    }

    for( size_t index = 0; index < quantileVector.size(); ++index ){
        if( !numeric::floatEquals( quantileVector[ index ], expectedQuantiles[ index ] ) ){
            throw std::runtime_error( std::to_string(  quantileVector[index] ) + " != " + std::to_string( expectedQuantiles[index] ) );
        }
    }

}
