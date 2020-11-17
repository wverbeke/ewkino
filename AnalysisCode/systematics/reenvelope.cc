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

#include "systematicsTools.h"

int main( int argc, char* argv[] ){

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc != 7 ){
	std::cerr << "### ERROR ###: reenvelope.cc requires 6 arguments to run: ";
	std::cerr << "input_file_path, output_file_path, prefix, ";
	std::cerr << "tag, envelopeName, type" << std::endl;
	return -1;
    }
    
    // command line args:
    // - input file path: file to run on
    // - output file path: path to where the (reenveloped) output should be placed
    // - prefix: the part of the histogram name containing <process>_<variable>
    //           (histograms are assumed to be named <process>_<variable>_<systematic>)
    //		 (it can also be just <variable> if only one <process> is in the file)
    // - tag: tag to select the histograms (e.g. pdfShapeVar)
    // - envelopeName: name of the histogram to store the result in
    //                 (e.g. pdfShapeEnv; will be appended by 'Up' and 'Down')
    //		       (the prefix will also be appended before envelopeName)
    // - type: either 'envelope' or 'RMS'

    // parse arguments
    std::string input_file_path = argvStr[1];
    std::string output_file_path = argvStr[2];
    std::string prefix = argvStr[3];
    std::string tag = argvStr[4];
    std::string envelopeName = argvStr[5];
    std::string type = argvStr[6];

    // check type
    if( type!="envelope" && type!="RMS"){
	std::cerr << "### ERROR ###: type '" << type <<"' not recognized." << std::endl; 
	return -1;
    }

    // the input and background files are supposed to contain histograms with names of the form
    // <processTag>_<variableName>_<systematic>
    std::vector<std::string> mayNotContain = {};
    std::vector<std::string> mustContain = {prefix,tag};

    // get and select histograms from the input file
    std::vector<std::shared_ptr<TH1>> inHistogramVector = getHistogramsFromFile(input_file_path);
    std::vector<std::shared_ptr<TH1>> selHistogramVector = selectHistograms(
        inHistogramVector, mustContain, mayNotContain );
    std::vector<std::shared_ptr<TH1>> nominalHistVector = selectHistograms(
	inHistogramVector, std::vector<std::string>({prefix,"_nominal"}), 
	std::vector<std::string>({}) );
    if( nominalHistVector.size() != 1){
	std::cerr << "### ERROR ###: found none or more than one nominal histogram";
	std::cerr << " corresponding to prefix '" << prefix << "'." << std::endl;
	return -1;
    }
    std::shared_ptr<TH1> nominalHist = nominalHistVector[0];
    // redefine prefix based on name of nominal hist
    prefix = stringTools::split(nominalHist->GetName(),"_nominal")[0];
    
    // calculate envelope or RMS
    std::map< std::string, std::shared_ptr<TH1> > envRes;
    if(type=="envelope"){
	envRes = getEnvelope(selHistogramVector,nominalHist);
    } else if(type=="RMS"){
	envRes = getRMS(selHistogramVector,nominalHist);
    }
    std::shared_ptr<TH1> upHist = envRes["up"];
    upHist->SetName( (prefix+"_"+envelopeName+"Up").c_str() );
    std::shared_ptr<TH1> downHist = envRes["down"];
    downHist->SetName( (prefix+"_"+envelopeName+"Down").c_str() );

    // open output file and write histograms to it
    TFile* f = TFile::Open(output_file_path.c_str(),"recreate");
    for(std::shared_ptr<TH1> histPtr : inHistogramVector){
	// skip previous envelopes if present
	if(stringTools::stringContains(histPtr->GetName(),prefix+"_"+envelopeName)) continue;
	// write all the other histograms
	histPtr->Write();
    }
    upHist->Write();
    downHist->Write();
    f->Close();
    return 0;
}
