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

    double looseDeepCSV2017();
    double mediumDeepCSV2017();
    double tightDeepCSV2017();

    double looseDeepCSV2018();
    double mediumDeepCSV2018();
    double tightDeepCSV2018();

    double looseDeepFlavor2016();
    double mediumDeepFlavor2016();
    double tightDeepFlavor2016();

    double looseDeepFlavor2017();
    double mediumDeepFlavor2017();
    double tightDeepFlavor2017();

    double looseDeepFlavor2018();
    double mediumDeepFlavor2018();
    double tightDeepFlavor2018();

    std::map<std::string, std::function<double()>> btagwpmap =
	{{ "looseDeepCSV2016", looseDeepCSV2016},
	 { "looseDeepCSV2017", looseDeepCSV2017},
	 { "looseDeepCSV2018", looseDeepCSV2018},
	 { "mediumDeepCSV2016", mediumDeepCSV2016},
	 { "mediumDeepCSV2017", mediumDeepCSV2017},
	 { "mediumDeepCSV2018", mediumDeepCSV2018},
	 { "tightDeepCSV2016", tightDeepCSV2016},
	 { "tightDeepCSV2017", tightDeepCSV2017},
	 { "tightDeepCSV2018", tightDeepCSV2018},

	 { "looseDeepFlavor2016", looseDeepFlavor2016},
         { "looseDeepFlavor2017", looseDeepFlavor2017},
         { "looseDeepFlavor2018", looseDeepFlavor2018},
         { "mediumDeepFlavor2016", mediumDeepFlavor2016},
         { "mediumDeepFlavor2017", mediumDeepFlavor2017},
         { "mediumDeepFlavor2018", mediumDeepFlavor2018},
         { "tightDeepFlavor2016", tightDeepFlavor2016},
         { "tightDeepFlavor2017", tightDeepFlavor2017},
         { "tightDeepFlavor2018", tightDeepFlavor2018}
	};
    
    double getWP(const std::string, const std::string, const std::string);

}
#endif
