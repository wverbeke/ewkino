/*****************************************************************
 * C++ code to call the plotting method built into the framework *
******************************************************************/
// to be used after histfiller.py had been called to fill the histogram data structures!

#include <iostream>
#include <string>
#include "../../plotting/plotCode.h"
//#include "TFile.h"
//#include "TH1D.h"

int makePlot(const std::string& input_file_path, 
              const std::string& output_file_path, 
              const std::string& variable_name,
              const std::string& variable_title="",
              const std::string& variable_unit=""){
    TFile* filePtr = TFile::Open(input_file_path.c_str(), "");
    // find number of MC histograms
    int i = 0;
    std::string name = std::string("mc_"+variable_name+"_")+std::to_string(i);
    while(filePtr->Get(name.c_str())){
        i++;
        name = "mc_"+variable_name+"_"+std::to_string(i);
    }
    int nmcfiles = i;
    // read MC histograms
    std::string labels[nmcfiles+1];
    TH1D* mchistPtrArray [nmcfiles];
    for(int i=0; i<nmcfiles; i++){
        name = std::string("mc_"+variable_name+"_")+std::to_string(i);
        mchistPtrArray[i] = (TH1D*) filePtr->Get(name.c_str());
        labels[i+1] = mchistPtrArray[i]->GetTitle();
    }
    // read data histogram
    TH1D* datahistPtr;
    name = "data_"+variable_name;
    if(!filePtr->Get(name.c_str())){ 
        std::cout<<"data histogram not found, exiting."<<std::endl;
        return -1;
    }
    datahistPtr = (TH1D*) filePtr->Get(name.c_str());
    labels[0] = datahistPtr->GetTitle();
    // prepare additional arguments for calling the plot function
    std::string analysis = "";
    std::string header = "";
    std::string variable_title_new = variable_title;
    if(variable_title == "") variable_title_new = variable_name;
    TH1D* mcSyst = (TH1D*) datahistPtr->Clone(); mcSyst->Reset();
    plotDataVSMC(datahistPtr, mchistPtrArray, labels, nmcfiles, 
                    output_file_path, analysis, 
                    false, false, header, mcSyst);
    return 0;
}

int main(int argc, char* argv[]){
    if(argc != 4 and argc != 5 and argc != 6){
        std::cout<<"### ERROR: histplotter.cc requires a different number of command-line arguments"<<std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string input_file_path = argvStr[1];
    std::string output_file_path = argvStr[2];
    std::string variable_name = argvStr[3];
    std::string variable_title = "";
    std::string variable_unit = "";
    if(argc>4) variable_title = argvStr[4];
    if(argc>5) variable_unit = argvStr[5];
    makePlot(input_file_path,output_file_path,variable_name,variable_title,variable_unit);
    return 0;
}
