// executable C++ file that calls the function mergeAndRemoveOverlap from the framework

// include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// inlcude function to call
#include "../../Tools/interface/mergeAndRemoveOverlap.h"

int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;
    
    if( argc < 4 ){
        std::cerr << "### ERROR ###: mergeAndRemoveOverlap.cc requires at least 3 arguments to run: ";
        std::cerr << "output_file_name, at least two input_files"<<std::endl;
        return -1;
    }
    
    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& output_file_path = argvStr[1];
    std::vector<std::string> input_files;
    for(int i=2; i<argc; i++){
	input_files.push_back(argvStr[i]);
    }

    mergeAndRemoveOverlap(input_files, output_file_path, false);
    std::cerr << "###done###" << std::endl;
}
