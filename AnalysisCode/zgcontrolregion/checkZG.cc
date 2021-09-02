//include c++ library classes 
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
#include "../../Tools/interface/systemTools.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"

double getVarValue(const std::string varname, std::map<std::string,double> varmap){
    // retrieve a value from a map of all variables associated to the event

    // remove "fineBinned" from the varname to allow for e.g. fineBinnedeventBDT to be used
    std::string modvarname = stringTools::removeOccurencesOf(varname,"fineBinned");
    modvarname = stringTools::removeOccurencesOf(modvarname,"coarseBinned");
    modvarname = stringTools::removeOccurencesOf(modvarname,"smallRange");
    // find this variable in the list of available variables
    std::map<std::string,double>::iterator it = varmap.find(modvarname);
    if(it == varmap.end()) return -9999;
    else return it->second;
}

void checkZG( const std::string& pathToFile, 
		const std::string& eventSelection,
		std::vector<std::tuple<std::string,double,double,int>> vars,
		long unsigned nEntries,
		std::string outputFileName ){
    
    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";

    // make list of variables
    std::vector<std::string> variables;
    for(unsigned int i=0; i<vars.size(); ++i){
        std::string variable = std::get<0>(vars[i]);
        variables.push_back(variable);
    } 

    // make reweighter (not yet sure if will need to use here)
    std::cout << "initializing Reweighter...\n\n";
    std::shared_ptr< ReweighterFactory> reweighterFactory;
    /*if( LeptonSelector::leptonID()=="tzqtight" || LeptonSelector::leptonID()=="tzqmedium0p4" ){
        reweighterFactory = std::shared_ptr<ReweighterFactory>(new tZqReweighterFactory());
    } else{
        std::cerr << "ERROR: reweighter for this lepton ID not yet implemented" << std::endl;
    }*/
    // use empty reweighter for testing:
    reweighterFactory = std::shared_ptr<ReweighterFactory>( new EmptyReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter(
                                        "../../weights/", year, thissample );

    // initialize photolepton origin map
    std::map< std::string, int > photoLepOrigin;
    photoLepOrigin["ZLepGamma"] = 0;
    //photoLepOrigin["lepton from photon from other"] = 1;
    photoLepOrigin["other"] = 1;

    // initialize output histograms
    std::vector<HistInfo> histInfoVec;
    unsigned int nbins = photoLepOrigin.size();
    histInfoVec.push_back( HistInfo("photoLepOrigin","", nbins,-0.5,nbins-0.5) );
    for(unsigned int i=0; i<vars.size(); ++i){
        std::string variable = std::get<0>(vars[i]);
        double xlow = std::get<1>(vars[i]);
        double xhigh = std::get<2>(vars[i]);
        int nbins = std::get<3>(vars[i]);
	for( std::map<std::string,int>::iterator j=photoLepOrigin.begin(); 
	    j!=photoLepOrigin.end(); ++j){
	    std::string origin = j->first;
	    std::string name = origin + "_" + variable;
	    histInfoVec.push_back( HistInfo( name.c_str(), name.c_str(), nbins, xlow, xhigh) );
	}
    }
    std::map< std::string, std::shared_ptr<TH1D> > histMap;
    for( HistInfo histInfo: histInfoVec ){
        histMap[histInfo.name()] = histInfo.makeHist( histInfo.name() );
    }

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEntries>0 && nEntries<numberOfEntries ) numberOfEntries = nEntries;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
        Event event = treeReader.buildEvent(entry);

	// do event selection
        if(!passES(event, eventSelection, "3prompt", "nominal")) continue;
	LeptonCollection lepcollection = event.leptonCollection();
	if( lepcollection.size()!=3 ){
	    std::cerr << "WARNING: unexpected number of leptons!" << std::endl;
	    continue;
	}

	// determine event variables
        std::map<std::string,double> varmap = eventFlattening::initVarMap();
	varmap = eventFlattening::eventToEntry(event, 1., reweighter, "3prompt",
                                        nullptr, nullptr, "nominal", false, nullptr);

	// printouts for testing
	std::cout << "--- event ---" << std::endl;
	for( LeptonCollection::const_iterator l1It = lepcollection.cbegin(); 
	    l1It != lepcollection.cend(); ++l1It ){
	    Lepton& lep = **l1It;
	    std::cout << "lepton: ";
	    std::cout << lep.matchPdgId() << " ";
	    std::cout << lep.momPdgId() << " " << std::endl;;
	}

	// determine the photolepton origin
	std::string cat = "other";
	for( LeptonCollection::const_iterator l1It = lepcollection.cbegin();
            l1It != lepcollection.cend(); ++l1It ){
            Lepton& lep = **l1It;
            if( lep.matchPdgId()==22 and lep.momPdgId()==23 ) cat="ZLepGamma";
        }

	// fill the histograms
	histMap["photoLepOrigin"]->Fill( photoLepOrigin[cat] );
        for(std::string variable : variables){
            histMap[ cat+"_"+variable]->Fill( getVarValue(variable,varmap),varmap["_normweight"] );
        }

	// make output file
	TFile* outputFilePtr = TFile::Open( outputFileName.c_str() , "RECREATE" );
	for( auto el: histMap ) el.second->Write();
	outputFilePtr->Close();
    }
}

int main( int argc, char* argv[] ){
    
    std::cerr<<"###starting###"<<std::endl;
    if( argc != 5 ){
        std::cerr << "checkZG requires 3 arguments to run: " << std::endl;
        std::cerr << "input_file_path, event_selection, nentries, output_file_name" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& event_selection = argvStr[2];
    long unsigned nentries = std::stoull(argvStr[3]);
    std::string& output_file_name = argvStr[4];
   
    // make structure for variables
    std::vector<std::tuple<std::string,double,double,int>> vars;
    vars.push_back(std::make_tuple("_abs_eta_recoil",0.,5.,20));
    vars.push_back(std::make_tuple("_Mjj_max",0.,1200.,20));
    vars.push_back(std::make_tuple("_lW_asymmetry",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_deepCSV_max",0.,1.,20));
    vars.push_back(std::make_tuple("_deepFlavor_max",0.,1.,20));
    vars.push_back(std::make_tuple("_lT",0.,800.,20));
    vars.push_back(std::make_tuple("_MT",0.,200.,20));
    vars.push_back(std::make_tuple("_pTjj_max",0.,300.,20));
    vars.push_back(std::make_tuple("_dRlb_min",0.,3.15,20));
    vars.push_back(std::make_tuple("_dPhill_max",0.,3.15,20));
    vars.push_back(std::make_tuple("_HT",0.,800.,20));
    vars.push_back(std::make_tuple("_nJets",-0.5,6.5,7));
    vars.push_back(std::make_tuple("_nBJets",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_dRlWrecoil",0.,10.,20));
    vars.push_back(std::make_tuple("_dRlWbtagged",0.,7.,20));
    vars.push_back(std::make_tuple("_M3l",0.,600.,20));
    vars.push_back(std::make_tuple("_abs_eta_max",0.,5.,20));
    //vars.push_back(std::make_tuple("_eventBDT",-1,1,15));
    vars.push_back(std::make_tuple("_nMuons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_nElectrons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_yield",0.,1.,1));
    vars.push_back(std::make_tuple("_leptonPtLeading",0.,300.,12));
    vars.push_back(std::make_tuple("_leptonPtSubLeading",0.,180.,12));
    vars.push_back(std::make_tuple("_leptonPtTrailing",0.,120.,12));
    vars.push_back(std::make_tuple("_leptonEtaLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaSubLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaTrailing",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_jetPtLeading",0.,100.,20));
    vars.push_back(std::make_tuple("_jetPtSubLeading",0.,100.,20));
    vars.push_back(std::make_tuple("_numberOfVertices",-0.5,70.5,71));
    vars.push_back(std::make_tuple("_bestZMass",0.,150.,15));
    vars.push_back(std::make_tuple("_lW_pt",10.,150.,14));
    vars.push_back(std::make_tuple("_Z_pt",0.,300.,15));
 
    checkZG( input_file_path, event_selection, vars, nentries, output_file_name );
    
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
