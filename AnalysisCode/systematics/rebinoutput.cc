// This is the main C++ class used to run the systematics.
// It is supposed to run on the output file of an event selection procedure
// and produce a root file containing a histogram of systematic up and down variations.

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes
#include "TROOT.h" 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/QuantileBinner.h"

std::vector< std::shared_ptr< TH1 > > getHistogramsFromFile( const std::string& fileName){
    // get a list of all histograms stored in a root file, assuming top-level directory.
    // based on ewkino/skimmer/skimmer.cc
 
    std::vector< std::shared_ptr< TH1 > > histogramVector;

    std::cout << "opening file " << fileName << std::endl;
    TFile* f = TFile::Open(fileName.c_str());
    TList* keyList = f->GetListOfKeys();

    for( const auto objectPtr : *keyList ){
	TH1* histPtr = dynamic_cast< TH1* >( f->Get( objectPtr->GetName() ) );
	if( histPtr ){
            histPtr->SetDirectory( gROOT );
	    histogramVector.emplace_back( histPtr );
	}
    }
    
    f->Close();
    return histogramVector;
}

std::vector< std::shared_ptr< TH1 > > selectHistograms( 
		std::vector<std::shared_ptr<TH1>> histogramVector,
		std::vector<std::string> mustContain,
                std::vector<std::string> mayNotContain ){
    
    std::vector< std::shared_ptr<TH1> > selHists;
    for( std::shared_ptr<TH1> histPtr : histogramVector ){
        // first check if histogram name contains necessary tags
        bool keep = true;
        for( std::string mustContainTag : mustContain ){
            if(!stringTools::stringContains(histPtr->GetName(),mustContainTag)){
                keep = false;
                break;
            }
        }
        if(!keep) continue;
        // then check if histogram does not contain forbidden tags
        for( std::string mayNotContainTag : mayNotContain ){
            if(stringTools::stringContains(histPtr->GetName(),mayNotContainTag)){
                keep = false;
                break;
            }
        }
        if(!keep) continue;
	selHists.push_back( std::shared_ptr<TH1>( (TH1*) histPtr->Clone() ) );
    }
    return selHists;
}

std::shared_ptr< TH1 > addHistograms( std::vector<std::shared_ptr<TH1>> histogramVector ){
    
    int nHists=0;
    std::shared_ptr<TH1> sumHist;
    for( std::shared_ptr<TH1> histPtr : histogramVector ){
	std::cout << "adding histogram " << histPtr->GetName() << std::endl;
	// if it is the first one to be found, simply copy it
	if(nHists==0){
	    sumHist = std::shared_ptr<TH1>( (TH1*) histPtr->Clone() );
	    sumHist->SetName("sum");
	    sumHist->SetTitle("sum");
	    ++nHists;
	    continue;
	}
	// else add it
	sumHist.get()->Add(histPtr.get());
	++nHists;
    }
    std::cout << "added " << nHists << " histograms" << std::endl;
    return sumHist;
}

int main( int argc, char* argv[] ){

    if( argc != 6 ){
        std::cerr << "### ERROR ###: rebinoutput.cc requires 5 arguments to run: ";
        std::cerr << "input_file_path, output_file_path, variable_name, eventsInLastBin, ";
	std::cerr << "binRatio" << std::endl;;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string input_file_path = argvStr[1];
    std::string output_file_path = argvStr[2];
    std::string variable_name = argvStr[3];
    double eventsInLastBin = std::stod(argvStr[4]);
    double binRatio = std::stod(argvStr[5]);

    // the input file is supposed to contain histograms with names of the form
    // <processTag>_<variableName>_<systematic>
    std::vector<std::string> mayNotContain = {"tZq","data"};
    std::vector<std::string> mustContain = {variable_name, "nominal"};

    // get all histograms
    std::vector<std::shared_ptr<TH1>> histogramVector = getHistogramsFromFile(input_file_path);
    // exclude previously rebinned histograms
    histogramVector = selectHistograms( histogramVector, std::vector<std::string>({}), 
					std::vector<std::string>({"rebinned"}) );
    // print for testing
    //for(std::shared_ptr<TH1> histPtr : histogramVector){
    //	std::cout << histPtr->GetName() << std::endl;
    //}
    // select the variable histogramms from background
    std::vector<std::shared_ptr<TH1>> selectedHistogramVector = selectHistograms(
	histogramVector, mustContain, mayNotContain );
    // add selected histograms
    std::shared_ptr<TH1> bckHist = addHistograms( selectedHistogramVector );
    
    // determine new bins using QuantileBinner
    std::vector<double> contents = exponentialBinningQuantiles( bckHist.get(), 
	eventsInLastBin=eventsInLastBin, binRatio=binRatio );
    QuantileBinner rebinner( bckHist.get(), contents );
    
    // print for testing
    //std::cout << "new content fractions: " << std::endl;
    //for(double content : contents){
    //	std::cout << " " << content;
    //}
    //std::cout << std::endl;

    // make rebinned histograms belonging to this variable
    std::vector<std::shared_ptr<TH1>> rebinnedHistogramVector;
    for( std::shared_ptr<TH1> histPtr: histogramVector ){
	if(!stringTools::stringContains(histPtr->GetName(),variable_name)) continue;
	std::shared_ptr<TH1> rebinnedHistogram = rebinner.rebinnedHistogram( histPtr.get() );
	// set new histogram name
	// note: make sure newvarname does not contain variable_name else replace will loop forever!
	std::string tempvarname = stringTools::removeOccurencesOf( variable_name, "fineBinned" );
	std::string newvarname = "_rebinned"+tempvarname.substr(1,std::string::npos);
	std::string name = stringTools::replace( std::string(histPtr->GetName()), 
						    variable_name, newvarname);
	rebinnedHistogram->SetName( name.c_str() );
	rebinnedHistogram->SetTitle( histPtr->GetTitle() );
	rebinnedHistogramVector.push_back( rebinnedHistogram );
    } 

    // open output file and write histograms to it
    TFile* f = TFile::Open(output_file_path.c_str(),"recreate");
    for(std::shared_ptr<TH1> histPtr : histogramVector){
	histPtr->Write();
    }
    for(std::shared_ptr<TH1> histPtr : rebinnedHistogramVector){
	histPtr->Write();
    }
    f->Close();
    return 0;
}
