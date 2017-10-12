#include "../interface/analysisTools.h"

//Include c++ library classes
#include <iostream>
#include <string>
#include <tuple>

void tools::printProgress(double progress){
	const unsigned barWidth = 100;
    std::cout << "[";
    unsigned pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << unsigned(progress * 100.0) << " %\r" << std::flush;
}

void tools::setNegativeZero(TH1D* h){
	for(unsigned b = 1; b < h->GetNbinsX() + 1; ++b){
		if(h->GetBinContent(b) == 0.) h->SetBinContent(b, 0.);	
	}
}

void tools::removeBackSpaces(std::string& s){
	while(s.find_last_of("\t") == s.size() - 1 || s.find(" ") == s.size() - 1){
		s.erase(s.size() - 1);
	}
}

void tools::removeFrontSpaces(std::string&s){
	while(s.find("\t") == 0 || s.find(" ") == 0){
		s.erase(0, 1);
	}
}

void tools::cleanSpaces(std::string& s){
	//remove tabs and spaces in the front of the string
	removeFrontSpaces(s);
	//remove tabs and spaces in the end of the string
	removeBackSpaces(s);
}

std::string tools::extractFirst(std::string& s){
	auto space = std::min(s.find(" "), s.find("\t") ); //tab and space!
	std::string substr = s.substr(0, space);
	s.erase(0, space);	
	removeFrontSpaces(s);
	return substr;
}

std::tuple<std::string, std::string, double> tools::readSampleLine(std::string& line){
	cleanSpaces(line);
	std::string name = extractFirst(line);
	std::string sample = extractFirst(line);
	std::string xSecStr = extractFirst(line);
	xSecStr = (xSecStr == "") ? "0": xSecStr;
	double xSec = std::stod(xSecStr);
	return std::make_tuple(name, sample, xSec);
}
