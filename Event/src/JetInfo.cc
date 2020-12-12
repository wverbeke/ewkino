#include "../interface/JetInfo.h"

// include c++ library classes
#include <iostream>
#include <stdexcept>

// include other parts of the framework
#include "../../Tools/interface/stringTools.h"

std::string cleanJECVariationName( std::string branchName ){
    // convert a branch name holding a JEC variation into the pure JEC variation name
    std::string jecName = stringTools::removeOccurencesOf(branchName,"_jetPt_");
    jecName = stringTools::removeOccurencesOf(jecName,"_jetSmearedPt_");
    jecName = stringTools::removeOccurencesOf(jecName,"_JECSourcesUp");
    jecName = stringTools::removeOccurencesOf(jecName,"_JECSourcesDown");
    jecName = stringTools::removeOccurencesOf(jecName,"_JECGroupedUp");
    jecName = stringTools::removeOccurencesOf(jecName,"_JECGroupedDown");
    return jecName;
}

JetInfo::JetInfo( const TreeReader& treeReader, 
		  const bool readAllJECVariations,
		  const bool readGroupedJECVariations ){
    _allJECVariations = std::vector< std::string >();
    _groupedJECVariations = std::vector< std::string >();
    if( readAllJECVariations ){
	for( auto mapEl: treeReader._jetSmearedPt_allVariationsUp ){
	    // note: in principle only checking one branch should be enough
	    // as up/down and pt/smearedPt are supposed to contain the same variations
	    _allJECVariations.push_back( cleanJECVariationName(mapEl.first) );
	}
    }
    if( readGroupedJECVariations ){
	for( auto mapEl: treeReader._jetSmearedPt_groupedVariationsUp ){
	    // note: in principle only checking one branch should be enough
            // as up/down and pt/smearedPt are supposed to contain the same variations
            _groupedJECVariations.push_back( cleanJECVariationName(mapEl.first) );
	}
    }
}

bool JetInfo::hasJECVariation( const std::string& jecName ) const{
    bool isInAllVariations = ( std::find(_allJECVariations.begin(),_allJECVariations.end(),
				jecName) != _allJECVariations.end() );
    bool isInGroupedVariations = ( std::find(_groupedJECVariations.begin(),
				    _groupedJECVariations.end(),
				    jecName) != _groupedJECVariations.end() );
    return (isInAllVariations || isInGroupedVariations);
}

void printAvailableInfo( const std::vector<std::string>& info, const std::string& message ){
    std::cout << message << std::endl;
    for( std::string el: info ){
	std::cout << el << std::endl;
    } 
}

void JetInfo::printAllJECVariations() const{
    std::string message = "Available JEC variations (complete set):";
    printAvailableInfo( _allJECVariations, message );
}

void JetInfo::printGroupedJECVariations() const{
    std::string message = "Available JEC variations (reduced set):";
    printAvailableInfo( _groupedJECVariations, message );
}
