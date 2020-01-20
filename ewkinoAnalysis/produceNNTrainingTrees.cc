

//include c++ library classes

//include ROOT classes 
#include "TTree.h"

//include general parts of framework
#include "../Tools/interface/analysisTools.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"

//include ewkino specific code
#include "interface/ewkinoSelection.h"
#include "interface/ewkinoCategorization.h"


void produceNNTrainingTrees( const std::string& year, const std::string& sampleDirectoryPath ){

	analysisTools::checkYearString( year );

    //map which will contain the training variables
    std::map< std::string, float > trainingVariables = {
        { "metPt", 0. },
        { "mllBestZ", 0. },
        { "mt", 0. },
        { "LTPlusMET", 0. },
        { "m3l", 0. },
        { "mt3l", 0. },
        { "eventWeight", 0. }
    };

    //make output directory for training files
    std::string outputDirectory = "trainingFiles_" + year;
    systemTools::makeDirectory( outputDirectory );

    //build TreeReader and loop over samples
    TreeReader treeReader( "sampleLists/samples_NNTraining_" + year + ".txt", sampleDirectoryPath );


    //use several WZTo3LNu samples at the same time for more statistics
    //to make sure the relative weights to other samples are correct, each sample must be weighted by its sum of weights divided by the total sum of weights of all 3 WZ samples
    std::map< std::string, double > WZWeightModifier;
    double totalSumOfWeights = 0.; 
    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){
        treeReader.initSampleFromFile( stringTools::formatDirectoryName( sampleDirectoryPath ) + treeReader.sampleVector()[sampleIndex].fileName() );

        //sum of weights
        double sumOfWeights = dynamic_cast< TH1D* >( treeReader.currentFilePtr()->Get("hCounter") )->GetSumOfWeights();

        //determine weight scale 
        treeReader.GetEntry(0);
        double weightScale = fabs( treeReader._weight );

        totalSumOfWeights += sumOfWeights/weightScale;
        WZWeightModifier[ treeReader.currentSample().uniqueName() ] = sumOfWeights/weightScale;
    }
    for( const auto& entry : WZWeightModifier ){
        WZWeightModifier[ entry.first ] /= totalSumOfWeights;
    }
    for( const auto& entry : WZWeightModifier ){
        std::cout << "modifier for " << entry.first << " = " << entry.second << std::endl;
    }

    /*
    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){
        treeReader.initSample();

        //skip data 
        if( treeReader.isData() ) continue;

        std::string treeName;

        //add parameter for signal
        if( treeReader.isSusy() ){
            trainingVariables["susyMassSplitting"] = 0.;
            treeName = "signalTree";
        } else {
            treeName = "backgroundTree";
        }

        //make training tree for the current sample
        TFile* trainingFile = TFile::Open( ( stringTools::formatDirectoryName( outputDirectory ) + "trainingFile_" + treeReader.currentSample().uniqueName()  + ".root" ).c_str(), "RECREATE" );
        TTree* trainingTree = new TTree( treeName.c_str(), treeName.c_str() );
        for( const auto& entry : trainingVariables ){
            trainingTree->Branch( entry.first.c_str(), &trainingVariables[ entry.first ], ( entry.first + "/F" ).c_str() );
        }

        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry );

            //ignore taus
            event.removeTaus();

            if( !ewkino::passBaselineSelection( event, false, true, false ) ) continue;
            
            //select tight leptons and require OSSF pair
            event.selectTightLeptons();
            if( event.numberOfLightLeptons() != 3 ) continue;
            if( !event.hasOSSFLightLeptonPair() ) continue;

            //veto b jets
            trainingVariables["metPt"] = event.metPt();
            trainingVariables["mllBestZ"] = event.bestZBosonCandidateMass();
            trainingVariables["mt"] = event.mtW();
            trainingVariables["LTPlusMET"] = ( event.LT() + event.metPt() );
            PhysicsObject leptonSum = event.leptonCollection().objectSum();
            trainingVariables["m3l"] = leptonSum.mass();
            trainingVariables["mt3l"] = mt( leptonSum, event.met() );
            trainingVariables["eventWeight"] = event.weight();

            if( treeReader.isSusy() ){
                trainingVariables["susyMassSplitting"] = ( event.susyMassInfo().massNLSP() - event.susyMassInfo().massLSP() );
            }
            trainingTree->Fill();
        }
        trainingFile->Write();
        trainingFile->Close();
    }
    */
}


int main(){
    produceNNTrainingTrees( "2016", "../test/testData/" );
    produceNNTrainingTrees( "2017", "../test/testData/" );
    produceNNTrainingTrees( "2018", "../test/testData/" );
    return 0;
}
