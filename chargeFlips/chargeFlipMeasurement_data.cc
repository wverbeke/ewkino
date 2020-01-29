

//include c++ library classes
#include <memory>
#include <string>
#include <fstream>

//include ROOT classes
#include "TH1D.h"

//include other parts of framework
#include "../Tools/interface/analysisTools.h"
#include "../Tools/interface/HistInfo.h"
#include "../Tools/interface/histogramTools.h"
#include "../Tools/interface/ConstantFit.h"
#include "../Tools/interface/systemTools.h"
#include "../Event/interface/Event.h"
#include "interface/chargeFlipSelection.h"
#include "interface/chargeFlipTools.h"
#include "../plotting/tdrStyle.h"
#include "../plotting/plotCode.h"

	
std::vector< HistInfo > makeDistributionInfo(){
    std::vector< HistInfo > histInfoVec = {
        HistInfo( "mll", "M_{ll} (GeV)", 10, 70, 120 ),
        HistInfo( "leptonPtLeading", "p_{T}^{leading lepton} (GeV)", 10, 25, 200 ),
        HistInfo( "leptonPtSubLeading", "p_{T}^{subleading lepton} (GeV)", 10, 15, 150 ),

        HistInfo( "leptonEtaLeading", "|#eta|^{leading lepton}", 10, 0, 2.5 ),
        HistInfo( "leptonEtaSubLeading", "|#eta|^{subleading lepton}", 10, 0, 2.5 ),

        HistInfo( "met", "E_{T}^{miss} (GeV)", 10, 0, 300 ),
        HistInfo( "ltmet", "L_{T} + E_{T}^{miss} (GeV)", 10, 0, 300 ),
        HistInfo( "ht", "H_{T} (GeV)", 10, 0, 600 ),
        HistInfo( "mt2l", "M_{T}^{2l} (GeV)", 10, 0, 300 ),

        HistInfo( "nJets", "number of jets", 8, 0, 8 ),
        HistInfo( "nBJets", "number of b-jets (tight deep CSV)", 4, 0, 4 ),
        HistInfo( "nVertex", "number of vertices", 10, 0, 70 )
    };
    return histInfoVec;
}


std::vector< double > computeVariables( const Event& event ){
    PhysicsObject electronSum = ( event.electron( 0 )  + event.electron( 1 ) );
    return {
        electronSum.mass(),
        event.electron( 0 ).pt(),
        event.electron( 1 ).pt(),
        event.electron( 0 ).absEta(),
        event.electron( 1 ).absEta(),
        event.metPt(),
        ( event.electron( 0 ).pt() + event.electron( 1 ).pt() + event.metPt() ),
        event.jetCollection().scalarPtSum(),
        mt( electronSum, event.met() ),
        static_cast< double >( event.numberOfJets() ),
        static_cast< double >( event.numberOfTightBTaggedJets() )
    };
}


void deriveChargeFlipCorrections( const std::string& year, const std::string& sampleDirectory ){

    //read MC charge-flip maps
    TFile* chargeFlipMapFile = TFile::Open( ( "chargeFlipMaps/chargeFlipMap_MC_" + year + ".root" ).c_str() );
    std::shared_ptr< TH2 > chargeFlipMap_MC( dynamic_cast< TH2* >( chargeFlipMapFile->Get( ( "chargeFlipRate_electron_" + year ).c_str() ) ) );
    chargeFlipMap_MC->SetDirectory( gROOT );
    chargeFlipMapFile->Close();

    //histograms for each contribution
	std::vector< std::string > contributions = { "Data", "Charge-flips", "Nonprompt", "Prompt" };
	std::map< std::string, std::vector< std::shared_ptr< TH1D > > > histogramMap;
	std::vector< HistInfo > histInfoVector = makeDistributionInfo();
	for( const auto& process : contributions ){
		for( const auto& dist : histInfoVector ){
			histogramMap[ process ].push_back( dist.makeHist( dist.name() + "_" + process + "_" + year ) );
		}
	}
    
    //loop over samples to fill histograms
    std::string sampleListFile = "sampleLists/samples_chargeFlipMeasurement_" + year + ".txt";
    TreeReader treeReader( sampleListFile, sampleDirectory );

    for( unsigned i = 0; i < treeReader.numberOfSamples(); ++i ){
        treeReader.initSample();

        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry );

            //apply selection
            if( ! chargeFlips::passChargeFlipEventSelection( event, true, true, true) ) continue;
    
            double weight = event.weight();
            bool isSameSign = event.leptonsAreSameSign();
            std::string contributionName;

            if( event.isData() && isSameSign ){
                contributionName = "Data";
            } else if( event.isData() ){
                contributionName = "Charge-flips";
                weight *= chargeFlips::chargeFlipWeight( event, chargeFlipMap_MC );

            } else if( isSameSign ){
                bool isPrompt = true;
                for( const auto& leptonPtr : event.leptonCollection() ){
                    if( !leptonPtr->isPrompt() ){
                        isPrompt = false;
                        break;
                    }
                }
                if( isPrompt ){
                    contributionName = "Prompt";
                } else {
                    contributionName = "Nonprompt";
                }

            //skip OS MC events
            } else {
                continue;
            }

            //fill histograms
            auto fillVariables = computeVariables( event );
            for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
                histogram::fillValue( histogramMap[ contributionName ][ dist ].get(), fillVariables[ dist ], weight );
            }
        }
    }

    //set negative bins to zero
    for( const auto& process : contributions ){
        for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
            analysisTools::setNegativeBinsToZero( histogramMap[ process ][ dist ].get() );
        }
    }

    //determine the scale factor by fitting Mll
    std::shared_ptr< TH1D > chargeFlipPrediction( dynamic_cast< TH1D* >( histogramMap.at( "Charge-flips" )[ 0 ]->Clone() ) );
    std::shared_ptr< TH1D > chargeFlipObserved( dynamic_cast< TH1D* >( histogramMap.at( "Data" )[ 0 ]->Clone() ) );

    //subtract backgrounds from data
    chargeFlipObserved->Add( histogramMap.at( "Prompt" )[ 0 ].get(), -1. );
    chargeFlipObserved->Add( histogramMap.at( "Nonprompt" )[ 0 ].get(), -1. );

    //avoid negative bins in data after the subtraction
    analysisTools::setNegativeBinsToZero( chargeFlipObserved.get() );

    //fit the ratio of observed to predicted to get the scale-factor
    chargeFlipObserved->Divide( chargeFlipPrediction.get() );
    ConstantFit chargeFlipRatioFit( chargeFlipObserved );
    
    //write scale factor to txt file
    std::ofstream scaleFactorDump( "chargeFlipScaleFactor_" + year + ".txt" );
    scaleFactorDump << "charge flip scale factor for " << year << " = " << chargeFlipRatioFit.value() << " +- " << chargeFlipRatioFit.uncertainty() << "\n";
    scaleFactorDump.close();

    //header for plots
	std::string header;
	if( year == "2016" ){
	    header = "35.9 fb^{-1}";
	} else if( year == "2017" ){
	    header = "41.5 fb^{-1}";
	} else{
	    header = "59.7 fb^{-1}";
	}

    //make closure plots before applying the scale-factor
	std::string outputDirectoryBeforeSF = ( "closurePlots_chargeFlips_data_beforeSF_" + year );
    systemTools::makeDirectory( outputDirectoryBeforeSF );
    for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){
        TH1D* predictedHistograms[3] = { histogramMap.at( "Charge-flips" )[ dist ].get(), histogramMap.at( "Prompt" )[ dist ].get(), histogramMap.at( "Nonprompt" )[ dist ].get() };

		plotDataVSMC( histogramMap.at( "Data" )[ dist ].get(), predictedHistograms, &contributions[0], 3, stringTools::formatDirectoryName( outputDirectoryBeforeSF ) + histInfoVector[ dist ].name() + "_closureTest_chargeFlips_data_beforeSF_" + year + ".pdf", "", false, false, header );
		plotDataVSMC( histogramMap.at( "Data" )[ dist ].get(), predictedHistograms, &contributions[0], 3, stringTools::formatDirectoryName( outputDirectoryBeforeSF ) + histInfoVector[ dist ].name() + "_closureTest_chargeFlips_data_beforeSF_" + year + "_log.pdf", "", true, false, header );
    }

    //make closure plots after applying the scale-factor
    std::string outputDirectoryAfterSF = ( "closurePlots_chargeFlips_data_afterSF_" + year );
    systemTools::makeDirectory( outputDirectoryAfterSF );
    for( size_t dist = 0; dist < histInfoVector.size(); ++dist ){

        //scale charge-flip prediction
        histogramMap.at( "Charge-flips" )[ dist ]->Scale( chargeFlipRatioFit.value() );

        TH1D* predictedHistograms[3] = { histogramMap.at( "Charge-flips" )[ dist ].get(), histogramMap.at( "Prompt" )[ dist ].get(), histogramMap.at( "Nonprompt" )[ dist ].get() };
		plotDataVSMC( histogramMap.at( "Data" )[ dist ].get(), predictedHistograms, &contributions[0], 3, stringTools::formatDirectoryName( outputDirectoryAfterSF ) + histInfoVector[ dist ].name() + "_closureTest_chargeFlips_data_afterSF_" + year + ".pdf", "", false, false, header );
		plotDataVSMC( histogramMap.at( "Data" )[ dist ].get(), predictedHistograms, &contributions[0], 3, stringTools::formatDirectoryName( outputDirectoryAfterSF ) + histInfoVector[ dist ].name() + "_closureTest_chargeFlips_data_afterSF_" + year + "_log.pdf", "", true, false, header );
    }
}


int main(){

    //plotting style
    setTDRStyle();
    deriveChargeFlipCorrections( "2016", "../test/testData/" );
    return 0;
}
