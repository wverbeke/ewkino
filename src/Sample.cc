#include "../interface/Sample.h"

void Sample::setData(){
    isDataSample = false;
    static std::vector<std::string> dataNames = {"data", "SingleMuon", "SingleElectron", "SingleMuon", "DoubleMuon", "DoubleEG"};
    for(auto it = dataNames.cbegin(); it != dataNames.cend(); ++it){
        if(fileName.find(*it) != std::string::npos){
            isDataSample = true;
        }
    }
}

void Sample::set2017(){
    is2017Sample = (fileName.find("Fall17") != std::string::npos);
}

Sample::Sample(const std::string& line){
    /*
    only works if input line is formatted as:
    processName    fileName    xSec
    */
    std::string xSecString;   //temporary string to read xSection
    std::string signalString; //temporary string to fill signal boolean
    //read all variables on the line
    std::istringstream stream(line);
    stream >> process >> fileName >> xSecString >> signalString;

    //if not Xsection is specified it is zero
    xSec = (xSecString == "" ? 0 : std::stod(xSecString) );

    //determine whether process is some kind of signal
    smSignal = (signalString.find("SMSignal") != std::string::npos);
    newPhysicsSignal = (signalString.find("newPhysicsSignal") != std::string::npos);

    //signal can not be both SM and BSM sigal
    if(smSignal && newPhysicsSignal){
        std::cerr << "Error in sample construction: sample is both SM and BSM signal" << std::endl;
    }

    setData();
    set2017();

    //data has no xSection
    if(isData() && xSecString != ""){
        std::cerr << "xSection specified for data: are you sure this was intended?" << std::endl;
    }
}

Sample::Sample(std::istream& is){
    //read sample info from txt file
    std::string line;
    //jump to next line if current line is a comment
    do{
        if(std::getline(is, line)){
            *this = Sample(line); 
        }
    } while( line[line.find_first_not_of(" \t")] == '#');
}

std::shared_ptr<TFile> Sample::getFile(const std::string& directory) const{
    std::string temp = directory;
    if(*(temp.cend() - 1) == '/') temp.pop_back();
    return std::make_shared<TFile>((const TString&) temp + "/" + fileName, "read");
}

//print Sample info
std::ostream& operator<<(std::ostream& os, const Sample& sam){
    os << sam.process << "\t" << sam.fileName << "\t" << sam.xSec << "\t" << ( sam.isData() ? "data" : "MC") << "\t" << ( sam.is2017() ? "Fall17" : "Summer16" ) << (sam.smSignal ? "\tSM signal" : "") << (sam.newPhysicsSignal ? "\tBSM signal" : "");
    return os;
}
