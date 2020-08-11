// This is the main C++ class used to run the systematics.
// It is supposed to run on the output file of an event selection procedure
// and produce a root file containing a histogram of systematic up and down variations.

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
#include "../../Event/interface/Event.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"

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

std::string systematicType(const std::string systematic){
    // get type of sytematic
    // so far either "acceptance" or "weight" or "other"
    if(systematic=="JEC" or systematic=="JER" or systematic=="Uncl"){
	return std::string("acceptance");
    }
    if(
	    //systematic=="muonID" // split into syst and stat
	    or systematic=="muonIDSyst"
	    or systematic=="muonIDStat"
	    or systematic=="electronID" 
	    or systematic=="pileup" 
	    or systematic=="bTag_heavy" or systematic=="bTag_light"
	    or systematic=="prefire"){ // still missing: electron reco
	return std::string("weight");
    }
    if(systematic=="fScale" or systematic=="rScale"
	    or systematic=="scales"
	    or systematic=="isrScale" or systematic=="fsrScale"){
	return std::string("scale");
    }
    if(systematic=="pdfvar"){
	return std::string("pdfvar");
    }
    if(systematic=="muonID"){
	return std::string("ignore");
    }
    std::cerr<<"### ERROR ###: systematic '"<<systematic<<"' not recognized."<<std::endl;
    return std::string("ERROR");
}

double getVarValue(const std::string varname, std::map<std::string,double> varmap){
    // retrieve a value from a map of all variables associated to the event
    std::map<std::string,double>::iterator it = varmap.find(varname);
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
                                const std::vector<std::string>& systematics,
				const std::string processName){
    // initialize the output histogram map
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap;
    // loop over variables
    for(unsigned int i=0; i<vars.size(); ++i){
        std::string variable = std::get<0>(vars[i]);
        double xlow = std::get<1>(vars[i]);
        double xhigh = std::get<2>(vars[i]);
        int nbins = std::get<3>(vars[i]);
        HistInfo histInfo( "", variable.c_str(), nbins, xlow, xhigh);
	std::string name = processName+"_"+variable;
	name = stringTools::removeOccurencesOf(name," ");
	name = stringTools::removeOccurencesOf(name,"/");
	name = stringTools::removeOccurencesOf(name,"#");
	name = stringTools::removeOccurencesOf(name,"{");
	name = stringTools::removeOccurencesOf(name,"}");
	name = stringTools::removeOccurencesOf(name,"+");
        histMap[variable]["nominal"] = histInfo.makeHist( name+"_nominal" );
	histMap[variable]["nominal"]->SetTitle(processName.c_str());
        // loop over systematics
        for(std::string systematic : systematics){
            // first special case for PDF variations: store 100 variations as well as envelope
            if(systematic=="pdfvar"){
                for(int i=0; i<100; ++i){
                    std::string temp = systematic + std::to_string(i);
                    histMap[variable][temp] = histInfo.makeHist( name+"_"+temp );
		    histMap[variable][temp]->SetTitle(processName.c_str());
                }
		std::string temp = "pdfEnvUp";
                histMap[variable][temp] = histInfo.makeHist( name+"_"+temp );
		histMap[variable][temp]->SetTitle(processName.c_str());
		temp = "pdfEnvDown";
                histMap[variable][temp] = histInfo.makeHist( name+"_"+temp );
		histMap[variable][temp]->SetTitle(processName.c_str());
            }
            // now general case: store up and down variation
            else{
                std::string temp = systematic + "Up";
                histMap[variable][temp] = histInfo.makeHist( name+"_"+temp );
		histMap[variable][temp]->SetTitle(processName.c_str());
                temp = systematic + "Down";
                histMap[variable][temp] = histInfo.makeHist( name+"_"+temp );
		histMap[variable][temp]->SetTitle(processName.c_str());
            }
        }
    }
    return histMap;
}

void fillEnvelope( std::map< std::string, std::map< std::string,std::shared_ptr<TH1D> >> histMap, 
		    std::string variable, std::string upname, std::string downname, std::string tag ){
    // calculate bin-per-bin envelope of a series of histograms containing "tag" in name
    // note: the envelope entries must beadded to the histMap as empty hist before this function!
    for( const auto& el : histMap[variable] ){
	if(!stringTools::stringContains(el.first,tag)) continue;
	if(stringTools::stringContains(el.first,upname)) continue;
	if(stringTools::stringContains(el.first,downname)) continue;
	std::shared_ptr<TH1D> hist = el.second;
	for(int i=1; i<hist->GetNbinsX()+1; ++i){
	    if( hist->GetBinContent(i) > histMap[variable][upname]->GetBinContent(i) ){
		histMap[variable][upname]->SetBinContent(i,hist->GetBinContent(i));
	    }
	    if( hist->GetBinContent(i) < histMap[variable][downname]->GetBinContent(i) ){
		histMap[variable][downname]->SetBinContent(i,hist->GetBinContent(i));
	    }
	}
    }
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

void fillSystematicsHistograms(const std::string& pathToFile, const double norm, 
				const std::string& outputFilePath, 
				const std::string& processName,
				const std::string& eventselection,
				const int signalcategory,
				const std::string& selection_type,
				const std::shared_ptr<TH2D>& frmap_muon,
				const std::shared_ptr<TH2D>& frmap_electron,
				const std::vector<std::tuple<std::string,double,double,int>> vars,
				TMVA::Reader* reader,
				std::vector<std::string> systematics){

    // initialize TreeReader from input file
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";

    // determine whether event selection is signal-like (important for categorization)
    bool issignallike = stringTools::stringContains(eventselection,"signal");

    // when considering the data sample, disregard all systematics
    if(processName=="data") systematics.clear();
    // warning: do not use isData() in the condition above, as systematic histograms must be 
    // created for fakes from data (see below)

    // make output collection of histograms
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap = 
	initHistMap(vars,systematics,processName);

    // make flat vector of variable names (often more convenient then argument structure)
    bool doBDT = false;
    std::vector<std::string> variables;
    for(unsigned int i=0; i<vars.size(); ++i){
	std::string variable = std::get<0>(vars[i]);
	if(variable=="_eventBDT") doBDT=true;
	variables.push_back(variable);
    }

    // make reweighter
    std::shared_ptr< ReweighterFactory >reweighterFactory( new tZqReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
					"../../weights/", year, thissample );

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 10000; 
    numberOfEntries = std::min(numberOfEntries,treeReader.numberOfEntries());
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	
	// initialize map of variables
	std::map<std::string,double> varmap = eventFlattening::initVarMap();

	// fill nominal histograms
	bool passnominal = true;
        Event event = treeReader.buildEvent(entry);
        if(!passES(event, eventselection, selection_type, std::string("nominal"))) passnominal = false;
	if(issignallike){
	    int eventcategory = eventCategory(event, "nominal");
	    if(eventcategory == -1 || eventcategory != signalcategory) passnominal = false;
	}
	if(passnominal){
	    varmap = eventFlattening::eventToEntry(event, norm, reweighter, selection_type, 
			frmap_muon, frmap_electron, "nominal", doBDT, reader);
	    for(std::string variable : variables){
		fillVarValue(histMap[variable][std::string("nominal")],
			    getVarValue(variable,varmap),varmap["_normweight"]);
		// for data carrying a different processName than "data" (e.g. fakes from data),
	        // loop over all systematics and fill with nominal values
		if(event.isData() and processName!="data"){
		    for(auto mapelement : histMap[variable]){
			if(stringTools::stringContains(mapelement.second->GetName(),"nominal")) continue;
			fillVarValue(mapelement.second,getVarValue(variable,varmap),varmap["_normweight"]);
		    }
		}
	    }
	}

	// stop further event processing in case of data
	if(event.isData()) continue;

	// loop over systematics
	for(std::string systematic : systematics){
	    // determine type of systematic (acceptance or weight)
	    std::string sysType = systematicType(systematic);
	    if(sysType=="ignore" || sysType=="ERROR") continue;
	    std::string upvar = systematic + "Up";
	    std::string downvar = systematic + "Down";
	    
	    // IF type is acceptance, special event selections are needed.
	    if(sysType=="acceptance"){
		// do event selection and flattening with up variation
		bool passup = true;
		if(!passES(event, eventselection, selection_type, upvar)) passup = false;
		if(issignallike){
		    int eventcategory = eventCategory(event, upvar);
		    if(eventcategory == -1 || eventcategory != signalcategory) passup = false;
		}
		if(passup){
		    varmap = eventFlattening::eventToEntry(event, norm, reweighter, selection_type, 
				frmap_muon, frmap_electron, upvar, doBDT, reader);
		    for(std::string variable : variables){
			fillVarValue(histMap[variable][upvar],
				    getVarValue(variable,varmap),varmap["_normweight"]);
		    }
		}
		// and with down variation
		bool passdown = true;
		if(!passES(event, eventselection, selection_type, downvar)) passdown = false;
                if(issignallike){
		    int eventcategory = eventCategory(event, downvar);
		    if(eventcategory == -1 || eventcategory != signalcategory) passdown = false;
		}
		if(passdown){
		    varmap = eventFlattening::eventToEntry(event, norm, reweighter, selection_type, 
				frmap_muon, frmap_electron, downvar, doBDT, reader);
		    for(std::string variable : variables){
			fillVarValue(histMap[variable][downvar],
				    getVarValue(variable,varmap),varmap["_normweight"]);
		    }
		}
	    }
	    // ELSE do nominal event selection
	    if(!passnominal) continue;
	    varmap = eventFlattening::eventToEntry(event, norm, reweighter, selection_type, 
			frmap_muon, frmap_electron, "nominal", doBDT, reader);
	    
	    // IF type is weight, apply reweighter with up and down weight
	    if(sysType=="weight"){
		double upweight = varmap["_normweight"];
		double downweight = varmap["_normweight"];
                if(event.isMC()){
		    double reweight = 1./reweighter[systematic]->weight(event);
		    upweight = upweight * reweight * reweighter[systematic]->weightUp(event);
		    downweight = downweight * reweight * reweighter[systematic]->weightDown(event);
		}
		for(std::string variable : variables){
		    fillVarValue(histMap[variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[variable][downvar],getVarValue(variable,varmap),downweight);
		}
	    }
	    // ELSE apply nominal weight (already in varmap["_normweight"])
	   
	    // run over scale systematics
	    if(systematic=="fScale" && event.generatorInfo().numberOfLheWeights()>=9){
		double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_1_MuF_2();
		double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_1_MuF_0p5();
		for(std::string variable : variables){
		    fillVarValue(histMap[variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[variable][downvar],getVarValue(variable,varmap),downweight);
		}
	    }
	    else if(systematic=="rScale" && event.generatorInfo().numberOfLheWeights()>=9){
		double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_2_MuF_1();
                double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_0p5_MuF_1();
		for(std::string variable : variables){
		    fillVarValue(histMap[variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[variable][downvar],getVarValue(variable,varmap),downweight);
		}
	    }
	    else if(systematic=="scales" && event.generatorInfo().numberOfLheWeights()>=9){
		// note: probably fScale, rScale and scales should not be added in quadrature
		// as this contains 'double counting' of variations, need to check how to take into account!
                double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_2_MuF_2();
                double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5();
                for(std::string variable : variables){
		    fillVarValue(histMap[variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[variable][downvar],getVarValue(variable,varmap),downweight);
		}
            }
	    else if(systematic=="isrScale" && event.generatorInfo().numberOfPsWeights()==14){
		double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_ISR_2();
                double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_ISR_0p5();
		for(std::string variable : variables){
		    fillVarValue(histMap[variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[variable][downvar],getVarValue(variable,varmap),downweight);
		}
	    }
	    else if(systematic=="fsrScale" && event.generatorInfo().numberOfPsWeights()==14){
		double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_FSR_2();
                double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_FSR_0p5();
		for(std::string variable : variables){
		    fillVarValue(histMap[variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[variable][downvar],getVarValue(variable,varmap),downweight);
		}
	    }
	    // run over pdf variations
	    else if(systematic=="pdfvar" && event.generatorInfo().numberOfLheWeights()>=109){
		for(std::string variable : variables){
		    for(int i=0; i<100; ++i){
			std::string temp = systematic + std::to_string(i);
			double pdfweight = varmap["_normweight"] * event.generatorInfo().relativeWeightPdfVar(i);
			fillVarValue(histMap[variable][temp],getVarValue(variable,varmap),pdfweight);
		    }
		}
	    }
        }
    }
    // make envelopes for systematics where this is needed
    for( std::string systematic : systematics ){
	if(systematic=="pdfvar"){
	    std::string upvar = "pdfEnvUp";
	    std::string downvar = "pdfEnvDown";
	    for(std::string variable : variables){
		for(int i=1; i<histMap[variable]["nominal"]->GetNbinsX()+1; ++i){
		    histMap[variable][upvar]->SetBinContent(i,histMap[variable]["nominal"]
								->GetBinContent(i));
		    histMap[variable][downvar]->SetBinContent(i,histMap[variable]["nominal"]
								->GetBinContent(i));
		}
		fillEnvelope(histMap, variable, upvar, downvar, "pdfvar");
	    }
	}
    }
    // make output ROOT file
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // save histograms to the output file
    // loop over variables
    for(std::string variable : variables){
	// first find nominal histogram for this variable
	std::shared_ptr<TH1D> nominalhist;
	for(auto mapelement : histMap[variable]){
	    if( stringTools::stringContains(mapelement.second->GetName(),"nominal") ){
		nominalhist = mapelement.second;
	    }
	}
	// if nominal histogram is empty, fill with dummy value (needed for combine);
	// also put all other histograms equal to the nominal one to avoid huge uncertainties
	if( nominalhist->GetEntries()==0 ){
	    nominalhist->SetBinContent(1,1e-6); 
	    for(auto mapelement : histMap[variable]){
		if(stringTools::stringContains(mapelement.second->GetName(),"nominal")) continue;
		copyHistogram(mapelement.second,nominalhist);
	    }
	}
	if(selection_type != "fakerate") clipHistogram(nominalhist);
	nominalhist->Write();
	// loop over rest of histograms
	for(auto mapelement : histMap[variable]){
	    if( stringTools::stringContains(mapelement.second->GetName(),"nominal")){ continue; }
	    std::shared_ptr<TH1D> hist = mapelement.second;
	    // selection: do not store all individual pdf variations
	    if(stringTools::stringContains(hist->GetName(),"pdfvar")) continue;
	    // special treatment of histograms with zero entries or all weights zero
	    if(hist->GetEntries()==0 or std::abs(hist->GetSumOfWeights())<1e-12){
		// use nominal histogram instead
		std::cout<<"### WARNING ###: histogram "<<hist->GetName();
		std::cout<<" contains 0 entries or all weights are 0.."<<std::endl;
		std::cout<<"Either 0 events pass selections";
		std::cout<<" or systematic is not present in sample or all weights are 0."<<std::endl;
		copyHistogram(hist,nominalhist);
	    }
	    // special histograms with zero mean and std, even if they have 'entries' (e.g. pdfEnv)
	    else if(std::abs(hist->GetMean())<1e-12 and std::abs(hist->GetStdDev())<1e-12
		    and stringTools::stringContains(hist->GetName(),"Env")){ 
		std::cout<<"### WARNING ###: envelope histogram "<<hist->GetName();
		std::cout<<" has 0 mean and std."<<std::endl;
		copyHistogram(hist,nominalhist);
	    }
	    // clip histograms to minimum zero
	    if(selection_type != "fakerate") clipHistogram(hist);
	    // save histograms
	    hist->Write();
	}
    }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){
    if( argc < 12 ){
        std::cerr << "### ERROR ###: runsystematics.cc requires at least 11 arguments to run: ";
        std::cerr << "input_file_path, norm, output_file_path, process_name, ";
	std::cerr << "event_selection, signal_category, selection_type, ";
	std::cerr << "frmap_muon, frmap_electron, pathToXMLFile ";
	std::cerr << "at least one systematic" << std::endl;
        return -1;
    }

    // make structure for variables
    std::vector<std::tuple<std::string,double,double,int>> vars;
    vars.push_back(std::make_tuple("_abs_eta_recoil",0.,5.,20));
    vars.push_back(std::make_tuple("_Mjj_max",0.,1200.,20));
    vars.push_back(std::make_tuple("_lW_asymmetry",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_deepCSV_max",0.,1.,20));
    vars.push_back(std::make_tuple("_lT",0.,800.,20));
    vars.push_back(std::make_tuple("_MT",0.,300.,20));
    vars.push_back(std::make_tuple("_pTjj_max",0.,300.,20));
    vars.push_back(std::make_tuple("_dRlb_min",0.,3.15,20));
    vars.push_back(std::make_tuple("_dPhill_max",0.,3.15,20));
    vars.push_back(std::make_tuple("_HT",0.,800.,20));
    vars.push_back(std::make_tuple("_nJets",-0.5,9.5,11));
    vars.push_back(std::make_tuple("_dRlWrecoil",0.,10.,20));
    vars.push_back(std::make_tuple("_dRlWbtagged",0.,7.,20));
    vars.push_back(std::make_tuple("_M3l",0.,600.,20));
    vars.push_back(std::make_tuple("_abs_eta_max",0.,5.,20));
    //vars.push_back(std::make_tuple("_eventBDT",-0.9,0.9,15));
    vars.push_back(std::make_tuple("_nMuons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_nElectrons",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_yield",0.,1.,1));

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    double norm = std::stod(argvStr[2]);
    std::string& output_file_path = argvStr[3];
    std::string& process_name = argvStr[4];
    std::string& event_selection = argvStr[5];
    int signal_category = std::stoi(argvStr[6]); // ignored if event_selection is not signal-like
    std::string& selection_type = argvStr[7];
    std::string& frmap_muon_name = argvStr[8]; // ignored if not data for np background
    std::string& frmap_electron_name = argvStr[9]; // ignored if not data for np background
    std::string& pathToXMLFile = argvStr[10]; // ignored if _eventBDT is not in list of variables
    std::vector<std::string> systematics;
    for(int i=11; i<argc; i++){
	systematics.push_back(argvStr[i]);
    }

    // load fake rate maps if needed
    std::shared_ptr<TH2D> frmap_muon;
    std::shared_ptr<TH2D> frmap_electron;
    if(selection_type=="fakerate"){
	std::string year = "2016";
	if(stringTools::stringContains(input_file_path,"2017")) year = "2017";
	else if(stringTools::stringContains(input_file_path,"2018")) year = "2018";
        frmap_muon = eventFlattening::readFRMap(frmap_muon_name,"muon",year);
        frmap_electron = eventFlattening::readFRMap(frmap_electron_name,"electron",year);
    }
    
    // load a TMVA Reader if needed
    bool doBDT = false;
    for(unsigned int i=0; i<vars.size(); ++i){
        if(std::get<0>(vars[i])=="_eventBDT") doBDT=true;
    }
    TMVA::Reader* reader = new TMVA::Reader();
    if(doBDT){ reader = eventFlattening::initializeReader(reader, pathToXMLFile); }

    // check validity
    bool validInput = checkReadability( input_file_path );
    if(!validInput) return -1;
    for(std::string systematic : systematics){
	std::string testsyst = systematicType(systematic);
	if(testsyst=="ERROR") return -1;
    }
    std::map<std::string,double> emptymap = eventFlattening::initVarMap();
    for(unsigned int i=0; i<vars.size(); ++i){
	std::string varname = std::get<0>(vars[i]);
	if(getVarValue(varname,emptymap)==-9999) return -1;
    }

    // fill the histograms
    fillSystematicsHistograms(input_file_path, norm, output_file_path, process_name,
				    event_selection, signal_category, selection_type,
				    frmap_muon, frmap_electron, 
				    vars, reader, systematics);

    //delete reader;
    std::cout<<"done"<<std::endl;
    return 0;
}
