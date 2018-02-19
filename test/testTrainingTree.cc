#include "../interface/TrainingTree.h"
#include "../interface/Sample.h"

//include c++ library classes
#include <iostream>
#include <fstream>
#include <vector>

int main(){
    std::vector<Sample> samples;
    std::ifstream file("sampleLists/samples_dilepCR.txt");
    do {
        samples.push_back(Sample(file));
    } while(!file.eof());
    samples.pop_back();
    file.close();       //close file after usage    

    //try to make a training tree for every sample
    //variable map:
    std::map< std::string, double> variableMap;
    variableMap["testVar1"] = 0;
    variableMap["testVar2"] = 0;
    variableMap["testVar3"] = 0;
    for(auto& sam : samples){
        TrainingTree testTree(sam, { {"A", "B", "C"}, {"X", "Y", "Z"} }, variableMap);
        for(unsigned c = 0; c < 9; ++c){
            testTree.fill(c, true, variableMap); 
            testTree.fill(c, false, variableMap); 
        }
    }
    return 0;

} 
