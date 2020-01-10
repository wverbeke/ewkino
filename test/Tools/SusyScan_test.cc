#include "../../Tools/interface/SusyScan.h"

//include c++ library classes
#include <iostream>
#include <exception>

//include other parts of framework
#include "../../Tools/interface/Sample.h"
#include "../../Event/interface/Event.h"


int main(){

    Sample susySample = Sample( "../testData/TChiWZ_1_Summer16.root", false, false, false );

    //load the information about the susy scan through the corresponding class
    SusyScan susyScan( susySample );

    /*
    std::cout << susySampleInfo.numberOfPoints() << std::endl;
    
    for( size_t i = 0; i < susySampleInfo.numberOfPoints(); ++i ){
        auto massPair = susySampleInfo.masses( i );
        std::cout << massPair.first << "\t" << massPair.second << std::endl;
    }
    */

    //manually load the number of points by looping over the sample
    std::set< std::pair< unsigned, unsigned > > manualMasses;
    TreeReader treeReader;
    treeReader.initSampleFromFile( "../testData/TChiWZ_1_Summer16.root" );
    for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
        Event event = treeReader.buildEvent( entry );
        manualMasses.insert( { static_cast< unsigned >( event.susyMassInfo().massNLSP() ), static_cast< unsigned >( event.susyMassInfo().massLSP() ) } );
    }

    //compare the number of points
    if( susyScan.numberOfPoints() != manualMasses.size() ){
        throw std::runtime_error( "SusyScan finds " + std::to_string( susyScan.numberOfPoints() ) + " mass points, while a manual loop over the sample finds " + std::to_string( manualMasses.size() ) + "." );
    }

    //check that each point found by the SusyScan class is also found in the manual loop
    for( const auto& manPair : manualMasses ){
        double mChi2 = static_cast< double >( manPair.first );
        double mChi1 = static_cast< double >( manPair.second );

        //rely on the internal error checks of SusyScan, they will catch unknown indices
        susyScan.index( mChi2, mChi1 ); 
    }

    //check that each point found by manually looping is also in the SusyScan class
    for( size_t index = 0; index < susyScan.numberOfPoints(); ++index ){
        auto massPair = static_cast< std::pair< unsigned, unsigned > >( susyScan.masses( index ) );
        auto it = manualMasses.find( massPair );
        if( it == manualMasses.cend() ){
            throw std::runtime_error( "Mass point " + susyScan.massesString( index ) + " not found in manual loop over sample." );
        }
    }
}
