/*
Code to determine the so-called 'magic-factor' in the lepton cone-correction when using a lepton MVA discriminator.
FO leptons are cone-corrected by dividing their pT by pTRatio( = pT^lepton/pT^jet ) to effectively become pT^jet.
The border between FO and tight is a lepton MVA cut, and we want to avoid a discontinuity in the pT spectrum as a function of the lepton MVA. This is what the magic factor attempts to address.
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
#include "../Tools/interface/analysisTools.h"
#include "../fakeRate/interface/fakeRateTools.h"


void determineMagicFactor( const std::string& leptonFlavor, const std::string& year, 
			    const std::string& leptonMVA, const double mvaThreshold, 
			    const std::string& sampleDirectory, const std::string& sampleList,
			    const unsigned sampleIndex ){
    // determine the 'magic factor' for a given lepton flavor, lepton MVA and threshold.
    // leptonMVA can only have the values 'leptonMVAttH', 'leptonMVAtZq', 'leptonMVATOP'

    // check flavour and year
    fakeRate::checkFlavorString( leptonFlavor );
    bool isMuon = ( leptonFlavor == "muon" );
    analysisTools::checkYearString( year );
    // check mva
    if( !(leptonMVA=="leptonMVAttH" || leptonMVA=="leptonMVAtZq" || leptonMVA=="leptonMVATOP") ){
	std::cout<<"### ERROR ###: choice of leptonMVA '"<<leptonMVA<<"' not recognized."<<std::endl;
	return;
    }

    // initialize histograms
    const unsigned numberOfBins = 80;
    std::shared_ptr< TH1D > pTWeightedLeptonMVAHistogram = std::make_shared< TH1D >( 
	"pTLeptonMVA", "pTLeptonMVA;lepton MVA;Average p_{T}^{cone} (GeV)", numberOfBins, -1, 1 );
    pTWeightedLeptonMVAHistogram->Sumw2();
    std::shared_ptr< TH1D > leptonMVAHistogram = std::make_shared< TH1D >( 
	"leptonMVA", "leptonMVA;lepton MVA;Events", numberOfBins, -1, 1 );
    leptonMVAHistogram->Sumw2();

    // make tree reader and set to correct sample
    std::cout << "creating TreeReader and setting to sample no. " << sampleIndex << std::endl;
    TreeReader treeReader( sampleList, sampleDirectory );
    treeReader.initSample();
    for( unsigned i=1; i<=sampleIndex; ++i){
        treeReader.initSample();
    }

    // extra check on year
    if((year=="2016" && !treeReader.is2016()) ||
	(year=="2017" && !treeReader.is2017()) ||
	(year=="2018" && !treeReader.is2018())){
	std::cerr<<"### ERROR ###: given year and sample index do not agree..."<<std::endl;
	return;
    }

    // loop over events in sample
    long unsigned nentries = treeReader.numberOfEntries();
    //long unsigned nentries = 100; // for testing
    std::cout<<"starting event loop for "<<nentries<<" events."<<std::endl;
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
			    (leptonMVA=="leptonMVATOP") ? leptonPtr->leptonMVATOP() : 0.;
            if( mvaVal <= mvaThreshold ){
                ptVal /= leptonPtr->ptRatio();
            }

	    // fill histograms
	    double weight = (event.weight()>0.) ? 1 : -1;
            pTWeightedLeptonMVAHistogram->Fill( mvaVal , weight*ptVal );
            leptonMVAHistogram->Fill( mvaVal , weight );
	}
    }
    std::cout<<"finished event loop"<<std::endl;

    std::string file_name = "magicFactor_" + leptonFlavor + "_" + year;
    file_name.append("_histograms_sample_"+std::to_string(sampleIndex)+".root");
    TFile* histogram_file = TFile::Open( file_name.c_str(), "RECREATE" );

    pTWeightedLeptonMVAHistogram->Write();
    leptonMVAHistogram->Write();

    histogram_file->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    if( argc == 8 ){
        std::string leptonFlavor = argvStr[1];
	std::string year = argvStr[2];
	std::string leptonMVA = argvStr[3];
        double mvaThreshold = std::stod( argvStr[4] );
	std::string sampleDirectory = argvStr[5];
	std::string sampleList = argvStr[6];
	int sampleIndex = std::stoi( argvStr[7] );
        determineMagicFactor( leptonFlavor, year, leptonMVA, mvaThreshold, 
				sampleDirectory, sampleList, sampleIndex );
    } else {
        std::cerr << argc - 1 << " command line arguments given, while 7 are expected." << std::endl;
        std::cerr << "< flavor > < year > < lepton MVA > < mva threshold >";
	std::cerr << " < sample directory > < sample list > < sample index >" << std::endl;
    }
    
    std::cerr << "###done###" << std::endl;
    return 0;
}
