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
#include "../Tools/interface/analysisTools.h"
#include "interface/CutsFitInfo.h"
#include "interface/fakeRateTools.h"
#include "interface/SlidingCut.h"


unsigned numberOfEmptyBins( const TH1* hist ){
    unsigned counter = 0;
    for( int b = 1; b < hist->GetNbinsX(); ++b ){
        if(  hist->GetBinContent(b) < 1e-8 ){
            ++counter;
        }
    }
    return counter;
}


bool ratioHasPathologicalBin( const TH1* hist ){
   for( int b = 1; b < hist->GetNbinsX() + 1; ++b ){
        if( hist->GetBinContent(b) > 50 && hist->GetBinError(b)/hist->GetBinContent(b) > 0.5 ){
            return true;
        }
    }
    return false;
}


bool binIsLowStat( double content, double error ){
    return ( error/content >= 1. );
}


bool isLowStatistics( const TH1* hist ){
    const double maxLowStatFraction = 0.3;
    int nLowStatBins = 0;
    for( int b = 1; b < hist->GetNbinsX() + 1; ++b ){
        if( binIsLowStat( hist->GetBinContent(b), hist->GetBinError(b) ) ){
            ++nLowStatBins;
        }
    }
    double ret = static_cast<double>(nLowStatBins)/hist->GetNbinsX();
    return ret > maxLowStatFraction;
}


void tuneFOSelection( const std::string& leptonFlavor, const std::string& year, const std::string& sampleDirectory ){

    bool isMuon;
    if( leptonFlavor == "muon" ){
        isMuon = true;
    } else if( leptonFlavor == "electron" ){
        isMuon = false;
    } else {
        throw std::invalid_argument( "leptonFlavor string should be either 'muon' or 'electron'." );
    }

    analysisTools::checkYearString( year );

    const double minPtRatioCut = 0.3;
    const double maxPtRatioCut = 0.6;
    const unsigned numberOfPtRatioCuts = 6;
    std::vector< double > ptRatioCuts;
    std::vector< std::string > ptRatioNames;
    for( unsigned c = 0; c < numberOfPtRatioCuts; ++c ){
        double cut = minPtRatioCut + c*( maxPtRatioCut - minPtRatioCut ) / numberOfPtRatioCuts;
        ptRatioCuts.push_back( cut );
        ptRatioNames.push_back( "pTRatio" + stringTools::doubleToString( cut, 3 ) );
    }


    const double minSlidePt = 10;
    const double maxSlidePt = 60;
    const double minDeepFlavorCut = 0.1;
    const double maxDeepFlavorCut = 0.3;
    const double deepFlavorGranularity = 0.001;
    SlidingCutCollection deepFlavorCutCollection( minSlidePt, maxSlidePt, minDeepFlavorCut, maxDeepFlavorCut, deepFlavorGranularity );
    std::vector< std::string > deepFlavorNames;
    for( size_t i = 0; i < deepFlavorCutCollection.size(); ++i ){
        deepFlavorNames.push_back( deepFlavorCutCollection[i].name( "pT", "deepFlavor" ) );
    }

    //categorization will include one 'category' for every ptRatio and deepFlavor cut
    Categorization categories( { ptRatioNames, deepFlavorNames } );
        
    //binning of fakerate as a function of pT 
    const unsigned numberOfPtBins = 10; 
    const double minPt = 10;
    const double maxPt = 60;
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
            event.removeTaus();
            event.selectLooseLeptons();
            event.cleanElectronsFromLooseMuons();	
            if( event.numberOfLightLeptons() < 1 ) continue;

            //apply cone-correction to leptons 
            event.applyLeptonConeCorrection();

            for( const auto& leptonPtr : event.lightLeptonCollection() ){

                LightLepton& lepton = *leptonPtr;

                //select correct lepton flavor
                if( isMuon && !lepton.isMuon() ) continue;
                if( !isMuon && !lepton.isElectron() ) continue;

                //veto prompt leptons and leptons originating from photons 
                if( lepton.isPrompt() ) continue; 

                //make sure lepton passes the minimum pt requirement (set when defining the pT histograms )
                if( lepton.pt() < minPt ) continue;

                //check whether lepton comes from a b ( provenanceCompressed = 0 ) or c decay ( provenanceCompressed = 1 )
                bool isHeavyFlavor = ( lepton.provenanceCompressed() == 1 || lepton.provenanceCompressed() == 2 );

                for( unsigned ptRatioI = 0; ptRatioI < numberOfPtRatioCuts; ++ptRatioI ){

                    //lepton must pass varying ptRatio threshold
                    if( lepton.ptRatio() < ptRatioCuts[ ptRatioI ] ) continue;

                    for( unsigned deepFlavorI = 0; deepFlavorI < deepFlavorCutCollection.size(); ++deepFlavorI ){

                        //lepton must pass varying deepFlavor cut on closest jet 
                        //if( lepton.closestJetDeepFlavor() > deepFlavorCuts[ deepFlavorI ] ) continue;
                        if( lepton.closestJetDeepFlavor() >= deepFlavorCutCollection[ deepFlavorI ].cut( lepton.uncorrectedPt() ) ) continue;

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
    }

    //divide numerator and denominator histograms
    //ratio will be stored in what were orignally the numerator histograms
    for( Categorization::size_type c = 0; c < categories.size(); ++c ){
        heavyFlavorNumerator[ c ]->Divide( heavyFlavorDenominator[ c ].get() );
        lightFlavorNumerator[ c ]->Divide( lightFlavorDenominator[ c ].get() );
    }

    
    CutsFitInfoCollection fitInfoCollection;
    for( Categorization::size_type c = 0; c < categories.size(); ++c ){

        //make sure the fit under consideration has decent statistics 
        if( numberOfEmptyBins( heavyFlavorNumerator[ c ].get() ) > 1 || numberOfEmptyBins( lightFlavorNumerator[ c ].get() ) > 1 ) continue;
        if( isLowStatistics( heavyFlavorNumerator[ c ].get() ) || isLowStatistics( lightFlavorNumerator[ c ].get() ) ) continue;
        std::shared_ptr< TH1 > ratio( dynamic_cast< TH1*>( heavyFlavorNumerator[ c ]->Clone() ) );
        ratio->Divide( lightFlavorNumerator[ c ].get() );
        if( ratioHasPathologicalBin( ratio.get() ) ) continue;
        

        std::map< std::string, double > cutMap;
        auto indices = categories.indices( c );
        cutMap["pTRatio"] =  ptRatioCuts[ indices[ 0 ] ];
        cutMap["deepFlavor left"] = deepFlavorCutCollection[ indices[ 1 ] ].cut( minSlidePt );
        cutMap["deepFlavor right"] = deepFlavorCutCollection[ indices[ 1 ] ].cut( maxSlidePt );
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
    fitInfoCollection.printBestCuts( 30 );
    fitInfoCollection.plotBestCuts( 30, plotDirectory );
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    fitInfoCollection.sortByChi2();
    fitInfoCollection.printBestCuts( 30 );
    fitInfoCollection.plotBestCuts( 30, plotDirectory );
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;
    constexpr double epsilon = 0.01;
    fitInfoCollection.sortByLossFunction( epsilon );
    fitInfoCollection.printBestCuts( 30 );
    fitInfoCollection.plotBestCuts( 30, plotDirectory );
}


void runTuningAsJob( const std::string& flavor, const std::string& year ){
	std::string scriptName = "tuneFOSelection_" + flavor + "_" + year + ".sh";
    std::string command = "./tuneFOSelection " + flavor + " " + year + "\n";
    systemTools::submitCommandAsJob( command, scriptName, "169:00:00" );
}


int main( int argc, char* argv[] ){

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    std::string sampleDirectory = "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino_fakerate/";
    if( argc == 3 ){
        std::string flavor = argvStr[1];
        std::string year = argvStr[2]; 

        fakeRate::checkFlavorString( flavor );
        analysisTools::checkYearString( year );

        tuneFOSelection( flavor, year, sampleDirectory );
    } else if( argc == 1 ){
        for( auto& year : std::vector< std::string >( { "2016", "2017", "2018" } ) ){
            for( auto& flavor : std::vector< std::string >( {"muon", "electron"} ) ){
				runTuningAsJob( flavor, year );
            }
        }
    } else if( argc == 2 ){
		std::string year = argvStr[1];
        analysisTools::checkYearString( year );
        for( auto& flavor : std::vector< std::string >( {"muon", "electron"} ) ){
			runTuningAsJob( flavor, year );
        }
	}
    return 0;
}
