// import c++ libraries
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>

// import framework
#include "../../plotting/tdrStyle.h"
#include "../../Event/interface/Event.h"

// import tools in this folder
#include "interface/prescaleMeasurementTools.h"

int main( int argc, char* argv[] ){
    // check number of command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 3 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 2 are needed."<<std::endl;
        std::cerr<<"usage: ./fillPrescaleMeasurement year sampleIndex"<<std::endl;
        return 1;
    }

    // set year and flavour
    std::string year = argvStr[1];
    const unsigned sampleIndex = std::stoi(argvStr[2]);
    
    // define and set configuration variables
    const double metLowerCut_prescaleMeasurement = 40;
    const double mTLowerCut_prescaleMeasurement = 0;
    const bool use_mT = true;
    const std::string& sampleDirectory = "/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate";
    const std::string& sampleList = "../samplelists/samples_fakeratemeasurement_"+year+".txt";

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
					triggerVectorMap[ year ], use_mT, 
					metLowerCut_prescaleMeasurement, 
					mTLowerCut_prescaleMeasurement );
    return 0;
}
