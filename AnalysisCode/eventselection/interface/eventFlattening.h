#ifndef eventFlattening_H
#define eventFlattening_H

// include ROOT classes
#include "TMVA/Reader.h"

// include other parts of framework
#include "../../../Event/interface/Event.h"

// include own functionality
#include "eventSelections.h"

// event id variables
extern ULong_t _runNb;
extern ULong_t _lumiBlock;
extern ULong_t _eventNb;

// event weight for simulation
extern Float_t _weight;
extern Float_t _normweight;

// event BDT variables
extern Float_t _abs_eta_recoil;
extern Float_t _Mjj_max;
extern Float_t _lW_asymmetry;
extern Float_t _deepCSV_max;
extern Float_t _lT;
extern Float_t _MT;
extern Float_t _pTjj_max;
extern Float_t _dRlb_min;
extern Float_t _dPhill_max;
extern Float_t _HT;
extern Float_t _nJets;
extern Float_t _nBJets;
extern Float_t _dRlWrecoil;
extern Float_t _dRlWbtagged;
extern Float_t _M3l;
extern Float_t _abs_eta_max;

// BDT output score
//extern Float_t _eventBDT;

// other variables
extern Int_t _nMuons;
extern Int_t _nElectrons;
extern Float_t _leptonMVAttH_min;
extern Float_t _leptonMVATOP_min;

// function declarations
void initOutputTree(TTree*);
//TMVA::Reader* initializeReader( TMVA::Reader* reader, const std::string& pathToXMLFile );
JetCollection getjetcollection(const Event&, const std::string& variation = "nominal");
Met getmet(const Event&, const std::string& variation = "nominal");
int eventCategory(Event&, const std::string& variation = "nominal");
std::shared_ptr< TH2D > readFRMap( const std::string&, const std::string&, const std::string&);
double fakeRateWeight( const Event&, const std::string&, 
			const std::shared_ptr< TH2D >&, const std::shared_ptr< TH2D >&);
void eventToEntry(Event&, double,
		    const bool isnpbackground = false, 
		    const std::shared_ptr< TH2D>& frMap_muon = nullptr, 
		    const std::shared_ptr< TH2D>& frMap_electron = nullptr,
		    const std::string& variation = "nominal");
std::pair<double,double> pmzcandidates(Lepton&, Met&);
std::pair<double,int> besttopcandidate(JetCollection&, Lepton&, Met&, double, double);

#endif

