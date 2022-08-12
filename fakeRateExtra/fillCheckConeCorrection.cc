/*
Code to check the rationale behind the cone correction for FO leptons.
The script fills 2D histograms of lepton pT vs mother parton pT at gen level,
both for the uncorrected and cone-corrected lepton pT.
The idea is that the cone-corrected pT is shown to be a good proxy for the mother parton pT.
See Fig. 10 of AN-2018-100.
Note: not yet clear what variable to use for the mother parton pT...
      the current variable is only a first attempt and not guaranteed to be correct!
*/


//include c++ library classes 
#include <stdexcept>

//include ROOT classes
#include "TH1D.h"
#include "TF1.h"

//include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "../Tools/interface/analysisTools.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"


void fillConeCorrectionCorrelationHistograms( 
	const std::string& leptonFlavor, 
	const std::string& year, 
	const std::string& sampleDirectory, 
	const std::string& sampleList,
	const unsigned sampleIndex ){

    // check flavour and year
    bool isMuon = ( leptonFlavor == "muon" );
    bool isElectron = ( leptonFlavor == "electron" );
    if( !(isMuon || isElectron) ){
	std::cerr << "ERROR: lepton flavour '"<<leptonFlavor<<"' not recognized." << std::endl;
    }
    analysisTools::checkYearString( year );

    // initialize histograms
    const float lowerPt = 10;
    const float upperPt = 100;
    const unsigned numberOfBins = 45;
    std::shared_ptr< TH2D > uncorrectedPtCorrPrompt = std::make_shared< TH2D >( 
	"uncorrectedPtCorrPrompt", "uncorrectedPtCorrPrompt;Parton p_{T} (GeV);Uncorrected RECO p_{T} (GeV)",
	numberOfBins, lowerPt, upperPt, numberOfBins, lowerPt, upperPt );
    uncorrectedPtCorrPrompt->Sumw2();
    std::shared_ptr< TH2D > uncorrectedPtCorrNonPrompt = std::make_shared< TH2D >(
        "uncorrectedPtCorrNonPrompt", "uncorrectedPtCorrNonPrompt;Parton p_{T} (GeV);Uncorrected RECO p_{T} (GeV)",
        numberOfBins, lowerPt, upperPt, numberOfBins, lowerPt, upperPt );
    uncorrectedPtCorrNonPrompt->Sumw2();
    std::shared_ptr< TH2D > correctedPtCorrPrompt = std::make_shared< TH2D >( 
	"correctedPtCorrPrompt", "correctedPtCorrPrompt;Parton p_{T} (GeV);Cone-corrected RECO p_{T} (GeV)", 
	numberOfBins, lowerPt, upperPt, numberOfBins, lowerPt, upperPt );
    correctedPtCorrPrompt->Sumw2();
    std::shared_ptr< TH2D > correctedPtCorrNonPrompt = std::make_shared< TH2D >(   
        "correctedPtCorrNonPrompt", "correctedPtCorrNonPrompt;Parton p_{T} (GeV);Cone-corrected RECO p_{T} (GeV)",
        numberOfBins, lowerPt, upperPt, numberOfBins, lowerPt, upperPt );
    correctedPtCorrNonPrompt->Sumw2();

    // make tree reader and set to correct sample
    std::cout << "creating TreeReader and setting to sample no. " << sampleIndex << std::endl;
    TreeReader treeReader( sampleList, sampleDirectory );
    treeReader.initSample();
    for( unsigned i=1; i<=sampleIndex; ++i){
        treeReader.initSample();
    }

    // extra check on year
    if( (year=="2016" && !treeReader.is2016()) ||
	(year=="2016PreVFP" && !treeReader.is2016PreVFP()) ||
	(year=="2016PostVFP" && !treeReader.is2016PostVFP()) ||
	(year=="2017" && !treeReader.is2017()) ||
	(year=="2018" && !treeReader.is2018()) ){
	std::cerr << "ERROR: given year and sample year do not agree..." << std::endl;
	return;
    }

    // loop over events in sample
    long unsigned nentries = treeReader.numberOfEntries();
    //long unsigned limit = 1000000;
    //nentries = std::min(nentries, limit); // for testing
    std::cout << "starting event loop for " << nentries << " events." << std::endl;
    for( long unsigned entry = 0; entry < nentries; ++entry ){
        Event event = treeReader.buildEvent( entry );

        // select loose leptons
        event.selectLooseLeptons();
        event.cleanElectronsFromLooseMuons();
        
        for( const auto& leptonPtr : event.lightLeptonCollection() ){

            // select correct lepton flavor
            if( isMuon && !leptonPtr->isMuon() ) continue;
            if( !isMuon && !leptonPtr->isElectron() ) continue;

            // apply baseline pT cut 
            if( leptonPtr->uncorrectedPt() <= 10 ) continue;

            // calculate the different pT variables 
            double uncorrectedPt = leptonPtr->uncorrectedPt();
	    double correctedPt = uncorrectedPt / leptonPtr->ptRatio();
            double genPt = leptonPtr->momPt();

	    // printouts for testing
	    /*std::cout << "lepton" << std::endl;
	    std::cout << genPt << std::endl;
	    std::cout << uncorrectedPt << std::endl;
	    std::cout << correctedPt << std::endl;*/

	    // fill histograms
	    double weight = (event.weight()<0.) ? -1 : 1;
	    if( leptonPtr->isPrompt() ){
		uncorrectedPtCorrPrompt->Fill( genPt, uncorrectedPt , weight );
		correctedPtCorrPrompt->Fill( genPt, correctedPt , weight );
	    } else{
		uncorrectedPtCorrNonPrompt->Fill( genPt, uncorrectedPt , weight );
                correctedPtCorrNonPrompt->Fill( genPt, correctedPt , weight );
	    }
	}
    }
    std::cout<<"finished event loop"<<std::endl;

    std::string fileName = "coneCorrectionCheck_" + leptonFlavor + "_" + year;
    fileName.append("_histograms_sample_"+std::to_string(sampleIndex)+".root");
    TFile* histogramFile = TFile::Open( fileName.c_str(), "RECREATE" );

    uncorrectedPtCorrPrompt->Write();
    correctedPtCorrPrompt->Write();
    uncorrectedPtCorrNonPrompt->Write();
    correctedPtCorrNonPrompt->Write();

    histogramFile->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc != 6 ){
        std::cerr << argc - 1 << " command line arguments given, while 5 are expected." << std::endl;
        std::cerr << "  - lepton flavor" << std::endl;
	std::cerr << "  - data taking year" << std::endl;
	std::cerr << "  - sample directory" << std::endl;
	std::cerr << "  - sample list" << std::endl; 
	std::cerr << "  - sample index" << std::endl;
	return 1;
    }
    
    std::string leptonFlavor = argvStr[1];
    std::string year = argvStr[2];
    std::string sampleDirectory = argvStr[3];
    std::string sampleList = argvStr[4];
    int sampleIndex = std::stoi( argvStr[5] );
    fillConeCorrectionCorrelationHistograms( leptonFlavor, year,
                                   sampleDirectory, sampleList, sampleIndex );

    std::cerr << "###done###" << std::endl;
    return 0;
}
