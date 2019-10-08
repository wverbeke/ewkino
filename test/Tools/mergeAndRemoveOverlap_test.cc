#include "../../Tools/interface/mergeAndRemoveOverlap.h"

//include c++ library classes
#include <vector>
#include <string>
#include <memory>
#include <iostream>

//include ROOT classes 
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"

//include other parts of framework
#include "../../Tools/interface/systemTools.h"
#include "../../TreeReader/interface/TreeReader.h"


//number of entries in file 
long unsigned numberOfEntries( const std::string& filePath ){
    std::shared_ptr< TFile > filePtr( new TFile( filePath.c_str() ) );
    std::shared_ptr< TTree > treePtr( dynamic_cast< TTree* >( filePtr->Get( "blackJackAndHookers/blackJackAndHookersTree" ) ) );
    return treePtr->GetEntries();
}


//sum the number of entries for a vector of TFile paths
long unsigned summedNumberOfEntries( const std::vector< std::string >& fileVector ){
    long unsigned entriesSum = 0;
    for( const auto& filePath : fileVector ){
        entriesSum += numberOfEntries( filePath );
    }
    return entriesSum;
}


//for a given string of files, make a map containing the histogram names and the sum of their SumOfWeights
std::map< std::string, double > summedHistSumOfWeights( const std::vector< std::string >& fileVector ){
    static std::vector< std::string > histNames = { "nVertices", "hCounter", "lheCounter", "nTrueInteractions", "psCounter" };
    std::map< std::string, double > summedSumOfWeights;
    for( const auto& filePath : fileVector ){
		std::shared_ptr< TFile > filePtr( new TFile( filePath.c_str() ) );
		for( const auto& hName : histNames ){
			std::shared_ptr< TH1 > histPtr( dynamic_cast< TH1* >( filePtr->Get( ( "blackJackAndHookers/" + hName ).c_str() ) ) );

			auto histIt = summedSumOfWeights.find( hName );
			if( histIt == summedSumOfWeights.cend() ){
				summedSumOfWeights[hName] = histPtr->GetSumOfWeights();	
			} else{
				summedSumOfWeights[hName] += histPtr->GetSumOfWeights();
			}
		}
    }
    return summedSumOfWeights;
}


//check equality of doubles
bool doubleEqual( const double lhs, const double rhs ){
    double diff = fabs( lhs - rhs )/lhs;
    if( diff < 1e-6 ){
        return true;
    } 
    return false;
}


int main(){

    //check whether the sum of entries of a set of merged orthogonal files corresponds to the sum of their entries
	std::vector< std::string > orthogonalFileVector; 
    for( unsigned i = 300; i < 330; ++i ){
        std::string filePath = "../testData/FR_" + std::to_string( i ) + "_2018.root";
        if( ! systemTools::fileExists( filePath ) ) continue;
        orthogonalFileVector.emplace_back( filePath );
    }

    mergeAndRemoveOverlap( orthogonalFileVector, "orthogonal_test.root"  );

    long unsigned mergedNumEntries = numberOfEntries( "orthogonal_test.root"  );
    long unsigned numberOfSummedEntries = summedNumberOfEntries( orthogonalFileVector );

    if( mergedNumEntries != numberOfSummedEntries ){
        throw std::runtime_error( "Number of entries in merged dataset of orthogonal files is " + std::to_string( mergedNumEntries ) + " while it should be " + std::to_string( numberOfSummedEntries ) + "." );
    } 

    //check whether the sum of entries of a set of merged overlapping files corresponds to the number of entries in the first file 
    std::vector< std::string > overlappingFileVector;
    for( unsigned i = 0; i < 10; ++i ){
        overlappingFileVector.emplace_back( "../testData/FR_300_2018.root" );
    }
    
    mergeAndRemoveOverlap( overlappingFileVector, "overlapping_test.root" );
    
    mergedNumEntries = numberOfEntries( "overlapping_test.root" );
    long unsigned initialNumEntries = numberOfEntries( "../testData/FR_300_2018.root" );

    if( mergedNumEntries != initialNumEntries ){
        throw std::runtime_error( "Number of entries in merged dataset of overlapping files is " + std::to_string( mergedNumEntries ) + " while it should be " + std::to_string( initialNumEntries ) + "." );
    }


    //Check whether merged sum of histograms is the same as the sum of the histograms 
    //simultaneously check whether TreeReader::getHistogramsFromCurrentFile() retrieves all the necessary histograms
    auto summedSumMap = summedHistSumOfWeights( orthogonalFileVector );
    TreeReader reader;
    reader.initSampleFromFile( "orthogonal_test.root", false, true );
    std::vector< std::shared_ptr< TH1 > > mergedHistVec = reader.getHistogramsFromCurrentFile();
    std::map< std::string, double > mergedSumMap;
    for( const auto& hist : mergedHistVec ){
        mergedSumMap[ hist->GetName() ] = hist->GetSumOfWeights();
    }

    for( const auto& pair : summedSumMap ){
        auto it = mergedSumMap.find( pair.first );
        if( it == mergedSumMap.cend() ){
            throw std::runtime_error( "Key '" + pair.first + "' not found in merged histogram map." );
        }
        if( !doubleEqual( it->second, pair.second ) ){
            throw std::runtime_error( "Sum of weights for key '" + pair.first + "' is " + std::to_string( it->second ) + " in merged file while for the sum of files it is " + std::to_string( pair.second ) + "." );
        }
    }

    
    return 0;
}
