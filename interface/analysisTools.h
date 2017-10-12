#ifndef tools_h
#define tools_h

//include c++ library classes
#include<tuple>
#include<string>
//include ROOT classes
#include "TH1D.h"

namespace tools{
	//show progress bar
	void printProgress(double);
	//Set negative histogram bins to zero
	void setNegativeZero(TH1D*);
	void removeBackSpaces(std::string& s);
	void removeFrontSpaces(std::string&s);
	void cleanSpaces(std::string& s);
	std::string extractFirst(std::string& s);
	std::tuple<std::string, std::string, double> readSampleLine(std::string& line);
}
#endif
