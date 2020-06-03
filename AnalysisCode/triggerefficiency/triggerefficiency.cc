// This is the main C++ class used to determine trigger efficiencies.
// It is supposed to run on the output file of a skimming procedure
// and produce a root file containing a histogram of trigger efficiencies.

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

std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms(std::string prefix){
    // make output collection of histograms
    double xlow = 0.;
    double xhigh = 200.;
    int nbins = 10.;
    HistInfo histInfo( "", "pT (GeV)", nbins, xlow, xhigh);
    std::map< std::string, std::shared_ptr<TH1D> > histMap;
    histMap[std::string("leadleppt_trig")] = histInfo.makeHist(prefix+std::string("_leadleppt_trig"));
    histMap[std::string("leadleppt_tot")] = histInfo.makeHist(prefix+std::string("_leadleppt_tot"));
    histMap[std::string("leadleppt_eff")] = histInfo.makeHist(prefix+std::string("_leadleppt_eff"));
    histMap[std::string("subleppt_trig")] = histInfo.makeHist(prefix+std::string("_subleppt_trig"));
    histMap[std::string("subleppt_tot")] = histInfo.makeHist(prefix+std::string("_subleppt_tot"));
    histMap[std::string("subleppt_eff")] = histInfo.makeHist(prefix+std::string("_subleppt_eff"));
    histMap[std::string("trailleppt_trig")] = histInfo.makeHist(prefix+std::string("_trailleppt_trig"));
    histMap[std::string("trailleppt_tot")] = histInfo.makeHist(prefix+std::string("_trailleppt_tot"));
    histMap[std::string("trailleppt_eff")] = histInfo.makeHist(prefix+std::string("_trailleppt_eff"));
    /*for(auto histMapEl : histMap){
	histMapEl.second->Sumw2();
    }*/ // superfluous as it is already in histogram constructor
    return histMap;
}

void fillEvent(const Event& event, std::map<std::string,std::shared_ptr<TH1D>> histMap, double weight){
    // make sure it corresponds to initializeHistograms!!!
    event.sortLeptonsByPt();
    LeptonCollection lepcollection = event.leptonCollection();
    double leadleppt = 0.;
    double subleppt = 0;
    double trailleppt = 0.;
    LeptonCollection::const_iterator lIt = lepcollection.cbegin();
    for(unsigned int i=0; i<lepcollection.size(); i++){
	if(i==0){ Lepton& leadlep = **lIt; leadleppt = leadlep.uncorrectedPt(); }
	if(i==1){ Lepton& sublep = **lIt; subleppt = sublep.uncorrectedPt(); }
	if(i==2){ Lepton& traillep = **lIt; trailleppt = traillep.uncorrectedPt(); }
	lIt++;
    }
    // fill denominator
    histMap["leadleppt_tot"]->Fill(leadleppt, weight);
    histMap["subleppt_tot"]->Fill(subleppt, weight);
    histMap["trailleppt_tot"]->Fill(trailleppt, weight);
    // fill numerator
    bool passanytrigger = event.passTriggers_e() || event.passTriggers_ee() 
			|| event.passTriggers_eee() || event.passTriggers_m() 
			|| event.passTriggers_mm() || event.passTriggers_mmm()
			|| event.passTriggers_em() || event.passTriggers_eem() 
			|| event.passTriggers_emm();
    /*std::cout<<event.passTriggers_e()<<" "<<event.passTriggers_ee()<<" "
		<<event.passTriggers_eee()<<" "<<event.passTriggers_m()<<" "
                <<event.passTriggers_mm()<<" "<<event.passTriggers_mmm()<<" "
                <<event.passTriggers_em()<<" "<<event.passTriggers_eem()<<" "
                <<event.passTriggers_emm()<<std::endl;
    std::cout<<passanytrigger<<std::endl;*/
    if(!passanytrigger) return;
    histMap["leadleppt_trig"]->Fill(leadleppt, weight);
    histMap["leadleppt_eff"]->Fill(leadleppt, weight);
    histMap["subleppt_trig"]->Fill(subleppt, weight);
    histMap["subleppt_eff"]->Fill(subleppt, weight);
    histMap["trailleppt_trig"]->Fill(trailleppt, weight);
    histMap["trailleppt_eff"]->Fill(trailleppt, weight);
}

void makeRatioHistograms(std::map<std::string,std::shared_ptr<TH1D>> histMap){
    // make sure it corresponds to initializeHistograms!!!
    histMap["leadleppt_eff"].get()->Divide( histMap["leadleppt_tot"].get() );
    histMap["subleppt_eff"].get()->Divide( histMap["subleppt_tot"].get() );
    histMap["trailleppt_eff"].get()->Divide( histMap["trailleppt_tot"].get() );
    // temp for testing:
    /*std::cout<<"-----"<<std::endl;
    for(int i=1; i<histMap["leadleppt_trig"]->GetNbinsX()+1; i++){
        std::cout<<histMap["leadleppt_trig"]->GetBinContent(i)<<" ";
    }
    std::cout<<""<<std::endl;
    std::cout<<"-----"<<std::endl;
    for(int i=1; i<histMap["leadleppt_tot"]->GetNbinsX()+1; i++){
        std::cout<<histMap["leadleppt_tot"]->GetBinContent(i)<<" ";
    }
    std::cout<<""<<std::endl;
    std::cout<<"-----"<<std::endl;
    for(int i=1; i<histMap["leadleppt_eff"]->GetNbinsX()+1; i++){
        std::cout<<histMap["leadleppt_eff"]->GetBinContent(i)<<" ";
    }
    std::cout<<""<<std::endl;*/
}

void fillTriggerEfficiencyHistograms(const std::string& pathToFile, 
				    const std::string& outputFilePath){

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    bool isData = treeReader.isData();
    std::string prefix;
    if(isData){
	prefix = stringTools::split( pathToFile, "/"  ).back();
	prefix = stringTools::removeOccurencesOf( prefix, ".root" );
    }
    else prefix = "mc";

    // make output histograms
    std::map< std::string, std::shared_ptr<TH1D> > histMap = initializeHistograms(prefix);

    // initialize list of event id's for data overlap removal
    std::set<std::tuple<long,long,long>> evtlist;

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 1000;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	// build event
        Event event = treeReader.buildEvent(entry,false,false);

	// full event selection -> too little statistics
        //if(!passES(event, "signalregion", false)) continue;
	//int eventcategory = eventCategory(event, "nominal");
        //if(eventcategory == -1) continue;

	// alternative: simply check for three loose light leptons
	//event.selectLooseLeptons();
	//event.cleanElectronsFromLooseMuons();
	//event.cleanTausFromLooseLightLeptons();
	//event.removeTaus();
	//if(event.leptonCollection().size()!=3) continue;

	// alternative: check for three tight light leptons
	//event.selectLooseLeptons();
	//event.cleanElectronsFromLooseMuons();
	//event.cleanTausFromLooseLightLeptons();
	//event.removeTaus();
	//event.selectTightLeptons();
	//if(event.leptonCollection().size()!=3) continue;

	// alternative: two leptons (statistics check)
	event.selectLooseLeptons();
	event.cleanElectronsFromLooseMuons();
        event.cleanTausFromLooseLightLeptons();
        event.removeTaus();
        event.selectTightLeptons();
        if(event.leptonCollection().size()<2) continue;
	
	double weight = event.weight();
	// for data: check if event passes orthogonal triggers and remove overlap
        if(isData){
            bool passreftrigger = event.passTriggers_ref();
            if(!passreftrigger) continue;
            std::tuple<long,long,long> evtid = std::make_tuple(event.runNumber(), 
						event.luminosityBlock(),
						event.eventNumber());
            if(std::binary_search(evtlist.begin(),evtlist.end(),evtid)) continue;
	    else evtlist.insert(evtid);
	    weight = 1.;
        }
        fillEvent(event, histMap, weight);
    }
    // make ratio histograms
    makeRatioHistograms(histMap);
    // make output ROOT file
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->cd();
    for(auto mapelement : histMap) mapelement.second->Write();
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){
    if( argc != 3 ){
        std::cerr << "### ERROR ###: triggerefficiency.cc requires 2 arguments to run: ";
        std::cerr << "input_file_path, output_file_path ";
        return -1;
    }
    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    // check validity
    bool validInput = checkReadability( input_file_path );
    if(!validInput) return -1;
    // fill the histograms
    fillTriggerEfficiencyHistograms(input_file_path, output_file_path);
    std::cout<<"done"<<std::endl;
    return 0;
}
