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
#include "splitSampleTools.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/SampleCrossSections.h"
#include "../../Event/interface/Event.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../../weights/interface/ReweighterBTagShape.h"
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

std::string systematicType(const std::string systematic){
    // get type of sytematic, useful to decide more automatically 
    // and streamlined how they are processed when looping over them.

    // regular JEC, JER and Unclustered energy
    if(systematic=="JEC" or systematic=="JER" or systematic=="Uncl"){
	return std::string("acceptance");
    }
    // split JEC
    if(systematic=="JECAll" or systematic=="JECGrouped"){
	return std::string("jecsplit");
    }
    // reweighting uncertainties 
    if(
	    //systematic=="muonID" // for ttH ID
	    systematic=="muonIDSyst" // for TOP ID
	    or systematic=="muonIDStat" // for TOP ID
	    //or systematic=="electronID" // for ttH ID
	    or systematic=="electronIDSyst" // for TOP ID
	    or systematic=="electronIDStat" // for TOP ID
	    or systematic=="pileup" 
	    or systematic=="bTag_heavy" // old method, replaced by bTag_shape
	    or systematic=="bTag_light" // old method, replaced by bTag_shape
	    or systematic=="prefire"){
	return std::string("weight");
    }
    if(systematic=="bTag_shape") return std::string("bTag_shape");
    // scale uncertainties
    if(	    systematic=="fScale"
	    or systematic=="fScaleNorm"
	    or systematic=="rScale"
	    or systematic=="rScaleNorm"
	    or systematic=="rfScales"
	    or systematic=="rfScalesNorm"
	    //or systematic=="isrScale" // replaced by isrShape and isrNorm
	    //or systematic=="fsrScale" // replaced by fsrShape and fsrNorm
    ) return std::string("scale");
    if(systematic=="pdfShapeVar" or systematic=="pdfNorm"
        or systematic=="qcdScalesShapeVar" or systematic=="qcdScalesNorm"){
        return std::string("lhe"); // special as need to take envelope/RMS
    }
    if(systematic=="isrShape" or systematic=="isrNorm"
	or systematic=="fsrShape" or systematic=="fsrNorm"){
	return std::string("ps"); // normal up and down variation for each of them suffice
    }
    // electron reco uncertainty
    if(systematic=="electronReco") return std::string("electronReco");
    std::cerr<<"### ERROR ###: systematic '"<<systematic<<"' not recognized."<<std::endl;
    return std::string("ERROR");
}

double getVarValue(const std::string varname, std::map<std::string,double> varmap){
    // retrieve a value from a map of all variables associated to the event
    
    // remove "fineBinned" from the varname to allow for e.g. fineBinnedeventBDT to be used
    std::string modvarname = stringTools::removeOccurencesOf(varname,"fineBinned");
    modvarname = stringTools::removeOccurencesOf(modvarname,"coarseBinned");
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

std::map< std::string, std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > > initHistMap(
				const std::vector<std::tuple<std::string,double,double,int>>& vars,
                                const std::vector<std::string>& systematics,
				const std::vector<std::string> processNames,
				unsigned numberOfPdfVariations=100,
				unsigned numberOfQcdScaleVariations=6,
				const std::vector<std::string> allJECVariations={},
				const std::vector<std::string> groupedJECVariations={},
				const std::vector<std::string> bTagShapeSystematics={} ){
    // map of process name to variable to systematic to histogram!
    // process name is mostly only one, but can be multiple if a sample is split in sub-categories
    
    // initialize the output histogram map
    std::map< std::string, std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > > histMap;
    // loop over process names
    for(std::string processName: processNames){
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
	    histMap[processName][variable]["nominal"] = histInfo.makeHist( name+"_nominal" );
	    histMap[processName][variable]["nominal"]->SetTitle(processName.c_str());
	    // loop over systematics
	    for(std::string systematic : systematics){
		// special case for PDF variations: store individual variations 
		// as well as envelope and rms
		if(systematic=="pdfShapeVar"){
		    for(unsigned i=0; i<numberOfPdfVariations; ++i){
			std::string temp = systematic + std::to_string(i);
			histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
			histMap[processName][variable][temp]->SetTitle(processName.c_str());
		    }
		    std::string temp = "pdfShapeEnvUp";
		    histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
		    histMap[processName][variable][temp]->SetTitle(processName.c_str());
		    temp = "pdfShapeEnvDown";
		    histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
		    histMap[processName][variable][temp]->SetTitle(processName.c_str());
		    temp = "pdfShapeRMSUp";
		    histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
		    histMap[processName][variable][temp]->SetTitle(processName.c_str());
		    temp = "pdfShapeRMSDown";
		    histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
		    histMap[processName][variable][temp]->SetTitle(processName.c_str());
		}
		// special case for QCD scales: store 6 variations (see TopSystematics) 
		// as well as envelope
		else if(systematic=="qcdScalesShapeVar"){
		    for(unsigned i=0; i<numberOfQcdScaleVariations; ++i){
			std::string temp = systematic + std::to_string(i);
			histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
			histMap[processName][variable][temp]->SetTitle(processName.c_str());
		    }
		    std::string temp = "qcdScalesShapeEnvUp";
		    histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
		    histMap[processName][variable][temp]->SetTitle(processName.c_str());
		    temp = "qcdScalesShapeEnvDown";
		    histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
		    histMap[processName][variable][temp]->SetTitle(processName.c_str());
		}
		// special case for split JEC variations: store all variations
		else if(systematic=="JECAll" || systematic=="JECGrouped"){
		    std::vector<std::string> varvector;
		    if(systematic=="JECAll") varvector = allJECVariations;
		    else if(systematic=="JECGrouped") varvector = groupedJECVariations;
		    for(std::string jecvar: varvector){
			std::string temp = systematic + "_" + jecvar + "Up";
			histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
			histMap[processName][variable][temp]->SetTitle(processName.c_str());
			temp = systematic + "_" + jecvar + "Down";
			histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
			histMap[processName][variable][temp]->SetTitle(processName.c_str());
		    }
		}
		// special case for bTag shape variations: store multiple systematics
		else if(systematic=="bTag_shape"){
		    for(std::string btagsys: bTagShapeSystematics){
			std::string temp = systematic + "_" + btagsys + "Up";
			histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
			histMap[processName][variable][temp]->SetTitle(processName.c_str());
			temp = systematic + "_" + btagsys + "Down";
			histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
			histMap[processName][variable][temp]->SetTitle(processName.c_str());
		    }
		}
		// now general case: store up and down variation
		else{
		    std::string temp = systematic + "Up";
		    histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
		    histMap[processName][variable][temp]->SetTitle(processName.c_str());
		    temp = systematic + "Down";
		    histMap[processName][variable][temp] = histInfo.makeHist( name+"_"+temp );
		    histMap[processName][variable][temp]->SetTitle(processName.c_str());
		}
	    }
	}
    }
    return histMap;
}

void setBTagShapeNorm( CombinedReweighter reweighter,
			const Sample& sample,
			unsigned long nentries,
			const std::string& event_selection,
			const std::string& selection_type,
			const std::vector<int> signalcategories,
                        const std::vector<int> signalchannels,
			const std::string& topcharge ){
    // normalize the b-tag shape reweighter

    // if no b-tag shape reweighter is present, do nothing
    if( !reweighter.hasReweighter("bTag_shape") ) return;
    std::cout << "normalizing b-tag shape reweighter" << std::endl;

    // determine whether to consider event categorization
    bool doCat = true;
    if(signalcategories.size()==1 && signalcategories[0]==0) doCat = false;
    // determine whether to consider event channels separately
    bool doChannels = true;
    if(signalchannels.size()==1 && signalchannels[0]==4) doChannels = false;
    // determine whether to split top and antitop
    int topCharge = (topcharge=="all")?0:
                    (topcharge=="top")?1:-1;

    // make a TreeReader
    std::string inputFilePath = sample.filePath();
    std::cout << " - making a TreeReader..." << std::endl;
    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );

    // initialize avg weights
    std::map<int,int> npass;
    std::map<int,double> sumweights;
    for( int i=0; i<11; i++){
        npass[i] = 0;
        sumweights[i] = 0;
    }

    // loop over events
    if( nentries==0 ) nentries = treeReader.numberOfEntries();
    long unsigned numberOfEntries = std::min( nentries, treeReader.numberOfEntries() );
    std::cout << " - starting event loop for " << numberOfEntries << " events..." << std::endl;
    for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
        Event event = treeReader.buildEvent( entry );

	// do correct event selection
	// put 'false' in passES and eventCategory to skip selecting b-jets
	if(!passES(event, event_selection, selection_type, "nominal", false)) continue;
        if(doChannels){
            int eventchannel = event.leptonCollection().numberOfMuons();
            if(std::find(signalchannels.begin(),signalchannels.end(),eventchannel)
                == signalchannels.end()){
                continue;
            }
        }
	if( topCharge*lWCharge(event) < -0.5 ){
            continue;
        }
        if(doCat){
            int eventcategory = eventCategory(event, "nominal", false);
	    if( eventcategory<0 ) continue;
        }

	// determine number of jets
        // (must be consistent with how it is determined internally in ReweighterBTagShape...)
        int njets = event.jetCollection().goodJetCollection().size();
        njets = std::min(10,njets);
        
	// determine b-tag reweighting
        sumweights[njets] += reweighter["bTag_shape"]->weight( event );
        npass[njets] += 1;
    }
    
    std::cout << "average weights (binned): " << std::endl;
    for( unsigned int i=0; i<sumweights.size(); i++){
        if( npass[i]==0 ){ sumweights[i] = 1; }
        else{ sumweights[i] /= npass[i]; }
        std::cout << i << ": " << sumweights[i] << " (on " << npass[i] << " events)" << std::endl;
    }
    dynamic_cast<ReweighterBTagShape*>(
            reweighter.getReweighterNonConst("bTag_shape"))->setNormFactors( sample, sumweights );
}

void fillEnvelope( std::map< std::string, std::map< std::string, std::map< std::string,std::shared_ptr<TH1D> >>> histMap, 
		std::string processName, std::string variable, std::string upname, std::string downname, std::string tag ){
    // calculate bin-per-bin envelope of a series of histograms containing "tag" in name
    // note: the envelope entries must be initialized in histMap before this function!
    for( const auto& el : histMap[processName][variable] ){
	if(!stringTools::stringContains(el.first,tag)) continue;
	if(stringTools::stringContains(el.first,upname)) continue;
	if(stringTools::stringContains(el.first,downname)) continue;
	std::shared_ptr<TH1D> hist = el.second;
	for(int i=1; i<hist->GetNbinsX()+1; ++i){
	    if( hist->GetBinContent(i) > histMap[processName][variable][upname]->GetBinContent(i) ){
		histMap[processName][variable][upname]->SetBinContent(i,hist->GetBinContent(i));
	    }
	    if( hist->GetBinContent(i) < histMap[processName][variable][downname]->GetBinContent(i) ){
		histMap[processName][variable][downname]->SetBinContent(i,hist->GetBinContent(i));
	    }
	}
    }
}

void fillRMS( std::map< std::string, std::map< std::string, std::map< std::string,std::shared_ptr<TH1D> >>> histMap,
                std::string processName, std::string variable, std::string upname, std::string downname, std::string tag ){
    // calculate bin-per-bin rms of a series of histograms containing "tag" in name
    // note: the rms entries must be added to the histMap as empty hist before this function!
    // note: the histmap must contain the nominal histograms as well!
    std::shared_ptr<TH1D> nominalHist = histMap[processName][variable]["nominal"];
    std::shared_ptr<TH1D> varRMSHist = std::shared_ptr<TH1D>( dynamic_cast<TH1D*>(
					nominalHist.get()->Clone()) );
    varRMSHist->Reset();
    int nElements = 0;
    for( const auto& el : histMap[processName][variable] ){
        if(!stringTools::stringContains(el.first,tag)) continue;
        if(stringTools::stringContains(el.first,upname)) continue;
        if(stringTools::stringContains(el.first,downname)) continue;
        std::shared_ptr<TH1D> hist = el.second;
	nElements++;
        for(int i=1; i<hist->GetNbinsX()+1; ++i){
	    double var = hist->GetBinContent(i) - nominalHist->GetBinContent(i);
	    varRMSHist->SetBinContent(i,varRMSHist->GetBinContent(i)+var*var);
	}
    }
    // catch exception of zero elements to average over 
    // (the denominator can be changed arbitrarily since numerator is zero as well)
    if( nElements==0 ) nElements=1;
    for(int i=1; i<nominalHist->GetNbinsX()+1; ++i){
        varRMSHist->SetBinContent(i,std::sqrt(varRMSHist->GetBinContent(i)/nElements));
	histMap[processName][variable][upname]->SetBinContent(i,nominalHist->GetBinContent(i)
						    +varRMSHist->GetBinContent(i));
	histMap[processName][variable][downname]->SetBinContent(i,nominalHist->GetBinContent(i)
						    -varRMSHist->GetBinContent(i));
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
				unsigned long nEntries, 
				const std::string& processName,
				const std::string& eventselection,
				const std::string& selection_type,
				const std::vector<int> signalcategories,
				bool splitSamples,
				const std::vector<int> signalchannels,
				const std::string& topcharge,
				const std::shared_ptr<TH2D>& frmap_muon,
				const std::shared_ptr<TH2D>& frmap_electron,
				const std::vector<std::tuple<std::string,double,double,int>> vars,
				TMVA::Reader* reader,
				const double bdtCut,
				std::vector<std::string> systematics){
    // initialize TreeReader from input file
    std::cout << "=== start function fillSystematicsHistograms ===\n\n\n";
    std::cout << "- input file: " << pathToFile << "\n";
    std::cout << "- process name: " << processName << "\n";
    std::cout << "- event selection " << eventselection << "(" << selection_type << ")\n\n";
    std::cout << "initializing TreeReader...\n\n";
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";

    // determine whether to consider event categorization
    bool doCat = true;
    if(signalcategories.size()==1 && signalcategories[0]==0) doCat = false;

    // determine whether to consider event channels separately
    bool doChannels = true;
    if(signalchannels.size()==1 && signalchannels[0]==4) doChannels = false;

    // determine whether to split top and antitop
    int topCharge = (topcharge=="all")?0:
		    (topcharge=="top")?1:-1;

    // when considering the data sample, disregard all systematics
    if(processName=="data") systematics.clear();
    // warning: do not use isData() in the condition above, as systematic histograms must be 
    // created for fakes from data (see below)

    // determine split process names (per sub-category) for this sample
    // see splitSampleTools!
    std::vector< std::string > processNames;
    processNames.push_back( processName );
    if( splitSamples ){
	processNames = splitProcessNames( processName );
	std::cout << "will split this sample into:" << std::endl;
	for( std::string pn: processNames ) std::cout << "- " << pn << std::endl;
	std::cout << " " << std::endl;
    }

    // make flat vector of variable names (often more convenient then argument structure)
    bool doBDT = false;
    std::vector<std::string> variables;
    for(unsigned int i=0; i<vars.size(); ++i){
	std::string variable = std::get<0>(vars[i]);
	if(variable=="_eventBDT") doBDT=true;
	variables.push_back(variable);
    }

    // make reweighter
    std::cout << "initializing Reweighter...\n\n";
    std::shared_ptr< ReweighterFactory> reweighterFactory;
    // NORMAL //
    if( LeptonSelector::leptonID()=="tzqtight" || LeptonSelector::leptonID()=="tzqmedium0p4" ){ 
	reweighterFactory = std::shared_ptr<ReweighterFactory>(new tZqReweighterFactory()); 
	// (tZqReweighterFactory internally checks for medium or tight ID !)
    } else if( LeptonSelector::leptonID()=="tth"){
	reweighterFactory = std::shared_ptr<ReweighterFactory>(new EwkinoReweighterFactory());
    } else{
	std::cerr << "ERROR: reweighter for this lepton ID not yet implemented" << std::endl;
	// (will cause nullpointerexception?)
    }
    // FOR TESTING //
    //reweighterFactory = std::shared_ptr<ReweighterFactory>( new EmptyReweighterFactory() );
    // FOR TESTING (2) //
    //reweighterFactory = std::shared_ptr<ReweighterFactory>( new BTagShapeReweighterFactory() );

    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
					"../../weights/", year, thissample );
    // normalize the bTag shape reweighter if present
    //std::shared_ptr<ReweighterBTagShape> reweighterBTagShape;
    bool hasBTagShapeReweighter = reweighter.hasReweighter("bTag_shape");
    if( hasBTagShapeReweighter ){
	setBTagShapeNorm( reweighter, treeReader.currentSample(), nEntries, eventselection,
			    selection_type, signalcategories, signalchannels, topcharge );
	// keep separate pointer to this reweighter as it has a special signature
	// seems to give invalid pointer exceptions, instead do dynamic_cast everywhere you need it
	//reweighterBTagShape = std::shared_ptr<ReweighterBTagShape>( 
	//    dynamic_cast<ReweighterBTagShape*>( 
	//    reweighter.getReweighterNonConst("bTag_shape") ) );
    }
    
    // determine global sample properties related to pdf and scale variations
    unsigned numberOfScaleVariations = 0;
    unsigned numberOfPdfVariations = 0;
    std::shared_ptr< SampleCrossSections > xsecs;
    std::vector< double > qcdScalesXSecRatios;
    double qcdScalesMinXSecRatio = 1.;
    double qcdScalesMaxXSecRatio = 1.;
    std::vector<double> pdfXSecRatios;
    double pdfMinXSecRatio = 1.;
    double pdfMaxXSecRatio = 1.;
    bool considerlhe = false; // check whether lhe parameters are needed
    for( std::string systematic : systematics){
	if( systematicType(systematic)=="lhe" ){ considerlhe=true; break; }
    }
    bool hasValidPdfs = false; // global switch to use later on in the event loop
    bool hasValidQcds = false; // global switch to use later on in the event loop
    if( treeReader.numberOfEntries()>0 
	&& considerlhe 
	&& !treeReader.isData() ){
	std::cout << "finding available PDF and QCD scale variations...\n";
	Event event = treeReader.buildEvent(0); 
        numberOfScaleVariations = event.generatorInfo().numberOfScaleVariations();
	numberOfPdfVariations = event.generatorInfo().numberOfPdfVariations();	
	// make vector of cross-section modifications due to pdf and qcd scale variations
	// the vectors contain the ratio of the varied cross-section to the nominal one
	xsecs = std::make_shared<SampleCrossSections>( treeReader.currentSample() );
	if(numberOfScaleVariations==9){
	    hasValidQcds = true;
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_1_MuF_2() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_2_MuF_1() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_2_MuF_2() );
	    qcdScalesXSecRatios.push_back( xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5() );
	    // note: order doesnt matter here as it is only used for min and max calculation
	    qcdScalesMinXSecRatio = *std::min_element( qcdScalesXSecRatios.begin(), 
							qcdScalesXSecRatios.end() );
	    qcdScalesMaxXSecRatio = *std::max_element( qcdScalesXSecRatios.begin(), 
							qcdScalesXSecRatios.end() );
	}
	if(numberOfPdfVariations>=100){
	    hasValidPdfs = true;
	    for(unsigned i=0; i<numberOfPdfVariations; i++){ 
		pdfXSecRatios.push_back( xsecs.get()->crossSectionRatio_pdfVar(i) ); }
	    pdfMinXSecRatio = *std::min_element( pdfXSecRatios.begin(), pdfXSecRatios.end() );
	    pdfMaxXSecRatio = *std::max_element( pdfXSecRatios.begin(), pdfXSecRatios.end() );
	}
	std::cout << "- hasValidQcds: " << hasValidQcds << "\n";
	std::cout << "- hasValidPdfs: " << hasValidPdfs << "\n\n";
    }

    // determine global sample properties related to ISR/FSR variations
    unsigned numberOfPSVariations = 0;
    //std::shared_ptr< SampleCrossSections > xsecs; // already declared for ME and PDF variations
    bool considerps = false; // check whether parton shower parameters are needed
    for( std::string systematic : systematics){
        if( systematicType(systematic)=="ps" ){ considerps=true; break; }
    }
    bool hasValidPSs = false; // global switch to use later on in the event loop
    if( treeReader.numberOfEntries()>0
        && considerps
        && !treeReader.isData() ){
        std::cout << "finding available PS scale variations...\n";
        Event event = treeReader.buildEvent(0);
        numberOfPSVariations = event.generatorInfo().numberOfPsWeights();
        if(numberOfPSVariations==14){ hasValidPSs = true; }
        std::cout << "- hasValidPSs: " << hasValidPSs << "\n";
    }

    // determine global sample properties related to split JEC variations
    std::vector<std::string> allJECVariations;
    std::vector<std::string> groupedJECVariations;
    bool considerjecall = false; // check whether all jec variations are needed
    bool considerjecgrouped = false; // check whether grouped jec variations are needed
    for( std::string systematic: systematics ){
	if( systematic=="JECAll") considerjecall = true;
	else if( systematic=="JECGrouped" ) considerjecgrouped = true;
    }
    if( treeReader.numberOfEntries()>0
	&& (considerjecall || considerjecgrouped) ){
	std::cout << "finding available JEC uncertainty sources...\n";
	Event event = treeReader.buildEvent(0,false,false,considerjecall,considerjecgrouped);
	allJECVariations = event.jetInfo().allValidJECVariations();
	groupedJECVariations = event.jetInfo().groupedValidJECVariations();
	event.jetInfo().printAllValidJECVariations();
	event.jetInfo().printGroupedValidJECVariations();
	std::cout << "\n";
    }

    // determine global sample properties related to bTag shape systematics
    std::vector<std::string> bTagShapeSystematics;
    bool considerbtagshape = (std::find(systematics.begin(),systematics.end(),"bTag_shape")
				!=systematics.end());
    if( considerbtagshape && !hasBTagShapeReweighter ){
	throw std::invalid_argument( std::string("### ERROR ### in runsystematics: ")
		+ "bTag_shape uncertainties requested, but no bTag_shape reweighter present" );
    }
    if( considerbtagshape ){
	bTagShapeSystematics = dynamic_cast<const ReweighterBTagShape*>( 
	    reweighter["bTag_shape"] )->availableSystematics();
    }

    // make output collection of histograms
    std::cout<<"making output collection of histograms...\n\n";
    std::map< std::string, std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > > histMap =
        initHistMap(vars,systematics,processNames,
		    numberOfPdfVariations,6,
		    allJECVariations,groupedJECVariations,
		    bTagShapeSystematics);

    // do event loop
    if( nEntries==0 ) nEntries = treeReader.numberOfEntries();
    long unsigned numberOfEntries = std::min( nEntries, treeReader.numberOfEntries() );
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	
	// initialize map of variables
	std::map<std::string,double> varmap = eventFlattening::initVarMap();
	// (store nominal event variables, call only once and use for all weight systematics)
	std::map<std::string,double> accvarmap = eventFlattening::initVarMap();
	// (store acceptance-modified event variables, can be overwritten per acceptance variation)

	// fill nominal histograms
	bool passnominal = true;
        Event event = treeReader.buildEvent(entry,false,false,
			considerjecall,considerjecgrouped);
        if(!passES(event, eventselection, selection_type, "nominal")) passnominal = false;
	if(doChannels){
	    int eventchannel = event.leptonCollection().numberOfMuons();
	    if(std::find(signalchannels.begin(),signalchannels.end(),eventchannel)
                == signalchannels.end()){
                continue;
		// note: here we can simply continue instead of put passnominal to false,
		// since alternative selections (due to e.g. JEC variations) cannot change 
		// the number of muons!
            } 
	}
	if( topCharge*lWCharge(event) < -0.5 ){ 
	    continue;
	    // note: here we can simply continue instead of put passnominal to false,
	    // since alternative selections (due to e.g. JEC variations) cannot change 
            // the lW charge!

	}
	if(doCat){
	    int eventcategory = eventCategory(event, "nominal");
	    if(std::find(signalcategories.begin(),signalcategories.end(),eventcategory) 
		== signalcategories.end()){
		passnominal = false;
	    }
	}
	// determine the sub-category to which this event belongs
	// note: assume this does not depend on jet content,
	// else need to determine it separately for each jet variation!
	// note: see splitSampleTools!
	std::string thisPName = (splitSamples)?splitProcessName( processName, event ):processName;
	if(passnominal){
	    varmap = eventFlattening::eventToEntry(event, norm, reweighter, selection_type, 
					frmap_muon, frmap_electron, "nominal", doBDT, reader);
	    // allow for additional selection on final BDT score
	    // note: in principle this should be re-evaluated for every jec systematic,
	    // but for now just do a cut-and-continue on nominal.
	    if( bdtCut > -1 ){
		if( varmap["_eventBDT"]<bdtCut ) continue;
	    }
	    for(std::string variable : variables){
		fillVarValue(histMap[thisPName][variable]["nominal"],
			    getVarValue(variable,varmap),varmap["_normweight"]);
		// for data carrying a different processName than "data" (e.g. fakes from data),
	        // loop over all systematics and fill with nominal values
		if(event.isData() and processName!="data"){
		    for(auto mapelement : histMap[thisPName][variable]){
			if(stringTools::stringContains(mapelement.second->GetName(),"nominal")){
			    continue;
			}
			fillVarValue(mapelement.second,getVarValue(variable,varmap),
					varmap["_normweight"]);
		    }
		}
	    }
	}

	// stop further event processing in case of data
	if(event.isData()) continue;

	// ad-hoc fix for pileup issue in 2017 tZq and ttHToNonbb samples
	// (small number of events get pileup weight 0 due to wrong pilup distribution)
	// and for bTag shape issue in 2017 tZq sample 
	// (small number of events get bTag_shape weight 0, origin unknown)
	std::vector<std::string> reweightersToCheck = {"pileup","bTag_shape"};
	bool skipevent = false;
	for(std::string r: reweightersToCheck){
	    if(reweighter.hasReweighter(r)){
		if(fabs(reweighter[r]->weight( event ))<1e-12){
		    //std::cout << "zero reweighting found for reweighter " << r << std::endl;
		    skipevent = true; break;
		}
	    }
	}
	if(skipevent) continue;

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
		if(doCat){
		    int eventcategory = eventCategory(event, upvar);
		    if( std::find(signalcategories.begin(),signalcategories.end(),
			eventcategory)==signalcategories.end()){ passup = false; }
		}
		if(passup){
		    accvarmap = eventFlattening::eventToEntry(event, norm, reweighter, 
				    selection_type, frmap_muon, frmap_electron, upvar, doBDT, 
				    reader);
		    double weight = accvarmap["_normweight"];
		    // for JEC: propagate into b-tag shape reweighting
		    if( systematic=="JEC" && considerbtagshape ){
			weight *= dynamic_cast<const ReweighterBTagShape*>( 
				    reweighter["bTag_shape"] )->weightJecVar( event, "JECUp" )
				    / reweighter["bTag_shape"]->weight( event );
		    }
		    // fill the variables
		    for(std::string variable : variables){
			fillVarValue(histMap[thisPName][variable][upvar],
				    getVarValue(variable,accvarmap),weight);
		    }
		}
		// and with down variation
		bool passdown = true;
		if(!passES(event, eventselection, selection_type, downvar)) passdown = false;
                if(doCat){
		    int eventcategory = eventCategory(event, downvar);
		    if( std::find(signalcategories.begin(),signalcategories.end(),
			eventcategory)==signalcategories.end()){ passdown = false; }
		}
		if(passdown){
		    accvarmap = eventFlattening::eventToEntry(event, norm, reweighter, 
				    selection_type, frmap_muon, frmap_electron, downvar, doBDT, 
				    reader);
		    double weight = accvarmap["_normweight"];
		    // for JEC: propagate into b-tag shape reweighting
                    if( systematic=="JEC" && considerbtagshape ){
                        weight *= dynamic_cast<const ReweighterBTagShape*>(
                                    reweighter["bTag_shape"] )->weightJecVar( event, "JECDown" )
                                    / reweighter["bTag_shape"]->weight( event );
                    }
                    // fill the variables
		    for(std::string variable : variables){
			fillVarValue(histMap[thisPName][variable][downvar],
				    getVarValue(variable,accvarmap),weight);
		    }
		}
		// skip checking other systematics as they are mutually exclusive
		continue;
	    }
	    // IF type is jecsplit, special event selections are needed as well
	    else if(sysType=="jecsplit"){
		std::vector<std::string> varvector;
		if(systematic=="JECAll") varvector = allJECVariations;
		else if(systematic=="JECGrouped") varvector = groupedJECVariations;
		for(std::string jecvar: varvector){
		    std::string thisupvar = jecvar+"Up";
		    std::string thisdownvar = jecvar+"Down";
		    // do event selection and flattening with up variation
		    bool passup = true;
		    if(!passES(event, eventselection, selection_type, thisupvar)) passup = false;
		    if(doCat){
			int eventcategory = eventCategory(event, thisupvar);
			if( std::find(signalcategories.begin(),signalcategories.end(),
			    eventcategory)==signalcategories.end()){ passup = false; }
		    }
		    if(passup){
			accvarmap = eventFlattening::eventToEntry(event, norm, reweighter, 
				    selection_type, frmap_muon, frmap_electron, thisupvar, 
				    doBDT, reader);
			double weight = accvarmap["_normweight"];
			// for JEC: propagate into b-tag shape reweighting
			if( considerbtagshape && jecvar!="RelativeSample" ){
			    std::string jesvar = "jes"+jecvar; // for checking if valid
			    if( dynamic_cast<const ReweighterBTagShape*>(
                                reweighter["bTag_shape"] )->hasVariation( jesvar) ){
				weight *= dynamic_cast<const ReweighterBTagShape*>(
					reweighter["bTag_shape"] )->weightJecVar( event, thisupvar )
					/reweighter["bTag_shape"]->weight( event );
			    } else{
				std::cerr << "WARNING: variation '"<<jecvar<<"' for bTag shape";
				std::cerr << "reweighter but not recognized" << std::endl;
			    }
			}
			// fill the variables
			for(std::string variable : variables){
			    fillVarValue(histMap[thisPName][variable][systematic+"_"+thisupvar],
                                    getVarValue(variable,accvarmap),weight);
			}
		    }
		    // and with down variation
		    bool passdown = true;
		    if(!passES(event, eventselection, selection_type, thisdownvar)) passdown=false;
		    if(doCat){
			int eventcategory = eventCategory(event, thisdownvar);
			if( std::find(signalcategories.begin(),signalcategories.end(),
			    eventcategory) == signalcategories.end()){ passdown=false; }
		    }
		    if(passdown){
			accvarmap = eventFlattening::eventToEntry(event, norm, reweighter, 
				selection_type, frmap_muon, frmap_electron, thisdownvar, 
				doBDT, reader);
			double weight = accvarmap["_normweight"];
                        // for JEC: propagate into b-tag shape reweighting
                        if( considerbtagshape && jecvar!="RelativeSample" ){
			    std::string jesvar = "jes"+jecvar; // for checking if valid
                            if( dynamic_cast<const ReweighterBTagShape*>(
                                reweighter["bTag_shape"] )->hasVariation( jesvar) ){
                                weight *= dynamic_cast<const ReweighterBTagShape*>(
					reweighter["bTag_shape"] )->weightJecVar( event,thisdownvar )
                                        /reweighter["bTag_shape"]->weight( event );
                            } else{
                                std::cerr << "WARNING: variation '"<<jesvar<<"' for bTag shape";
                                std::cerr << "reweighter but not recognized" << std::endl;
                            }
                        }
			for(std::string variable : variables){
			    fillVarValue(histMap[thisPName][variable][systematic+"_"+thisdownvar],
                                    getVarValue(variable,accvarmap),accvarmap["_normweight"]);
			}
		    }
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    // ELSE do nominal event selection
	    else if(!passnominal) continue;
	    // (nominal event already stored in varmap)
	    
	    // IF type is weight, apply reweighter with up and down weight
	    if(sysType=="weight"){
		double upweight = varmap["_normweight"];
		double downweight = varmap["_normweight"];
		double reweight = 1./reweighter[systematic]->weight(event);
		upweight = upweight * reweight * reweighter[systematic]->weightUp(event);
		downweight = downweight * reweight * reweighter[systematic]->weightDown(event);
		for(std::string variable : variables){
		    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
		}
		//if(systematic=="pileup"){
		//    if( isnan(upweight) ){
		//	std::cout << "nominal: " << varmap["_normweight"] << std::endl;
		//	std::cout << "up: " << upweight << std::endl;
		//    }
		//}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    // special case for bTag_shape reweighting (several systematics)
	    else if(systematic=="bTag_shape"){
		double nombweight = reweighter["bTag_shape"]->weight( event );
		for(std::string btagsys: bTagShapeSystematics){
		    double upweight = varmap["_normweight"] / nombweight
					* dynamic_cast<const ReweighterBTagShape*>(
					    reweighter["bTag_shape"])->weightUp( event, btagsys );
		    double downweight = varmap["_normweight"] / nombweight
                                        * dynamic_cast<const ReweighterBTagShape*>(
                                            reweighter["bTag_shape"])->weightDown( event, btagsys );
		    for(std::string variable : variables){
			fillVarValue(histMap[thisPName][variable][systematic+"_"+btagsys+"Up"],
			    getVarValue(variable,varmap),upweight);
			fillVarValue(histMap[thisPName][variable][systematic+"_"+btagsys+"Down"],
			    getVarValue(variable,varmap),downweight);
		    }
		}
	    }
	    // ELSE apply nominal weight (already in varmap["_normweight"])
	    else if(systematic=="fScale" && hasValidQcds){
		double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_1_MuF_2()
							    / xsecs.get()->crossSectionRatio_MuR_1_MuF_2();
		double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_1_MuF_0p5()
							    / xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5();
		// prints for testing:
		/*std::cout << "nominal weight: " << varmap["_normweight"] << std::endl;
		std::cout << "fScale:" << std::endl;
		std::cout << "  relative upweight: ";
		std::cout << event.generatorInfo().relativeWeight_MuR_1_MuF_2() << std::endl;
		std::cout << "  relative downweight ";
		std::cout << event.generatorInfo().relativeWeight_MuR_1_MuF_0p5() << std::endl;*/
		for(std::string variable : variables){
		    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="fScaleNorm" && hasValidQcds){
		double upweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_MuR_1_MuF_2();
                double downweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5();
                for(std::string variable : variables){
                    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
                    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rScale" && hasValidQcds){
		double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_2_MuF_1()
							    / xsecs.get()->crossSectionRatio_MuR_2_MuF_1();
                double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_0p5_MuF_1()
							    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1();
		// prints for testing:
		/*std::cout << "nominal weight: " << varmap["_normweight"] << std::endl;
		std::cout << "rScale:" << std::endl;
                std::cout << "  relative upweight: ";
		std::cout << event.generatorInfo().relativeWeight_MuR_2_MuF_1() << std::endl;
                std::cout << "  relative downweight ";
		std::cout << event.generatorInfo().relativeWeight_MuR_0p5_MuF_1() << std::endl;*/
		for(std::string variable : variables){
		    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rScaleNorm" && hasValidQcds){
                double upweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_MuR_2_MuF_1();
                double downweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1();
                for(std::string variable : variables){
                    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
                    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="rfScales" && hasValidQcds){
                double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_2_MuF_2()
							    / xsecs.get()->crossSectionRatio_MuR_2_MuF_2();
                double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5()
							    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5();
                for(std::string variable : variables){
                    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
                    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="rfScalesNorm" && hasValidQcds){
                double upweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_MuR_2_MuF_2();
                double downweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5();
                for(std::string variable : variables){
                    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
                    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="isrShape" && hasValidPSs){
		double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_ISR_2()
							    / xsecs.get()->crossSectionRatio_ISR_2();
                double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_ISR_0p5()
							    / xsecs.get()->crossSectionRatio_ISR_0p5();
		// prints for testing:
		/*std::cout << "--- event ---" << std::endl;
		std::cout << varmap["_normweight"] << std::endl;
		std::cout << upweight << std::endl;
		std::cout << downweight << std::endl;*/
		for(std::string variable : variables){
		    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="isrNorm" && hasValidPSs){
                double upweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_ISR_2();
                double downweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_ISR_0p5();
                for(std::string variable : variables){
                    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
                    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="fsrShape" && hasValidPSs){
		double upweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_FSR_2()
							    / xsecs.get()->crossSectionRatio_FSR_2();
                double downweight = varmap["_normweight"] * event.generatorInfo().relativeWeight_FSR_0p5()
							    / xsecs.get()->crossSectionRatio_FSR_0p5();
		for(std::string variable : variables){
		    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="fsrNorm" && hasValidPSs){
                double upweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_FSR_2();
                double downweight = varmap["_normweight"] * xsecs.get()->crossSectionRatio_FSR_0p5();
                for(std::string variable : variables){
                    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
                    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
                }
                // skip checking other systematics as they are mutually exclusive
                continue;
            }

	    // apply electron reco uncertainty
	    else if(systematic=="electronReco"){
		double upweight = varmap["_normweight"];
		double downweight = varmap["_normweight"];
		if( !event.is2018() ){
		    downweight *= reweighter[ "electronReco_pTBelow20" ]->weightDown( event ) 
				  * reweighter[ "electronReco_pTAbove20" ]->weightDown( event ) 
				  / ( reweighter[ "electronReco_pTBelow20" ]->weight( event ) 
				      * reweighter[ "electronReco_pTAbove20" ]->weight( event ) );
		    upweight *= reweighter[ "electronReco_pTBelow20" ]->weightUp( event ) 
				* reweighter[ "electronReco_pTAbove20" ]->weightUp( event ) 
				/ ( reweighter[ "electronReco_pTBelow20" ]->weight( event ) 
				    * reweighter[ "electronReco_pTAbove20" ]->weight( event ) );
		} else {
		    downweight *= reweighter[ "electronReco" ]->weightDown( event ) 
				    / ( reweighter[ "electronReco" ]->weight( event ) );
		    upweight *= reweighter[ "electronReco" ]->weightUp( event ) 
				    / ( reweighter[ "electronReco" ]->weight( event ) );
		}
		for(std::string variable : variables){
                    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
                    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }

	    // run over qcd scale variations to calculate envolope later
	    else if(systematic=="qcdScalesShapeVar" && hasValidQcds){
                std::vector<double> qcdvariations;
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_2_MuF_1()
					    / xsecs.get()->crossSectionRatio_MuR_2_MuF_1() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_0p5_MuF_1()
					    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_1() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_2_MuF_2()
					    / xsecs.get()->crossSectionRatio_MuR_2_MuF_2() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_1_MuF_2()
					    / xsecs.get()->crossSectionRatio_MuR_1_MuF_2() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_1_MuF_0p5()
					    / xsecs.get()->crossSectionRatio_MuR_1_MuF_0p5() );
                qcdvariations.push_back( event.generatorInfo().relativeWeight_MuR_0p5_MuF_0p5()
					    / xsecs.get()->crossSectionRatio_MuR_0p5_MuF_0p5() );
		for(unsigned i=0; i<qcdvariations.size(); ++i){
                    std::string temp = systematic + std::to_string(i);
		    double reweight = qcdvariations[i];
                    double qcdweight = varmap["_normweight"] * reweight;
		    for(std::string variable : variables){
                        fillVarValue(histMap[thisPName][variable][temp],getVarValue(variable,varmap),qcdweight);
		    }
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
            }
	    else if(systematic=="qcdScalesNorm" && hasValidQcds){
		for(std::string variable : variables){
		    double upweight = varmap["_normweight"]*qcdScalesMaxXSecRatio;
		    double downweight = varmap["_normweight"]*qcdScalesMinXSecRatio;
		    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
		    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    // run over pdf variations to calculate envelope later
	    else if(systematic=="pdfShapeVar" && hasValidPdfs){
		for(unsigned i=0; i<numberOfPdfVariations; ++i){
                    std::string temp = systematic + std::to_string(i);
		    double reweight = event.generatorInfo().relativeWeightPdfVar(i)
                                        / xsecs.get()->crossSectionRatio_pdfVar(i);
                    double pdfweight = varmap["_normweight"] * reweight;
		    for(std::string variable : variables){
			fillVarValue(histMap[thisPName][variable][temp],getVarValue(variable,varmap),pdfweight);
		    }
		    // for testing: 
                    /*double relweight = event.generatorInfo().relativeWeightPdfVar(i);
                    double xsecrat = xsecs.get()->crossSectionRatio_pdfVar(i);
		    double reweight = relweight / xsecrat;
                    if(reweight<0){
                        std::cout << "event: " << event.eventNumber() << ", ";
                        std::cout << "variation: " << i << ": ";
                        std::cout << "relative weight: " << relweight << ", ";
			std::cout << "xsec ratio: " << xsecrat << std::endl;
                    }*/
		}
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
	    else if(systematic=="pdfNorm" && hasValidPdfs){
		for(std::string variable : variables){
                    double upweight = varmap["_normweight"]*pdfMaxXSecRatio;
                    double downweight = varmap["_normweight"]*pdfMinXSecRatio;
                    fillVarValue(histMap[thisPName][variable][upvar],getVarValue(variable,varmap),upweight);
                    fillVarValue(histMap[thisPName][variable][downvar],getVarValue(variable,varmap),downweight);
                }
		// skip checking other systematics as they are mutually exclusive
                continue;
	    }
        } // end loop over systematics
    } // end loop over events

    // make envelopes and/or RMS for systematics where this is needed
    for( std::string thisPName: processNames ){
	for( std::string systematic : systematics ){
	    if(systematic=="pdfShapeVar"){
		std::string upvar = "pdfShapeEnvUp";
		std::string downvar = "pdfShapeEnvDown";
		for(std::string variable : variables){
		    // first initialize the up and down variations to be equal to nominal
		    // (needed for correct envelope computation)
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			histMap[thisPName][variable][upvar]->SetBinContent(i,histMap[thisPName][variable]["nominal"]
								->GetBinContent(i));
			histMap[thisPName][variable][downvar]->SetBinContent(i,histMap[thisPName][variable]["nominal"]
								->GetBinContent(i));
		    }
		    // print for testing
		    /*std::cout << variable << " before enveloping" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			std::cout << "bin " << i << std::endl;
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
		    }*/
		    // then fill envelope in case valid pdf variations are present
		    if(hasValidPdfs){ fillEnvelope(histMap, thisPName, variable, upvar, downvar, "pdfShapeVar"); }
		    // print for testing
		    /*std::cout << variable << " after enveloping" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			std::cout << "bin " << i << std::endl;
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
		    }*/
		}
		upvar = "pdfShapeRMSUp";
		downvar = "pdfShapeRMSDown";
		for(std::string variable : variables){
		    // first initialize the up and down variations to be equal to nominal
		    // (needed for correct envelope computation)
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			histMap[thisPName][variable][upvar]->SetBinContent(i,histMap[thisPName][variable]["nominal"]
                                                                ->GetBinContent(i));
			histMap[thisPName][variable][downvar]->SetBinContent(i,histMap[thisPName][variable]["nominal"]
                                                                ->GetBinContent(i));
		    }
		    // print for testing
		    /*std::cout << variable << " before rmsing" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			std::cout << "bin " << i << std::endl;
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
		    }*/
		    // then fill rms in case valid pdf variations are present
		    if(hasValidPdfs){ fillRMS(histMap, thisPName, variable, upvar, downvar, "pdfShapeVar"); }
		    // print for testing
		    /*std::cout << variable << " after rmsing" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			std::cout << "bin " << i << std::endl;
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl;
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl;
		    }*/
		}
	    }
	    else if(systematic=="qcdScalesShapeVar"){
		std::string upvar = "qcdScalesShapeEnvUp";
		std::string downvar = "qcdScalesShapeEnvDown";
		for(std::string variable : variables){
		    // first initialize the up and down variations to be equal to nominal
		    // (needed for correct envelope computation)
		    for(int i=1; i<histMap[thisPName][variable]["nominal"]->GetNbinsX()+1; ++i){
			histMap[thisPName][variable][upvar]->SetBinContent(i,histMap[thisPName][variable]["nominal"]
                                                                ->GetBinContent(i));
			histMap[thisPName][variable][downvar]->SetBinContent(i,histMap[thisPName][variable]["nominal"]
                                                                ->GetBinContent(i));
		    }
		    // prints for testing:
		    /*std::cout << "---------------------" << std::endl;
		    std::cout << "variable: " << variable << std::endl;
		    std::cout << "scale up:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][upvar]->GetNbinsX()+1; ++i){
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl; }
		    std::cout << "scale down:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][downvar]->GetNbinsX()+1; ++i){ 
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl; } */
		    // then fill envelope in case valid qcd variations are present
		    if(hasValidQcds){ 
			fillEnvelope(histMap, thisPName, variable, upvar, downvar, "qcdScalesShapeVar"); }
		    // prints for testing
		    /*std::cout << "scale up:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][upvar]->GetNbinsX()+1; ++i){
			std::cout << histMap[thisPName][variable][upvar]->GetBinContent(i) << std::endl; }
		    std::cout << "scale down:" << std::endl;
		    for(int i=1; i<histMap[thisPName][variable][downvar]->GetNbinsX()+1; ++i){
			std::cout << histMap[thisPName][variable][downvar]->GetBinContent(i) << std::endl; } */
		}
	    }
	}
    } // end loop over systematics to fill envelope and/or RMS

    // make output ROOT file
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // save histograms to the output file
    for( std::string thisPName: processNames ){
	// loop over variables
	for(std::string variable : variables){
	    // need to distinguish between normal histograms and finely binned ones
	    // the latter will be rebinned in a later stage
	    // to do this correctly, they must not be clipped and all pdf and qcd variations are needed
	    bool storeLheVars = false;
	    bool doClip = true;
	    if( stringTools::stringContains(variable,"fineBinned") ){
		storeLheVars = true;
		doClip = false;
	    }
	    if( selection_type == "fakerate") doClip = false;
	    // first find nominal histogram for this variable
	    std::shared_ptr<TH1D> nominalhist;
	    for(auto mapelement : histMap[thisPName][variable]){
		if( stringTools::stringContains(mapelement.second->GetName(),"nominal") ){
		    nominalhist = mapelement.second;
		}
	    }
	    // if nominal histogram is empty, fill with dummy value (needed for combine);
	    // also put all other histograms equal to the nominal one to avoid huge uncertainties
	    if( nominalhist->GetEntries()==0 ){
		nominalhist->SetBinContent(1,1e-6); 
		for(auto mapelement : histMap[thisPName][variable]){
		    if(stringTools::stringContains(mapelement.second->GetName(),"nominal")) continue;
		    copyHistogram(mapelement.second,nominalhist);
		}
	    }
	    // clip and write nominal histogram
	    if( doClip ) clipHistogram(nominalhist);
	    nominalhist->Write();
	    // loop over rest of histograms
	    for(auto mapelement : histMap[thisPName][variable]){
		if( stringTools::stringContains(mapelement.second->GetName(),"nominal")){ continue; }
		std::shared_ptr<TH1D> hist = mapelement.second;
		// selection: do not store all individual pdf variations
		if(stringTools::stringContains(hist->GetName(),"pdfShapeVar") 
		   && !storeLheVars) continue;
		// selection: do not store all individual qcd scale variations
		if(stringTools::stringContains(hist->GetName(),"qcdScalesShapeVar")
		   && !storeLheVars) continue;
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
		if( doClip ) clipHistogram(hist);
		// save histograms
		hist->Write();
	    }
	}
    }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc < 16 ){
        std::cerr << "### ERROR ###: runsystematics.cc requires at least 14 arguments to run...: ";
        std::cerr << "input_file_path, norm, output_file_path, nentries, process_name, ";
	std::cerr << "event_selection, selection_type, signal_categories, "; 
	std::cerr << "split_samples, signal_channels, topcharge, ";
	std::cerr << "bdtCombineMode, pathToXMLFile, bdtCut, ";
	std::cerr << "at least one systematic" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1]; // path to input file to be processed
    double norm = std::stod(argvStr[2]); // normalization factor, i.e. xsec*lumi/hCounter
    std::string& output_file_path = argvStr[3]; // output file to put the result in
    long unsigned nentries = std::stol(argvStr[4]); // number of entries to process (0 for all)
    std::string& process_name = argvStr[5]; // tag to identify this process by
    std::string& event_selection = argvStr[6]; // event selection to perform (see event selection code)
    std::string& selection_type = argvStr[7]; // type of event selection (see event selection code)
    std::string& signal_catstring = argvStr[8]; // category/categrories to consider in selection
    // convert string to vector of ints
    std::vector< int > signal_categories;
    for(char c: signal_catstring){ signal_categories.push_back( (int)c-(int)'0' ); }
    bool split_samples = (argvStr[9]=="true" || argvStr[9]=="True"); // whether to use sample splitting
    // (see split sample code for current configuration and make sure it is set correctly!)
    std::string& signal_chstring = argvStr[10]; // lepton channel(s) to consider in selection
    // convert string to vector of ints
    std::vector< int > signal_channels;
    for(char c: signal_chstring){ signal_channels.push_back( (int)c-(int)'0' ); }
    std::string& topcharge = argvStr[11]; // top charge to consider in selection
    if( topcharge!="all" && topcharge!="top" && topcharge!="antitop" ){
	std::cerr << "top charge '" << topcharge << "' not recognized" << std::endl;
	return -1;
    }
    std::string& bdtCombineMode = argvStr[12]; // ignored if _eventBDT is not in list of variables
    std::string& pathToXMLFile = argvStr[13]; // ignored if _eventBDT is not in list of variables
    double bdtCut = std::stof(argvStr[14]); // put very small, e.g. -99, if not needed
    std::vector<std::string> systematics;
    for(int i=15; i<argc; i++){
	systematics.push_back(argvStr[i]);
    }

    // make structure for variables
    std::vector<std::tuple<std::string,double,double,int>> vars;
    vars.push_back(std::make_tuple("_abs_eta_recoil",0.,5.,20));
    vars.push_back(std::make_tuple("_Mjj_max",0.,1200.,20));
    vars.push_back(std::make_tuple("_lW_asymmetry",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_deepCSV_max",0.,1.,20));
    vars.push_back(std::make_tuple("_deepFlavor_max",0.,1.,20));
    vars.push_back(std::make_tuple("_lT",0.,800.,20));
    vars.push_back(std::make_tuple("_MT",0.,200.,20));
    vars.push_back(std::make_tuple("_coarseBinnedMT",0.,200.,4.));
    vars.push_back(std::make_tuple("_pTjj_max",0.,300.,20));
    vars.push_back(std::make_tuple("_dRlb_min",0.,3.15,20));
    vars.push_back(std::make_tuple("_dPhill_max",0.,3.15,20));
    vars.push_back(std::make_tuple("_HT",0.,800.,20));
    vars.push_back(std::make_tuple("_nJets",-0.5,6.5,7));
    vars.push_back(std::make_tuple("_nBJets",-0.5,3.5,4));
    vars.push_back(std::make_tuple("_dRlWrecoil",0.,10.,20));
    vars.push_back(std::make_tuple("_dRlWbtagged",0.,7.,20));
    vars.push_back(std::make_tuple("_M3l",0.,600.,20));
    vars.push_back(std::make_tuple("_fineBinnedM3l",75.,105,20));
    vars.push_back(std::make_tuple("_abs_eta_max",0.,5.,20));
    vars.push_back(std::make_tuple("_eventBDT",-1,1,15));
    vars.push_back(std::make_tuple("_fineBinnedeventBDT",-1,1,1000));
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
    vars.push_back(std::make_tuple("_lW_pt",0.,150.,15));
    vars.push_back(std::make_tuple("_coarseBinnedlW_pt",0.,105.,4));
    vars.push_back(std::make_tuple("_Z_pt",0.,300.,15));
    vars.push_back(std::make_tuple("_coarseBinnedZ_pt",0.,275.,4));

    // load fake rate maps if needed
    std::shared_ptr<TH2D> frmap_muon;
    std::shared_ptr<TH2D> frmap_electron;
    if(selection_type=="fakerate"){
	std::string year = "2016";
	if(stringTools::stringContains(input_file_path,"2017")) year = "2017";
	else if(stringTools::stringContains(input_file_path,"2018")) year = "2018";
	std::string interpendix = "";
	if(LeptonSelector::leptonID()=="tzqtight") interpendix = "output_tzqtightid";
	else if(LeptonSelector::leptonID()=="tzqmedium0p4") interpendix = "output_tzqmedium0p4id";
	else if(LeptonSelector::leptonID()=="tth") interpendix = "output_tthid";
	std::string frdir = "/user/llambrec/ewkino/AnalysisCode/fakerate/"+interpendix+"/fakeRateMaps";
	//std::string frdir = "/user/llambrec/ewkino/AnalysisCode/fakerate/fakeRateMaps";
	std::string frmap_muon_name = frdir+"/fakeRateMap_data_muon_"+year+"_mT.root";
	std::string frmap_electron_name = frdir+"/fakeRateMap_data_electron_"+year+"_mT.root";
        frmap_muon = eventFlattening::readFRMap(frmap_muon_name,"muon",year);
        frmap_electron = eventFlattening::readFRMap(frmap_electron_name,"electron",year);

	std::cout << "read fake rate maps" << std::endl;
    }
    
    // load a TMVA Reader if needed
    bool doBDT = false;
    for(unsigned int i=0; i<vars.size(); ++i){
        if(std::get<0>(vars[i])=="_eventBDT") doBDT=true;
    }
    TMVA::Reader* reader = new TMVA::Reader();
    if(doBDT){
	std::string subDir = "bdts_"+LeptonSelector::leptonID();
	std::string::size_type pos = pathToXMLFile.find("/bdt/");
	if( pos != std::string::npos ){
	    pathToXMLFile.erase( pos, 5 );
	    pathToXMLFile.insert( pos, "/bdt/"+subDir+"/" );
	}
	reader = eventFlattening::initializeReader(reader, pathToXMLFile, bdtCombineMode); 
    }
    std::cout << "read BDT reader" << std::endl;

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
    fillSystematicsHistograms(input_file_path, norm, output_file_path, nentries, process_name,
				    event_selection, selection_type, signal_categories,
				    split_samples, signal_channels, topcharge,
				    frmap_muon, frmap_electron, 
				    vars, reader, bdtCut, systematics);

    delete reader;
    std::cerr << "###done###" << std::endl;
    return 0;
}
