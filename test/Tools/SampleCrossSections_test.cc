#include "../../Tools/interface/SampleCrossSections.h"

//include c++ library classes
#include <iostream>
#include <exception>

//include other parts of framework
#include "../../Tools/interface/Sample.h"


int main(){

    //read samples
    std::vector< Sample > sampleVector = readSampleList( "../testData/samples_test.txt", "../testData/" );

    for( const auto& sample : sampleVector ){
        SampleCrossSections sampleXsec( sample );
        for( size_t i = 0; i < 100; ++i ){
            std::cout << "pdf var " << i << " = " << sampleXsec.crossSectionRatio_pdfVar( i ) << std::endl;
        }
        for( size_t i = 0; i < 9; ++i ){
            std::cout << "scale var " << i << " = " << sampleXsec.crossSectionRatio_scaleVar( i ) << std::endl;
        }
    }
}
