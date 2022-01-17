// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TFile.h"
#include "TTree.h"
#include "TMVA/Reader.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"


bool checkReadability(const std::string& pathToFile){
    // temporary function to perform error handling when file cannot be read.
    // maybe to be replaced by exception throwing
    TreeReader treeReader;
    try{treeReader.initSampleFromFile( pathToFile );}
    catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return false;
    }
    return true;
}


void selectEvents(const std::string& pathToFile,
		    const std::string& outputFilePath,
		    unsigned long nEntries, 
		    const std::string& eventselection,
		    const std::string& selection_type,
		    const std::vector<int> signalcategories,
		    const std::vector<int> signalchannels,
		    const std::string& topcharge,
		    TMVA::Reader* reader,
		    const double bdtCut){

    // initialize TreeReader from input file
    std::cout << "initializing TreeReader...\n\n";
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";

    // initialize reweighter (needed for syntax)
    std::shared_ptr< ReweighterFactory> reweighterFactory;
    reweighterFactory = std::shared_ptr<ReweighterFactory>( new EmptyReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter(
                                        "../../weights/", year, thissample );

    // initialize output structure
    std::vector<std::tuple<long,long,long>> evtIds;

    // determine whether to consider event categorization
    bool doCat = true;
    if(signalcategories.size()==1 && signalcategories[0]==0) doCat = false;

    // determine whether to consider event channels separately
    bool doChannels = true;
    if(signalchannels.size()==1 && signalchannels[0]==4) doChannels = false;

    // determine whether to split top and antitop
    int topCharge = (topcharge=="all")?0:
		    (topcharge=="top")?1:-1;

    // do event loop
    if( nEntries==0 ) nEntries = treeReader.numberOfEntries();
    long unsigned numberOfEntries = std::min( nEntries, treeReader.numberOfEntries() );
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	
	// initialize map of variables
	std::map<std::string,double> varmap = eventFlattening::initVarMap();

	// do all selections
        Event event = treeReader.buildEvent(entry,false,false,false,false);
        if(!passES(event, eventselection, selection_type, "nominal")) continue;
	if(doChannels){
	    int eventchannel = event.leptonCollection().numberOfMuons();
	    if(std::find(signalchannels.begin(),signalchannels.end(),eventchannel)
                == signalchannels.end()){ continue; } 
	}
	if( topCharge*lWCharge(event) < -0.5 ){ continue; }
	if(doCat){
	    int eventcategory = eventCategory(event, "nominal");
	    if(std::find(signalcategories.begin(),signalcategories.end(),eventcategory) 
		== signalcategories.end()){ continue; }
	}
	varmap = eventFlattening::eventToEntry(event, 1., reweighter, selection_type, 
						nullptr, nullptr, "nominal", true, reader);
	if( bdtCut > -1 ){
	    if( varmap["_eventBDT"]<bdtCut ) continue;
	}

	// add event to list
	std::tuple<long,long,long> evtId = std::make_tuple<long,long,long>(
						(long) varmap["_runNb"],
						(long) varmap["_lumiBlock"],
						(long) varmap["_eventNb"] );
	evtIds.push_back(evtId);
    }

    // make output file
    std::ofstream outputFile;
    outputFile.open(outputFilePath);
    for( std::tuple<long,long,long> evtId: evtIds ){
	outputFile << std::get<0>(evtId) << ",";
	outputFile << std::get<1>(evtId) << ",";
	outputFile << std::get<2>(evtId) << std::endl;
    }
    outputFile.close();
}


int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    int nargs = 10;
    if( argc != nargs+1 ){
        std::cerr << "ERROR: selectEvents.cc requires " << nargs << " arguments to run: ";
        std::cerr << "input_file_path, output_file_path, nentries, ";
	std::cerr << "event_selection, selection_type, signal_categories, "; 
	std::cerr << "signal_channels, topcharge, ";
	std::cerr << "pathToXMLFile, bdtCut, " << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1]; // path to input file to be processed
    std::string& output_file_path = argvStr[2]; // output file to put the result in
    long unsigned nentries = std::stol(argvStr[3]); // number of entries to process (0 for all)
    std::string& event_selection = argvStr[4]; // event selection to perform (see event selection code)
    std::string& selection_type = argvStr[5]; // type of event selection (see event selection code)
    std::string& signal_catstring = argvStr[6]; // category/categrories to consider in selection
    // convert string to vector of ints
    std::vector< int > signal_categories;
    for(char c: signal_catstring){ signal_categories.push_back( (int)c-(int)'0' ); }
    std::string& signal_chstring = argvStr[7]; // lepton channel(s) to consider in selection
    // convert string to vector of ints
    std::vector< int > signal_channels;
    for(char c: signal_chstring){ signal_channels.push_back( (int)c-(int)'0' ); }
    std::string& topcharge = argvStr[8]; // top charge to consider in selection
    if( topcharge!="all" && topcharge!="top" && topcharge!="antitop" ){
	std::cerr << "top charge '" << topcharge << "' not recognized" << std::endl;
	return -1;
    }
    std::string& pathToXMLFile = argvStr[9]; // ignored if _eventBDT is not in list of variables
    double bdtCut = std::stof(argvStr[10]); // put very small, e.g. -99, if not needed

    // printouts useful for debugging:
    std::cout << "running ./runsystematics with following settings:" << std::endl;
    std::cout << "input_file_path: " << input_file_path << std::endl;
    std::cout << "output_file_path: " << output_file_path << std::endl;
    std::cout << "nentries: " << nentries << std::endl;
    std::cout << "event_selection: " << event_selection << std::endl;
    std::cout << "selection_type: " << selection_type << std::endl;
    std::cout << "signal_catstring: " << signal_catstring << std::endl;
    std::cout << "signal_chstring: " << signal_chstring << std::endl;
    std::cout << "topcharge:" << topcharge << std::endl;
    std::cout << "path_to_xml_file: " << pathToXMLFile << std::endl;
    std::cout << "bdtcut: "<< bdtCut << std::endl;

    // load a TMVA Reader if needed
    TMVA::Reader* reader = new TMVA::Reader();
    std::string subDir = "bdts_"+LeptonSelector::leptonID();
    std::string::size_type pos = pathToXMLFile.find("/bdt/");
    if( pos != std::string::npos ){
	pathToXMLFile.erase( pos, 5 );
	pathToXMLFile.insert( pos, "/bdt/"+subDir+"/" );
    }
    reader = eventFlattening::initializeReader(reader, pathToXMLFile, "all");
    std::cout << "read BDT reader" << std::endl;

    // check validity
    bool validInput = checkReadability( input_file_path );
    if(!validInput) return -1;

    // fill the histograms
    selectEvents(input_file_path, output_file_path, nentries,
		event_selection, selection_type, signal_categories,
		signal_channels, topcharge, reader, bdtCut);

    delete reader;
    std::cerr << "###done###" << std::endl;
    return 0;
}
