#include "../interface/systemTools.h"

//Include c++ library classes
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstdlib>

//include other parts of code 
#include "../interface/stringTools.h"


//call std::system with std::string argyment
void systemTools::system( const std::string& command ){
    int response = std::system(command.c_str() );
    if( response < 0 ){
        throw std::runtime_error( "system command '" + command + " failed." );
    }
}


void systemTools::deleteFile( const std::string& fileName ){
    systemTools::system( "rm " + fileName );
}


void systemTools::makeFile( const std::string& fileName ){
    systemTools::system( "touch " + fileName );
}


void systemTools::makeDirectory( const std::string& directoryName ){
    systemTools::system( "mkdir -p " + directoryName );
}


void systemTools::sleep( unsigned seconds ){
    std::this_thread::sleep_for(std::chrono::milliseconds(seconds*1000));
}


bool systemTools::fileExists( const std::string& fileName ){
    std::ifstream inputStream( fileName );
    return inputStream.good();
}


//check if file name is already in use, and return a randomized filename if it is 
std::string systemTools::uniqueFileName( const std::string& fileName){

	std::string uniqueName = fileName;
    while( systemTools::fileExists(uniqueName) ){

        std::pair< std::string, std::string> fileAndExtension = stringTools::splitFileExtension( fileName );

        //add random number to temporary file name
        uniqueName = fileAndExtension.first + "_" + std::to_string( std::rand() ) + fileAndExtension.second;
    }
	return uniqueName;
} 

//read lines of text file to vector 
std::vector < std::string > systemTools::readLines( const std::string& textFile, const std::string& mustContain, const std::string& mustEndWith){
	std::vector<std::string> lines;
    std::ifstream textStream(textFile);
    std::string line;
    while(std::getline(textStream, line)){
        if(!line.empty() && stringTools::stringContains(line, mustContain) && stringTools::stringEndsWith(line, mustEndWith) ){
            lines.push_back( stringTools::cleanSpaces(line) );
        }
    }
    textStream.close();
    return lines;
}

//check number of columns in tabular txt file
unsigned systemTools::numberOfColumnsInFile( const std::string& textFile ){
    
    //read file 
    std::ifstream fileStream( textFile );

    //read first line, and second if possible since the first line tends to be some header 
    std::string line;
    std::string lineToCheck;
    if( std::getline( fileStream, line ) ){
        lineToCheck = line;
        if( std::getline( fileStream, line ) ){
            lineToCheck = line; 
        }
    } else {
        return 0;
    }

    //compute number of columns in line
    std::istringstream lineStream( lineToCheck );
    unsigned numberOfColumns = 0;
    do{
        std::string entry;
        lineStream >> entry;
        if ( !( entry.empty() ) ){
            ++numberOfColumns;
        }
    } while( lineStream );
    
    return numberOfColumns;
}

//read first line of a file 
std::string systemTools::readFirstLine( const std::string& textFile){
    std::ifstream textStream(textFile);
    std::string line;
    std::getline(textStream, line);
    textStream.close();
    return stringTools::cleanSpaces(line);
}


//output list of files in given directory to a txt file
std::vector<std::string> systemTools::listFiles( const std::string& directory, const std::string& mustContain, const std::string& fileExtension ){

    //write list of files to temporary file
    //make sure file does not exist to make application thread safe 
	std::string outFileName = "fileList.txt";
    outFileName = systemTools::uniqueFileName(outFileName);

    //remake the file
    systemTools::makeFile( outFileName );

    //pipe directory contents to file
    systemTools::system( "for f in " + stringTools::formatDirectoryName(directory) + "*; do echo $f >> " + outFileName + "; done");

    //read the txt file into a vector
	std::vector< std::string > fileList = systemTools::readLines(outFileName, mustContain, fileExtension);

    //clean up temporary file
    system( "rm " + outFileName );

    return fileList;
}


//get CMSSW version that is currently used
std::string systemTools::CMSSWDirectory(){

    //cache CMSSW version in static string since it should not change during program execution
    static std::string cmsswDir = "";
    if(cmsswDir == ""){
        std::string outFileName = systemTools::uniqueFileName( "cmsswDir.txt" );
        systemTools::system("echo $CMSSW_BASE > " + outFileName);
        cmsswDir = systemTools::readFirstLine( outFileName );       
        systemTools::deleteFile(outFileName);
    }
    return cmsswDir;
}


//initialize a submission script for running on cluster
std::ostream& systemTools::initJobScript(std::ostream& os, const std::string& CMSSWDir ){
    if( CMSSWDir == "" ){
        os << "cd " << systemTools::CMSSWDirectory() << "\n";
    } else {
        os << "cd " << CMSSWDir << "\n";
    }
    os << "source /cvmfs/cms.cern.ch/cmsset_default.sh \n";
    os << "eval `scram runtime -sh` \n";
    os << "cd " << systemTools::currentDirectory() << "\n";     //go back to directory from where job was submitted 
    return os;
}


//submit script as cluster job and catch potential errors
void systemTools::submitScript( const std::string& scriptName, const std::string& walltime, const std::string& queue, const unsigned numberOfThreads ){

    //as long as submission failed, sleep and try again
    bool submitted = false;
    do{

        //submit script and pipe output to text file to check if submission succeeded
        std::string outFileName =  systemTools::uniqueFileName("submissionOutput.txt");
        
        //check if extra arguments are given, specifying the number of threads or the submission queue 
        std::string extraArguments;
        if( queue != "" ){
            extraArguments += ( " -q " + queue );
        } 
        if( numberOfThreads != 1 ){
            extraArguments += ( " -lnodes=1:ppn=" + std::to_string( numberOfThreads ) );
        }

        system( "qsub " + scriptName + " -l walltime=" + walltime + extraArguments + " > " + outFileName + " 2>> " + outFileName );
        std::ifstream submissionOutput(outFileName);

        //check for errors in output file
        static std::vector< std::string > errorMessages = {"Invalid credential", "Expired credential", "Error"};
        std::string line; 
        while(std::getline(submissionOutput, line)){

            bool errorFound = false;
            for(const std::string& message : errorMessages ){
                if( stringTools::stringContains(line, message) ){
                    errorFound = true;
                    break;
                }
            }

            if( !errorFound ){
                submitted = true;
                std::cout << line << std::endl;
            }
        }
        submissionOutput.close();

        //delete temporary file 
        systemTools::deleteFile( outFileName );

        //sleep for 2 seconds before attempting resubmission
        if(!submitted){
            std::cerr << "submission failed: reattempting submission" << std::endl;
            sleep(2);
        }
    } while(!submitted);
}


void systemTools::submitCommandAsJob( const std::string& commandString, const std::string& scriptName, const std::string& walltime, const std::string& queue, const unsigned numberOfThreads ){
    
    //make script to submit
	std::ofstream scriptToSubmit( scriptName );
	systemTools::initJobScript( scriptToSubmit );
	scriptToSubmit << ( commandString  );
	scriptToSubmit.close();

	//submit job
	systemTools::submitScript( scriptName, walltime, queue, numberOfThreads );

	//clean up
	systemTools::deleteFile( scriptName );
}


//check whether there are running jobs
bool systemTools::runningJobs( const std::string& jobName ){
    std::string job;

    //if jobname is not specified, look for any running job:
    if(jobName == ""){
        job = "cream02";
    }

    //if the jobname is specified limit it to the last 10 characters, since qstat will not display all
    else{
        job = std::string(jobName);
        job = std::string(job.cend() - 11, job.cend());
    }

    //pipe qstat output to temporary txt file
    std::string outFileName = systemTools::uniqueFileName("runningJobs.txt");
    systemTools::system("qstat -u$USER > " + outFileName);
    std::vector< std::string > currentJobs = systemTools::readLines(outFileName, job);

    //clean temporary file 
    systemTools::deleteFile(outFileName);

    //check if any jobs are running
    return ( currentJobs.size() != 0 );
}


//return current working directory as string
std::string systemTools::currentDirectory(){
    static std::string directory = ""; 

    //if the current directory is not determined yet interact with terminal to retrieve it
    //directory is assumed not to change during program execution
    if(directory == ""){
        std::string outFileName = systemTools::uniqueFileName("directory.txt");
        systemTools::system("echo $PWD > "  + outFileName);
        directory = readFirstLine( outFileName );
        systemTools::deleteFile(outFileName);
    }
    return directory;
}

