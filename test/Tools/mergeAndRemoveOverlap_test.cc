#include "../../Tools/interface/mergeAndRemoveOverlap.h"

//include c++ library classes
#include <vector>
#include <string>
#include <memory>
#include <iostream>

//include ROOT classes 
#include "TFile.h"
#include "TTree.h"

//include other parts of framework
#include "../../Tools/interface/systemTools.h"


long unsigned summedNumberOfEntries( const std::vector< std::string >& fileVector ){
    long unsigned entriesSum = 0;
    for( const auto& filePath : fileVector ){
        //if( ! systemTools::fileExists( filePath ) ) continue;
        std::shared_ptr< TFile > filePtr( new TFile( filePath.c_str() ) );
        std::shared_ptr< TTree > treePtr( dynamic_cast< TTree* >( filePtr->Get( "blackJackAndHookers/blackJackAndHookersTree" ) ) );
        entriesSum += treePtr->GetEntries();
    }
    return entriesSum;
}


int main(){

	/*
	std::vector< std::string > argvStr;
    for( int i = 1; i < argc; ++i ){
        argvStr.push_back( argv[i] );
    }
    mergeAndRemoveOverlap( argvStr, "test.root" );
	*/

	std::vector< std::string > orthogonalFileVector; 
    for( unsigned i = 300; i < 330; ++i ){
        std::string filePath = "../testData/FR_" + std::to_string( i ) + "_2018.root";
        if( ! systemTools::fileExists( filePath ) ) continue;
        orthogonalFileVector.emplace_back( filePath );
    }

	mergeAndRemoveOverlap( orthogonalFileVector, "orthogonal_test.root" );

    std::shared_ptr< TFile > mergedFilePtr( new TFile( "orthogonal_test.root" ) );
    std::shared_ptr< TTree > mergedTreePtr( dynamic_cast< TTree* >( mergedFilePtr->Get( "blackJackAndHookers/blackJackAndHookersTree" ) ) );
    long unsigned mergedNumEntries = mergedTreePtr->GetEntries();
    long unsigned numberOfsummedEntries = summedNumberOfEntries( orthogonalFileVector );

    if( mergedNumEntries != summedNumberOfEntries( orthogonalFileVector ) ){
        throw std::runtime_error( "Number of entries in merged dataset is " + std::to_string( mergedNumEntries ) + " while it should be " + std::to_string( numberOfsummedEntries ) + "." );
    } 
    
    return 0;
}
