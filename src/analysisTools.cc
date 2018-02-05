#include "../interface/analysisTools.h"

//Include c++ library classes
#include <iostream>
#include <string>
#include <tuple>
#include <fstream>
//for sleeping:
#include <chrono>
#include <thread>

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
	if(h->GetBinContent(b) < 0.) h->SetBinContent(b, 0.);	
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


//Function to print dataCard to be analysed by CMS combination tool
void tools::printDataCard(const double obsYield, const double sigYield, const std::string& sigName, const double* bkgYield, const unsigned nBkg, const std::string* bkgNames, const std::vector<std::vector<double> >& systUnc, const unsigned nSyst, const std::string* systNames, const std::string* systDist, const std::string& cardName, const bool shapeCard, const std::string& shapeFileName){
    std::ofstream card;
    card.open(cardName + ((cardName.find(".") == std::string::npos) ? ".txt" : "") ); //add .txt to name if no file extension is given
    //define number of channels, background sources and systematics
    card << "imax 1 number of channels \n";
    card << "jmax " << nBkg << " number of backgrounds \n";
    card << "kmax " << nSyst << " number of nuisance parameters (sources of systematical uncertainties) \n";
    card << "---------------------------------------------------------------------------------------- \n";
    //define the channels and the number of observed events
    card << "bin 1 \n";
    card << "observation " << obsYield << "\n";
    //define all backgrounds and their yields
    card << "---------------------------------------------------------------------------------------- \n";
    if(shapeCard){
	card << "shapes * * " << shapeFileName + ".root  $PROCESS $PROCESS_$SYSTEMATIC\n";
	card << "---------------------------------------------------------------------------------------- \n";
    }
    card << "bin	";
    for(unsigned proc = 0; proc < nBkg + 1; ++proc){
	card << "	" << 1;
    }
    card << "\n";
    card << "process";
    card << "	" << sigName;
    for(unsigned bkg = 0; bkg < nBkg; ++bkg){
	card << "	" << bkgNames[bkg];
    }
    card << "\n";
    card << "process";
    for(unsigned bkg = 0; bkg < nBkg + 1; ++bkg){
	card << "	" << bkg;
    }
    card << "\n";
    card <<	"rate";
    card << "	" << sigYield;
    for(unsigned bkg = 0; bkg < nBkg; ++bkg){
	if(bkgYield[bkg] <= 0) card << "	" << "0.00";
	else card << "	" << bkgYield[bkg];
    }
    card << "\n";
    card << "---------------------------------------------------------------------------------------- \n";
    //define sources of systematic uncertainty, what distibution they follow and how large their effect is
    for(unsigned syst = 0; syst < nSyst; ++syst){
	card << systNames[syst] << "	" << systDist[syst];
	for(unsigned proc = 0; proc < nBkg + 1; ++proc){
	    card << "	";
	    if(systUnc[syst][proc] == 0) card << "-";
	    else card << systUnc[syst][proc];
	}
	card << "\n";
    }
    card.close();		
}


//initialize a submission script for running on cluster
std::ostream& tools::initScript(std::ostream& os){
     os << "cd /user/wverbeke/CMSSW_9_4_2/src \n";
     os << "source /cvmfs/cms.cern.ch/cmsset_default.sh \n";
     os << "eval \\`scram runtime -sh\\` \n";
     os << "cd /user/wverbeke/Work/AnalysisCode/ewkino/ \n";
     return os;
}

//submit script as cluster job and catch errors
void tools::submitScript(const std::string& scriptName, const std::string& walltimeString){
    //as long as submission failed, sleep and try again
    bool submitted = false;
    do{
        //submit sctipt and pipe output to text file
        std::system( std::string("qsub " + scriptName + " -l walltime=" + walltimeString + " > submissionOutput.txt").c_str() );
        std::ifstream submissionOutput("submissionOutput.txt");
        //check for errors in output file
        std::string line; 
        while(std::getline(submissionOutput, line)){
           if(line.find("Invalid credential") != std::string::npos) submitted = true;
        }
        submissionOutput.close();
        //sleep for 2 seconds before attempting resubmission
        if(!submitted) std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    } while(!submitted);
}



























