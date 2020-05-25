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
#include "TMVA/Reader.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
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

void eventloopEF_SR(const std::string& pathToFile, const double norm, 
		    const std::string& outputDirectory,
		    const std::string& outputFileName,
		    const std::string& variation,
		    const bool isdataforbackground = false,
		    const std::shared_ptr< TH2D>& frMap_muon = nullptr,
                    const std::shared_ptr< TH2D>& frMap_electron = nullptr,
		    const bool doMVA = false,
		    const std::string& pathToXMLFile = ""){

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";

    // make output ROOT file
    std::string outputdir = "blackJackAndHookers";
    std::string treecat1 = "treeCat1";
    std::string treecat2 = "treeCat2";
    std::string treecat3 = "treeCat3";
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    //outputFilePath += stringTools::removeOccurencesOf( pathToFile, "/" );
    outputFilePath += outputFileName;
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

    // initialize TMVA reader
    TMVA::Reader* reader = new TMVA::Reader();
    if(doMVA){ reader = initializeReader(reader, pathToXMLFile); }

    // make reweighter
    std::shared_ptr< ReweighterFactory >reweighterFactory( new tZqReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
    					"../../weights/", year, thissample );

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 2000;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	// build event and perform event selection
        Event event = treeReader.buildEvent(entry,true,true);
        if(!passES(event, "signalregion", isdataforbackground)) continue;
        int eventcategory = eventCategory(event, variation);
        if(eventcategory == -1) continue;
	// set all high-level variables and make BDT output
        eventToEntry(event, norm, isdataforbackground, frMap_muon, frMap_electron, variation);
	if(doMVA){ _eventBDT = reader->EvaluateMVA( "BDT" ); }
	// determine reweighting 
        if(event.isMC()){
            _normweight *= reweighter.totalWeight(event);
        }
        if(eventcategory == 1) treeCat1Ptr->Fill();
        else if(eventcategory == 2) treeCat2Ptr->Fill();
        else if(eventcategory == 3) treeCat3Ptr->Fill();
    }
    outputFilePtr->cd( outputdir.c_str() );
    treeCat1Ptr->Write("", BIT(2) );
    treeCat2Ptr->Write("", BIT(2));
    treeCat3Ptr->Write("", BIT(2));
    outputFilePtr->Close();

    // delete TMVA reader
    delete reader;
}

void eventloopEF_CR(const std::string& pathToFile, const double norm,
		    const std::string& outputDirectory,
		    const std::string& outputFileName,
                    const std::string& eventselection,
		    const std::string& variation,
		    const bool isdataforbackground = false,
		    const std::shared_ptr< TH2D>& frMap_muon = nullptr,
                    const std::shared_ptr< TH2D>& frMap_electron = nullptr,
		    const bool doMVA = false,
		    const std::string& pathToXMLFile = ""){

    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";
    std::string outputdir = "blackJackAndHookers";
    std::string treecat1 = "blackJackAndHookersTree";
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    //outputFilePath += stringTools::removeOccurencesOf( pathToFile, "/" );
    outputFilePath += outputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->mkdir( outputdir.c_str() );
    outputFilePtr->cd( outputdir.c_str() );
    std::vector< std::shared_ptr< TH1 > > histVector = treeReader.getHistogramsFromCurrentFile();
    for( const auto& histPtr : histVector ){
        histPtr->Write();
    }
    std::shared_ptr< TTree > treeCat1Ptr( std::make_shared< TTree >( treecat1.c_str(), treecat1.c_str() ) );
    initOutputTree(treeCat1Ptr.get());

    // initialize TMVA reader
    TMVA::Reader* reader = new TMVA::Reader();
    if(doMVA){ reader = initializeReader(reader, pathToXMLFile); }

    // make reweighter
    std::shared_ptr< ReweighterFactory >reweighterFactory( new tZqReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
                                      "../../weights/", year, thissample );
    
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //unsigned numberOfEntries = 5000;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
        Event event = treeReader.buildEvent(entry,true,true);
        if(!passES(event, eventselection, isdataforbackground)) continue;
        eventToEntry(event, norm, isdataforbackground, frMap_muon, frMap_electron, variation);
	if(doMVA){ _eventBDT = reader->EvaluateMVA( "BDT" ); }
	// determine reweighting 
        if(event.isMC()){
            _normweight *= reweighter.totalWeight(event);
        }
        treeCat1Ptr->Fill();
    }
    outputFilePtr->cd( outputdir.c_str() );
    treeCat1Ptr->Write("", BIT(2) );
    outputFilePtr->Close();

    // delete TMVA reader
    delete reader;
} 

int main( int argc, char* argv[] ){
    if( argc != 12  ){
        std::cerr << "### ERROR ###: event flattening requires 11 arguments to run: ";
        std::cerr << "input_file_path, norm, output_directory, output_file_name";
	std::cerr << "event_selection, variation, isdataforbackground, ";
	std::cerr << "pathToMuonFakeRateMap, pathToElectronFakeRateMap";
	std::cerr << "doMVA, pathToXMLFile" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    // necessary arguments:
    std::string& input_file_path = argvStr[1];
    double norm = std::stod(argvStr[2]);
    std::string& output_directory = argvStr[3];
    std::string& output_file_name = argvStr[4];
    std::string& event_selection = argvStr[5];
    bool validInput = checkReadability( input_file_path );
    if(!validInput){return -1;}
    // type of variation to consider:
    std::string& variation = argvStr[6];
    // first set of other arguments: 
    std::shared_ptr< TH2D > frMap_muon;
    std::shared_ptr< TH2D > frMap_electron;
    std::string year = "2016";
    if(stringTools::stringContains(input_file_path,"2017")) year = "2017";
    else if(stringTools::stringContains(input_file_path,"2018")) year = "2018";
    bool isdataforbackground = (argvStr[7]=="True" || argvStr[7]=="true");
    if(isdataforbackground){
	frMap_muon = readFRMap(argvStr[8],"muon",year);
	frMap_electron = readFRMap(argvStr[9],"electron",year);
    }
    // second set of other arguments:
    bool doMVA = (argvStr[10]=="True" || argvStr[10]=="true");
    std::string pathToXMLFile = "";
    if(doMVA){ pathToXMLFile = argvStr[11]; }

    // call functions
    std::string sigreg = "signalregion";
    if(event_selection == sigreg) eventloopEF_SR(input_file_path,norm,output_directory,
				    output_file_name,variation,
				    isdataforbackground,frMap_muon,frMap_electron,
				    doMVA, pathToXMLFile);
    else eventloopEF_CR(input_file_path, norm, output_directory, 
				    output_file_name,event_selection,variation,
				    isdataforbackground,frMap_muon,frMap_electron,
				    doMVA, pathToXMLFile);
    std::cout<<"done"<<std::endl;
    return 0;
}
