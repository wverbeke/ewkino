/*
Code to tune to FO definition for light leptons.
The goal of the tuning is having a fake-rate (tight/FO) that is equal for light and heavy-flavor fakes 
*/

//include c++ library classes
#include <limits>

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


void tuneFOSelection( const std::string& leptonFlavor ){

    bool isMuon;
    if( leptonFlavor == "muon" ){
        isMuon = true;
    } else if( leptonFlavor == "electron" ){
        isMuon = false;
    } else {
        throw std::invalid_argument( "leptonFlavor string should be either 'muon' or 'electron'." );
    }
   
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
	TreeReader treeReader( "samples_magicFactor.txt", "../test/testData");
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

    //divide light and heavy flavor histograms and fit the ratio
    std::vector< std::shared_ptr< TH1D > > frRatios( categories.size() );
    for( Categorization::size_type c = 0; c < categories.size(); ++c ){
        frRatios[ c ] = std::shared_ptr< TH1D >( (TH1D*) heavyFlavorNumerator[ c ]->Clone() );
        frRatios[ c ]->GetYaxis()->SetTitle("heavy-flavor/light-flavor fake-rate");
        frRatios[ c ]->Divide( lightFlavorNumerator[ c ].get() );
    }

    std::shared_ptr< TF1 > constFunc = std::make_shared< TF1 >( "constFunc", "[0]", minPt, maxPt );

    //look for the constant fit with the smallest deviation from 1
    double minDiff = std::numeric_limits< double >::max();
    Categorization::size_type bestIndex = 0;
    for( Categorization::size_type c = 0; c < categories.size(); ++c ){
        frRatios[ c ]->Fit( "constFunc", "Q" ); //Q = quiet mode 

        double fitValue = constFunc->GetParameter( 0 );
        double diff = fabs( (fitValue - 1.)/fitValue );

        if( diff < minDiff ){
            minDiff = diff;
            bestIndex = c;
        }
    }

    //set plotting style and plot histogram
    setTDRStyle();
    plotHistograms(frRatios[ bestIndex ].get(), "light/heavy fake-rate", "fakeRateRatio_" + categories[ bestIndex ], false);

    //print out best ptRatio and deepFlavor cut
    auto bestIndices = categories.indices( bestIndex );
    std::cout << "best ptRatio lower limit is " << ptRatioCuts[ bestIndices[0] ] << std::endl;
    std::cout << "best deepFlavor upper limit is " << ptRatioCuts[ bestIndices[1] ] << std::endl;
}


int main(){
    tuneFOSelection( "muon" );
    return 0;
}
