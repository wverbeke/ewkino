
//include class to test 
#include "../../objects/interface/LorentzVector.h"

//include c++ library classes
#include <iostream>
#include <chrono>
#include <random>
#include <utility>

//include ROOT TLorentzVector for performance comparison
#include "TLorentzVector.h"


std::vector< std::vector< double > > getRandomVectors( unsigned numberOfVectors, const bool largePhiRange = false){

    //algorithm to generate random numbers 
    std::ranlux48 random_engine;    

    //define distribution from which to sample vector properties
    std::uniform_real_distribution<double> pt_distribution(0, 1000);
    std::uniform_real_distribution<double> eta_distribution(-8, 8);
    std::uniform_real_distribution<double> phi_distribution;

    //intentionally allow for phi values that need to be corrected to the appropriate range ]-pi, pi]
    if( largePhiRange ){
        phi_distribution = std::uniform_real_distribution<double>(-100, 100);
    } else {
        phi_distribution = std::uniform_real_distribution<double>( -M_PI, M_PI );
    }

    std::uniform_real_distribution<double> energy_distribution(0, 3000);

    std::vector< std::vector< double > > ret; 
    for( unsigned i = 0; i < numberOfVectors; ++i){

        //random lorentz vector 
        ret.push_back(
            {
            pt_distribution( random_engine ),
            eta_distribution( random_engine ),
            phi_distribution( random_engine ),
            energy_distribution( random_engine )
            }
        );
    }
    return ret;
}


std::vector< LorentzVector > makeLorentzVectors( const std::vector< std::vector< double > >& randomVector ){
    std::vector< LorentzVector > ret;
    ret.reserve( randomVector.size() );
    for( auto& components : randomVector ){
        ret.push_back( LorentzVector( components[0], components[1], components[2], components[3] ) );
    }
    return ret;
}


std::vector< TLorentzVector > makeTLorentzVectors( const std::vector< std::vector< double > >& randomVector ){
    std::vector< TLorentzVector > ret;
    ret.reserve( randomVector.size() );
    for( auto& components : randomVector ){
        TLorentzVector v;
        v.SetPtEtaPhiE( components[0], components[1], components[2], components[3] );
        ret.push_back( v );
    }
    return ret;
}


bool doubleEqual( const double lhs, const double rhs ){
    static double epsilon = 1e-6;
    if( fabs(lhs) == 0 && fabs(rhs) == 0){
        return true;
    }
    if( fabs( (lhs - rhs)/lhs ) < epsilon ){
        return true;
    } else {
        return false;
    }
}


bool compareVectorObjects( const LorentzVector& newClass, const TLorentzVector& rootClass ){
    if( 
        !doubleEqual( newClass.pt(), rootClass.Pt() ) ||
        !doubleEqual( newClass.eta(), rootClass.Eta() ) ||
        !doubleEqual( newClass.phi(), rootClass.Phi() ) ||
        !doubleEqual( newClass.energy(), rootClass.E() ) || 
        !doubleEqual( newClass.mass(), rootClass.M() ) 
    ){
        std::cout << newClass.pt() << "\t" << newClass.eta() << "\t" << newClass.phi() << "\t" << newClass.energy() << "\t" << newClass.mass() << std::endl;
        std::cout << newClass.px() << "\t" << newClass.py() << "\t" << newClass.pz() << "\t" << newClass.energy() << "\t" << newClass.mass() << std::endl;
        std::cout << rootClass.Pt() << "\t" << rootClass.Eta() << "\t" << rootClass.Phi() << "\t" << rootClass.E() << "\t" << rootClass.M() << std::endl;
        std::cout << rootClass.Px() << "\t" << rootClass.Py() << "\t" << rootClass.Pz() << "\t" << rootClass.E() << "\t" << rootClass.M() << std::endl;
        return false;
    } else {
        return true;
    }
}


bool compareVectorCorrelations( const std::pair< LorentzVector, LorentzVector >& newClassPair, const std::pair< TLorentzVector, TLorentzVector >& rootClassPair ){
    if( ! doubleEqual( deltaPhi( newClassPair.first, newClassPair.second ), fabs( rootClassPair.first.DeltaPhi( rootClassPair.second ) ) ) ){
        return false;
    } 
    if( ! doubleEqual( deltaR( newClassPair.first, newClassPair.second ), rootClassPair.first.DeltaR( rootClassPair.second ) ) ){
        return false;
    }
    if( ! doubleEqual( ( newClassPair.first + newClassPair.second ).mass(), ( rootClassPair.first + rootClassPair.second ).M() ) ){
        return false;
    }
    if( ! compareVectorObjects( newClassPair.first  + newClassPair.second, rootClassPair.first + rootClassPair.second ) ){
        return false;
    }
    if( ! compareVectorObjects( newClassPair.first - newClassPair.second, rootClassPair.first - rootClassPair.second ) ){
        return false;
    }
    return true;
}


int main(){
    
    const unsigned numIterations = 100000;

    //make random vectors to initialize objects with normal phi range 
    std::vector< std::vector< double > > randomVectors = getRandomVectors( numIterations, false);

    //make LorentzVectors and time it
    auto start = std::chrono::high_resolution_clock::now();
    std::vector< LorentzVector > LorentzVectorCollection = makeLorentzVectors( randomVectors );
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_LorentzVector = end - start;


    //make TLorentzVectors and time it
    start = std::chrono::high_resolution_clock::now();
    std::vector< TLorentzVector > TLorentzVectorCollection = makeTLorentzVectors( randomVectors );
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_TLorentzVector = end - start;

    //print timing comparison
    std::cout << "Elapsed time for creating " << numIterations << " LorentzVectors: " << elapsed_LorentzVector.count() << " s\n";
    std::cout << "Elapsed time for creating " << numIterations << " TLorentzVectors: " << elapsed_TLorentzVector.count() << " s\n";

    //print comparison
    bool equalObjects = true;
    for( unsigned i = 0; i < randomVectors.size(); ++i ){
        if( !compareVectorObjects( LorentzVectorCollection[i], TLorentzVectorCollection[i] ) ){
            equalObjects = false;
            std::cout << LorentzVectorCollection[i] << ", mass = " << LorentzVectorCollection[i].mass() << std::endl;
            std::cout << "(pT = " << TLorentzVectorCollection[i].Pt() << ", eta = " << TLorentzVectorCollection[i].Eta() << ", phi = " << TLorentzVectorCollection[i].Phi() << ", energy = " << TLorentzVectorCollection[i].E()
            << ", mass = " << TLorentzVectorCollection[i].M() << std::endl;
            break;
        }
    } 
    if( equalObjects ){
        std::cout << "All " << numIterations << " LorentzVectors are equal to the corresponding TLorentzVector " << std::endl;
    } else {
        std::cerr << "Error : LorentzVector and TLorentzVector objects are not equal " << std::endl;
    }
    
    std::cout << std::endl;

    //this time use a large phi range to also make a comparison of objects using this 
    randomVectors = getRandomVectors( numIterations, true);
    start = std::chrono::high_resolution_clock::now();
    std::vector< LorentzVector > LorentzVectorCollection_2 = makeLorentzVectors( randomVectors );
    end = std::chrono::high_resolution_clock::now();
    elapsed_LorentzVector = end - start;

    //make a second set of random Lorentz vectors to compare correlations 
    start = std::chrono::high_resolution_clock::now();
    std::vector< TLorentzVector > TLorentzVectorCollection_2 = makeTLorentzVectors( randomVectors );
    end = std::chrono::high_resolution_clock::now();
    elapsed_TLorentzVector = end - start;

    //print timing comparison
    std::cout << "Elapsed time for creating " << numIterations << " LorentzVectors with large phi range: " << elapsed_LorentzVector.count() << " s\n";
    std::cout << "Elapsed time for creating " << numIterations << " TLorentzVectors with large phi range: " << elapsed_TLorentzVector.count() << " s\n";

   	//print comparison
    equalObjects = true;
    for( unsigned i = 0; i < randomVectors.size(); ++i ){
        if( !compareVectorObjects( LorentzVectorCollection_2[i], TLorentzVectorCollection_2[i] ) ){
            equalObjects = false;
            std::cout << LorentzVectorCollection_2[i] << ", mass = " << LorentzVectorCollection_2[i].mass() << std::endl;
            std::cout << "(pT = " << TLorentzVectorCollection_2[i].Pt() << ", eta = " << TLorentzVectorCollection_2[i].Eta() << ", phi = " << TLorentzVectorCollection_2[i].Phi() << ", energy = " << TLorentzVectorCollection_2[i].E()
            << ", mass = " << TLorentzVectorCollection_2[i].M() << std::endl;
            break;
        }
    }
    if( equalObjects ){
        std::cout << "All " << numIterations << " LorentzVectors are equal to the corresponding TLorentzVector " << std::endl;
    } else {
        std::cerr << "Error : LorentzVector and TLorentzVector objects are not equal " << std::endl;
    } 

    //compare correlations 
    bool equalCorrelations = true;
    for( unsigned i = 0; i < randomVectors.size(); ++i ){
        if( !compareVectorCorrelations( {LorentzVectorCollection[i], LorentzVectorCollection_2[i]}, {TLorentzVectorCollection[i], TLorentzVectorCollection_2[i]} ) ){
            equalCorrelations = false;
            break;
        }
    }
    if( equalCorrelations ){
        std::cout << "All correlations between LorentzVectors are equal to those for the corresponding TLorentzVectors " << std::endl;
    } else {
        std::cerr << "Error : LorentzVector and TLorentzVector correlations are not equal " << std::endl;
    }

    

}
