//include c++ library classes 
#include <stdexcept>

//include ROOT classes
#include "TH1D.h"
#include "TF1.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../plotting/plotCode.h"
#include "../../plotting/tdrStyle.h"
#include "../../fakeRate/interface/fakeRateTools.h"
#include "../../Tools/interface/systemTools.h"


void plotMagicFactor( std::shared_ptr<TH1D> leptonMVAHistogram, 
		      std::shared_ptr<TH1D> pTWeightedLeptonMVAHistogram,
		      const std::string& leptonFlavor, const std::string& year,
		      const double mvaThreshold ){

    // make output directory
    systemTools::makeDirectory("magicFactorPlots");

    // make average pT histogram as a function of lepton MVA by dividing the pT-weighted histogram by the normal one
    // first remove the errors on the denominator to avoid double counting 
    for( int bin = 1; bin < leptonMVAHistogram->GetNbinsX() + 1; ++bin ){
        leptonMVAHistogram->SetBinError( bin, 0. );
    }
    pTWeightedLeptonMVAHistogram->Divide( leptonMVAHistogram.get() );

    // plot average pt histogram before applying magic factor
    pTWeightedLeptonMVAHistogram.get()->SetStats( 0 );
    plotHistograms(pTWeightedLeptonMVAHistogram.get(), "", 
		    "magicFactorPlots/averagePTVSLeptonMVA_" + leptonFlavor + "_" + year, false);

    // determine magic factor 
    auto binIndexAboveThreshold = pTWeightedLeptonMVAHistogram->FindBin( mvaThreshold );
    double aboveThreshold = pTWeightedLeptonMVAHistogram->GetBinContent( binIndexAboveThreshold );
    double belowThreshold = pTWeightedLeptonMVAHistogram->GetBinContent( binIndexAboveThreshold - 1 );
    double magicFactor = aboveThreshold/belowThreshold;
    std::cout << "##################################" << std::endl;
    std::cout << "magic factor from equalizing bins = " << magicFactor << std::endl;
    std::cout << "##################################" << std::endl;

    if( mvaThreshold < 0.6 && mvaThreshold > -0.6 ){

        // try determining magic factor with a fit 
        std::shared_ptr< TF1 > f_aboveThreshold = std::shared_ptr< TF1 >( new TF1("f_aboveThreshold", "[0]*x + [1]" ) );
        std::shared_ptr< TF1 > f_belowThreshold = std::shared_ptr< TF1 >( new TF1("f_belowThreshold", "[0]*x + [1]" ) );

        pTWeightedLeptonMVAHistogram->Fit( "f_aboveThreshold", "", "", mvaThreshold, 0.6  );
        pTWeightedLeptonMVAHistogram->Fit( "f_belowThreshold", "", "", -0.6, mvaThreshold );

        std::cout << "fit above threshold = " << f_aboveThreshold->GetParameter(0) << "*x + " << f_aboveThreshold->GetParameter(1) << "\t chisquare = " << f_aboveThreshold->GetChisquare() << std::endl;
        std::cout << "fit below threshold = " << f_belowThreshold->GetParameter(0) << "*x + " << f_belowThreshold->GetParameter(1) << "\t chisquare = " << f_belowThreshold->GetChisquare() << std::endl;
        std::cout << "##################################" << std::endl;
        std::cout << "magic factor from fit = " << (  f_aboveThreshold->GetParameter(0)*mvaThreshold + f_aboveThreshold->GetParameter(1) )/( f_belowThreshold->GetParameter(0)*mvaThreshold + f_belowThreshold->GetParameter(1) ) << std::endl;
        std::cout << "##################################" << std::endl;
    }
    
    // plot average pT histogram after application of magic factor
    for( int bin = 1; bin < binIndexAboveThreshold; ++bin ){
        pTWeightedLeptonMVAHistogram->SetBinContent( bin, pTWeightedLeptonMVAHistogram->GetBinContent( bin )*magicFactor );
    }
    plotHistograms( pTWeightedLeptonMVAHistogram.get(), "", 
			"magicFactorPlots/averagePTVSLeptonMVA_postMagicFactor_" + leptonFlavor + "_" + year, false );
}


int main( int argc, char* argv[] ){
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc == 4 ){
        std::string leptonFlavor = argvStr[1];
	std::string year = argvStr[2];
	double mvaThreshold = std::stod( argvStr[3] );
	// read histograms
	std::string file_name = "magicFactor_" + leptonFlavor + "_" + year;
	file_name.append("_histograms.root");
	TFile* measurement_filePtr = TFile::Open( file_name.c_str() );

	std::shared_ptr<TH1D> leptonMVAHistogram(dynamic_cast<TH1D*> (
            measurement_filePtr->Get("leptonMVA")));
	leptonMVAHistogram->SetDirectory(gROOT);
	std::shared_ptr<TH1D> pTWeightedLeptonMVAHistogram(dynamic_cast<TH1D*> (
            measurement_filePtr->Get("pTLeptonMVA")));
	pTWeightedLeptonMVAHistogram->SetDirectory(gROOT);
        plotMagicFactor( leptonMVAHistogram, pTWeightedLeptonMVAHistogram,
			    leptonFlavor, year, mvaThreshold );
    } else {
        std::cerr << argc - 1 << " command line arguments given, while 2 are expected." << std::endl;
        std::cerr << "< lepton flavor > < mvaThreshold >" << std::endl;
    }   
    return 0;
}
