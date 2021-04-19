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

bool isValidSource( std::string sourceName ){
    // info based on the twiki page on split JEC uncertainties:
    // https://twiki.cern.ch/twiki/bin/view/CMS/JECUncertaintySources
    if( stringTools::stringContains(sourceName,"Total") ) return false;
    if( stringTools::stringContains(sourceName,"AbsoluteFlavMap") ) return false;
    if( stringTools::stringContains(sourceName,"Flavor")
         && !stringTools::stringContains(sourceName,"QCD") ) return false;
    if( stringTools::stringContains(sourceName,"PileUpMuZero") ) return false;
    if( stringTools::stringContains(sourceName,"PileUpEnvelope") ) return false;
    return true;
}

JetInfo::JetInfo( const TreeReader& treeReader, 
		  const bool readAllJECVariations,
		  const bool readGroupedJECVariations ){
    _JECSources = std::vector< std::string >();
    _JECSourcesValid = std::vector< std::string >(); // subcollection removing superfluous ones
    _JECGrouped = std::vector< std::string >();
    _JECGroupedValid = std::vector< std::string >(); // subcollection removing superfluous ones
    if( readAllJECVariations ){
	for( auto mapEl: treeReader._jetSmearedPt_JECSourcesUp ){
	    // note: in principle only checking one branch should be enough
	    // as up/down and pt/smearedPt are supposed to contain the same variations
	    std::string jecName = cleanJECVariationName(mapEl.first);
	    _JECSources.push_back( jecName );
	    if( isValidSource(jecName) ) _JECSourcesValid.push_back( jecName );
	}
    }
    if( readGroupedJECVariations ){
	for( auto mapEl: treeReader._jetSmearedPt_JECGroupedUp ){
	    // note: in principle only checking one branch should be enough
            // as up/down and pt/smearedPt are supposed to contain the same variations
            std::string jecName = cleanJECVariationName(mapEl.first);
	    _JECGrouped.push_back( jecName );
	    if( isValidSource(jecName) ) _JECGroupedValid.push_back( jecName );
	}
    }
}

bool JetInfo::hasJECVariation( const std::string& jecName ) const{
    bool isInAllVariations = ( std::find(_JECSources.begin(),_JECSources.end(),
				jecName) != _JECSources.end() );
    bool isInGroupedVariations = ( std::find(_JECGrouped.begin(),
				    _JECGrouped.end(),
				    jecName) != _JECGrouped.end() );
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
    printAvailableInfo( _JECSources, message );
}

void JetInfo::printAllValidJECVariations() const{
    std::string message = "Available valid JEC variations (complete set):";
    printAvailableInfo( _JECSources, message );
}

void JetInfo::printGroupedJECVariations() const{
    std::string message = "Available JEC variations (reduced set):";
    printAvailableInfo( _JECGrouped, message );
}

void JetInfo::printGroupedValidJECVariations() const{
    std::string message = "Available valid JEC variations (reduced set):";
    printAvailableInfo( _JECGrouped, message );
}
