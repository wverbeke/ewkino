/*
Collect the current POG b-tagging working points here 
*/

// see here for latest working points recommendations: 
// https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation
// note: the values are different for UL and pre-UL,
//       but the ewkino framework so far does not explicitly distinguish between them...
//	 for now, use UL working points as that will be the most probable use case from now on.

#include "bTagWP.h"

// deepCSV working points

double bTagWP::looseDeepCSV2016(){ return 0.2217; }
double bTagWP::mediumDeepCSV2016(){ return 0.6321; }
double bTagWP::tightDeepCSV2016(){ return 0.8953; }

double bTagWP::looseDeepCSV2016PreVFP(){ return 0.2027; }
double bTagWP::mediumDeepCSV2016PreVFP(){ return 0.6001; }
double bTagWP::tightDeepCSV2016PreVFP(){ return 0.8819; }

double bTagWP::looseDeepCSV2016PostVFP(){ return 0.1918; }
double bTagWP::mediumDeepCSV2016PostVFP(){ return 0.5847; }
double bTagWP::tightDeepCSV2016PostVFP(){ return 0.8767; }

double bTagWP::looseDeepCSV2017(){ return 0.1355; }
double bTagWP::mediumDeepCSV2017(){ return 0.4506; }
double bTagWP::tightDeepCSV2017(){ return 0.7738; }

double bTagWP::looseDeepCSV2018(){ return 0.1208; }
double bTagWP::mediumDeepCSV2018(){ return 0.4168; }
double bTagWP::tightDeepCSV2018(){ return 0.7665; }

// deepFlavor working points

double bTagWP::looseDeepFlavor2016(){ return 0.0480; }
double bTagWP::mediumDeepFlavor2016(){ return 0.2489; }
double bTagWP::tightDeepFlavor2016(){ return 0.6377; }

double bTagWP::looseDeepFlavor2016PreVFP(){ return 0.0508; }
double bTagWP::mediumDeepFlavor2016PreVFP(){ return 0.2598; }
double bTagWP::tightDeepFlavor2016PreVFP(){ return 0.6502; }

double bTagWP::looseDeepFlavor2016PostVFP(){ return 0.0480; }
double bTagWP::mediumDeepFlavor2016PostVFP(){ return 0.2489; }
double bTagWP::tightDeepFlavor2016PostVFP(){ return 0.6377; }

double bTagWP::looseDeepFlavor2017(){ return 0.0532; }
double bTagWP::mediumDeepFlavor2017(){ return 0.3040; }
double bTagWP::tightDeepFlavor2017(){ return 0.7476; }

double bTagWP::looseDeepFlavor2018(){ return 0.0490; }
double bTagWP::mediumDeepFlavor2018(){ return 0.2783; }
double bTagWP::tightDeepFlavor2018(){ return 0.7100; }

// extension for more flexible calling in selections

double bTagWP::getWP(const std::string tagger, 
		    const std::string wp, 
		    const std::string year){
    std::string fname = wp+tagger+year;
    std::map<std::string, std::function<double()>>::iterator it = btagwpmap.find(fname);
    if(it != btagwpmap.end()){ return btagwpmap[fname](); }
    else{
	std::string msg = "ERROR in objectSelection/src/bTagWP.cc";
	msg = " b-tag threshold to use not recognized ('"+fname<+"')";
	throw std::runtime_error(msg);
    }
}
