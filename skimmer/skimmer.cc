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
#include "../TreeReader/interface/TreeReader.h"
#include "../Tools/interface/stringTools.h"
#include "../Event/interface/Event.h"
#include "interface/skimSelections.h"


void skimFile( const std::string& pathToFile, const std::string& outputDirectory, const std::string& skimCondition ){

    std::cout << "skimming " << pathToFile << std::endl;

    //check which year the file belongs to 
    bool is2017 = stringTools::stringContains( pathToFile, "MiniAOD2017" );
    bool is2018 = stringTools::stringContains( pathToFile, "MiniAOD2018" );
    if( is2017 && is2018 ){
        throw std::logic_error( "is2017 and is2018 are both true, which is not allowed." );
    }

    //initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile, is2017, is2018 );

    //input ROOT file 
    std::shared_ptr< TFile > inputFilePtr = treeReader.getCurrentFilePtr();

    //read histograms from input tree
    std::shared_ptr< TH1D > nVertices( (TH1D*) inputFilePtr->Get( "blackJackAndHookers/nVertices" ) );
    std::shared_ptr< TH1D > hCounter;
    std::shared_ptr< TH1D > lheCounter;
    std::shared_ptr< TH1D > nTrueInteractions;
    std::shared_ptr< TH1D> psCounter;
    
    if( treeReader.isMC() ){
        hCounter = std::shared_ptr< TH1D >( (TH1D*) inputFilePtr->Get( "blackJackAndHookers/hCounter" ) );
        lheCounter = std::shared_ptr< TH1D >( (TH1D*) inputFilePtr->Get( "blackJackAndHookers/lheCounter" ) );
        nTrueInteractions = std::shared_ptr< TH1D >( (TH1D*) inputFilePtr->Get( "blackJackAndHookers/nTrueInteractions" ) );
        psCounter = std::shared_ptr< TH1D >( (TH1D*) inputFilePtr->Get( "blackJackAndHookers/psCounter" ) );
    }

    //make output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory ) + stringTools::removeOccurencesOf( pathToFile, "/" );
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->mkdir( "blackJackAndHookers" );
    outputFilePtr->cd( "blackJackAndHookers" );

    //write histograms to new tree
    nVertices->Write( "nVertices" );
    if( treeReader.isMC() ){
        hCounter->Write( "hCounter" );
        lheCounter->Write( "lheCounter" );
        nTrueInteractions->Write( "nTrueInteractions" );
        psCounter->Write( "psCounter" );
    }

    //make output tree
    std::shared_ptr< TTree > outputTreePtr( std::make_shared< TTree >( "blackJackAndHookersTree","blackJackAndHookersTree" ) );
    treeReader.setOutputTree( outputTreePtr.get() );

    for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){

        //build event
        Event event = treeReader.buildEvent( entry, true, true );

        //apply event selection
        if( !passSkim( event, skimCondition ) ) continue;

        //fill new tree
        outputTreePtr->Fill();
    }

    //write new tree
    outputTreePtr->Write( "",  BIT(2) );

    //close output file
    outputFilePtr->Close();
}


int main( int argc, char* argv[] ){
    if( argc != 4 ){
        std::cerr << "skimmer requires exactly three arguments to run : input_file_path, output_directory, skim_condition" << std::endl;
        return -1;
    }

    std::vector< std::string > argvStr;
    for( int i = 1; i < argc; ++i ){
        argvStr.push_back( argv[i] );
    }

    std::string& input_file_path = argvStr[0];
    std::string& output_file_path = argvStr[1];
    std::string& skimCondition = argvStr[2];
    skimFile( input_file_path, output_file_path, skimCondition );

    return 0;
}
