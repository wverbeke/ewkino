
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/GeneratorInfo.h"

//include c++ library classes
#include <iostream>
#include <chrono>


int main(){
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    treeReader.initSample();

    auto start = std::chrono::high_resolution_clock::now();

    for(unsigned i = 0; i < treeReader.nEntries; ++i){

        treeReader.GetEntry(i);
        
        GeneratorInfo generatorInfo( treeReader );

		for( unsigned i = 0; i < std::min( unsigned(9), generatorInfo.numberOfLheWeights() ); ++i ){
       		generatorInfo.relativeWeightScaleVar( i );
		}
        if( generatorInfo.numberOfLheWeights() >= 9 ){
            generatorInfo.relativeWeight_MuR_1_MuF_1();
            generatorInfo.relativeWeight_MuR_1_MuF_0p5();
            generatorInfo.relativeWeight_MuR_2_MuF_1();
            generatorInfo.relativeWeight_MuR_2_MuF_2();
            generatorInfo.relativeWeight_MuR_2_MuF_0p5();
            generatorInfo.relativeWeight_MuR_0p5_MuF_1();
            generatorInfo.relativeWeight_MuR_0p5_MuF_2();
            generatorInfo.relativeWeight_MuR_0p5_MuF_0p5();
        }

		for( unsigned i = 0; i < std::min( unsigned(100), generatorInfo.numberOfLheWeights() - 9 ); ++i ){
        	generatorInfo.relativeWeightPdfVar( i );
		}

		for( unsigned i = 0; i < generatorInfo.numberOfPsWeights(); ++i ){
        	generatorInfo.relativeWeightPsVar( i );
		}
        if( generatorInfo.numberOfPsWeights() == 14 ){
            generatorInfo.relativeWeight_ISR_InverseSqrt2();
            generatorInfo.relativeWeight_FSR_InverseSqrt2_();
            generatorInfo.relativeWeight_ISR_Sqrt2();
            generatorInfo.relativeWeight_FSR_Sqrt2();
            generatorInfo.relativeWeight_ISR_0p5();
            generatorInfo.relativeWeight_FSR_0p5();
            generatorInfo.relativeWeight_ISR_2();
            generatorInfo.relativeWeight_FSR_2();
            generatorInfo.relativeWeight_ISR_0p25();
            generatorInfo.relativeWeight_FSR_0p25();
            generatorInfo.relativeWeight_ISR_4();
            generatorInfo.relativeWeight_FSR_4();
        }

        generatorInfo.ttgEventType();
        generatorInfo.zgEventType();
        generatorInfo.partonLevelHT();
        generatorInfo.numberOfTrueInteractions();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    return 0;
}
