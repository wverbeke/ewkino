/*
code snipped to test the BDTReader class
*/

//include other parts of code
#include "../interface/KerasModelReader.h"

//include c++ library classes
#include <iostream>

//include ROOT classes

int main(){
    KerasModelReader reader("model_4hiddenLayers_16unitsPerLayer_relu_learningRate0p0001.h5", 36);
    std::vector<double> vector(36, 1.);

    for(unsigned i = 0; i < 10000; ++i){
        reader.predict( vector );
    }
    std::cout << "reader.predict( vector ) = " << reader.predict( vector ) << std::endl;
    return 0;
}

