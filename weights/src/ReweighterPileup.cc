#include "../interface/ReweighterPileup.h"

//include c++ library classes
#include <vector>

//include ROOT classes
#include "TFile.h"

//include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/histogramTools.h"


//helper function to produce files with pileup weights for each MC sample
void computeAndWritePileupWeights( const Sample& sample, const std::string& weightDirectory ){
    if( sample.isData() ) return;

    //open sample and extract pileup distribution
    std::shared_ptr< TFile > sampleFilePtr = sample.filePtr();
    std::shared_ptr< TH1 > pileupMC( dynamic_cast< TH1* >( sampleFilePtr->Get( "blackJackAndHookers/nTrueInteractions" ) ) );
    if( pileupMC == nullptr ){
        throw std::runtime_error( "File " + sample.fileName() + " does not contain 'blackJackAndHookers/nTrueInteractions'." );
    }

    //make sure the pileup distribution is normalized to unity
    pileupMC->Scale( 1. / pileupMC->GetSumOfWeights() );

    //store all pileup weights in a map
    std::map< std::string, std::map< std::string, std::shared_ptr< TH1 > > > pileupWeights;

    for( const auto& year : { "2016", "2017", "2018" } ){
        for( const auto& var : { "central", "down", "up" } ){

            //read data pileup distribution from given file
            std::string dataPuFilePath = ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/pileupData/" + "dataPuHist_" + year + "Inclusive_" + var + ".root" );
            if( !systemTools::fileExists( dataPuFilePath ) ){
                throw std::runtime_error( "File " + dataPuFilePath + " with data pileup weights, necessary for reweighting, is not present." );
            }

            TFile* dataPileupFilePtr = TFile::Open( dataPuFilePath.c_str() );
            std::shared_ptr< TH1 > pileupData( dynamic_cast< TH1* >( dataPileupFilePtr->Get( "pileup" ) ) );
            pileupData->SetDirectory( gROOT );

            //make sure the pileup distribution is normalized to unity
            pileupData->Scale( 1. / pileupData->GetSumOfWeights() );

            //divide data and MC histograms to get the weights
            pileupData->Divide( pileupMC.get() );

            pileupWeights[ year ][ var ] = std::shared_ptr< TH1 >( dynamic_cast< TH1* >( pileupData->Clone() ) );

            //close the file and make sure the histogram persists
            pileupWeights[ year ][ var ]->SetDirectory( gROOT );
            dataPileupFilePtr->Close();
        }
    }

    //write pileup weights to a new ROOT file 
    std::string outputFilePath = stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/pileupWeights/pileupWeights_" + sample.fileName();

    //make output directory if needed
    systemTools::makeDirectory( stringTools::directoryNameFromPath( outputFilePath ) );
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str(), "RECREATE" );
    for( const auto& year : { "2016", "2017", "2018" } ){
        for( const auto& var : { "central", "down", "up" } ){
            pileupWeights[ year ][ var ]->Write( ( std::string( "pileupWeights_" ) + year + "_" + var ).c_str() );
        }
    }
    outputFilePtr->Close();
}



ReweighterPileup::ReweighterPileup( const std::vector< Sample >& sampleList, const std::string& weightDirectory ){
    
    //read each of the pileup weights into the maps
    for( const auto& sample : sampleList ){

        //skip data samples!
        if( sample.isData() ) continue;

        std::string pileupWeightPath = ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/pileupWeights/pileupWeights_" + sample.fileName() );

        //for each sample check if the necessary pileup weights are available, and produce them if not 
        if( !systemTools::fileExists( pileupWeightPath ) ){
            computeAndWritePileupWeights( sample, weightDirectory );
        }

        //extract the pileupweights from the file
        std::string yearSuffix;
        if( sample.is2016() ){
            yearSuffix = "2016";
        } else if( sample.is2017() ){
            yearSuffix = "2017";
        } else{
            yearSuffix = "2018";
        }
        TFile* puWeightFilePtr = TFile::Open( pileupWeightPath.c_str() );
        puWeightsCentral[ sample.uniqueName() ] = std::shared_ptr< TH1 >( dynamic_cast< TH1* >( puWeightFilePtr->Get( ( "pileupWeights_" + yearSuffix + "_central" ).c_str() ) ) );
        puWeightsCentral[ sample.uniqueName() ]->SetDirectory( gROOT );
        puWeightsDown[ sample.uniqueName() ] = std::shared_ptr< TH1 >( dynamic_cast< TH1* >( puWeightFilePtr->Get( ( "pileupWeights_" + yearSuffix + "_down" ).c_str() ) ) );
        puWeightsDown[ sample.uniqueName() ]->SetDirectory( gROOT );
        puWeightsUp[ sample.uniqueName() ] = std::shared_ptr< TH1 >( dynamic_cast< TH1* >( puWeightFilePtr->Get( ( "pileupWeights_" + yearSuffix + "_up" ).c_str() ) ) );
        puWeightsUp[ sample.uniqueName() ]->SetDirectory( gROOT );
        puWeightFilePtr->Close();
    }
}


double ReweighterPileup::weight( const Event& event, const std::map< std::string, std::shared_ptr< TH1 > >& weightMap ) const{
    auto it = weightMap.find( event.sample().uniqueName() );
    if( it == weightMap.cend() ){
        throw std::invalid_argument( "No pileup weights for sample " + event.sample().uniqueName() + " found, this sample was probably not present in the vector used to construct the Reweighter." );
    }
    TH1* histPtr = it->second.get();
    return histogram::contentAtValue( histPtr, event.generatorInfo().numberOfTrueInteractions() );
}


double ReweighterPileup::weight( const Event& event ) const{
    return weight( event, puWeightsCentral );
}


double ReweighterPileup::weightDown( const Event& event ) const{
    return weight( event, puWeightsDown );
}


double ReweighterPileup::weightUp( const Event& event ) const{
    return weight( event, puWeightsUp );
}
