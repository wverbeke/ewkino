// import c++ libraries
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>

// import framework
#include "../plotting/tdrStyle.h"
#include "../Event/interface/Event.h"

// import tools in this folder
#include "interface/prescaleMeasurementTools.h"

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    // check number of command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 6 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 5 are needed."<<std::endl;
        std::cerr<<"usage: ./fillPrescaleMeasurement year sampleDirectory sampleList sampleIndex";
	std::cerr<<" isTestRun"<<std::endl;
        return 1;
    }

    // set year and flavour
    std::string year = argvStr[1];
    std::string& sampleDirectory = argvStr[2];
    std::string& sampleList = argvStr[3];
    const unsigned sampleIndex = std::stoi(argvStr[4]);
    const bool isTestRun = (argvStr[5]=="true" || argvStr[5]=="True");
    
    // define and set configuration variables
    const double metLowerCut_prescaleMeasurement = 40;
    const double mTLowerCut_prescaleMeasurement = 0;
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
        { "2018", std::vector< std::string >( { 
		    "HLT_Mu3_PFJet40", "HLT_Mu8", "HLT_Mu17", "HLT_Mu20", "HLT_Mu27", 
		    "HLT_Ele8_CaloIdM_TrackIdM_PFJet30", "HLT_Ele17_CaloIdM_TrackIdM_PFJet30", 
		    "HLT_Ele23_CaloIdM_TrackIdM_PFJet30" 
	} ) }
    };

    fillPrescaleMeasurementHistograms(year, sampleDirectory, sampleList, sampleIndex, 
					isTestRun,
					triggerVectorMap[ year ], use_mT, 
					metLowerCut_prescaleMeasurement, 
					mTLowerCut_prescaleMeasurement );
    std::cerr << "###done###" << std::endl;
    return 0;
}
