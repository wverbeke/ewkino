
#include "../../weights/interface/LeptonReweighter.h"
#include "../../weights/interface/ConcreteLeptonReweighter.h"

//include c++ library classes
#include <iostream>
#include <memory>

//include ROOT classes
#include "TFile.h"
#include "TH2D.h"
#include "TROOT.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../copyMoveTest.h"
#include "../../weights/interface/ConcreteSelection.h"


double etaFunction( const Muon& muon ){
    return muon.absEta();
}


double etaFunction( const Electron& electron ){
    return fabs( electron.etaSuperCluster() );
}


template< typename LeptonType > double leptonSFManual( const LeptonType& lepton, const std::shared_ptr< TH2 >& weights, const unsigned var = 0 ){
    double weight = 1.;
    if( lepton.isTight() ){
        double maxEta = ( lepton.isMuon() ? 2.39 : 2.49 );
        int bin = weights->FindBin( std::min( etaFunction( lepton ), maxEta ), std::min( lepton.pt(), 119. ) );
        weight = weights->GetBinContent( bin );
        if( var == 1 ){
            weight -= weights->GetBinError( bin );
        } else if( var == 2 ){
            weight += weights->GetBinError( bin );
        }   
    }
    return weight;
}


bool doubleEqual( double lhs, double rhs ){
    return fabs( ( lhs - rhs )/lhs ) < 1e-6;
}


int main(){


    //Read muon scale factors
    TFile* muonSFFile = TFile::Open( "testSF/looseToTight_2016_m_3l.root" );
    std::shared_ptr< TH2 > muonWeights( dynamic_cast< TH2F* >( muonSFFile->Get( "EGamma_SF2D" ) ) );
    muonWeights->SetDirectory( gROOT );
    muonSFFile->Close();

    //Read electron scale factors
    TFile* eleSFFile = TFile::Open( "testSF/looseToTight_2016_e_3l.root" );
    std::shared_ptr< TH2 > electronWeights( dynamic_cast< TH2F* >( eleSFFile->Get( "EGamma_SF2D" ) ) );
    electronWeights->SetDirectory( gROOT );
    eleSFFile->Close(); 

    //make Reweighters, and check their behavior under copy/move etc
    MuonReweighter muonReweighter( muonWeights, new TightSelector );
    copyMoveTest( muonReweighter );
    ElectronIDReweighter electronReweighter( electronWeights, new TightSelector );
    copyMoveTest( electronReweighter );

    //loop over samples
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries();  ++entry ){
            Event event = treeReader.buildEvent( entry );

            //compare scale factors computed with LeptonReweighter to manual computation
            for( const auto& muonPtr : event.muonCollection() ){
                
                double nominalWeight = muonReweighter.weight( *muonPtr );
                double nominalWeightManual = leptonSFManual( *muonPtr, muonWeights, 0 );
                if( ! doubleEqual( nominalWeight, nominalWeightManual ) ){
                    std::cout << "pt = " << muonPtr->pt() << "\teta = " << muonPtr->eta() << std::endl;
                    throw std::runtime_error( "Nominal muon weight given by Reweighter is " + std::to_string( nominalWeight ) + ", while manual computation gives " + std::to_string( nominalWeightManual ) );
                }

                double weightDown = muonReweighter.weightDown( *muonPtr );
                double weightDownManual = leptonSFManual( *muonPtr, muonWeights, 1 );
                if( ! doubleEqual( weightDown, weightDownManual ) ){
                    throw std::runtime_error( "Down varied muon weight given by Reweighter is " + std::to_string( weightDown ) + ", while manual computation gives " + std::to_string( weightDownManual ) );
                }

                double weightUp = muonReweighter.weightUp( *muonPtr );
                double weightUpManual = leptonSFManual( *muonPtr, muonWeights, 2 );
                if( ! doubleEqual( weightUp, weightUpManual ) ){
                    throw std::runtime_error( "Up varied muon weight given by Reweighter is " + std::to_string( weightUp ) + ", while manual computation gives " + std::to_string( weightUpManual ) );
                }
                    
            }
            for( const auto& electronPtr : event.electronCollection() ){

                double nominalWeight = electronReweighter.weight( *electronPtr );
                double nominalWeightManual = leptonSFManual( *electronPtr, electronWeights, 0 );
                if( ! doubleEqual( nominalWeight, nominalWeightManual ) ){
                    throw std::runtime_error( "Nominal electron weight given by Reweighter is " + std::to_string( nominalWeight ) + ", while manual computation gives " + std::to_string( nominalWeightManual ) );
                }

                double weightDown = electronReweighter.weightDown( *electronPtr );
                double weightDownManual = leptonSFManual( *electronPtr, electronWeights, 1 );
                if( ! doubleEqual( weightDown, weightDownManual ) ){
                    throw std::runtime_error( "Down varied electron weight given by Reweighter is " + std::to_string( weightDown ) + ", while manual computation gives " + std::to_string( weightDownManual ) );
                }

                double weightUp = electronReweighter.weightUp( *electronPtr );
                double weightUpManual = leptonSFManual( *electronPtr, electronWeights, 2 );
                if( ! doubleEqual( weightUp, weightUpManual ) ){
                    throw std::runtime_error( "Up varied electron weight given by Reweighter is " + std::to_string( weightUp ) + ", while manual computation gives " + std::to_string( weightUpManual ) );
                }

            }
        }
    }

    return 0;
}
