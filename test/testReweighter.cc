/*
small testing code for the Reweighter class
*/

#include "../interface/Reweighter.h"

//include other parts of code 

//include c++ library classes
#include <iostream>

//include ROOT classes 


int main(){
    for(unsigned i = 0; i < 10; ++i){
        Reweighter reweighter; 
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        std::cout << "reweighter.puWeight(23) = " << reweighter.puWeight(23) << std::endl;
        std::cout << "reweighter.bTagWeight(4, 130., 1.3, 0.65) = " << reweighter.bTagWeight(4, 130., 1.3, 0.65) << std::endl;
        std::cout << "reweighter.bTagEff(4, 130., 1.3) = " << reweighter.bTagEff(4, 130., 1.3) << std::endl;
        std::cout << "reweighter.muonWeight(26, 2.1) = " << reweighter.muonWeight(26., 2.1) << std::endl;
        std::cout << "reweighter.electronWeight(26, 2.1, 2.1) = " << reweighter.electronWeight(26., 2.1, 2.1) << std::endl;
        std::cout << "reweighter.muonFakeRate(26, 2.1) = " << reweighter.muonFakeRate(26., 2.1) << std::endl;
        std::cout << "reweighter.electronFakeRate(26, 2.1) = " << reweighter.muonFakeRate(26, 2.1) << std::endl;
    }
    return 0;
}
