///// a test executable for the ReweighterBTagShape class /////

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
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/SampleCrossSections.h"
#include "../../Event/interface/Event.h"
#include "../interface/ConcreteReweighterFactory.h"
#include "../interface/ReweighterBTagShape.h"
#include "../../AnalysisCode/eventselection/interface/eventSelections.h"
#include "../../AnalysisCode/eventselection/interface/eventFlattening.h"

int main(int argc, char* argv[]){

    // parse arguments    
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( argvStr.size() != 7 ){
	std::cerr << "ERROR: wrong number of arguments. ";
	std::cerr << "Need ./testReweighterBTagShape inputFilePath outputDirectory reweightingMode";
	std::cerr << " eventSelection doBDT pathToXMLFile" << std::endl;
	return 1;
    }
    std::string inputFilePath = argvStr[1];
    std::string outputDirectory = argvStr[2];
    std::string reweightingMode = argvStr[3];
    std::string eventSelection = argvStr[4];
    bool doBDT = (argvStr[5]=="true" || argvStr[5]=="True");
    std::string pathToXMLFile = argvStr[6];

    // define variables
    std::vector< std::tuple<std::string,double,double,int> > vars;
    vars.push_back(std::make_tuple("_abs_eta_recoil",0.,5.,20));
    vars.push_back(std::make_tuple("_Mjj_max",0.,1200.,20));
    vars.push_back(std::make_tuple("_lW_asymmetry",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_deepCSV_max",0.,1.,20));
    vars.push_back(std::make_tuple("_deepFlavor_max",0.,1.,20));
    vars.push_back(std::make_tuple("_lT",0.,800.,20));
    vars.push_back(std::make_tuple("_MT",0.,300.,20));
    vars.push_back(std::make_tuple("_pTjj_max",0.,300.,20));
    vars.push_back(std::make_tuple("_dRlb_min",0.,3.15,20));
    vars.push_back(std::make_tuple("_dPhill_max",0.,3.15,20));
    vars.push_back(std::make_tuple("_HT",0.,800.,20));
    vars.push_back(std::make_tuple("_nJets",-0.5,10.5,11));
    vars.push_back(std::make_tuple("_nBJets",-0.5,5.5,6));
    vars.push_back(std::make_tuple("_dRlWrecoil",0.,10.,20));
    vars.push_back(std::make_tuple("_dRlWbtagged",0.,7.,20));
    vars.push_back(std::make_tuple("_M3l",0.,600.,20));
    vars.push_back(std::make_tuple("_abs_eta_max",0.,5.,20));
    vars.push_back(std::make_tuple("_nMuons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_nElectrons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_yield",0.,1.,1));
    vars.push_back(std::make_tuple("_leptonPtLeading",0.,300.,12));
    vars.push_back(std::make_tuple("_leptonPtSubLeading",0.,180.,12));
    vars.push_back(std::make_tuple("_leptonPtTrailing",0.,120.,12));
    vars.push_back(std::make_tuple("_leptonEtaLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaSubLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaTrailing",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_numberOfVertices",-0.5,70.5,71));
    vars.push_back(std::make_tuple("_bestZMass",0.,150.,15));
    if( doBDT ) vars.push_back(std::make_tuple("_eventBDT",-1.,1.,15));
    std::vector< std::string > variables;
    for( auto el: vars ) variables.push_back( std::get<0>(el) );

    // load a TMVA Reader if needed
    TMVA::Reader* tmvaReader = new TMVA::Reader();
    if(doBDT){
        std::string subDir = "bdts_"+LeptonSelector::leptonID();
        std::string::size_type pos = pathToXMLFile.find("/bdt/");
        if( pos != std::string::npos ){
            pathToXMLFile.erase( pos, 5 );
            pathToXMLFile.insert( pos, "/bdt/"+subDir+"/" );
        }
        tmvaReader = eventFlattening::initializeReader(tmvaReader, pathToXMLFile, "all");
    }

    // define systematic variations
    std::vector< std::string > systematics;
    if( reweightingMode=="shape" ){
	systematics = {"jes","hf","lf","hfstats1","hfstats2",
			"lfstats1","lfstats2","cferr1","cferr2"};
    } else if( reweightingMode=="wp" ){ systematics = {"light","heavy"}; }
 
    // define histograms
    std::map< std::string, std::shared_ptr<TH1D> > histMap;
    for( auto el: vars ){
        std::string variable = std::get<0>(el);
        double xlow = std::get<1>(el);
        double xhigh = std::get<2>(el);
        int nbins = std::get<3>(el);
        HistInfo histInfo( "", variable.c_str(), nbins, xlow, xhigh);
        histMap[variable+"_noweight"] = histInfo.makeHist( 
	    variable+"_noweight" );
	histMap[variable+"_bweight_nominal"] = histInfo.makeHist( 
	    variable+"_bweight_nominal" );
	histMap[variable+"_bweight_nonorm"] = histInfo.makeHist(
	    variable+"_bweight_nonorm" );
	for( auto sys: systematics){
	    histMap[variable+"_bweight_"+sys+"_up"] = histInfo.makeHist( 
		variable+"_bweight_"+sys+"_up" );
	    histMap[variable+"_bweight_"+sys+"_down"] = histInfo.makeHist( 
		variable+"_bweight_"+sys+"_down" );
	}
    }

    // make a TreeReader
    std::cout << "making TreeReader..." << std::endl;
    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );
    std::string year = (treeReader.is2016())?"2016":
			(treeReader.is2017())?"2017":
			(treeReader.is2018())?"2018":"";
    std::string sampleName = treeReader.currentSamplePtr()->fileName();

    // make the Reweighter
    std::cout << "making Reweighter..." << std::endl;
    std::shared_ptr<ReweighterFactory> reweighterFactory( new EmptyReweighterFactory() );
    if( reweightingMode=="shape" ){
	std::shared_ptr<ReweighterFactory> temp( new BTagShapeReweighterFactory() );
	reweighterFactory = temp;
    }
    else if( reweightingMode=="wp" ){
	std::shared_ptr<ReweighterFactory> temp( new BTagWPReweighterFactory() );
	reweighterFactory = temp;
    }
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter(
                                        "/user/llambrec/ewkino/weights/", 
					year, thissample );

    // set number of entries to loop over (both first and second loop)
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //numberOfEntries = std::min( (unsigned long) 1000, treeReader.numberOfEntries() );

    ///// do first event loop to determine normalization /////

    if( reweightingMode=="shape" ){
	
	// initialize avg weights
	std::map<int,int> npass;
        std::map<int,double> sumweights;
	for( int i=0; i<11; i++){
	    npass[i] = 0;
	    sumweights[i] = 0;
	}

	// loop over events
	std::cout << " - starting event loop for " << numberOfEntries << " events..." << std::endl;
	for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
	    Event event = treeReader.buildEvent( entry );

	    // do correct event selection
	    // put 'false' in passES and eventCategory to skip selecting b-jets
	    if(!passES(event, eventSelection, "3tight", "nominal", false)) continue;
	    int eventcategory = eventCategory(event, "nominal", false);
	    if( eventcategory<0 ) continue;
	    // TEMP for testing:
	    //if(!passES(event, eventSelection, "3tight", "nominal")) continue;
            //int eventcategory = eventCategory(event, "nominal");
            //if( eventcategory<0 ) continue;


	    // determine number of jets
	    // (must be consistent with how it is determined internally in ReweighterBTagShape...)
	    int njets = event.jetCollection().goodJetCollection().size();
	    njets = std::min(10,njets);

	    // determine b-tag reweighting
	    sumweights[njets] += reweighter["bTag_shape"]->weight( event );
	    npass[njets] += 1;
	}

	// calculate average weights
	std::cout << "average weights (binned): " << std::endl;
	for( unsigned int i=0; i<sumweights.size(); i++){
	    if( npass[i]==0 ){ sumweights[i] = 1; }
	    else{ sumweights[i] /= npass[i]; }
	    std::cout << i << ": " << sumweights[i] << " (on " << npass[i] << " events)" << std::endl;
	}

	// write average weight
	dynamic_cast<ReweighterBTagShape*>(
            reweighter.getReweighterNonConst("bTag_shape"))->setNormFactors( 
		thissample[0], sumweights );
    }

    ///// do second event loop and apply weights /////

    // initialize average of b-tag weights
    std::map<int,double> sumBTagWeight;
    std::map<int,int> sumNEntries;
    for( int i=0; i<11; i++){
        sumNEntries[i] = 0;
        sumBTagWeight[i] = 0;
    }

    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	Event event = treeReader.buildEvent( entry );

	// do basic jet cleaning -> corresponds to cleaning in ReweighterBTagShape initialization
        //event.cleanJetsFromFOLeptons();
        //event.jetCollection().selectGoodJets();

	// optional: do event selection (average b-tag weight can deviate from 1 in this case)
	if(!passES(event, eventSelection, "3tight", "nominal")) continue;
	if( eventCategory(event, "nominal")<0 ) continue;
	// for testing: print and continue
	//std::cout << "event passing" << std::endl;
	//continue;

	// determine the numerical values of all variables
	std::map<std::string,double> varmap = eventFlattening::initVarMap();
	varmap = eventFlattening::eventToEntry(event, 1., reweighter, "3tight",
                                        nullptr, nullptr, "nominal", doBDT, tmvaReader);

	// determine number of jets
	// (must be consistent with first loop)
	int njets = event.jetCollection().goodJetCollection().size();
	njets = std::min(10,njets);
	//if( reweightingMode=="wp" ) njets=0; // no binning for WP type reweighting

	// determine nominal event reweighting
	double reweight = reweighter.totalWeight( event );
	sumBTagWeight[njets] += reweight;
	sumNEntries[njets] += 1;

	// fill histograms
	double baseweight = event.weight();
	//double baseweight = 1; // alternative
	for( std::string variable: variables ){
	    double varvalue = varmap.at(variable);
	    histMap[variable+"_noweight"]->Fill(varvalue,baseweight);
	    histMap[variable+"_bweight_nominal"]->Fill(varvalue,baseweight*reweight);
	    if(reweightingMode=="shape"){
		histMap[variable+"_bweight_nonorm"]->Fill(varvalue,
			baseweight*dynamic_cast<const ReweighterBTagShape*>(
                        reweighter["bTag_shape"])->weightNoNorm( event ) );
		for( std::string sys: systematics ){
		    histMap[variable+"_bweight_"+sys+"_up"]->Fill(varvalue,
			baseweight*dynamic_cast<const ReweighterBTagShape*>(
			reweighter["bTag_shape"])->weightUp( event, sys ) );
		    histMap[variable+"_bweight_"+sys+"_down"]->Fill(varvalue,
			baseweight*dynamic_cast<const ReweighterBTagShape*>(
			reweighter["bTag_shape"])->weightDown( event, sys ) );
		}
	    } else if(reweightingMode=="wp"){
		histMap[variable+"_bweight_nonorm"]->Fill(varvalue,reweight);
		for( std::string sys: systematics ){
		    histMap[variable+"_bweight_"+sys+"_up"]->Fill(varvalue,
                        baseweight*reweight
			/reweighter["bTag_"+sys]->weight( event )
			*reweighter["bTag_"+sys]->weightUp( event ) );
		    histMap[variable+"_bweight_"+sys+"_down"]->Fill(varvalue,
                        baseweight*reweight
			/reweighter["bTag_"+sys]->weight( event) 
			*reweighter["bTag_"+sys]->weightDown( event ) );
		}
	    }
	}

	// print info
	/*if( entry < 0 ){
	    std::cout << "----- entry " << entry << " -----" << std::endl;
	    std::cout << "- btag weight: " << reweight << std::endl;
	    std::cout << "- jes up: " << reweight_jesup << std::endl;
	    std::cout << "- jes down: " << reweight_jesdown << std::endl;
	}*/
    
    }

    // calculate average weights
    std::cout << "average weights (binned): " << std::endl;
    for( unsigned int i=0; i<sumBTagWeight.size(); i++){
        if( sumNEntries[i]==0 ){ sumBTagWeight[i] = 1; }
        else{ sumBTagWeight[i] /= sumNEntries[i]; }
        std::cout << i << ": " << sumBTagWeight[i];
	std::cout << " (on " << sumNEntries[i] << " events)" << std::endl;
    }
 
    // write output file
    systemTools::makeDirectory(outputDirectory);
    std::string outFileName = stringTools::removeOccurencesOf(sampleName,".root");
    outFileName += "_"+reweightingMode+".root";
    std::string outputFilePath = stringTools::formatDirectoryName(outputDirectory);
    outputFilePath += outFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str(), "RECREATE" );
    for( auto el : histMap ) el.second->Write();
    outputFilePtr->Close();
   
    delete tmvaReader;

    std::cerr << "done" << std::endl;
    return 0;
}
