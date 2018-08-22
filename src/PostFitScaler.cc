#include "../interface/PostFitScaler.h"

//include c++ library classes 
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <cmath>

//include other parts of code 
#include "../interface/systemTools.h"

PostFitScaler::PostFitScaler( const std::string& inputFileName ){

	//read input file
	std::ifstream fileStream( inputFileName );   

	//read each line of the input file 
	std::string line;

	//skip first line 
	std::getline( fileStream, line ); 

    //check whether txt file has the correct format, it should atleast have four columns
    if( systemTools::numberOfColumnsInFile( inputFileName ) < 4 ){
        std::cerr << "Error in PostFitScaler constructor : input txt file has less than 4 columns and can not be processed. Yields will be left empty!" << std::endl;
        return;
    } 

	while( std::getline( fileStream, line ) ){
		
        //values to be read
        std::string channel, process;
        double preFitYield, postFitYield; 

		//split line and read each part 
		std::istringstream lineStream( line );
        lineStream >> channel >> process >> preFitYield >> postFitYield; 
    
        postAndPreFitYields[process].push_back( {preFitYield, postFitYield} );
        
        //temporary print, remove later
        std::cout << process << " : " << preFitYield << "\t" << postFitYield << std::endl;
	}
    std::cout << "finished constructing" << std::endl;

}

std::pair< std::string, size_t > PostFitScaler::findProcessAndBin( const double preFitYield ) const{
    
    std::cout << "Attempting to find process and bin" << std::endl;

    std::string process;
    size_t bin;

    //find minimum difference between all stored pre fit yields and the argument passed 
    double minDifference = std::numeric_limits<double>::max();

    std::cout << "start loop over map" << std::endl;
    for( const auto& mapEntry : postAndPreFitYields ){
        const auto& yieldVector = mapEntry.second;
        for( size_t b = 0; b < yieldVector.size(); ++b ){
            double diff = fabs( preFitYield - yieldVector[b].first );
            if( diff < minDifference ){                    
                minDifference = diff;
                bin = b;
                process = mapEntry.first;
            }
        }
    }
    std::cout << "process = " << process << "\tbin = " << bin << std::endl;
    std::cout << "end loop over map" << std::endl;
    
    return {process, bin};
}

double PostFitScaler::postFitYield( const double preFitYield ) const{
    std::pair< std::string, size_t> processAndBin = findProcessAndBin( preFitYield );
    const auto& mapIt = postAndPreFitYields.find( processAndBin.first );
    const auto& yieldVector = mapIt->second;
    const auto& yieldPair = yieldVector[ processAndBin.second ];
    return yieldPair.second;
}

double PostFitScaler::postFitScaling( const double preFitYield ) const{
    double newYield = postFitYield( preFitYield );
    return ( newYield / preFitYield );
}
