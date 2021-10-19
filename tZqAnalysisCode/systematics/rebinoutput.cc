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

#include "systematicsTools.h"

int main( int argc, char* argv[] ){

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc != 8 ){
	std::cerr << "### ERROR ###: rebinoutput.cc requires 7 arguments to run: ";
	std::cerr << "background_file_path, input_file_path, output_file_path, variable_name,";
	std::cerr << " mode, lastBinCount, binRatio" << std::endl;;
	return -1;
    }
    
    // command line args:
    // - input file path: file to run on
    // - output file path: path to where the (rebinned) output should be placed
    // - variable_name: name of the variable to rebin
    // - mode: either "signal" or "background" (on which the rebinning will be based)
    // - eventsInLastBin and binRatio: rebinning parameters
    // - background_file_path: path containing all the backgrounds (needed for rebinning)
    //			       it can be the same as input_file_path if that file is 
    //			       already merged and contains all the backgrounds

    // parse arguments
    std::string bkg_file_path = argvStr[1];
    std::string input_file_path = argvStr[2];
    std::string output_file_path = argvStr[3];
    std::string variable_name = argvStr[4];
    std::string mode = argvStr[5];
    std::string lastBinCount = argvStr[6];
    double eventsInLastBin;
    bool useFraction = false;
    if(lastBinCount[lastBinCount.length()-1]!='f'){
	eventsInLastBin = std::stod(lastBinCount);
    }
    else{
	useFraction = true;
	eventsInLastBin = std::stod(lastBinCount.substr(0,lastBinCount.length()-1));
    }
    double binRatio = std::stod(argvStr[7]);

    // the input and background files are supposed to contain histograms with names of the form
    // <processTag>_<variableName>_<systematic>
    std::vector<std::string> mayNotContain;
    std::vector<std::string> mustContain;
    if(mode=="background"){
	mayNotContain = {"tZq","data"};
	mustContain = {variable_name, "nominal"};
    }
    else if(mode=="signal"){
	mayNotContain = {};
	mustContain = {variable_name,"nominal","tZq"};	
    }
    else{
	std::cerr << "### ERROR ###: mode '" << mode << "' not recognized." << std::endl;
	return -1;
    }

    // get all histograms from the background file
    std::vector<std::shared_ptr<TH1>> bkgHistogramVector = getHistogramsFromFile(bkg_file_path);
    // select the variable histograms from background
    bkgHistogramVector = selectHistograms(
        bkgHistogramVector, mustContain, mayNotContain );
    // add selected histograms
    std::shared_ptr<TH1> bkgHist = addHistograms( bkgHistogramVector );
    // recalculate events in last bin if fraction was speciied
    if( useFraction ){
	eventsInLastBin = eventsInLastBin*bkgHist->Integral();
    }
    
    // determine new bins using QuantileBinner
    std::vector<double> contents = exponentialBinningQuantiles( bkgHist.get(), 
	eventsInLastBin=eventsInLastBin, binRatio=binRatio );
    QuantileBinner rebinner( bkgHist.get(), contents );
    
    // print for testing
    //std::cout << "new content fractions: " << std::endl;
    //for(double content : contents){
    //	std::cout << " " << content;
    //}
    //std::cout << std::endl;

    // get all histograms from the input file
    std::vector<std::shared_ptr<TH1>> inHistogramVector = getHistogramsFromFile(input_file_path);
    // exclude previously rebinned histograms
    inHistogramVector = selectHistograms( inHistogramVector, std::vector<std::string>({}), 
                                      std::vector<std::string>({"rebinned"}) );
    // make rebinned histograms belonging to this variable
    std::vector<std::shared_ptr<TH1>> rebinnedHistogramVector;
    for( std::shared_ptr<TH1> histPtr: inHistogramVector ){
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
    for(std::shared_ptr<TH1> histPtr : inHistogramVector){
	histPtr->Write();
    }
    for(std::shared_ptr<TH1> histPtr : rebinnedHistogramVector){
	histPtr->Write();
    }
    f->Close();
    return 0;
}
