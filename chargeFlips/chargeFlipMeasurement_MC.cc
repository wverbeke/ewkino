

//include c++ library classes 
#include <memory>
#include <thread>

//include ROOT classes
#include "TH2D.h"
#include "TStyle.h"

//include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/histogramTools.h"
#include "../Tools/interface/analysisTools.h"
#include "interface/chargeFlipSelection.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"


void determineMCChargeFlipRate( const std::string& year, const std::string& sampleListFile, const std::string& sampleDirectory ){

    analysisTools::checkYearString( year );

    const std::vector< double > ptBins = {10., 20., 30., 45., 65., 100., 200.};
    const std::vector< double > etaBins = { 0., 0.8, 1.442, 2.5 };


	//initialize 2D histograms for numerator and denominator
	std::string numerator_name = "chargeFlipRate_electron_" + year;
    std::shared_ptr< TH2D > numeratorMap( new TH2D( numerator_name.c_str(), ( numerator_name+ "; p_{T} (GeV); |#eta|").c_str(), ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) );
    numeratorMap->Sumw2();

	std::string denominator_name = "chargeFlipRate_denominator_electron_" + year;
    std::shared_ptr< TH2D > denominatorMap( new TH2D( denominator_name.c_str(), denominator_name.c_str(), ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) );
    denominatorMap->Sumw2();


	//loop over samples to fill histograms
    TreeReader treeReader( sampleListFile, sampleDirectory );

    for( unsigned i = 0; i < treeReader.numberOfSamples(); ++i ){
        treeReader.initSample();
    
        std::cout << treeReader.currentSample().uniqueName() << std::endl;
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry );

            //apply electron selection
            if( ! chargeFlips::passChargeFlipEventSelection( event, false, false, false) ) continue;

            for( auto& electronPtr : event.electronCollection() ){

                Electron& electron = *electronPtr;
            
                //require prompt leptons
                if( !( electron.isPrompt() ) ) continue;
                if( electron.matchPdgId() == 22 ) continue;

                //fill denominator histogram 
                histogram::fillValues( denominatorMap.get(), electron.pt(), electron.absEta(), 1. );
    
                //fill numerator histogram
                if( electron.isChargeFlip() ){
                    histogram::fillValues( numeratorMap.get(), electron.pt(), electron.absEta(), 1. );
                }
            }
        }
    }

    //divide numerator and denominator to get fake-rate
    numeratorMap->Divide( denominatorMap.get() );

    //create output directory if it does not exist 
    std::string outputDirectory = "chargeFlipMaps";
    systemTools::makeDirectory( outputDirectory );

    //plot fake-rate map
    //write numbers in exponential notation because charge flip rates tend to be very small
    gStyle->SetPaintTextFormat( "4.2e" );
    std::string plotOutputPath =  stringTools::formatDirectoryName( outputDirectory ) + "chargeFlipMap_MC_" + year + ".pdf";
    plot2DHistogram( numeratorMap.get(), plotOutputPath );

    //write fake-rate map to file 
    std::string rootOutputPath = stringTools::formatDirectoryName( outputDirectory ) + "chargeFlipMap_MC_" + year + ".root";
    TFile* outputFile = TFile::Open( rootOutputPath.c_str(), "RECREATE" );
    numeratorMap->Write();
    outputFile->Close();
}


int main(){

   	//make sure ROOT behaves itself when running multithreaded
    ROOT::EnableThreadSafety();

    //plotting style
    setTDRStyle();
    //determineMCChargeFlipRate( "2016", "sampleLists/samples_chargeFlips_MC_2016.txt", "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino_chargeflips/" );

    //measure electron charge flip rate for each year in multithreaded fashion
    std::vector< std::thread > threadVector;
    threadVector.reserve( 3 );
    for( const auto& year : {"2016", "2017", "2018" } ){
        threadVector.emplace_back( determineMCChargeFlipRate, year, std::string( "sampleLists/samples_chargeFlips_MC_" ) + year + ".txt", "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino_chargeflips/" );
    }

    for( auto& t : threadVector ){
        t.join();
    }
    
    return 0;
}
