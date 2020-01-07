#include "../interface/Sample.h"

//include c++ library classes 
#include <sstream>
#include <fstream>
#include <stdexcept>

//include other parts of code 
#include "../interface/stringTools.h"
#include "../interface/systemTools.h"
#include "../interface/analysisTools.h"


Sample::Sample( const std::string& line, const std::string& sampleDirectory ) :
    _directory( sampleDirectory)
{
    /*
    only works if input line is formatted as:
    processName    fileName    xSec
    */
    std::string xSecString;   //temporary string to read xSection
    std::string signalString; //temporary string to fill signal boolean

    //first 3 words on the line are the process name, filename and cross section
    std::istringstream stream(line);
    stream >> _processName >> _fileName >> xSecString;

    //if Xsection is not specified it is zero
    _xSec = ( xSecString == "" ? 0 : std::stod( xSecString ) );

    setIsData();
    auto is2017Or2018 = analysisTools::fileIs2017Or2018( _fileName );
    _is2017 = is2017Or2018.first;
    _is2018 = is2017Or2018.second;

    //unique name is equal to fileName without file extension
    _uniqueName = stringTools::fileNameWithoutExtension( _fileName );

    //data should not have an xSection
    if( isData() && xSecString != "" ){
        std::string errorMessage = "Error in sample list on line:\n";
        errorMessage += ( line + "\n" );
        errorMessage += "xSection specified for data sample, this is not allowed!"; 
        throw std::invalid_argument( errorMessage );
    }

    //extract all optional strings at the end of the line
    std::string optionString;
    std::string tempString;
    while( stream ){
        stream >> tempString;
        optionString.append( tempString );
    }

    //read options
    //This might modify uniqueName. uniqueName has to be set before calling this function!
    setOptions( optionString );
}


Sample::Sample( const std::string& directory, const std::string& fileName, const bool is2017, const bool is2018, const bool isData, const std::string& processName, const double xSec, const bool isSMSignal, const bool isNewPhysicsSignal ) :
    _fileName( fileName ),
    _directory( directory ),
    _uniqueName( stringTools::formatDirectoryName( _directory ) + _fileName ),
    _processName( processName ),
    _xSec( xSec ),
    _isData( isData ),
    _is2017( is2017 ),
    _is2018( is2018 ),
    _isSMSignal( isSMSignal ),
    _isNewPhysicsSignal( isNewPhysicsSignal )
{}


Sample::Sample( const std::string& pathToFile, const bool is2017, const bool is2018, const bool isData, const std::string& processName, const double xSec, const bool isSMSignal, const bool isNewPhysicsSignal ) :
    _processName( processName ),
    _xSec( xSec ),
    _isData( isData ),
    _is2017( is2017 ),
    _is2018( is2018 ),
    _isSMSignal( isSMSignal ),
    _isNewPhysicsSignal( isNewPhysicsSignal )
{
    std::pair< std::string, std::string > directoryAndFileName = stringTools::splitDirectoryFileName( pathToFile );
    _directory = directoryAndFileName.first;
    _fileName = directoryAndFileName.second;
    _uniqueName = stringTools::formatDirectoryName( _directory ) + _fileName;
}



//check whether line in the sample list txt file should be read to build a Sample
bool considerLine( const std::string& line ){
    
    //if line starts with # it is a comment and should be ignored 
    if( line[ line.find_first_not_of( " \t" ) ] == '#' ){
        return false;
    }

    //if line only contains spaces and tabs, it should be ignored 
    //WARNING : CHECK WHETHER THIS IS CORRECT WHEN INTERNET WORKS
    if( line.find_first_not_of( " \t" ) == std::string::npos ){
        return false;
    }    

    return true;
}


Sample::Sample( std::istream& is, const std::string& directory ){

    //read sample info from txt file
    std::string line;

    bool lineToConsider;
    do{
        lineToConsider = true;
        if( std::getline( is, line ) ){
            
            //skip comments or empty lines 
            lineToConsider = considerLine( line );
            if( !lineToConsider ) continue;

            *this = Sample( line, directory ); 
        }
    } while( !lineToConsider );
}



void Sample::setIsData(){
    _isData = false;
    static std::vector<std::string> dataNames = { "data", "SingleMuon", "SingleElectron", "SingleMuon", "DoubleMuon", "DoubleEG", "EGamma", "JetHT", "MET" };
    for( auto it = dataNames.cbegin(); it != dataNames.cend(); ++it ){
        if( _fileName.find( *it ) != std::string::npos ){
            _isData = true;
        }
    }
}


void Sample::setOptions( const std::string& optionString ){

    if( optionString == "" ){
        _isSMSignal = false;
        _isNewPhysicsSignal = false;
        return;
    } 

    //signal flags
    //determine whether process is some kind of signal
    _isSMSignal = ( optionString.find( "SMSignal" ) != std::string::npos );
    _isNewPhysicsSignal = ( optionString.find( "newPhysicsSignal" ) != std::string::npos );

    //signal can not be both SM and BSM sigal
    if( _isSMSignal && _isNewPhysicsSignal ){
        throw std::invalid_argument( "Error in sample construction: sample is both SM and BSM signal!" );
    }
    
    //check if sample needs to be used in different era it was intended for (i.e. 2016 sample when comparing to 2017 or 2018 data, or vice-versa)
    bool flag2016 = stringTools::stringContains( optionString, "forceIs2016" );
    bool flag2017 = stringTools::stringContains( optionString, "forceIs2017" );
    bool flag2018 = stringTools::stringContains( optionString, "forceIs2018" );

    if( flag2016 && flag2017 ){
        throw std::invalid_argument( "Error in sample construction: both forceIs2016 and forceIs2017 flags were set, can not set both " );
    }

    if( flag2016 ){
        _is2017 = false;
        _is2018 = false;
        _uniqueName += "_forcedIs2016";
    } else if( flag2017 ){
        _is2017 = true;
        _is2018 = false;
        _uniqueName += "_forcedIs2017";
    } else if( flag2018 ){
        _is2017 = false;
        _is2018 = true;
        _uniqueName += "_forcedIs2018";
    }
}


std::shared_ptr<TFile> Sample::filePtr() const{
    return std::make_shared<TFile>( ( stringTools::formatDirectoryName( _directory ) + _fileName ).c_str() , "read");
}


//print Sample info
std::ostream& operator<<( std::ostream& os, const Sample& sam ){
    os << sam._processName << "\t" << 
        sam._fileName << "\t" << 
        sam._xSec << "\t" << 
        ( sam._isData ? "data" : "MC" ) << "\t" << 
        ( sam.is2016() ? "Summer16" : ( sam._is2017 ? "Fall17" : "Autumn18" ) ) << 
        ( sam._isSMSignal ? "\tSM signal" : "" ) << 
        ( sam._isNewPhysicsSignal ? "\tBSM signal" : "" );
    return os;
}


//read a list of samples into a vector 
std::vector< Sample > readSampleList( const std::string& listFile, const std::string& directory ){

    //check if input file exists 
    if( !systemTools::fileExists( listFile ) ){
        throw std::invalid_argument( "Sample list '" + listFile + "' does not exist." );
    }
    
	
	std::vector< Sample> sampleList;

    //read sample info from txt file
    std::ifstream inFile(listFile);
    while( !inFile.eof() ){
        sampleList.push_back( Sample( inFile, directory ) );
    }
    sampleList.pop_back();

    //close file after usage
    inFile.close();

    return sampleList;
}
