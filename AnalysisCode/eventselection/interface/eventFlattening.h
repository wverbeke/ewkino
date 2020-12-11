#ifndef eventFlattening_H
#define eventFlattening_H

// include ROOT classes
#include "TMVA/Reader.h"

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../weights/interface/ConcreteReweighterFactory.h"

// include own functionality
#include "eventSelections.h"

// function declarations

namespace eventFlattening{
    void setVariables(std::map<std::string,double>);
    std::map< std::string, double > initVarMap();
    void initOutputTree(TTree*);
    TMVA::Reader* initializeReader( TMVA::Reader* reader, 
				    const std::string& pathToXMLFile,
				    const std::string& bdtCombineMode="all" );
    std::shared_ptr< TH2D > readFRMap( const std::string&, const std::string&, const std::string&);
    double fakeRateWeight( const Event&, 
    			const std::shared_ptr< TH2D >&, const std::shared_ptr< TH2D >&);
    int fakeRateFlavour( const Event& );
    std::map< std::string, double > eventToEntry(Event& event, double norm,
				const CombinedReweighter& reweighter,
				const std::string& selection_type, 
				const std::shared_ptr< TH2D>& frMap_muon = nullptr, 
				const std::shared_ptr< TH2D>& frMap_electron = nullptr,
				const std::string& variation = "nominal",
				const bool doMVA = false,
				TMVA::Reader* = nullptr);
    std::pair<double,double> pmzcandidates(Lepton&, Met&);
    std::pair<double,int> besttopcandidate(JetCollection&, Lepton&, Met&, double, double);
}

#endif
