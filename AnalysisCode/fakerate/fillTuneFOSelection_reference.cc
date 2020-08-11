/*
Code to tune to FO definition for light leptons.
The goal of the tuning is having a fake-rate (tight/FO) that is equal for light and heavy-flavor fakes 
*/

//include c++ library classes
#include <fstream>

//include ROOT classes
#include "TF1.h"

//include other parts of code 
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/Categorization.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../plotting/plotCode.h"
#include "../../plotting/tdrStyle.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/analysisTools.h"
#include "../../fakeRate/interface/CutsFitInfo.h"
#include "../../fakeRate/interface/fakeRateTools.h"
#include "../../fakeRate/interface/SlidingCut.h"

#include "interface/tuneFOSelectionTools.h"

// main function

void fillTuneFOSelection( const std::string& leptonFlavor, const std::string& year, 
			const std::string& sampleList, const std::string& sampleDirectory,
			const unsigned int sampleIndex ){

    bool isMuon;
    if( leptonFlavor == "muon" ){
        isMuon = true;
    } else if( leptonFlavor == "electron" ){
        isMuon = false;
    } else {
        throw std::invalid_argument( "leptonFlavor string should be either 'muon' or 'electron'." );
    }

    analysisTools::checkYearString( year );

    // categorization will include one 'category' for every ptRatio and deepFlavor cut
    // note: the function is in interface/tuneFOSelectionTools
    std::tuple< std::vector<double>, SlidingCutCollection, Categorization > temp = getCutCollection();
    std::vector<double> ptRatioCuts = std::get<0>(temp);
    SlidingCutCollection deepFlavorCutCollection = std::get<1>(temp);
    Categorization categories = std::get<2>(temp);
        
    // binning of fakerate as a function of pT 
    const unsigned numberOfPtBins = 10; 
    const double minPt = 10;
    const double maxPt = 60;
    HistInfo ptHistInfo( "pT", "p_{T} (GeV)", numberOfPtBins, minPt, maxPt );

    // initialize histograms for heavy- and light flavor, 
    // they will be compared in the end to select the optimal FO cuts 
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

    // create TreeReader and set to right sample
    TreeReader treeReader( sampleList, sampleDirectory );
    treeReader.initSample();
    for( unsigned i = 1; i <= sampleIndex; ++i ){
        treeReader.initSample();
    }

    // loop over events in sample
    long unsigned nentries = treeReader.numberOfEntries();
    std::cout<<"starting event loop for "<<nentries<<" events"<<std::endl;
    for( long unsigned entry = 0; entry < nentries; ++entry ){
        
	Event event = treeReader.buildEvent( entry ); //no need to read triggers 
    
        // select- and clean leptons and require exactly 1 loose lepton
	// note: at least one loose lepton instead of exactly?
        event.removeTaus();
        event.selectLooseLeptons();
        event.cleanElectronsFromLooseMuons();	
        if( event.numberOfLightLeptons() < 1 ) continue;

        // apply cone-correction to leptons 
        event.applyLeptonConeCorrection();

        for( const auto& leptonPtr : event.lightLeptonCollection() ){

            LightLepton& lepton = *leptonPtr;

	    // select correct lepton flavor
            if( isMuon && !lepton.isMuon() ) continue;
            if( !isMuon && !lepton.isElectron() ) continue;

            // veto prompt leptons and leptons originating from photons 
            if( lepton.isPrompt() ) continue; 

            // make sure lepton passes the minimum pt requirement (set when defining the pT histograms)
            if( lepton.pt() < minPt ) continue;

            // check whether lepton comes from a b-quark 
	    // ( provenanceCompressed = 1 ) or c-quark decay ( provenanceCompressed = 2 )
            bool isHeavyFlavor = ( lepton.provenanceCompressed() == 1 || 
	    			lepton.provenanceCompressed() == 2 );

	    // lepton must pass varying ptRatio and deepFlavor thresholds
            for( unsigned ptRatioI = 0; ptRatioI < ptRatioCuts.size(); ++ptRatioI ){
                if( lepton.ptRatio() < ptRatioCuts[ ptRatioI ] ) continue;
                for( unsigned deepFlavorI = 0; deepFlavorI < deepFlavorCutCollection.size(); ++deepFlavorI ){
                    //if( lepton.closestJetDeepFlavor() > deepFlavorCuts[ deepFlavorI ] ) continue;
                    if( lepton.closestJetDeepFlavor() >= deepFlavorCutCollection[ deepFlavorI ].cut( lepton.uncorrectedPt() ) ) continue;

                    // compute histogram vector index
                    auto histIndex = categories.index( {ptRatioI, deepFlavorI } );
                
                    // fill histograms for heavy flavor leptons 
                    if( isHeavyFlavor ){

                        // numerator = nonprompt leptons that are also tight 
                        if( lepton.isTight() ){
                            heavyFlavorNumerator[ histIndex ]->Fill( std::min( lepton.pt(), ptHistInfo.maxBinCenter() ), event.weight() );
                        }                        

                        // denominator = all nonprompt leptons
                        heavyFlavorDenominator[ histIndex ]->Fill( std::min( lepton.pt(), ptHistInfo.maxBinCenter() ), event.weight() );
                            
                    // fill histograms for light flavor leptons
                    } else {

                        // numerator = nonprompt leptons that are also tight
                        if( lepton.isTight() ){
                                lightFlavorNumerator[ histIndex ]->Fill( std::min( lepton.pt(), ptHistInfo.maxBinCenter() ), event.weight() );
                        }
                        
                        // denominator = all nonprompt leptons
                        lightFlavorDenominator[ histIndex ]->Fill( std::min( lepton.pt(), ptHistInfo.maxBinCenter() ), event.weight() );
                    }

                }
            }
        }
    }

    std::cout<<"finished event loop"<<std::endl;

    std::string file_name = "tuneFOSelection_" + leptonFlavor + "_" + year;
    file_name.append("_histograms_sample_"+std::to_string(sampleIndex)+".root");
    TFile* histogram_file = TFile::Open( file_name.c_str(), "RECREATE" );

    for(unsigned i=0; i < heavyFlavorNumerator.size(); ++i){
	heavyFlavorNumerator[i]->Write();
	heavyFlavorDenominator[i]->Write();
	lightFlavorNumerator[i]->Write();
	lightFlavorDenominator[i]->Write();
    }   

    histogram_file->Close();
    std::cout<<"done"<<std::endl;
}

int main( int argc, char* argv[] ){

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc != 4 ){
	std::cout<<"### ERROR ###: unrecognized number of arguments."<<std::endl;
	return -1;
    }
    std::string flavor = argvStr[1];
    std::string year = argvStr[2]; 
    unsigned sampleIndex = std::stoi(argvStr[3]);

    fakeRate::checkFlavorString( flavor );
    analysisTools::checkYearString( year );

    std::string sampleList = "/user/llambrec/ewkino/AnalysisCode/samplelists/";
    sampleList += "samples_tunefoselection_"+flavor+"_"+year+".txt";
    std::string sampleDirectory = "/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate/";

    fillTuneFOSelection( flavor, year, sampleList, sampleDirectory, sampleIndex );
    return 0;
}
