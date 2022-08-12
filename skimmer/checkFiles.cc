/*
Check for file corruption
*/

// supposed to be run on the output files of skimming and/or merging jobs,
// in order to check if all files are readable, or if some form of corruption occurred.

// include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include other parts of the framework
#include "../TreeReader/interface/TreeReader.h"


bool checkReadability(const std::string& pathToFile){
    // returns false if something seems to be wrong with the file.
    // to be extended with more severe checking once more issues pop up.
    TreeReader treeReader;
    try{treeReader.initSampleFromFile( pathToFile );}
    catch( std::domain_error& ){ return false; }
    return true;
}


int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;

    if( argc < 2 ){
        std::cerr << "ERROR: need at least one file to check as command line arg. " << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::vector< std::string > files;
    for( unsigned int i=1; i<argvStr.size(); i++ ){ files.push_back(argvStr[i]); }

    // loop over files
    int nfiles = files.size();
    int nerror = 0;
    std::cout << "checking " << nfiles << " files..." << std::endl;
    for( std::string f: files ){
	bool fileIsReadable = checkReadability(f);
	if( !fileIsReadable ){
	    std::string msg = "found issue in file ";
	    msg += f;
	    std::cout << msg << std::endl;
	    nerror++;
	}
    }

    // print conclusion
    std::cout << "number of files checked: " << nfiles << std::endl;
    std::cout << "number of files with errors: " << nerror << std::endl;
    std::cout << "number of files without apparent error: " << nfiles - nerror << std::endl;
    
    std::cerr << "###done###" << std::endl;
}

