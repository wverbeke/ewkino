// This is the main C++ class used to perform event flattening.
// It is supposed to run on the output file of an event selection procedure
// and produce a flat tree containing relevant variables.

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"
#include "interface/eventSelections.h"
#include "interface/eventFlattening.h"

bool checkReadability(const std::string& pathToFile){
    // temporary function to perform error handling when file cannot be read.
    // to be replaced by exception throwing in eventloopEF once figured out how this works in c++.
    TreeReader treeReader;
    try{treeReader.initSampleFromFile( pathToFile );}
    catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return false;
    }
    return true;
}

void eventloopEF_SR(const std::string& pathToFile, double norm, const std::string& outputDirectory){
    // loop over events and write flat trees for signal region categories

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );

    // make output ROOT file
    std::string outputdir = "blackJackAndHookers";
    std::string treecat1 = "treeCat1";
    std::string treecat2 = "treeCat2";
    std::string treecat3 = "treeCat3";
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += stringTools::removeOccurencesOf( pathToFile, "/" );
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->mkdir( outputdir.c_str() );
    outputFilePtr->cd( outputdir.c_str() );

    // read histograms from input file and write them to the new file
    std::vector< std::shared_ptr< TH1 > > histVector = treeReader.getHistogramsFromCurrentFile();
    for( const auto& histPtr : histVector ){
        histPtr->Write();
    }

    // make output tree
    std::shared_ptr< TTree > treeCat1Ptr( std::make_shared< TTree >( treecat1.c_str(), treecat1.c_str() ) );
    initOutputTree(treeCat1Ptr.get());
    std::shared_ptr< TTree > treeCat2Ptr( std::make_shared< TTree >( treecat2.c_str(), treecat2.c_str() ) );
    initOutputTree(treeCat2Ptr.get());
    std::shared_ptr< TTree > treeCat3Ptr( std::make_shared< TTree >( treecat3.c_str(), treecat3.c_str() ) );
    initOutputTree(treeCat3Ptr.get());

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 5000;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
        Event event = treeReader.buildEvent(entry,true,true);
        if(!passES(event,"signalregion")) continue;
        int eventcategory = eventCategory(event);
        if(eventcategory == -1) continue;
        eventToEntry(event,norm);
        if(eventcategory == 1) treeCat1Ptr->Fill();
        else if(eventcategory == 2) treeCat2Ptr->Fill();
        else if(eventcategory == 3) treeCat3Ptr->Fill();
    }
    treeCat1Ptr->Write("", BIT(2) );
    treeCat2Ptr->Write("", BIT(2));
    treeCat3Ptr->Write("", BIT(2));
    outputFilePtr->Close();
}

void eventloopEF_CR(const std::string& pathToFile, double norm,
		    const std::string& outputDirectory,
                    const std::string& eventselection){

    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string outputdir = "blackJackAndHookers";
    std::string treecat1 = "blackJackAndHookersTree";
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += stringTools::removeOccurencesOf( pathToFile, "/" );
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->mkdir( outputdir.c_str() );
    outputFilePtr->cd( outputdir.c_str() );
    std::vector< std::shared_ptr< TH1 > > histVector = treeReader.getHistogramsFromCurrentFile();
    for( const auto& histPtr : histVector ){
        histPtr->Write();
    }
    std::shared_ptr< TTree > treeCat1Ptr( std::make_shared< TTree >( treecat1.c_str(), treecat1.c_str() ) );
    initOutputTree(treeCat1Ptr.get());
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //unsigned numberOfEntries = 5000;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
        Event event = treeReader.buildEvent(entry,true,true);
        if(!passES(event,eventselection)) continue;
        eventToEntry(event,norm);
        treeCat1Ptr->Fill();
    }
    treeCat1Ptr->Write("", BIT(2) );
    outputFilePtr->Close();
} 

int main( int argc, char* argv[] ){
    if( argc != 5 ){
        std::cerr << "### ERROR ###: event flattening requires exactly four arguments to run: ";
        std::cerr << "               input_file_path, norm, output_directory, event_selection" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    double norm = std::stod(argvStr[2]);
    std::string& output_directory = argvStr[3];
    std::string& event_selection = argvStr[4];
    bool validInput = checkReadability( input_file_path );
    if(!validInput){return -1;}
    std::string sigreg = "signalregion";
    if(event_selection == sigreg) eventloopEF_SR(input_file_path,norm,output_directory);
    else eventloopEF_CR(input_file_path, norm, output_directory, event_selection);
    std::cout<<"done"<<std::endl;
    return 0;
}
