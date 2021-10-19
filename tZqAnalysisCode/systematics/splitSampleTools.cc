/*
Tools for splitting samples into sub-categories
*/

#include "splitSampleTools.h"


// note on usage:
// best to choose the split names as <unsplit name>_<some qualifier>!
// when merging, only the part before the first underscore will define the process,
// so process-specific systematics will be treated correctly for arbitrarily split processes,
// e.g. tZq_top_nMuons_isrShape_tZqUp instead of tZq_top_nMuons_isrShape_tZq_topUp


std::vector< std::string > splitProcessNames( const std::string& processName ){
    // return a vector of strings representing split process names
    // e.g. tZq -> tZq_top and tZq_antitop
    // note: make sure it corresponds to the event splitting in the function below!
    std::vector< std::string > splitNames;
    //if( processName=="tZq" ){
	// splitting into top/antitop
	//splitNames = {"tZq_top", "tZq_antitop"};
	// splitting into lepton final states
	//splitNames = {"tZq_eee","tZq_eem","tZq_emm","tZq_mmm"};
    //}
    if( processName=="Xgamma" ) splitNames = {"Xgamma_int", "Xgamma_ext"};
    else splitNames = {processName};
    return splitNames;
}

std::string splitProcessName( const std::string& processName, Event& event ){
    // return a split process name for a given processName and event
    // e.g. tZq event -> tZq_top or tZq_antitop
    // note: make sure it corresponds to the function above
    //if( processName=="tZq" ){
	// splitting into top/antitop
        //if( lWCharge(event) > 0 ) return "tZq_top";
        //else return "tZq_antitop";
	// splitting into lepton final states
	//int nMuons = event.leptonCollection().numberOfMuons();
	//if( nMuons==0 ) return "tZq_eee";
	//if( nMuons==1 ) return "tZq_eem";
	//if( nMuons==2 ) return "tZq_emm";
	//else return "tZq_mmm";
    //}
    if( processName=="Xgamma" ){
        if( hasLeptonFromMEExternalConversion(event) ) return "Xgamma_ext";
        else return "Xgamma_int";
    }
    else return processName;
}
