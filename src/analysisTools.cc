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
    for (unsigned i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << unsigned(progress * 100.0) << " %\r" << std::flush;
}

void tools::setNegativeZero(TH1D* h){
    for(int b = 1; b < h->GetNbinsX() + 1; ++b){
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
    os << "cd /user/wverbeke/CMSSW_9_4_4/src \n";
    os << "source /cvmfs/cms.cern.ch/cmsset_default.sh \n";
    os << "eval \\`scram runtime -sh\\` \n";
    os << "cd /user/wverbeke/Work/AnalysisCode/ewkino/ \n";
    os << "cd " << currentDirectory() << "\n";     //go back to directory from where job was submitted 
    return os;
}

void tools::sleep(unsigned seconds){
    std::this_thread::sleep_for(std::chrono::milliseconds(seconds*1000));
}

//submit script as cluster job and catch errors
void tools::submitScript(const std::string& scriptName, const std::string& walltimeString){
    //as long as submission failed, sleep and try again
    bool submitted = false;
    do{
        //submit sctipt and pipe output to text file
        std::system( std::string("qsub " + scriptName + " -l walltime=" + walltimeString + " > submissionOutput.txt 2>> submissionOutput.txt").c_str() );
        std::ifstream submissionOutput("submissionOutput.txt");
        //check for errors in output file
        std::string line; 
        while(std::getline(submissionOutput, line)){
            if(line.find("Invalid credential") == std::string::npos){
                submitted = true;
                std::cout << line << std::endl;
            }
        }
        submissionOutput.close();
        //sleep for 2 seconds before attempting resubmission
        if(!submitted){
            std::cerr << "submission failed: reattempting submission" << std::endl;
            sleep(2);
        }
    } while(!submitted);
}

//check whether there are running jobs
bool tools::runningJobs(const std::string& jobName){
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
    std::system("qstat -u$USER > runningJobs.txt");
    std::ifstream jobFile("runningJobs.txt");
    std::string line;
    while(std::getline(jobFile, line)){
        if(line.find(job) != std::string::npos){
            std::system("rm runningJobs.txt");
            return true;
        }
    }
    std::system("rm runningJobs.txt");
    return false;
}

//return current working directory as string
std::string tools::currentDirectory(){
    static std::string directory = ""; 
    //if the current directory is not determined yet interact with terminal to retrieve it
    if(directory == ""){
        std::system("echo $PWD > directory.txt");
        std::ifstream directoryFile("directory.txt");
        std::getline(directoryFile, directory);
        directoryFile.close();
        std::system("rm directory.txt");
        //clean up spaces in resulting string
        cleanSpaces(directory);
    }
    return directory;
}

//output list of files in given directory to a txt file
void tools::makeFileList(const std::string& directory, const std::string& outputFileName){
    //first clean up file if it already existed
    std::system( std::string("rm " + outputFileName).c_str() );
    //remake the file
    std::system( std::string("touch " + outputFileName).c_str() );
    //append / to directory name if needed
    std::string extraSlash = "";
    if(directory.back() != '/') extraSlash = "/";
    //pipe directory contents to file
    std::system( std::string("for f in " + directory + extraSlash + "*; do echo $f >> " + outputFileName + "; done").c_str() );
}

void tools::deleteFile(const std::string& fileName){
    std::system( std::string("rm " + fileName).c_str() );
}
