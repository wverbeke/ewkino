//include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

//include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"


std::vector<std::string> remove( std::vector<std::string> origList,
				    std::vector<std::string> toRemove ){
    std::vector<std::string> newList;
    for( std::string el: origList ){
	if( std::find(toRemove.begin(), toRemove.end(), el) == toRemove.end() ){
	    newList.push_back(el);
	}
    }
    return newList;
}

std::vector<std::string> getSourcesToRemove(){
    // see https://twiki.cern.ch/twiki/bin/viewauth/CMS/JECUncertaintySources
    std::vector< std::string > res = { "FlavorZJet", "FlavorPhotonJet", "FlavorPureGluon", "FlavorPureQuark", "FlavorPureCharm", "FlavorPureBottom", "PileUpDataMC", "PileUpPtRef", "PileUpPtBB", "PileUpPtEC1", "PileUpPtEC2", "PileUpPtHF", "SubTotalPileUp", "SubTotalRelative", "SubTotalPt", "SubTotalScale", "SubTotalAbsolute", "SubTotalMC", "TotalNoFlavor", "TotalNoTime" ,"TotalNoFlavorNoTime", "Total" };
    return res;
}

std::vector<std::string> getGroupedToRemove(){
    std::vector< std::string > res = { "Total" };
    return res;
}


void testSplitJec( const std::string& pathToFile, const std::string& outputDirectory,
		    const std::string& outputFileName ){

    // initialize TreeReader, input files might be corrupt in rare cases
    TreeReader treeReader;
    try{
        treeReader.initSampleFromFile( pathToFile );
    } catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return;
    }

    // format output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName(outputDirectory)+outputFileName;
    //TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );

    // read first event to access available variations
    if(treeReader.numberOfEntries()==0){
	std::cerr << "WARNING: no events in treeReader... returning." << std::endl;
	return;
    }
    std::cout << "here" << std::endl;
    Event event = treeReader.buildEvent( 0, false, false, false, true);
    std::cout << "here too" << std::endl;
    std::vector<std::string> jecSources = event.jetInfo().allJECVariations();
    jecSources = remove( jecSources, getSourcesToRemove() );
    std::vector<std::string> jecGrouped = event.jetInfo().groupedJECVariations();
    jecGrouped = remove( jecGrouped, getGroupedToRemove() );
    std::cout << "available JEC sources: " << std::endl;
    for( std::string s: jecSources ) std::cout << "  " << s << std::endl;
    std::cout << "available grouped JEC sources: " << std::endl;
    for( std::string s: jecGrouped ) std::cout << "  " << s << std::endl;

    // loop over events
    //long unsigned nentries = treeReader.numberOfEntries();
    long unsigned nentries = 10;
    for( long unsigned entry = 0; entry < nentries; ++entry ){

	std::cout << "----- " << entry << " -----" << std::endl;	

        //build event
        Event event = treeReader.buildEvent( entry, false, false, true, true );

	// print total JEC uncertainty on MET
	std::cout << "MET:" << std::endl;
	std::cout << "- nominal: " << event.met().pt() << std::endl;
	std::cout << "- original variation: " << event.met().MetJECUp() << "/" << event.met().MetJECDown() << std::endl;
	std::cout << "- source total: " << event.met().MetJECUp( "Total" ) << "/" << event.met().MetJECDown( "Total" ) << std::endl;

    }

    //close output file
    //outputFilePtr->Close();
}


int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;
    if( argc != 4 ){
        std::cerr << "skimmer requires exactly three arguments to run :"; 
	std::cerr << "input_file_path, output_directory, output_file_name" << std::endl;
        return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    std::string& input_file_path = argvStr[1];
    std::string& output_directory = argvStr[2];
    std::string& output_file_name = argvStr[3];
    testSplitJec( input_file_path, output_directory, output_file_name );
    std::cerr << "###done###" << std::endl;
    return 0;
}
