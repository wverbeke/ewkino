
#include "../../Tools/interface/RangedMap.h"

//include c++ library classes 
#include <iostream>

int main(){
    RangedMap<int> testMap( {
        {0, 1},
        {100, 2},
        {200, 3},
        {300, 4},
        {600, 5},
        {6000, 6}
    } );

    std::cout << "testMap[0] = " << testMap[0] << std::endl;
    std::cout << "testMap[150] = " << testMap[150] << std::endl;
    std::cout << "testMap[250] = " << testMap[250] << std::endl;
    std::cout << "testMap[400] = " << testMap[400] << std::endl;
    std::cout << "testMap[3000] = " << testMap[3000] << std::endl;
    std::cout << "testMap[6200] = " << testMap[6200] << std::endl;

    try{
        testMap[-1]; 
    } catch( const std::invalid_argument& ){
        std::cout << "Indexing below lower bound throws std::invalid_argument." << std::endl;
    }

    try{
        RangedMap<int> testMap_2({});
        testMap_2[0];
    } catch( const std::out_of_range& ){
        std::cout << "Indexing empty RangedMap throws std::out_of_range." << std::endl;
    }

    return 0;

}
