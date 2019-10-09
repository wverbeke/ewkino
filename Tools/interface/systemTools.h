/*
methods to interact with the system 
*/

#ifndef systemTools_h
#define systemTools_h

//include c++ library classes
#include <string>
#include <iostream>
#include <vector>

namespace systemTools{

    //call std::system, but allow an std::string to be used
    void system( const std::string& );

    //delete file of given name
    void deleteFile( const std::string& );

    //make file of given name
    void makeFile( const std::string& );

    //make directory of given name 
    void makeDirectory( const std::string& );

    //check if file exists
    bool fileExists( const std::string& );

    //make sure output file name is unused
    std::string uniqueFileName( const std::string& );

    //sleep for x seconds
    void sleep( unsigned );

    //convert a txt file to a vector of lines 
    std::vector< std::string > readLines( const std::string& fileName, const std::string& mustContain = "", const std::string& mustEndWith = "");

    //check number of columns in tabular txt file
    unsigned numberOfColumnsInFile( const std::string& fileName );

	//get first line of file
	std::string readFirstLine( const std::string& textFile);

    //list all files in a directory and save this to a txt file of given name
    std::vector< std::string > listFiles( const std::string& directory, const std::string& mustContain = "", const std::string& fileExtension = "" );

    //get current directory
    std::string currentDirectory();

    //get CMSSW directory currently used
    std::string CMSSWDirectory();

    //initialize a submission script
    std::ostream& initJobScript( std::ostream&, const std::string& CMSSWDir = "" );

    //submit script of given name and catch submission errors
    void submitScript( const std::string&, const std::string&, const std::string& queue = "", const unsigned numberOfThreads = 1 );

    //check whether there are running jobs
    bool runningJobs( const std::string& jobName = "" );
}
#endif
