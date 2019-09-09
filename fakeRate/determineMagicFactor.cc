/*
Code to determine the so-called 'magic-factor' in the lepton cone-correction when using a lepton MVA discriminator.
FO leptons are cone-corrected by dividing their pT by pTRatio( = pT^lepton/pT^jet ) to effectively become pT^jet.
The border between FO and tight is a lepton MVA cut, and we want to avoid a discontinuity in the pT spectrum as a function of the lepton MVA. This is what the magic factor attempts to address
*/


//include c++ library classes 
#include <stdexcept>

//include ROOT classes
#include "TH1D.h"
#include "TF1.h"

//include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"


void determineMagicFactor( const std::string& leptonFlavor, const double mvaThreshold ){

    bool isMuon;
    if( leptonFlavor == "muon" ){
        isMuon = true;
    } else if( leptonFlavor == "electron" ){
        isMuon = false;
    } else {
        throw std::invalid_argument( "leptonFlavor string should be either 'muon' or 'electron'." );
    }
    

    const unsigned numberOfBins = 80;
    std::shared_ptr< TH1D > pTWeightedLeptonMVAHistogram = std::make_shared< TH1D >( "pTLeptonMVA", "pTLeptonMVA;lepton MVA;Average p_{T}^{cone} (GeV)", numberOfBins, -1, 1 );
    pTWeightedLeptonMVAHistogram->Sumw2();
    std::shared_ptr< TH1D > LeptonMVAHistogram = std::make_shared< TH1D >( "leptonMVA", "leptonMVA;lepton MVA;Events", numberOfBins, -1, 1 );
    LeptonMVAHistogram->Sumw2();

	TreeReader treeReader( "samples_magicFactor.txt", "../test/testData");
    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry );
        
            //preselect loose leptons
            //magic factor tuning will happen before FO definition is final
            event.selectLooseLeptons();
        
            for( const auto& leptonPtr : event.lightLeptonCollection() ){

                //select correct lepton flavor
                if( isMuon && !leptonPtr->isMuon() ) continue;
                if( !isMuon && !leptonPtr->isElectron() ) continue;

                //apply cone-correction to leptons failing the MVA cut 
                double ptVal = leptonPtr->pt();
                if( leptonPtr->leptonMVAttH() <= mvaThreshold ){
                    ptVal /= leptonPtr->ptRatio();
                }

                pTWeightedLeptonMVAHistogram->Fill( leptonPtr->leptonMVAttH(), event.weight()*ptVal );
                LeptonMVAHistogram->Fill( leptonPtr->leptonMVAttH(), event.weight() );
            }
		}
    }

    //make average pT histogram as a function of lepton MVA by dividing the pT-weighted histogram by the normal one
    //first remove the errors on the denominator to avoid double counting 
    for( int bin = 1; bin < LeptonMVAHistogram->GetNbinsX() + 1; ++bin ){
        LeptonMVAHistogram->SetBinError( bin, 0. );
    }
    pTWeightedLeptonMVAHistogram->Divide( LeptonMVAHistogram.get() );

    //set CMS plotting style 
   	setTDRStyle();

    pTWeightedLeptonMVAHistogram.get()->SetStats( 0 );
    plotHistograms(pTWeightedLeptonMVAHistogram.get(), "", "averagePTVSLeptonMVA_" + leptonFlavor, false);

    //determine magic factor 
    auto binIndexAboveThreshold = pTWeightedLeptonMVAHistogram->FindBin( mvaThreshold );
    double aboveThreshold = pTWeightedLeptonMVAHistogram->GetBinContent( binIndexAboveThreshold );
    double belowThreshold = pTWeightedLeptonMVAHistogram->GetBinContent( binIndexAboveThreshold - 1 );
    double magicFactor = aboveThreshold/belowThreshold;
    std::cout << "##################################" << std::endl;
    std::cout << "magic factor from equalizing bins = " << magicFactor << std::endl;
    std::cout << "##################################" << std::endl;

    if( mvaThreshold < 0.6 && mvaThreshold > -0.6 ){

        //try determining magic factor with a fit 
        TF1* f_aboveThreshold = new TF1( "f_aboveThreshold", "[0]*x + [1]" );
        TF1* f_belowThreshold = new TF1( "f_belowThreshold", "[0]*x + [1]" );

        pTWeightedLeptonMVAHistogram->Fit( "f_aboveThreshold", "", "", mvaThreshold, 0.6  );
        pTWeightedLeptonMVAHistogram->Fit( "f_belowThreshold", "", "", -0.6, mvaThreshold );

        std::cout << "fit above threshold = " << f_aboveThreshold->GetParameter(0) << "*x + " << f_aboveThreshold->GetParameter(1) << "\t chisquare = " << f_aboveThreshold->GetChisquare() << std::endl;
        std::cout << "fit below threshold = " << f_belowThreshold->GetParameter(0) << "*x + " << f_belowThreshold->GetParameter(1) << "\t chisquare = " << f_belowThreshold->GetChisquare() << std::endl;
        std::cout << "##################################" << std::endl;
        std::cout << "magic factor from fit = " << (  f_aboveThreshold->GetParameter(0)*mvaThreshold + f_aboveThreshold->GetParameter(1) )/( f_belowThreshold->GetParameter(0)*mvaThreshold + f_belowThreshold->GetParameter(1) ) << std::endl;
        std::cout << "##################################" << std::endl;
    }
    

    //plot average pT histogram after application of magic factor
    for( int bin = 1; bin < binIndexAboveThreshold; ++bin ){
        pTWeightedLeptonMVAHistogram->SetBinContent( bin, pTWeightedLeptonMVAHistogram->GetBinContent( bin )*magicFactor );
    }
    plotHistograms( pTWeightedLeptonMVAHistogram.get(), "", "averagePTVSLeptonMVA_postMagicFactor_" + leptonFlavor, false );
}


int main( int argc, char* argv[] ){
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc == 3 ){
        std::string leptonFlavor = argvStr[1];
        double mvaThreshold = std::stod( argvStr[2] );
        determineMagicFactor( leptonFlavor, mvaThreshold );
    } else {
        std::cerr << argc - 1 << " command line arguments given, while 2 are expected." << std::endl;
        std::cerr << "Usage : ./determineMagicFactor < lepton flavor > < mva threshold >" << std::endl;
    }
    
    return 0;
}
