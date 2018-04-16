#include "../interface/treeReader.h"

void treeReader::checkSampleEraConsistency() const{
    for(auto& sample : samples2016){
        if( sample.is2017() ){
            std::cerr << "Error: 2017 sample detected in list of 2016 samples, this will lead to inconsistent lumi-scaling and cuts being applied!" << std::endl;
        }
    }
    for(auto& sample : samples2017){
        if( sample.is2016() ){
            std::cerr << "Error: 2016 sample detected in list of 2017 samples, this will lead to inconsistent lumi-scaling and cuts being applied!" << std::endl;
        }
    }
}

void treeReader::checkEraOrthogonality() const{
    bool bothTrue = is2017() && is2016();
    if(bothTrue){
        std::cerr << "Error: both is2016 and is2017 flags are returning TRUE for current sample! Sample has to be either one." << std::endl;
    }
    bool bothFalse = !( is2017() || is2016() );
    if(bothFalse){
        std::cerr << "Error: both is2016 and is2017 flags are returning FALSE for current sample! Sample has to be either one." << std::endl;
    }
}
