#include "../interface/stringTools.h"
#include "../interface/Category.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


int main(){
    std::cout << "########### testing Category class ##################" << std::endl;
    Category testCat( { {"a", "b", "c", "d"}, {"1", "2", "3", "4"}, {"x", "y", "z"} } );

    std::vector<std::string> byMultiIndices;
    for(size_t i = 0; i < 3; ++i){
        for(size_t j = 0; j < 4; ++j){
            for(size_t k = 0; k < 4; ++k){
                byMultiIndices.push_back( testCat.name({k, j, i}) ); 
            }
        }
    }
    
    for(size_t i = 0; i < testCat.size(); ++i){
        std::cout << testCat[i] << "\t" << byMultiIndices[i];
        if( testCat[i] != byMultiIndices[i] ){
            std::cout << "\tError, iterating throught 1D indices and multiD indices gives different result!";
        }
        std::cout << std::endl;
    }
    return 0;
}
