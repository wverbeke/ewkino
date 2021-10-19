// import c++ libraries
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>

// include framework
#include "../../fakeRate/interface/fakeRateTools.h"
#include "../../plotting/plotCode.h"
#include "../../plotting/tdrStyle.h"

// include tools
#include "interface/fakeRateMeasurementTools.h"

int main( int argc, char* argv[] ){
   std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 3 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 2 are needed."<<std::endl;
        std::cerr<<"usage: ./plotMCFakeRateMeasurement flavour year"<<std::endl;
        return 1;
    }
    std::string year = argvStr[2];
    std::string flavor = argvStr[1];

    setTDRStyle();

    // read 2D histograms
    std::string instanceName = flavor + "_" + year;
    std::string file_name = "fakeRateMeasurement_MC_" + instanceName;
    file_name.append("_histograms.root");
    TFile* measurement_filePtr = TFile::Open( file_name.c_str() );

    std::shared_ptr<TH2D> numerator(dynamic_cast<TH2D*> (
	    measurement_filePtr->Get(("fakeRate_numerator_"+instanceName).c_str())));
    numerator->SetDirectory(gROOT);
    std::shared_ptr<TH2D> denominator(dynamic_cast<TH2D*> (
	    measurement_filePtr->Get(("fakeRate_denominator_"+instanceName).c_str())));
    denominator->SetDirectory(gROOT);
    std::shared_ptr<TH2D> ratio(dynamic_cast<TH2D*>( numerator->Clone()));
    ratio->SetDirectory(gROOT);
    // read heavyflavor histograms
    std::shared_ptr<TH1D> heavynumerator(dynamic_cast<TH1D*> (
            measurement_filePtr->Get(("fakeRate_numerator_heavyflavor_"+instanceName).c_str())));
    heavynumerator->SetDirectory(gROOT);
    std::shared_ptr<TH1D> heavydenominator(dynamic_cast<TH1D*> (
            measurement_filePtr->Get(("fakeRate_denominator_heavyflavor_"+instanceName).c_str())));
    heavydenominator->SetDirectory(gROOT);
    std::shared_ptr<TH1D> heavyratio(dynamic_cast<TH1D*>( heavynumerator->Clone()));
    heavyratio->SetDirectory(gROOT);
    // read lightflavor histograns
    std::shared_ptr<TH1D> lightnumerator(dynamic_cast<TH1D*> (
            measurement_filePtr->Get(("fakeRate_numerator_lightflavor_"+instanceName).c_str())));
    lightnumerator->SetDirectory(gROOT);
    std::shared_ptr<TH1D> lightdenominator(dynamic_cast<TH1D*> (
            measurement_filePtr->Get(("fakeRate_denominator_lightflavor_"+instanceName).c_str())));
    lightdenominator->SetDirectory(gROOT);
    std::shared_ptr<TH1D> lightratio(dynamic_cast<TH1D*>( lightnumerator->Clone()));
    lightratio->SetDirectory(gROOT);
    measurement_filePtr->Close();

    // take ratios
    ratio->Divide(denominator.get());
    lightratio->Divide(lightdenominator.get());
    heavyratio->Divide(heavydenominator.get());
    std::shared_ptr<TH1D> heavytolight(dynamic_cast<TH1D*>(heavyratio->Clone()));
    heavytolight->Divide(lightratio.get());

    /*std::cout<<"some example values:"<<std::endl;
    for(unsigned xbin=1; xbin<=5; ++xbin){
	for(unsigned ybin=1; ybin<=3; ++ybin){
	    std::cout<<"bin: "<<xbin<<" "<<ybin<<std::endl;
	    std::cout<<numerator->GetBinContent(xbin,ybin)<<std::endl;
	    std::cout<<denominator->GetBinContent(xbin,ybin)<<std::endl;
	    std::cout<<ratio->GetBinContent(xbin,ybin)<<std::endl;
	}
    }*/
    
    // make the plot
    systemTools::makeDirectory("fakeRateMaps");
    plot2DHistogram( ratio.get(), ( "fakeRateMaps/fakeRateMap_MC_" + instanceName + ".png").c_str() );
    // for testing purposes: also plot the numerator and denominator separately
    /*gStyle->SetPaintTextFormat("5.3g"); // scientific notation of bin contents 
    plot2DHistogram( numerator.get(), ( "fakeRateMaps/fakeRateMap_MC_" + instanceName 
					+ "_numerator.png").c_str() );
    std::cout << "numerator: " << numerator.get()->GetEntries() << std::endl;
    plot2DHistogram( denominator.get(), ( "fakeRateMaps/fakeRateMap_MC_" + instanceName 
                                        + "_denominator.png").c_str() );
    std::cout << "denominator: " << denominator.get()->GetEntries() << std::endl;*/

    // write the corresponding histogram to a root file
    TFile* writeFile = TFile::Open( ( "fakeRateMaps/fakeRateMap_MC_" + instanceName + ".root" ).c_str(),
                                        "RECREATE" );
    ratio->Write( ("fakeRate_" + instanceName).c_str() );
    // also write the heavy and light fake rates and their ratio to the file
    lightratio->Write( ("fakeRate_lightflavor_"+instanceName).c_str() );
    heavyratio->Write( ("fakeRate_heavyflavor_"+instanceName).c_str() );
    heavytolight->Write( ("heavytolight"+instanceName).c_str() );
    writeFile->Close();

    return 0;
}

