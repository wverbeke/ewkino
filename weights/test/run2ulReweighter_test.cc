/*
Testing script for Run2ULReweighter
*/

#include "../interface/CombinedReweighter.h"
#include "../interface/ConcreteReweighterFactory.h"

//include c++ library classes
#include <iostream>
#include <memory>

//include ROOT classes
#include "TFile.h"
#include "TH2D.h"
#include "TROOT.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../test/copyMoveTest.h"


int main( int argc, char* argv[] ){

    int nargs = 3;
    if( argc != nargs+1 ){
        std::cerr << "### ERROR ###: run2ulReweighter_test.cc requires " << nargs;
        std::cerr << " arguments to run:" << std::endl;
        std::cerr << "- directory of input file(s)" << std::endl;
        std::cerr << "- name of input file (.root) OR samplelist (.txt)" << std::endl;
        std::cerr << "- number of events (use 0 for all events)" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& inputDirectory = argvStr[1];
    std::string& sampleList = argvStr[2];
    long unsigned nEvents = std::stoul(argvStr[3]);

    // read the input file
    TreeReader treeReader;
    std::vector<Sample> samples;
    bool modeSampleList = false;
    // case 1: single input file
    if( stringTools::stringEndsWith(sampleList, ".root") ){
        std::string inputFilePath = stringTools::formatDirectoryName(inputDirectory)+sampleList;
        treeReader.initSampleFromFile( inputFilePath );
        samples.push_back( treeReader.currentSample() );
    }
    // case 2: samplelist
    else if( stringTools::stringEndsWith(sampleList, ".txt") ){
        treeReader.readSamples( sampleList, inputDirectory );
        samples = treeReader.sampleVector();
        modeSampleList = true;
    }
    std::cout << "will use the following samples:" << std::endl;
    for( Sample sample: samples ) std::cout << "- " << sample.fileName() << std::endl;

    // initialize year from first sample
    // note: in this simple test no check is done to assure all samples in the list are of same year!
    if( modeSampleList ) treeReader.initSample();
    std::string year = treeReader.getYearString();

    // make the reweighter
    std::shared_ptr< ReweighterFactory >reweighterFactory( new Run2ULReweighterFactory() );
    // for testing the testing script:
    //std::shared_ptr< ReweighterFactory >reweighterFactory( new EmptyReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
	"../", year, treeReader.sampleVector() );

    // initialize some histograms
    HistInfo histInfo = HistInfo( "", "reweighting factor", 50, -0.1, 2.1 );
    std::shared_ptr<TH1D> nominalWeights = histInfo.makeHist( "nominalWeights" );

    // loop over samples
    unsigned numberOfSamples = samples.size();
    for( unsigned i = 0; i < numberOfSamples; ++i ){
        std::cout<<"start processing sample n. "<<i+1<<" of "<<numberOfSamples<<std::endl;
        if( modeSampleList ) treeReader.initSample( samples[i] );
	// find number of enries
	long unsigned numberOfEntries = treeReader.numberOfEntries();
        if( nEvents==0 ) nEvents = numberOfEntries;
        else nEvents = std::min(nEvents, numberOfEntries);
	// loop over entries
        std::cout << "starting event loop for " << nEvents << " events..." << std::endl;
        for( long unsigned entry = 0; entry < nEvents; ++entry ){
            if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	    
	    // build the event
            Event event = treeReader.buildEvent( entry );
	    
	    // do some selection (optional)

	    // determine the weight
	    double weight = reweighter.totalWeight( event );

	    // fill the histograms
	    nominalWeights->Fill( weight );
        }
    }

    // write histograms to output file
    std::string outputFileName = "output_run2ulReweighter_test.root";
    TFile* filePtr = TFile::Open( outputFileName.c_str(), "recreate" );
    nominalWeights->Write();
    filePtr->Close();
    return 0;
}
