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
//#include "TMVA/Reader.h"
#include "TGraphAsymmErrors.h"

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

std::vector<double> ptThresholds(){
    //return {25., 15., 10.}; // for tZq
    return {40., 20., 10.}; // for ttZ (Marek)
}

bool passPtThresholds( const std::vector<double>& pts, const std::vector<double>& thresholds ){
    // determine whether a given vector of pts passes a given vector of thresholds.
    // note that both are implicitly assumed to be sorted (in the same way)!
    if( pts[0]<thresholds[0] ) return false;
    if( pts[1]<thresholds[1] ) return false;
    if( pts[2]<thresholds[2] ) return false;
    return true;
}

std::map< std::string, std::shared_ptr<TH1D> > initializeHistograms(std::string prefix,
				    std::vector<std::tuple<std::string,double,double,int>> variables){
    // make output collection of histograms
    std::map< std::string, std::shared_ptr<TH1D> > histMap;
    for(unsigned i=0; i<variables.size(); ++i){
	std::string varname = std::get<0>(variables[i]);
	double xlow = std::get<1>(variables[i]);
	double xhigh = std::get<2>(variables[i]);
	int nbins = std::get<3>(variables[i]);
	HistInfo histInfo( "", "pT (GeV)", nbins, xlow, xhigh);
	histMap[varname+"_trig"] = histInfo.makeHist(prefix+"_"+varname+"_trig");
	histMap[varname+"_tot"] = histInfo.makeHist(prefix+"_"+varname+"_tot");
    }
    return histMap;
}

void fillHistogram(std::shared_ptr<TH1D> hist, double value, double weight){
    double xmin = hist->GetBinLowEdge(1);
    double xmax = hist->GetBinLowEdge(hist->GetNbinsX()+1);
    if( value>xmin && value<xmax ) hist->Fill(value,weight);
}

void fillEvent(const Event& event, double weight, 
		std::vector<std::tuple<std::string,double,double,int>> variables,
		std::map<std::string,std::shared_ptr<TH1D>> histMap){

    // do all calculations and set variables locally.
    // make sure the naming convention in 'allvars' matches the one in 'variables'!!!
    //event.sortLeptonsByPt(); // note: not 100% reliable as it uses corrected pt!
    std::vector<double> recopt;
    recopt.push_back(event.leptonCollection()[0].uncorrectedPt());
    recopt.push_back(event.leptonCollection()[1].uncorrectedPt());
    recopt.push_back(event.leptonCollection()[2].uncorrectedPt());
    std::sort(recopt.begin(),recopt.end());
    std::map< std::string,double > allvars;
    allvars["leptonptleading"] = recopt[2]; 
    allvars["leptonptsubleading"] = recopt[1]; 
    allvars["leptonpttrailing"] = recopt[0];
    allvars["yield"] = 0.5;
    // fill denominator
    for(unsigned i=0; i<variables.size(); ++i){
	std::string varname = std::get<0>(variables[i]);
	fillHistogram(histMap[varname+"_tot"], allvars[varname], weight);
    }
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
    for(unsigned i=0; i<variables.size(); ++i){
        std::string varname = std::get<0>(variables[i]); 
        fillHistogram(histMap[varname+"_trig"], allvars[varname], weight);
    }
}

void fillTriggerEfficiencyHistograms(const std::string& pathToFile, 
				    const std::string& outputFilePath,
				    const std::string& eventSelection,
				    const std::vector<std::tuple<std::string,double,double,int>> variables){

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
    std::map< std::string, std::shared_ptr<TH1D> > histMap = initializeHistograms(prefix,variables);

    // initialize list of event id's for data overlap removal
    std::set<std::tuple<long,long,long>> evtlist;

    int debugcounter = 0;

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 1000;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	// build event
        Event event = treeReader.buildEvent(entry,false,false,false,false);
	event.applyLeptonConeCorrection();

	// split the event selection string into a list
	std::vector<std::string> selectionTags = stringTools::split(eventSelection,"_");

	// full event selection -> too little statistics
	if(std::find(selectionTags.begin(),selectionTags.end(),"full")!=selectionTags.end()){
	    if(!passES(event, "signalregion", "3tight", "nominal")) continue;
	    int eventcategory = eventCategory(event, "nominal");
	    if(eventcategory == -1) continue;
	}

	// select 3 loose leptons
	if(std::find(selectionTags.begin(),selectionTags.end(),"3loose")!=selectionTags.end()){
	    event.selectLooseLeptons();
	    event.cleanElectronsFromLooseMuons();
	    event.cleanTausFromLooseLightLeptons();
	    event.removeTaus();
	    if(event.leptonCollection().size()!=3) continue;
	}

	// select three FO light leptons
	if(std::find(selectionTags.begin(),selectionTags.end(),"3fo")!=selectionTags.end()){
	    event.selectLooseLeptons();
	    event.cleanElectronsFromLooseMuons();
	    event.cleanTausFromLooseLightLeptons();
	    event.removeTaus();
	    event.selectFOLeptons();
	    if(event.leptonCollection().size()!=3) continue;
	    if(std::find(selectionTags.begin(),selectionTags.end(),"3tightveto")!=selectionTags.end()){
		if(event.leptonCollection().numberOfTightLeptons()==3) continue;
	    }
	}

	// select three tight light leptons
	if(std::find(selectionTags.begin(),selectionTags.end(),"3tight")!=selectionTags.end()){
	    event.selectLooseLeptons();
	    event.cleanElectronsFromLooseMuons();
	    event.cleanTausFromLooseLightLeptons();
	    event.removeTaus();
	    event.selectTightLeptons();
	    if(event.leptonCollection().size()!=3) continue;
	}

	// additional selection: reco pt cuts
	if(std::find(selectionTags.begin(),selectionTags.end(),"recoptcuts")!=selectionTags.end()){
	    std::vector<double> recopt;
	    recopt.push_back(event.leptonCollection()[0].uncorrectedPt());
	    recopt.push_back(event.leptonCollection()[1].uncorrectedPt());
	    recopt.push_back(event.leptonCollection()[2].uncorrectedPt());
	    std::sort(recopt.begin(),recopt.end(),std::greater<double>());
	    if( !passPtThresholds( recopt, ptThresholds() )) continue;
	}

	// additional selection: cone pt cuts
	if(std::find(selectionTags.begin(),selectionTags.end(),"ptcuts")!=selectionTags.end() || 
	    std::find(selectionTags.begin(),selectionTags.end(),"coneptcuts")!=selectionTags.end()){
	    event.sortLeptonsByPt();
	    std::vector<double> conept;
	    conept.push_back(event.leptonCollection()[0].pt());
	    conept.push_back(event.leptonCollection()[1].pt());
	    conept.push_back(event.leptonCollection()[2].pt());
	    if( !passPtThresholds( conept, ptThresholds() )) continue;
	}

	double weight = event.weight();
	// for data: check if event passes orthogonal triggers and remove overlap
        if(isData){
            bool passreftrigger = event.passTriggers_ref();
            if(!passreftrigger) continue;
            std::tuple<long,long,long> evtid = std::make_tuple(event.runNumber(), 
						event.luminosityBlock(),
						event.eventNumber());
            if(std::binary_search(evtlist.begin(),evtlist.end(),evtid)){
		std::cout << "### WARNING ###: found duplicate event in data;";
		std::cout << " this should not happen if data was properly merged..." << std::endl;
		continue;
	    }
	    else evtlist.insert(evtid);
	    weight = 1.;
        }
	debugcounter++;
        fillEvent(event, weight, variables, histMap);
    }

    std::cout << debugcounter << " events passed selection." << std::endl;

    // for MC it can happen that #pass > #tot (due to negative weights).
    // this gives errors when doing correct division, so need to manually fix here.
    // it could also happen that #tot < 0 or #trig < 0, so put minimum to zero first.
    if(!isData){
	for(unsigned i=0; i<variables.size(); ++i){
	    std::string varname = std::get<0>(variables[i]);
	    std::cout << varname << std::endl;
	    std::cout << histMap[varname+"_tot"]->GetBinContent(1) << std::endl;
	    std::cout << histMap[varname+"_trig"]->GetBinContent(1) << std::endl;
	    for(int j=0; j<histMap[varname+"_tot"]->GetNbinsX()+2; ++j){
		if(histMap[varname+"_tot"]->GetBinContent(j)<0){
		    histMap[varname+"_tot"]->SetBinContent(j,0);
		    histMap[varname+"_tot"]->SetBinError(j,0);
		}
		if(histMap[varname+"_trig"]->GetBinContent(j)<0){
		    histMap[varname+"_trig"]->SetBinContent(j,0);
		    histMap[varname+"_trig"]->SetBinError(j,0);
		}
		if(histMap[varname+"_trig"]->GetBinContent(j)>histMap[varname+"_tot"]->GetBinContent(j)){
		    histMap[varname+"_trig"]->SetBinContent(j,histMap[varname+"_tot"]->GetBinContent(j));
		    histMap[varname+"_trig"]->SetBinError(j,histMap[varname+"_tot"]->GetBinError(j));
		}
	    }
        }
    }
    // TGraphAsymmErrors::Divide seems to go fail on empty denominator bins.
    // so put an artificial entry there with zero error, 
    // so the division result will be zero content en zero error, as we want for this case
    for(unsigned i=0; i<variables.size(); ++i){
        std::string varname = std::get<0>(variables[i]);
        for(int j=0; j<histMap[varname+"_tot"]->GetNbinsX()+2; ++j){
            if(histMap[varname+"_tot"]->GetBinContent(j)==0){
                histMap[varname+"_tot"]->SetBinContent(j,1);
                histMap[varname+"_tot"]->SetBinError(j,0);
            }
        }
    }
    // make ratio TGraphAsymmErrors
    std::map< std::string,std::shared_ptr<TGraphAsymmErrors> > tGraphMap;
    for(unsigned i=0; i<variables.size(); ++i){
        std::string varname = std::get<0>(variables[i]);
        tGraphMap[varname+"_eff"] = std::make_shared<TGraphAsymmErrors>();
	tGraphMap[varname+"_eff"].get()->Divide(histMap[varname+"_trig"].get(),
						histMap[varname+"_tot"].get(),
						"cl=0.683 b(1,1) mode");
	tGraphMap[varname+"_eff"].get()->SetName( std::string(prefix+"_"+varname+"_eff").c_str() );
	tGraphMap[varname+"_eff"].get()->SetTitle( std::string(prefix+"_"+varname+"_eff").c_str() );
	// loop over bins and catch special cases
	/*for(int j=1; j<histMap[varname+"_trig"]->GetNbinsX()+1; ++j){
	    // case 1: no entries in total histogram (need to insert or set point manually)
	    if(histMap[varname+"_tot"]->GetBinContent(j)==0){
		if(j==tGraphMap[varname+"_eff"]->GetN()+1){
		    tGraphMap[varname+"_eff"]->SetPoint(j-1,histMap[varname+"_tot"]->GetBinCenter(j),0);
		} else{
		    tGraphMap[varname+"_eff"]->InsertPointBefore(j-1,
		    histMap[varname+"_tot"]->GetBinCenter(j),0);
		}
		tGraphMap[varname+"_eff"]->SetPointError(j-1,
		    histMap[varname+"_tot"]->GetBinCenter(j)-histMap[varname+"_tot"]->GetBinLowEdge(j),
		    histMap[varname+"_tot"]->GetBinLowEdge(j+1)-histMap[varname+"_tot"]->GetBinCenter(j),
		    0,0);
	    }    
	}*/
	// prints for debugging	
	/*for(int j=0; j<histMap[varname+"_trig"]->GetNbinsX()+2; ++j){
	    std::cout << "--------------" <<std::endl;
	    // print bin
	    std::cout << "bin: " << histMap[varname+"_trig"]->GetBinLowEdge(j) << " -> "
	    << histMap[varname+"_trig"]->GetBinLowEdge(j) + histMap[varname+"_trig"]->GetBinWidth(j) 
	    << std::endl;
	    // print bin contents
	    std::cout << "trig: " << histMap[varname+"_trig"]->GetBinContent(j) << std::endl;
	    std::cout << "total: " << histMap[varname+"_tot"]->GetBinContent(j) << std::endl;
	    std::cout << "ratio: " << tGraphMap[varname+"_eff"]->GetY()[j] << std::endl;
	    // print left and right errors
	    std::cout << "center: " << tGraphMap[varname+"_eff"]->GetX()[j] << std::endl;
	    std::cout << "left: " << tGraphMap[varname+"_eff"]->GetErrorXlow(j) << std::endl;
	    std::cout << "right: " << tGraphMap[varname+"_eff"]->GetErrorXhigh(j) << std::endl;
	}
	// print number of points in graph
	std::cout << "npoints: " << tGraphMap[varname+"_eff"]->GetN() << std::endl;
	*/
    }   
    // make output ROOT file
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->cd();
    // write histograms
    for(auto mapelement : histMap) mapelement.second->Write();
    // write tgraphs
    for(auto mapelement : tGraphMap) mapelement.second->Write();
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){
    std::cerr<<"###starting###"<<std::endl;
    if( argc != 4 ){
        std::cerr << "### ERROR ###: triggerefficiency.cc requires 3 arguments to run: ";
        std::cerr << "input_file_path, output_file_path event_selection" << std::endl;
        return -1;
    }
    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    std::string& event_selection = argvStr[3];
    // check validity
    bool validInput = checkReadability( input_file_path );
    if(!validInput) return -1;
    // define variables (arbitrary names, only used for histogram titles)
    std::vector< std::tuple<std::string,double,double,int> > variables;
    variables.push_back(std::make_tuple("leptonptleading",0.,300.,12));
    variables.push_back(std::make_tuple("leptonptsubleading",0.,180.,12));
    variables.push_back(std::make_tuple("leptonpttrailing",0.,120,12));
    variables.push_back(std::make_tuple("yield",0.,1.,1));
    // fill the histograms
    fillTriggerEfficiencyHistograms(input_file_path, output_file_path, event_selection, variables);
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
