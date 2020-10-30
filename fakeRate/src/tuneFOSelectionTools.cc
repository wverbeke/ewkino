// include header
#include "../interface/tuneFOSelectionTools.h"

const double minPtRatioCut = 0.;
const double maxPtRatioCut = 0.9;
const unsigned numberOfPtRatioCuts = 2; // small number for testing

const double minSlidePt = 10;
const double maxSlidePt = 60;
const double minDeepFlavorCut = 0.1;
const double maxDeepFlavorCut = 0.9;
const double deepFlavorGranularity = 0.4; // coarse granularity for testing

std::map< std::string, double > getCutParameters(){
    std::map< std::string, double> res;
    res["minPtRatioCut"] = minPtRatioCut;
    res["maxPtRatioCut"] = maxPtRatioCut;
    res["numberOfPtRatioCuts"] = numberOfPtRatioCuts;
    res["minSlidePt"] = minSlidePt;
    res["maxSlidePt"] = maxSlidePt;
    res["minDeepFlavorCut"] = minDeepFlavorCut;
    res["maxDeepFlavorCut"] = maxDeepFlavorCut;
    res["deepFlavorGranularity"] = deepFlavorGranularity;
    return res;
}

std::tuple< std::vector<double>, SlidingCutCollection, Categorization > getCutCollection(){

    // make numberOfPtRatioCuts pt ratio cuts between 
    // minPtRatioCut (included) and maxPtRatioCut (not included)
    std::vector< double > ptRatioCuts;
    std::vector< std::string > ptRatioNames;
    for( unsigned c = 0; c < numberOfPtRatioCuts; ++c ){
        double cut = minPtRatioCut + c*( maxPtRatioCut - minPtRatioCut ) / numberOfPtRatioCuts;
        ptRatioCuts.push_back( cut );
        ptRatioNames.push_back( "pTRatio" + stringTools::doubleToString( cut, 3 ) );
    }

    // make a collection of sliding cuts:
    // minSlidePt and maxSlidePt are kept fixed;
    // all interpolations between minDeepFlavorCut and maxDeepFlavorCut (with given granularity)
    // at minSlidePt and maxSlidePt are added to the collection
    SlidingCutCollection deepFlavorCutCollection( minSlidePt, maxSlidePt, minDeepFlavorCut, maxDeepFlavorCut, deepFlavorGranularity );
    std::vector< std::string > deepFlavorNames;
    for( size_t i = 0; i < deepFlavorCutCollection.size(); ++i ){
        deepFlavorNames.push_back( deepFlavorCutCollection[i].name( "pT", "deepFlavor" ) );
    }

    // categorization will include one 'category' for every ptRatio and deepFlavor cut
    Categorization categories( { ptRatioNames, deepFlavorNames } );
    return std::make_tuple(ptRatioCuts,deepFlavorCutCollection,categories);
}
