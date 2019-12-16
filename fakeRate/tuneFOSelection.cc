/*
Code to tune to FO definition for light leptons.
The goal of the tuning is having a fake-rate (tight/FO) that is equal for light and heavy-flavor fakes 
*/

//include c++ library classes
#include <fstream>

//include ROOT classes
#include "TF1.h"

//include other parts of code 
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/Categorization.h"
#include "../Tools/interface/HistInfo.h"
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"
#include "../Tools/interface/systemTools.h"
#include "interface/CutsFitInfo.h"
#include "interface/fakeRateTools.h"


void tuneFOSelection( const std::string& leptonFlavor, const std::string& year, const std::string& sampleDirectory ){

    fakeRate::checkFlavorString( leptonFlavor );
    bool isMuon = ( leptonFlavor == "muon" );

    fakeRate::checkYearString( year );

    const double minPtRatioCut = 0;
    const double maxPtRatioCut = 1;
    const unsigned numberOfPtRatioCuts = 100;
    std::vector< double > ptRatioCuts;
    std::vector< std::string > ptRatioNames;
    for( unsigned c = 0; c < numberOfPtRatioCuts; ++c ){
        double cut = minPtRatioCut + c*( maxPtRatioCut - minPtRatioCut ) / numberOfPtRatioCuts;
        ptRatioCuts.push_back( cut );
        ptRatioNames.push_back( "pTRatio" + stringTools::doubleToString( cut, 3 ) );
    }

    const double minDeepFlavorCut = 0.1;
    const double maxDeepFlavorCut = 1; 
    const unsigned numberOfDeepFlavorCuts = 90;
    std::vector< double > deepFlavorCuts;
    std::vector< std::string > deepFlavorNames;
    for( unsigned c = 0; c < numberOfDeepFlavorCuts; ++c ){
        double cut = minDeepFlavorCut + c*( maxDeepFlavorCut - minDeepFlavorCut ) / numberOfDeepFlavorCuts;
        deepFlavorCuts.push_back( cut );
        deepFlavorNames.push_back( "deepCSV" + stringTools::doubleToString( cut, 3 ) );
    }

    //categorization will include one 'category' for every ptRatio and deepFlavor cut
    Categorization categories( { ptRatioNames, deepFlavorNames } );
        
    //binning of fakerate as a function of pT 
    const unsigned numberOfPtBins = 50; 
    const double minPt = 10;
    const double maxPt = 150;
    HistInfo ptHistInfo( "pT", "p_{T} (GeV)", numberOfPtBins, minPt, maxPt );

    //initialize histograms for heavy- and light flavor, they will be compared in the end to select the optimal FO cuts 
    std::vector< std::shared_ptr< TH1D > > heavyFlavorNumerator;
    std::vector< std::shared_ptr< TH1D > > heavyFlavorDenominator;
    std::vector< std::shared_ptr< TH1D > > lightFlavorNumerator;
    std::vector< std::shared_ptr< TH1D > > lightFlavorDenominator;

    for( const auto& category : categories ){
        heavyFlavorNumerator.push_back( ptHistInfo.makeHist( "heavyFlavorNumerator_pT_" + category ) );
        heavyFlavorDenominator.push_back( ptHistInfo.makeHist( "heavyFlavorDenominator_pT_" + category ) );
        lightFlavorNumerator.push_back( ptHistInfo.makeHist( "lightFlavorNumerator_pT_" + category ) );
        lightFlavorDenominator.push_back( ptHistInfo.makeHist( "lightFlavorDenominator_pT_" + category ) );
    }

    //loop over samples and fill histograms
    std::string sampleListFile = "sampleLists/samples_tuneFOSelection_" + leptonFlavor + "_" + year + ".txt";
    TreeReader treeReader( sampleListFile, sampleDirectory );
    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry ); //no need to read triggers 

            //select- and clean leptons and require exactly 1 loose lepton
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();	
            event.cleanTausFromLooseLightLeptons();
            if( event.numberOfLightLeptons() != 1 ) continue;

            //apply cone-correction to leptons 
            event.applyLeptonConeCorrection();

            LightLepton& lepton = event.lightLeptonCollection()[0];

            //select correct lepton flavor
            if( isMuon && !lepton.isMuon() ) continue;
            if( !isMuon && !lepton.isElectron() ) continue;

            //veto prompt leptons and leptons originating from photons 
            if( lepton.isPrompt() ) continue; 
            if( lepton.matchPdgId() == 22 ) continue;

            //make sure lepton passes the minimum pt requirement (set when defining the pT histograms )
            if( lepton.pt() < minPt ) continue;

            //check whether lepton comes from a b ( provenanceCompressed = 0 ) or c decay ( provenanceCompressed = 1 )
            bool isHeavyFlavor = ( lepton.provenanceCompressed() == 1 || lepton.provenanceCompressed() == 2 );

            for( unsigned ptRatioI = 0; ptRatioI < numberOfPtRatioCuts; ++ptRatioI ){

                //lepton must pass varying ptRatio threshold
                if( lepton.ptRatio() < ptRatioCuts[ ptRatioI ] ) continue;

                for( unsigned deepFlavorI = 0; deepFlavorI < numberOfDeepFlavorCuts; ++deepFlavorI ){

                    //lepton must pass varying deepFlavor cut on closest jet 
                    if( lepton.closestJetDeepFlavor() > deepFlavorCuts[ deepFlavorI ] ) continue;

                    //compute histogram vector index
                    auto histIndex = categories.index( {ptRatioI, deepFlavorI } );
            
                    //fill histograms for heavy flavor leptons 
                    if( isHeavyFlavor ){

                        //numerator 
                        if( lepton.isTight() ){
                            heavyFlavorNumerator[ histIndex ]->Fill( std::min( lepton.pt(), ptHistInfo.maxBinCenter() ), event.weight() );
                        }                        

                        //denominator
                        heavyFlavorDenominator[ histIndex ]->Fill( std::min( lepton.pt(), ptHistInfo.maxBinCenter() ), event.weight() );
                        
                    //fill histograms for light flavor leptons
                    } else {

                        //numerator
                        if( lepton.isTight() ){
                            lightFlavorNumerator[ histIndex ]->Fill( std::min( lepton.pt(), ptHistInfo.maxBinCenter() ), event.weight() );
                        }
                    
                        //denominator
                        lightFlavorDenominator[ histIndex ]->Fill( std::min( lepton.pt(), ptHistInfo.maxBinCenter() ), event.weight() );
                    }

                }
            }
		}
    }

    //divide numerator and denominator histograms
    //ratio will be stored in what were orignally the numerator histograms
    for( Categorization::size_type c = 0; c < categories.size(); ++c ){
        heavyFlavorNumerator[ c ]->Divide( heavyFlavorDenominator[ c ].get() );
        lightFlavorNumerator[ c ]->Divide( lightFlavorDenominator[ c ].get() );
    }

    
    //std::vector< CutsFitInfo > fakeRateFitInfoVec;
    CutsFitInfoCollection fitInfoCollection;
    for( Categorization::size_type c = 0; c < categories.size(); ++c ){
        std::map< std::string, double > cutMap;
        auto indices = categories.indices( c );
        cutMap["pTRatio"] =  ptRatioCuts[ indices[ 0 ] ];
        cutMap["deepFlavor"] = ptRatioCuts[ indices[ 1 ] ];
        fitInfoCollection.push_back( CutsFitInfo( heavyFlavorNumerator[ c ], lightFlavorNumerator[ c ], cutMap ) );
    }


    //we will try 3 different optimization metrics for the fit: 
    //1 : minimum difference between fit value and 1 
    //2 : smallest Chi2 of constant fit
    //3 : define a loss function as | fitValue - 1 + epsilon | * | chi2 - min( 1, chi2 ) + epsilon | and minimize it

    std::string plotDirectory = "tuningPlots_" + leptonFlavor + "_" + year + "/";

    //make plotting directory if it does not already exist
    systemTools::makeDirectory( plotDirectory );

    fitInfoCollection.sortByDiffFromUnity();
    fitInfoCollection.printBestCuts( 5 );
    fitInfoCollection.plotBestCuts( 5, plotDirectory );
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    fitInfoCollection.sortByChi2();
    fitInfoCollection.printBestCuts( 5 );
    fitInfoCollection.plotBestCuts( 5, plotDirectory );
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    constexpr double epsilon = 0.01;
    fitInfoCollection.sortByLossFunction( epsilon );
    fitInfoCollection.printBestCuts( 5 );
    fitInfoCollection.plotBestCuts( 5, plotDirectory );
}


void runTuningAsJob( const std::string& flavor, const std::string& year ){
	std::string scriptName = "tuneFOSelection_" + flavor + "_" + year + ".sh";
    std::ofstream jobScript( scriptName );
    systemTools::initJobScript( jobScript );
    jobScript << "./tuneFOSelection " << flavor << year << "\n";
    jobScript.close();
    systemTools::submitScript( scriptName, "100:00:00" );
}


int main( int argc, char* argv[] ){

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    std::string sampleDirectory = "../test/testData/";
    if( argc == 3 ){
        std::string flavor = argvStr[1];
        std::string year = argvStr[2]; 

        fakeRate::checkFlavorString( flavor );
        fakeRate::checkYearString( year );

        tuneFOSelection( flavor, year, sampleDirectory );
    } else if( argc == 1 ){
        for( auto& year : std::vector< std::string >( { "2016", "2017", "2018" } ) ){
            for( auto& flavor : std::vector< std::string >( {"muon", "electron"} ) ){
				runTuningAsJob( flavor, year );
            }
        }
    } else if( argc == 2 ){
		std::string year = argvStr[1];
        fakeRate::checkYearString( year );
        for( auto& flavor : std::vector< std::string >( {"muon", "electron"} ) ){
			runTuningAsJob( flavor, year );
        }
	}
    return 0;
}
