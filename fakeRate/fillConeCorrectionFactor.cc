/*
Code to determine the cone correction factor (also known as 'magic factor') 
in the lepton cone-correction when using a lepton MVA discriminator.
FO leptons are cone-corrected by dividing their pT by pTRatio( = pT^lepton/pT^jet ) 
to effectively become pT^jet.
The border between FO and tight is a lepton MVA cut, and we want to avoid a discontinuity 
in the pT spectrum as a function of the lepton MVA. 
This is what the cone correction factor attempts to address.
*/


//include c++ library classes 
#include <stdexcept>

//include ROOT classes
#include "TH1D.h"
#include "TF1.h"

//include other parts of framework
#include "interface/fakeRateTools.h"
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "../Tools/interface/analysisTools.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"


void determineConeCorrectionFactor( 
	const std::string& leptonFlavor, 
	const std::string& year, 
	const std::string& leptonMVA,
	const std::string& wpName, 
	const double wpThreshold, 
	const std::string& sampleDirectory, 
	const std::string& sampleList,
	const unsigned sampleIndex ){
    // determine the 'cone correction factor' for a given lepton flavor, 
    // lepton MVA and threshold.
    // leptonMVA is an string to identifiy which MVA to use,
    // see below for allowed values!

    // check flavour and year
    fakeRate::checkFlavorString( leptonFlavor );
    bool isMuon = ( leptonFlavor == "muon" );
    analysisTools::checkYearString( year );
    // check mva
    if( !(  leptonMVA=="leptonMVAttH" 
	    || leptonMVA=="leptonMVAtZq" 
	    || leptonMVA=="leptonMVATOP"
	    || leptonMVA=="leptonMVATOPUL"
	    || leptonMVA=="leptonMVATOPv2UL" ) ){
	std::cerr << "ERROR: choice of leptonMVA '"<<leptonMVA<<"' not recognized." << std::endl;
	return;
    }

    // initialize histograms
    const float lowerBound = -1;
    const float upperBound = 1;
    const unsigned numberOfBins = 200;
    std::shared_ptr< TH1D > pTWeightedLeptonMVAHistogram = std::make_shared< TH1D >( 
	"pTLeptonMVA", "pTLeptonMVA;lepton MVA;Average p_{T}^{cone} (GeV)", 
	numberOfBins, lowerBound, upperBound );
    pTWeightedLeptonMVAHistogram->Sumw2();
    std::shared_ptr< TH1D > leptonMVAHistogram = std::make_shared< TH1D >( 
	"leptonMVA", "leptonMVA;lepton MVA;Events", 
	numberOfBins, lowerBound, upperBound );
    leptonMVAHistogram->Sumw2();

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
    //long unsigned nentries = treeReader.numberOfEntries();
    long unsigned nentries = 50000; // for testing
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

            // apply cone-correction to leptons failing the MVA cut 
            double ptVal = leptonPtr->pt();
	    double mvaVal = (leptonMVA=="leptonMVAttH") ? leptonPtr->leptonMVAttH() : 
			    (leptonMVA=="leptonMVAtZq") ? leptonPtr->leptonMVAtZq() :
			    (leptonMVA=="leptonMVATOP") ? leptonPtr->leptonMVATOP() :
			    (leptonMVA=="leptonMVATOPUL") ? leptonPtr->leptonMVATOPUL() :
			    (leptonMVA=="leptonMVATOPv2UL") ? leptonPtr->leptonMVATOPv2UL() : 0.0;
            if( mvaVal <= wpThreshold ){
                ptVal /= leptonPtr->ptRatio();
            }

	    // fill histograms
	    double weight = (event.weight()>0.) ? 1 : -1;
            pTWeightedLeptonMVAHistogram->Fill( mvaVal , weight*ptVal );
            leptonMVAHistogram->Fill( mvaVal , weight );
	}
    }
    std::cout<<"finished event loop"<<std::endl;

    std::string fileName = "coneCorrectionFactor_" + leptonMVA + "_" + wpName + "_" + leptonFlavor + "_" + year;
    fileName.append("_histograms_sample_"+std::to_string(sampleIndex)+".root");
    TFile* histogramFile = TFile::Open( fileName.c_str(), "RECREATE" );

    pTWeightedLeptonMVAHistogram->Write();
    leptonMVAHistogram->Write();

    histogramFile->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc != 9 ){
        std::cerr << argc - 1 << " command line arguments given, while 8 are expected." << std::endl;
        std::cerr << "  - lepton flavor" << std::endl;
	std::cerr << "  - data taking year" << std::endl;
	std::cerr << "  - lepton MVA identifier" << std::endl;
	std::cerr << "  - working point name" << std::endl;
	std::cerr << "  - numerical working point threshold" << std::endl;
	std::cerr << "  - sample directory" << std::endl;
	std::cerr << "  - sample list" << std::endl; 
	std::cerr << "  - sample index" << std::endl;
	return 1;
    }
    
    std::string leptonFlavor = argvStr[1];
    std::string year = argvStr[2];
    std::string leptonMVA = argvStr[3];
    std::string wpName = argvStr[4];
    double wpThreshold = std::stod( argvStr[5] );
    std::string sampleDirectory = argvStr[6];
    std::string sampleList = argvStr[7];
    int sampleIndex = std::stoi( argvStr[8] );
    determineConeCorrectionFactor( leptonFlavor, year, leptonMVA, wpName, wpThreshold,
                                   sampleDirectory, sampleList, sampleIndex );

    std::cerr << "###done###" << std::endl;
    return 0;
}
