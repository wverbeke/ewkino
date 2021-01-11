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
#include "../../Tools/interface/HistInfo.h"
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
    std::vector< std::string > res = { "FlavorZJet", "FlavorPhotonJet", "FlavorPureGluon", "FlavorPureQuark", "FlavorPureCharm", "FlavorPureBottom", "PileUpMuZero", "PileUpEnvelope", "SubTotalPileUp", "SubTotalRelative", "SubTotalPt", "SubTotalScale", "SubTotalAbsolute", "SubTotalMC", "TotalNoFlavor", "TotalNoTime" ,"TotalNoFlavorNoTime", "Total" };
    return res;
}

std::vector<std::string> getGroupedToRemove(){
    std::vector< std::string > res = { "Total" };
    return res;
}

std::pair<double,double> getSquaredSumVariation( const Jet& jet, 
			    std::vector<std::string> variations ){
    double nompt = jet.pt();
    double upvar = 0;
    double downvar = 0;
    for( std::string var: variations ){
	upvar += pow( jet.JetJECUp( var ).pt() - nompt, 2 );
	downvar += pow( jet.JetJECDown( var ).pt() - nompt, 2 );
    }
    upvar = nompt + sqrt(upvar);
    downvar = nompt - sqrt(downvar);
    return std::make_pair(upvar,downvar);
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

    // set global flags (later add as arguments)
    bool doPrint = false;

    // read first event to access available variations
    if(treeReader.numberOfEntries()==0){
	std::cerr << "WARNING: no events in treeReader... returning." << std::endl;
	return;
    }
    bool readAll = true;
    bool readGrouped = true;
    Event event = treeReader.buildEvent( 0, false, false, readAll, readGrouped );
    std::vector<std::string> jecSources = event.jetInfo().allJECVariations();
    jecSources = remove( jecSources, getSourcesToRemove() );
    std::vector<std::string> jecGrouped = event.jetInfo().groupedJECVariations();
    jecGrouped = remove( jecGrouped, getGroupedToRemove() );
    std::cout << "available JEC sources: " << std::endl;
    for( std::string s: jecSources ) std::cout << "  " << s << std::endl;
    std::cout << "available grouped JEC sources: " << std::endl;
    for( std::string s: jecGrouped ) std::cout << "  " << s << std::endl;

    // define output collection of histograms
    // - simple histograms of nominal and jec up variation
    std::vector<HistInfo> jetHistInfoVec;
    jetHistInfoVec.push_back( HistInfo("jetpt_nominal","p_{T} (GeV)",25,0,250) );
    jetHistInfoVec.push_back( HistInfo("jetpt_JECUp","p_{T} (GeV)",25,0,250) );
    jetHistInfoVec.push_back( HistInfo("jetpt_JECSourcesTotalUp","p_{T} (GeV)",25,0,250) );
    jetHistInfoVec.push_back( HistInfo("jetpt_JECSourcesSumUp","p_{T}",25,0,250) );
    jetHistInfoVec.push_back( HistInfo("jetpt_JECGroupedSumUp","p_{T}",25,0,250) );
    // - histograms of relative differences regarding up variations
    jetHistInfoVec.push_back( HistInfo("jetpt_JECUp_d_nominal", "rel. difference", 100, 0, 0.1) );
    jetHistInfoVec.push_back( HistInfo("jetpt_JECSourcesTotalUp_d_JECUp","rel. difference", 20, 0, 0.01) );
    jetHistInfoVec.push_back( HistInfo("jetpt_JECSourcesSumUp_d_JECUp","rel. difference", 20, 0, 0.01) );
    jetHistInfoVec.push_back( HistInfo("jetpt_JECGroupedSumUp_d_JECUp","rel. difference", 20, 0, 0.01) );
    // - simple histograms of nominal and jec up variation for met
    std::vector<HistInfo> metHistInfoVec;
    metHistInfoVec.push_back( HistInfo("metpt_nominal","p_{T} (GeV)",25,0,250) );
    metHistInfoVec.push_back( HistInfo("metpt_JECUp","p_{T} (GeV)",25,0,250) );
    metHistInfoVec.push_back( HistInfo("metpt_JECSourcesTotalUp","p_{T} (GeV)",25,0,250) );
    // - histograms of relative differences regarding up variations for met
    metHistInfoVec.push_back( HistInfo("metpt_JECUp_d_nominal", "rel. difference", 100, 0, 0.1) );
    metHistInfoVec.push_back( HistInfo("metpt_JECSourcesTotalUp_d_JECUp","rel. difference", 20, 0, 0.05) );
    // make output collection of histograms
    std::map< std::string, std::shared_ptr<TH1D> > jetHistMap;
    for( HistInfo histInfo: jetHistInfoVec ){
	jetHistMap[histInfo.name()] = histInfo.makeHist( histInfo.name() );
    }
    std::map< std::string, std::shared_ptr<TH1D> > metHistMap;
    for( HistInfo histInfo: metHistInfoVec ){
        metHistMap[histInfo.name()] = histInfo.makeHist( histInfo.name() );
    }

    // loop over events
    long unsigned nentries = treeReader.numberOfEntries();
    //long unsigned nentries = 1000;
    for( long unsigned entry = 0; entry < nentries; ++entry ){

        //build event
        Event event = treeReader.buildEvent( entry, false, false, readAll, readGrouped );

	JetCollection jetCollection = event.jetCollection();
	for( std::shared_ptr<Jet> jetPtr: jetCollection ){
	    Jet jet = *jetPtr.get();
	    std::pair<double,double> sourcesSum = getSquaredSumVariation( jet, jecSources );
            std::pair<double,double> groupedSum = getSquaredSumVariation( jet, jecGrouped );
	    std::map< std::string,double> valMap;
	    valMap["jetpt_nominal"] = jet.pt();
	    valMap["jetpt_JECUp"] = jet.JetJECUp().pt();
	    valMap["jetpt_JECSourcesTotalUp"] = jet.JetJECUp("Total").pt();
	    valMap["jetpt_JECSourcesSumUp"] = sourcesSum.first; 
            valMap["jetpt_JECGroupedSumUp"] = groupedSum.first;
	    valMap["jetpt_JECUp_d_nominal"] = fabs(jet.JetJECUp().pt()-jet.pt())/jet.pt();
	    valMap["jetpt_JECSourcesTotalUp_d_JECUp"] = fabs(jet.JetJECUp("Total").pt()-jet.JetJECUp().pt())/jet.JetJECUp().pt();
	    valMap["jetpt_JECSourcesSumUp_d_JECUp"] = fabs(sourcesSum.first-jet.JetJECUp().pt())/jet.JetJECUp().pt();
	    valMap["jetpt_JECGroupedSumUp_d_JECUp"] = fabs(groupedSum.first-jet.JetJECUp().pt())/jet.JetJECUp().pt();
	    for( auto el: jetHistMap ){
		el.second->Fill( valMap.at(el.first) );
	    }
	}
	std::map< std::string,double > valMap;
	valMap["metpt_nominal"] = event.met().pt();
	valMap["metpt_JECUp"] = event.met().MetJECUp().pt();
	valMap["metpt_JECSourcesTotalUp"] = event.met().MetJECUp("Total").pt();
	valMap["metpt_JECUp_d_nominal"] = fabs(valMap["metpt_JECUp"]
					    -valMap["metpt_nominal"])
					    /valMap["metpt_nominal"];
	valMap["metpt_JECSourcesTotalUp_d_JECUp"] = fabs(valMap["metpt_JECSourcesTotalUp"]
					    -valMap["metpt_JECUp"])
					    /valMap["metpt_JECUp"];
	for( auto el: metHistMap ){
	    el.second->Fill( valMap.at(el.first) );
        }
	if( doPrint ){
	    // print JEC uncertainties on MET
	    std::cout << "MET:" << std::endl;
	    std::cout << "- nominal: " << valMap["metpt_nominal"] << std::endl;
	    std::cout << "- original variation: " << valMap["metpt_JECUp"] << "/" << event.met().MetJECDown().pt() << std::endl;
	    std::cout << "- source total: " << valMap["metpt_JECSourcesTotalUp"] << "/" << event.met().MetJECDown( "Total" ).pt() << std::endl;
	}

	// print JEC uncertainties on the first jet
	/*if( event.jetCollection().size()>0 ){
	    int index = 0;
	    Jet jet = event.jetCollection()[index];
	    std::cout << "Jet " << index << ":" << std::endl;
	    std::cout << "- nominal: " << jet.pt() << std::endl;
	    std::cout << "- original variation: " << jet.JetJECUp().pt() << "/" << jet.JetJECDown().pt() << std::endl;
	    std::cout << "- source total: " << jet.JetJECUp( "Total" ).pt() << "/" << jet.JetJECDown( "Total" ).pt() << std::endl;
	    std::pair<double,double> sourcesSum = getSquaredSumVariation( jet, jecSources );
	    std::cout << "- sources sum: " << sourcesSum.first << "/" << sourcesSum.second << std::endl;
	    std::pair<double,double> groupedSum = getSquaredSumVariation( jet, jecGrouped );
            std::cout << "- grouped sum: " << groupedSum.first << "/" << groupedSum.second << std::endl;
	}*/
    }

    // write output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName(outputDirectory)+outputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    for( auto el: jetHistMap ) el.second->Write();
    for( auto el: metHistMap) el.second->Write();
    outputFilePtr->Close();
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
