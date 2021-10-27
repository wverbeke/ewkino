/*
Definition of event variables for H->W+Ds analysis
*/


#ifndef eventFlattening_H
#define eventFlattening_H

// include ROOT classes
#include "TMVA/Reader.h"

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../objects/interface/PhysicsObject.h"
#include "../../../weights/interface/ConcreteReweighterFactory.h"

// include other parts of the analysis code
#include "eventSelections.h"
#include "../../tools/interface/fakeRateTools.h"
#include "../../nureco/interface/nuReco.h"

namespace eventFlattening{

    // help functions
    double getVariable( const std::map<std::string,double>& varmap, 
					 const std::string& varname );
    void setVariables(std::map<std::string,double> varmap);
    std::map< std::string, double > initVarMap();
    void initOutputTree(TTree* outputTree);
    TMVA::Reader* initializeReader( TMVA::Reader* reader, 
	const std::string& pathToXMLFile );

    // main function
    std::map< std::string, double > eventToEntry(Event& event, 
		const double norm,
		const CombinedReweighter& reweighter,
		const std::string& selection_type, 
		const std::shared_ptr< TH2D>& frMap_muon, 
		const std::shared_ptr< TH2D>& frMap_electron,
		const std::string& variation,
		const bool doMVA,
		TMVA::Reader* reader);

    // internal help functions for reconstruction
    std::pair<PhysicsObject,PhysicsObject> makeHiggsAndNeutrinoCandidate( 
	const Lepton& lepton, const Met& met,
        double nupz, const DMeson& dmeson);
    std::pair<PhysicsObject,PhysicsObject> bestHiggsAndNeutrinoCandidate( 
	const Lepton& lepton, const Met& met,
        std::pair<double,double> nupzcands, const DMeson& dmeson);
}

#endif
