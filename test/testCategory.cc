#include "../interface/stringTools.h"
#include "../interface/Category.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


int main(){
    std::cout << "########### testing Category class ##################" << std::endl;
    Category testCat( { {"a", "b", "c", "d"}, {"1", "2", "3", "4"}, {"x", "y", "z"} } );
    for(auto& cat : testCat){
        std::cout << cat << "\t" << std::endl;
    }
    return 0;
}
