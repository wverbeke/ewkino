// A standalone class that allows progress tracking for grid jobs by writing to a temporary .txt file

#ifndef progressTracker_H
#define progressTracker_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

class progressTracker{
    
    public:
	// constructor
	progressTracker( std::string filename );
	
	// member functions
	std::string getFileName() { return _filename; }
	void writeProgress( double progressFraction );
	void writeMessage( const std::string& message );
	void close();

    private:
	std::string _filename;
	std::fstream _progressfile;
};

#endif
