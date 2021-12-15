// import c++ libraries
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>

// import tools
#include "interface/prescaleMeasurementTools.h"
#include "interface/fakeRateMeasurementTools.h"

int main( int argc, char* argv[] ){
    
    std::cerr << "###starting###" << std::endl;

    // check command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 7 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 6 are needed."<<std::endl;
        std::cerr<<"usage: ./fillFakeRateMeasurement flavour year";
	std::cerr<<" sampleDirectory sampleList sampleIndex isTestRun"<<std::endl;
        return 1;
    }
    std::string year = argvStr[2];
    std::string flavor = argvStr[1];
    const std::string& sampleDirectory = argvStr[3];
    const std::string sampleList = argvStr[4];
    const unsigned sampleIndex = std::stoi(argvStr[5]);
    const bool isTestRun = (argvStr[6]=="true" || argvStr[6]=="True");

    // configuration and variable definition
    const double mTLowerCut_prescaleFit = 90; // 90
    const double mTUpperCut_prescaleFit = 130; // 130
    const double metUpperCut_fakeRateMeasurement = 20; // 20
    const double mTUpperCut_fakeRateMeasurement = 160; // 160
    const bool use_mT = true; 

    std::map< std::string, std::vector< std::string > > triggerVectorMap = {
        { "2016", std::vector< std::string >( {
                    "HLT_Mu3_PFJet40", "HLT_Mu8", "HLT_Mu17", "HLT_Mu20", "HLT_Mu27",
                    "HLT_Ele8_CaloIdM_TrackIdM_PFJet30", "HLT_Ele12_CaloIdM_TrackIdM_PFJet30",
                    "HLT_Ele17_CaloIdM_TrackIdM_PFJet30", "HLT_Ele23_CaloIdM_TrackIdM_PFJet30"
        } ) },
        { "2017", std::vector< std::string >( {
                    "HLT_Mu3_PFJet40", "HLT_Mu8", "HLT_Mu17", "HLT_Mu20", "HLT_Mu27",
                    "HLT_Ele8_CaloIdM_TrackIdM_PFJet30", "HLT_Ele17_CaloIdM_TrackIdM_PFJet30",
                    "HLT_Ele23_CaloIdM_TrackIdM_PFJet30"
        } ) },
        { "2018", std::vector< std::string >( { "HLT_Mu3_PFJet40", "HLT_Mu8", "HLT_Mu17",
                    "HLT_Mu20", "HLT_Mu27", "HLT_Ele8_CaloIdM_TrackIdM_PFJet30",
                    "HLT_Ele17_CaloIdM_TrackIdM_PFJet30", "HLT_Ele23_CaloIdM_TrackIdM_PFJet30"
        } ) }
    };

    setTDRStyle();

    std::map< std::string, Prescale > prescaleMap;
    std::string file_name = std::string( "prescaleMeasurement_" ) + ( use_mT ? "mT" : "met" );
    file_name.append("_histograms_" + year + ".root");
    TFile* prescale_filePtr = TFile::Open( file_name.c_str() );
    prescaleMap = fakeRate::fitTriggerPrescales_cut( prescale_filePtr, mTLowerCut_prescaleFit, 
		    mTUpperCut_prescaleFit, false, false );
    prescale_filePtr->Close();

    fillFakeRateMeasurementHistograms(flavor, year, sampleDirectory, sampleList, sampleIndex,
	triggerVectorMap[ year ], prescaleMap, mTUpperCut_fakeRateMeasurement, 
	metUpperCut_fakeRateMeasurement, isTestRun );

    std::cerr << "###done###" << std::endl;
    return 0;
}
