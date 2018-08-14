#include "../interface/stringTools.h"
#include "../interface/Sample.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


int main(){
    std::cout << "########### reading 2016 sample list ##################" << std::endl;
    std::vector< Sample > sampleList = readSampleList("../sampleLists/samples_dilepCR_2016.txt", "/");
    for(auto& sample : sampleList ){
        std::cout << sample << std::endl;
    }
    std::cout << "########### reading 2017 sample list ##################" << std::endl;
    sampleList = readSampleList("../sampleLists/samples_dilepCR_2017.txt", "/");
    for(auto& sample : sampleList ){
        std::cout << sample << std::endl;
    }
    return 0;
}
