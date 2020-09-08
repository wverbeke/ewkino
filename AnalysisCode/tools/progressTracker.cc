// A standalone class that allows progress tracking for grid jobs by writing to a temporary .txt file

#include "progressTracker.h"

progressTracker::progressTracker( std::string filename ):
    _filename( filename )
{
    // check if filename has correct extension
    size_t dotPosition = filename.find_last_of( "." );
    std::string filename_noext;
    if( dotPosition == std::string::npos ) filename_noext = filename;
    else filename_noext = filename.substr( 0, dotPosition);
    // check if file already exists
    std::ifstream temp;
    temp.open( filename_noext+".txt" );
    while(temp.good()){
	filename_noext = filename_noext+"_temp";
	temp.open( filename_noext+".txt" );
    }
    filename = filename_noext+".txt";
    _filename = filename;
    // split into file and directory name
    size_t splitPosition = filename.find_last_of("/");
    if( splitPosition != std::string::npos ){
	// make directory
	std::string command = "mkdir -p "+filename.substr(0,splitPosition);
	std::system( command.c_str() );
    }
    // make file
    _progressfile.open( filename );
    _progressfile << "--- progress tracking file has been opened ---\n\n";
    _progressfile.close();
}

void progressTracker::writeProgress( double progressFraction ){
    // convert double to string
    std::ostringstream stringStream;
    stringStream << std::setprecision( 2 ) << progressFraction*100.;
    std::string progressString = stringStream.str();
    std::string message = "progress: "+progressString+" %";
    // write message 
    writeMessage( message );   
}

void progressTracker::writeMessage( const std::string& message ){
    _progressfile.open(_filename,std::ios_base::app);
    _progressfile << message << "\n";
    _progressfile.close();
}

void progressTracker::close(){
    std::remove( _filename.c_str() );
}
