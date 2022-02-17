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
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/Sample.h"
#include "../../Event/interface/Event.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "interface/eventSelections.h"
#include "interface/eventFlattening.h"

double getVarValue(const std::string varname, std::map<std::string,double> varmap){
    // retrieve a value from a map of all variables associated to the event
    
    // remove "fineBinned" from the varname to allow for e.g. fineBinnedeventBDT to be used
    std::string modvarname = stringTools::removeOccurencesOf(varname,"fineBinned");
    modvarname = stringTools::removeOccurencesOf(modvarname,"coarseBinned");
    modvarname = stringTools::removeOccurencesOf(modvarname,"smallRange");
    modvarname = stringTools::removeOccurencesOf(modvarname,"altBinned");
    // find this variable in the list of available variables
    std::map<std::string,double>::iterator it = varmap.find(modvarname);
    if(it == varmap.end()) return -9999; 
    else return it->second;
}

void fillVarValue(std::shared_ptr<TH1D> hist, const double varvalue, const double weight){
    // extension of hist->Fill(), explicitly adding under- and overflow to first resp. last bin
    double lowx = hist->GetBinLowEdge(1);
    double highx = hist->GetBinLowEdge(hist->GetNbinsX())+hist->GetBinWidth(hist->GetNbinsX());
    double newvarvalue = varvalue;
    if(newvarvalue<lowx) newvarvalue = lowx + hist->GetBinWidth(1)/2.;
    if(newvarvalue>highx) newvarvalue = highx - hist->GetBinWidth(hist->GetNbinsX())/2.;
    hist->Fill(newvarvalue,weight);
}

std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > initHistMap(
				const std::vector<std::tuple<std::string,double,double,int>>& vars,
				const std::string processName ){
    // map of process name to variable to histogram!
    
    // initialize the output histogram map
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap;
    // loop over variables
    for(unsigned int i=0; i<vars.size(); ++i){
	// get variable info from input struct (vector of tuples)
	std::string variable = std::get<0>(vars[i]);
	double xlow = std::get<1>(vars[i]);
	double xhigh = std::get<2>(vars[i]);
	int nbins = std::get<3>(vars[i]);
	HistInfo histInfo( "", variable.c_str(), nbins, xlow, xhigh);
	// parse the process name and variable name to form the histogram name
	std::string name = processName+"_"+variable;
	name = stringTools::removeOccurencesOf(name," ");
	name = stringTools::removeOccurencesOf(name,"/");
	name = stringTools::removeOccurencesOf(name,"#");
	name = stringTools::removeOccurencesOf(name,"{");
	name = stringTools::removeOccurencesOf(name,"}");
	name = stringTools::removeOccurencesOf(name,"+");
	histMap[processName][variable] = histInfo.makeHist( name+"_nominal" );
	histMap[processName][variable]->SetTitle(processName.c_str());
    }
    return histMap;
}

void clipHistogram( std::shared_ptr<TH1D> hist ){
    // put all bins of a histogram to minimum zero (no net negative weights)
    for(int i=0; i<hist->GetNbinsX()+2; ++i){
	if(hist->GetBinContent(i)<0.){
	    hist->SetBinContent(i,0.);
	    hist->SetBinError(i,0.);
	}
    }
}

void copyHistogram( std::shared_ptr<TH1D> histlhs, std::shared_ptr<TH1D> histrhs){
    // copy bin content of histrhs into histlhs while keeping original name and title
    histlhs->Reset();
    for(int i=0; i<histrhs->GetNbinsX()+2; ++i){
	histlhs->SetBinContent(i,histrhs->GetBinContent(i));
    }
}

void fillHistograms(const std::string& inputDirectory,
			const std::string& sampleList,
			int sampleIndex,
			const std::string& outputDirectory,
			const std::string& eventselection,
			const std::string& selection_type,
			const std::string& variation,
			const std::string& muonfrmap,
			const std::string& electronfrmap,
			const std::vector<std::tuple<std::string,double,double,int>> vars){
    std::cout << "=== start function fillHistograms ===" << std::endl;
    
    // initialize TreeReader from input file
    std::cout<<"initializing TreeReader and setting to sample n. "<<sampleIndex<<std::endl;
    TreeReader treeReader( sampleList, inputDirectory, true );
    treeReader.initSample();
    for(int idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();
    std::string processName = treeReader.currentSample().processName();
    if( selection_type=="fakerate" ) processName = "nonprompt";

    // TEMP: determine additional normalization factor 
    // due to faulty hCounters in current samples
    double hCounterFactor = 1;
    if( treeReader.isMC() ){
	std::string rfSampleList = "../normalization/hcounters_controlregions_" + year+".txt";
	std::vector<Sample> rfSamples = readSampleList( rfSampleList, inputDirectory );
	double trueHCounter = 0;
	std::string refName = stringTools::removeOccurencesOf( 
			    treeReader.currentSample().fileName(), ".root" );
	for( Sample s: rfSamples ){
	    if( s.fileName()==refName ){ trueHCounter = s.xSec(); }	
	}
	if( trueHCounter==0 ){
	    std::string msg = "ERROR: found no matching sample for current sample";
	    msg += " in hcounter normalization sample list";
	    throw std::runtime_error(msg);
	}
	TH1D* hcounter = (TH1D*) treeReader.getFromCurrentFile("blackJackAndHookers/hCounter");
	double falseHCounter = hcounter->GetBinContent(1);
	hCounterFactor = falseHCounter / trueHCounter;
	std::cout << "determined hCounter normalization factor: " << std::endl;
	std::cout << " - true hCounter: " << trueHCounter << std::endl;
	std::cout << " - faulty hCounter: " << falseHCounter << std::endl;
	std::cout << " - normalization factor: " << hCounterFactor << std::endl;
    }

    // make flat vector of variable names (often more convenient then argument structure)
    std::vector<std::string> variables;
    for(unsigned int i=0; i<vars.size(); ++i){
	std::string variable = std::get<0>(vars[i]);
	variables.push_back(variable);
    }

    // load fake rate maps if needed
    std::shared_ptr<TH2D> frmap_muon;
    std::shared_ptr<TH2D> frmap_electron;
    if(selection_type=="fakerate"){
        frmap_muon = eventFlattening::readFRMap(muonfrmap, "muon", year);
        frmap_electron = eventFlattening::readFRMap(electronfrmap, "electron", year);
        std::cout << "read fake rate maps" << std::endl;
    }
    
    // make reweighter
    std::cout << "initializing Reweighter..." << std::endl;
    std::shared_ptr< ReweighterFactory> reweighterFactory;
    reweighterFactory = std::shared_ptr<ReweighterFactory>( new EmptyReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
					"../../weights/", year, thissample );
    
    // make output collection of histograms
    std::cout << "making output collection of histograms..." << std::endl;
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap =
        initHistMap(vars, processName );

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    std::cout << "starting event loop for " << numberOfEntries << " events." << std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	
	// initialize map of variables
	std::map<std::string,double> varmap = eventFlattening::initVarMap();

	// fill nominal histograms
	bool passnominal = true;
        Event event = treeReader.buildEvent(entry,false,false,false,false);
        if(!passES(event, eventselection, selection_type, variation)) passnominal = false;
	if(passnominal){
	    varmap = eventFlattening::eventToEntry(event, reweighter, selection_type, 
					frmap_muon, frmap_electron, variation);
	    double weight = varmap["_normweight"] * hCounterFactor;
	    for(std::string variable : variables){
		fillVarValue(histMap[processName][variable],
			    getVarValue(variable,varmap), weight);
	    }
	}
    } // end loop over events

    // make output ROOT file
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // loop over variables
    for(std::string variable : variables){
	// need to distinguish between normal histograms and finely binned ones
	// the latter will be rebinned in a later stage
	// to do this correctly, they must not be clipped
	bool doClip = true;
	if( stringTools::stringContains(variable,"fineBinned") ) doClip = false;
	if( selection_type == "fakerate") doClip = false;
	// first find nominal histogram for this variable
	std::shared_ptr<TH1D> nominalhist = histMap[processName][variable];
	// if nominal histogram is empty, fill with dummy value (needed for combine);
	// also put all other histograms equal to the nominal one to avoid huge uncertainties
	if( nominalhist->GetEntries()==0 ){
	    nominalhist->SetBinContent(1,1e-6); 
	}
	// clip and write nominal histogram
	if( doClip ) clipHistogram(nominalhist);
	nominalhist->Write();
    }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc < 10 ){
        std::cerr << "ERROR: event binning requires at different number of arguments to run...:";
        std::cerr << " input_directory, sample_list, sample_index, output_directory,";
	std::cerr << " event_selection, selection_type, variation,";
	std::cerr << " muonfrmap, electronfrmap" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_directory = argvStr[4];
    std::string& event_selection = argvStr[5];
    std::string& selection_type = argvStr[6];
    std::string& variation = argvStr[7];
    std::string& muonfrmap = argvStr[8];
    std::string& electronfrmap = argvStr[9];

    // make structure for variables
    std::vector<std::tuple<std::string,double,double,int>> vars;
    vars.push_back(std::make_tuple("_Mjj_max",0.,1200.,20));
    vars.push_back(std::make_tuple("_lW_asymmetry",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_deepCSV_max",0.,1.,20));
    vars.push_back(std::make_tuple("_deepFlavor_max",0.,1.,20));
    vars.push_back(std::make_tuple("_lT",0.,800.,20));
    vars.push_back(std::make_tuple("_MT",0.,200.,20));
    vars.push_back(std::make_tuple("_smallRangeMT", 0., 150., 15));
    vars.push_back(std::make_tuple("_pTjj_max",0.,300.,20));
    vars.push_back(std::make_tuple("_dPhill_max",0.,3.15,20));
    vars.push_back(std::make_tuple("_HT",0.,800.,20));
    vars.push_back(std::make_tuple("_nJets",-0.5,6.5,7));
    vars.push_back(std::make_tuple("_nBJets",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_M3l",0.,600.,20));
    vars.push_back(std::make_tuple("_altBinnedM3l", 100., 400., 20)); 
    vars.push_back(std::make_tuple("_fineBinnedM3l",75.,105,20));
    vars.push_back(std::make_tuple("_nMuons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_nElectrons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_yield",0.,1.,1));
    vars.push_back(std::make_tuple("_leptonPtLeading",0.,300.,12));
    vars.push_back(std::make_tuple("_leptonPtSubLeading",0.,180.,12));
    vars.push_back(std::make_tuple("_leptonPtTrailing",0.,120.,12));
    vars.push_back(std::make_tuple("_fineBinnedleptonPtTrailing",0.,50.,20));
    vars.push_back(std::make_tuple("_leptonEtaLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaSubLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaTrailing",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_jetPtLeading",0.,100.,20));
    vars.push_back(std::make_tuple("_jetPtSubLeading",0.,100.,20));
    vars.push_back(std::make_tuple("_numberOfVertices",-0.5,70.5,71));
    vars.push_back(std::make_tuple("_bestZMass",0.,150.,15));

    // check variables
    std::map<std::string,double> emptymap = eventFlattening::initVarMap();
    for(unsigned int i=0; i<vars.size(); ++i){
        std::string varname = std::get<0>(vars[i]);
        if(getVarValue(varname,emptymap)==-9999) return -1;
    }

    // fill the histograms
    fillHistograms(input_directory, sample_list, sample_index, output_directory,
				    event_selection, selection_type, variation,
				    muonfrmap, electronfrmap, 
				    vars);
    std::cerr << "###done###" << std::endl;
    return 0;
}
