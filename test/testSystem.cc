#include "../interface/systemTools.h"
#include "../interface/stringTools.h"

#include <iostream>
#include <vector>
#include <string>


int main(){
    std::cout << "########### listing all files in directory ##################" << std::endl;
    std::vector< std::string > fileList = systemTools::listFiles("~/Downloads", "", ".txt");
    for( auto& file : fileList){
        std::cout << file << std::endl;
    }

    std::cout << "########### testing unique file names ################### " << std::endl;
    for( unsigned i = 0; i < 1000; ++i){
        std::string fileName = systemTools::uniqueFileName("test.txt");
        systemTools::makeFile(fileName);
    }
    
    std::cout << "########### list all testFiles ####################" << std::endl;
    fileList = systemTools::listFiles("./", "test", ".txt");
    for( auto& file : fileList){
        std::cout << file << std::endl;
    }
    std::cout << "########### cleaning up all test files ###############" << std::endl;
    for( auto& file : fileList){
        systemTools::deleteFile(file);
    }
    
    return 0;
}
