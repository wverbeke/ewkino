/*
Return the current POG b-tagging working points
*/

#ifndef bTagWP_H
#define bTagWP_H

#include <functional>
#include <map>

namespace bTagWP{
    
    double looseDeepCSV2016();
    double mediumDeepCSV2016();
    double tightDeepCSV2016();

    double looseDeepCSV2016PreVFP();
    double mediumDeepCSV2016PreVFP();
    double tightDeepCSV2016PreVFP();

    double looseDeepCSV2016PostVFP();
    double mediumDeepCSV2016PostVFP();
    double tightDeepCSV2016PostVFP();

    double looseDeepCSV2017();
    double mediumDeepCSV2017();
    double tightDeepCSV2017();

    double looseDeepCSV2018();
    double mediumDeepCSV2018();
    double tightDeepCSV2018();

    double looseDeepFlavor2016();
    double mediumDeepFlavor2016();
    double tightDeepFlavor2016();

    double looseDeepFlavor2016PreVFP();
    double mediumDeepFlavor2016PreVFP();
    double tightDeepFlavor2016PreVFP();

    double looseDeepFlavor2016PostVFP();
    double mediumDeepFlavor2016PostVFP();
    double tightDeepFlavor2016PostVFP();

    double looseDeepFlavor2017();
    double mediumDeepFlavor2017();
    double tightDeepFlavor2017();

    double looseDeepFlavor2018();
    double mediumDeepFlavor2018();
    double tightDeepFlavor2018();


    std::map<std::string, std::function<double()>> btagwpmap =
	{{ "looseDeepCSV2016", looseDeepCSV2016},
	{ "looseDeepCSV2016PreVFP", looseDeepCSV2016PreVFP},
	{ "looseDeepCSV2016PostVFP", looseDeepCSV2016PostVFP},
	{ "looseDeepCSV2017", looseDeepCSV2017},
	{ "looseDeepCSV2018", looseDeepCSV2018},
	{ "mediumDeepCSV2016", mediumDeepCSV2016},
	{ "mediumDeepCSV2016PreVFP", mediumDeepCSV2016PreVFP},
	{ "mediumDeepCSV2016PostVFP", mediumDeepCSV2016PostVFP},
	{ "mediumDeepCSV2017", mediumDeepCSV2017},
	{ "mediumDeepCSV2018", mediumDeepCSV2018},
	{ "tightDeepCSV2016", tightDeepCSV2016},
	{ "tightDeepCSV2016PreVFP", tightDeepCSV2016PreVFP},
	{ "tightDeepCSV2016PostVFP", tightDeepCSV2016PostVFP},
	{ "tightDeepCSV2017", tightDeepCSV2017},
	{ "tightDeepCSV2018", tightDeepCSV2018},

	{ "looseDeepFlavor2016", looseDeepFlavor2016},
	{ "looseDeepFlavor2016PreVFP", looseDeepFlavor2016PreVFP},
	{ "looseDeepFlavor2016PostVFP", looseDeepFlavor2016PostVFP},
        { "looseDeepFlavor2017", looseDeepFlavor2017},
        { "looseDeepFlavor2018", looseDeepFlavor2018},
        { "mediumDeepFlavor2016", mediumDeepFlavor2016},
	{ "mediumDeepFlavor2016PreVFP", mediumDeepFlavor2016PreVFP},
	{ "mediumDeepFlavor2016PreVFP", mediumDeepFlavor2016PreVFP},
        { "mediumDeepFlavor2017", mediumDeepFlavor2017},
        { "mediumDeepFlavor2018", mediumDeepFlavor2018},
        { "tightDeepFlavor2016", tightDeepFlavor2016},
	{ "tightDeepFlavor2016PreVFP", tightDeepFlavor2016PreVFP},
	{ "tightDeepFlavor2016PostVFP", tightDeepFlavor2016PostVFP},
        { "tightDeepFlavor2017", tightDeepFlavor2017},
        { "tightDeepFlavor2018", tightDeepFlavor2018}
    };
    
    double getWP(const std::string, const std::string, const std::string);
}
#endif
