//include c++ library classes 
#include <stdexcept>

//include ROOT classes
#include "TH1D.h"
#include "TF1.h"

//include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"
#include "interface/fakeRateTools.h"
#include "../Tools/interface/systemTools.h"


void plotConeCorrectionFactor( std::shared_ptr<TH1D> leptonMVAHistogram, 
		      std::shared_ptr<TH1D> pTWeightedLeptonMVAHistogram,
		      const std::string& leptonFlavor, const std::string& year,
		      const std::string& leptonMVA, const double mvaThreshold ){

    // make output directory
    std::string outputDirectory = "coneCorrectionPlots";
    systemTools::makeDirectory(outputDirectory);

    // make average pT histogram as a function of lepton MVA 
    // by dividing the pT-weighted histogram by the normal one.
    // first remove the errors on the denominator to avoid double counting 
    for( int bin = 1; bin < leptonMVAHistogram->GetNbinsX() + 1; ++bin ){
        leptonMVAHistogram->SetBinError( bin, 0. );
    }
    pTWeightedLeptonMVAHistogram->Divide( leptonMVAHistogram.get() );

    // plot average pt histogram before applying cone correction factor
    pTWeightedLeptonMVAHistogram.get()->SetStats( 0 );
    plotHistograms(pTWeightedLeptonMVAHistogram.get(), "", 
		    outputDirectory+"/averagePTVSLeptonMVA_" + leptonMVA 
		    + "_" + leptonFlavor + "_" + year, false);

    // determine cone correction factor 
    auto binIndexAboveThreshold = pTWeightedLeptonMVAHistogram->FindBin( mvaThreshold );
    double aboveThreshold = pTWeightedLeptonMVAHistogram->GetBinContent( binIndexAboveThreshold );
    double belowThreshold = pTWeightedLeptonMVAHistogram->GetBinContent( binIndexAboveThreshold - 1 );
    double coneCorrectionFactor = aboveThreshold/belowThreshold;
    std::cout << "#########################################################" << std::endl;
    std::cout << "cone correction factor from equalizing bins = " << coneCorrectionFactor << std::endl;
    std::cout << "#########################################################" << std::endl;

    if( mvaThreshold < 0.6 && mvaThreshold > -0.6 ){

        // try determining cone correction factor with a fit 
        std::shared_ptr< TF1 > f_aboveThreshold = std::shared_ptr< TF1 >( new TF1("f_aboveThreshold", "[0]*x + [1]" ) );
        std::shared_ptr< TF1 > f_belowThreshold = std::shared_ptr< TF1 >( new TF1("f_belowThreshold", "[0]*x + [1]" ) );

        pTWeightedLeptonMVAHistogram->Fit( "f_aboveThreshold", "", "", mvaThreshold, 0.6  );
        pTWeightedLeptonMVAHistogram->Fit( "f_belowThreshold", "", "", -0.6, mvaThreshold );

        std::cout << "fit above threshold = " << f_aboveThreshold->GetParameter(0) << "*x + " << f_aboveThreshold->GetParameter(1) << "\t chisquare = " << f_aboveThreshold->GetChisquare() << std::endl;
        std::cout << "fit below threshold = " << f_belowThreshold->GetParameter(0) << "*x + " << f_belowThreshold->GetParameter(1) << "\t chisquare = " << f_belowThreshold->GetChisquare() << std::endl;
        std::cout << "##################################" << std::endl;
        std::cout << "cone correction factor from fit = " << (  f_aboveThreshold->GetParameter(0)*mvaThreshold + f_aboveThreshold->GetParameter(1) )/( f_belowThreshold->GetParameter(0)*mvaThreshold + f_belowThreshold->GetParameter(1) ) << std::endl;
        std::cout << "##################################" << std::endl;
    }
    
    // plot average pT histogram after application of cone correction factor
    for( int bin = 1; bin < binIndexAboveThreshold; ++bin ){
        pTWeightedLeptonMVAHistogram->SetBinContent( bin, 
	    pTWeightedLeptonMVAHistogram->GetBinContent( bin )*coneCorrectionFactor );
    }
    plotHistograms( pTWeightedLeptonMVAHistogram.get(), "", 
		outputDirectory+"/averagePTVSLeptonMVA_postConeCorrectionFactor_" 
		+ leptonMVA + "_" + "_" + leptonFlavor + "_" + year, false );
}


int main( int argc, char* argv[] ){
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc != 5 ){
        std::cerr << argc - 1 << " command line arguments given, while 4 are expected." << std::endl;
        std::cerr << "  - lepton flavour" << std::endl;
	std::cerr << "  - data taking year" << std::endl;
	std::cerr << "  - lepton MVA identifier" << std::endl;
	std::cerr << "  - numerical MVA threshold" << std::endl;
	return 1;
    }

    std::string leptonFlavor = argvStr[1];
    std::string year = argvStr[2];
    std::string leptonMVA = argvStr[3];
    double mvaThreshold = std::stod( argvStr[4] );

    // read histograms
    std::string fileName = "coneCorrectionFactor_";
    fileName.append( leptonMVA + "_" + leptonFlavor + "_" + year );
    fileName.append( "_histograms.root" );
    TFile* measurement_filePtr = TFile::Open( fileName.c_str() );

    std::shared_ptr<TH1D> leptonMVAHistogram(dynamic_cast<TH1D*> (
        measurement_filePtr->Get("leptonMVA")));
    leptonMVAHistogram->SetDirectory(gROOT);
    std::shared_ptr<TH1D> pTWeightedLeptonMVAHistogram(dynamic_cast<TH1D*> (
        measurement_filePtr->Get("pTLeptonMVA")));
    pTWeightedLeptonMVAHistogram->SetDirectory(gROOT);
    plotConeCorrectionFactor( leptonMVAHistogram, pTWeightedLeptonMVAHistogram,
			      leptonFlavor, year, leptonMVA, mvaThreshold );
    return 0;
}
