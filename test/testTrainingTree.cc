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
    std::map< std::string, float> variableMap;
    variableMap["testVar1"] = 0;
    variableMap["testVar2"] = 0;
    variableMap["testVar3"] = 0;
    unsigned j = 0;
    for(auto& sam : samples){
        TrainingTree* tree;
        if(j%2 == 0) tree = new TrainingTree(sam, { {"A", "B", "C"}, {"X", "Y", "Z"} }, variableMap, true);
        else tree = new TrainingTree(sam, { {"A", "B", "C"}, {"X", "Y", "Z"} }, variableMap, false);
        for(unsigned c = 0; c < 9; ++c){
            for(int i = 0; i < 100; ++i){
                variableMap["testVar1"] = (double) i;
                variableMap["testVar2"] = (double) i;
                variableMap["testVar3"] = (double) i;
                tree->fill(c, variableMap); 
            }
        }
        delete tree;
    }
    return 0;

} 
