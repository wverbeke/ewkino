#include "../interface/Sample.h"

//include c++ library classes 
#include <sstream>
#include <fstream>
#include <stdexcept>

//include other parts of code 
#include "../interface/stringTools.h"
#include "../interface/systemTools.h"
#include "../interface/analysisTools.h"


Sample::Sample( const std::string& line, 
		const std::string& sampleDirectory,
		const std::string& year ) :
    // note: year can be an empty string, in which case the year is exctracted from the filename;
    //       if year is not empty, it will override the filename extracted from the filename
    //	     (use this in case the filename does not contain an unambiguous year string,
    //        and the year needs to be extracted from e.g. the sample list)
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

    // set the correct year for this sample
    if( year.size()==0 ){
	_is2016 = analysisTools::fileIs2016( _fileName );
	_is2016PreVFP = analysisTools::fileIs2016PreVFP( _fileName );
	_is2016PostVFP = analysisTools::fileIs2016PostVFP( _fileName );
	_is2017 = analysisTools::fileIs2017( _fileName );
	_is2018 = analysisTools::fileIs2018( _fileName );
    } else{
	_is2016 = (year=="2016");
        _is2016PreVFP = (year=="2016PreVFP");
        _is2016PostVFP = (year=="2016PostVFP");
        _is2017 = (year=="2017");
        _is2018 = (year=="2018");
    }

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


Sample::Sample( const std::string& directory,
		const std::string& fileName, 
		const bool is2016,
		const bool is2016PreVFP,
		const bool is2016PostVFP,
		const bool is2017, 
		const bool is2018, 
		const bool isData, 
		const std::string& processName, 
		const double xSec, 
		const bool isSMSignal, 
		const bool isNewPhysicsSignal ) :
    // manual initialization of Sample
    _fileName( fileName ),
    _directory( directory ),
    _uniqueName( stringTools::formatDirectoryName( _directory ) + _fileName ),
    _processName( processName ),
    _xSec( xSec ),
    _isData( isData ),
    _is2016( is2016 ),
    _is2016PreVFP( is2016PreVFP ),
    _is2016PostVFP( is2016PostVFP ),
    _is2017( is2017 ),
    _is2018( is2018 ),
    _isSMSignal( isSMSignal ),
    _isNewPhysicsSignal( isNewPhysicsSignal )
{}


Sample::Sample( const std::string& pathToFile, 
		const bool is2016,
                const bool is2016PreVFP,
                const bool is2016PostVFP,		
		const bool is2017, 
		const bool is2018, 
		const bool isData, 
		const std::string& processName, 
		const double xSec, 
		const bool isSMSignal, 
		const bool isNewPhysicsSignal ) :
    // same as above but extract directory and filename automatically from given path
    _processName( processName ),
    _xSec( xSec ),
    _isData( isData ),
    _is2016( is2016 ),
    _is2016PreVFP( is2016PreVFP ),
    _is2016PostVFP( is2016PostVFP ),
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


std::string Sample::year() const{
    if( _is2016 ) return "2016";
    if( _is2016PreVFP ) return "2016PreVFP";
    if( _is2016PostVFP ) return "2016PostVFP";
    if( _is2017 ) return "2017";
    if( _is2018 ) return "2018";
    throw std::runtime_error("ERROR in Sample::year: no valid year was set for this sample.");
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


Sample::Sample( std::istream& is, 
		const std::string& directory, 
		const std::string& year ){

    //read sample info from txt file
    std::string line;

    bool lineToConsider;
    do{
        lineToConsider = true;
        if( std::getline( is, line ) ){
            
            //skip comments or empty lines 
            lineToConsider = considerLine( line );
            if( !lineToConsider ) continue;

            *this = Sample( line, directory, year ); 
        }
    } while( !lineToConsider );
}



void Sample::setIsData(){
    _isData = false;
    static std::vector<std::string> dataNames = { "data", "Data", 
	"SingleMuon", "SingleElectron", "DoubleMuon", "DoubleEG", "EGamma", 
	"MuonEG", "JetHT", "MET" };
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
    
    // check if sample needs to be used in different era it was intended for 
    // (i.e. 2016 sample when comparing to 2017 or 2018 data, or vice-versa)
    // note: not yet updated to UL samples, not recommended to be used in that case!
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
        ( sam.year() ) << 
        ( sam._isSMSignal ? "\tSM signal" : "" ) << 
        ( sam._isNewPhysicsSignal ? "\tBSM signal" : "" );
    return os;
}


//read a list of samples into a vector 
std::vector< Sample > readSampleList( const std::string& listFile, 
				      const std::string& directory,
				      bool useYearFromList ){
    // note: if useYearFromList is False (default), the year is extracted per sample from its name;
    //       if useYearFromList is True, the year is extracted from the sample list name
    //	     and overrides the year extracted from the samples individually.

    // check if input file exists 
    if( !systemTools::fileExists( listFile ) ){
        throw std::invalid_argument( "Sample list '" + listFile + "' does not exist." );
    }
    	
    std::vector< Sample> sampleList;
    
    // set year if required
    std::string year = "";
    if( useYearFromList ){
	if( stringTools::stringContains(listFile,"2016PreVFP") ) year = "2016PreVFP";
	else if( stringTools::stringContains(listFile,"2016PostVFP") ) year = "2016PostVFP";
	else if( stringTools::stringContains(listFile,"2016") ) year = "2016";
	else if( stringTools::stringContains(listFile,"2017") ) year = "2017";
	else if( stringTools::stringContains(listFile,"2018") ) year = "2018";
    }

    // read sample info from txt file
    std::ifstream inFile(listFile);
    while( !inFile.eof() ){
        sampleList.push_back( Sample( inFile, directory, year ) );
    }
    sampleList.pop_back();

    // close file after usage
    inFile.close();

    return sampleList;
}
