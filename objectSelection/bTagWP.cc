/*
Collect the current POG b-tagging working points here 
*/

#include "bTagWP.h"

double bTagWP::looseDeepCSV2016(){ return 0.2217; }
double bTagWP::mediumDeepCSV2016(){ return 0.6321; }
double bTagWP::tightDeepCSV2016(){ return 0.8953; }

double bTagWP::looseDeepCSV2017(){ return 0.1522; }
double bTagWP::mediumDeepCSV2017(){ return 0.4941; }
double bTagWP::tightDeepCSV2017(){ return 0.8001; }

double bTagWP::looseDeepCSV2018(){ return 0.1241; }
double bTagWP::mediumDeepCSV2018(){ return 0.4184; }
double bTagWP::tightDeepCSV2018(){ return 0.7527; }

double bTagWP::looseDeepFlavor2016(){ return 0.0614; }
double bTagWP::mediumDeepFlavor2016(){ return 0.3093; }
double bTagWP::tightDeepFlavor2016(){ return 0.7221; }

double bTagWP::looseDeepFlavor2017(){ return 0.0521; }
double bTagWP::mediumDeepFlavor2017(){ return 0.3033; }
double bTagWP::tightDeepFlavor2017(){ return 0.7489; }

double bTagWP::looseDeepFlavor2018(){ return 0.0494; }
double bTagWP::mediumDeepFlavor2018(){ return 0.2770; }
double bTagWP::tightDeepFlavor2018(){ return 0.7264; }

// extension for more flexible calling in selections:
double bTagWP::getWP(const std::string tagger, 
			const std::string wp, 
			const std::string year){
    std::string fname = wp+tagger+year;
    std::map<std::string, std::function<double()>>::iterator it = btagwpmap.find(fname);
    if(it != btagwpmap.end()){
	return btagwpmap[fname]();
    }
    else{
	std::cout<<"### ERROR ###: in ewkino/objectSelection/src/bTagWP.cc"<<std::endl;
	std::cout<<"               b-tag threshold to use not recognized ('"<<fname<<"')"<<std::endl;
	return btagwpmap[fname](); // will give an error to safely terminate function
    }
}
