// include header
#include "../interface/eventCategorization.h"

// event id variables
ULong_t _runNb;
ULong_t _lumiBlock;
ULong_t _eventNb;

// event BDT variables
Double_t _abs_eta_recoil;
Double_t _Mjj_max;
Double_t _lW_asymmetry;
Double_t _deepCSV_max;
Double_t _lT;
Double_t _MT;
Double_t _pTjj_max;
Double_t _dRlb_max;
Double_t _dPhill_max;
Double_t _HT;
UInt_t _nJets;
Double_t _dRlWrecoil;
Double_t _dRlWbtagged;
Double_t _M3l;
Double_t _abs_eta_max;

void initOutputTree(TTree* outputTree){
    // set branches for a flat output tree, to be used instead of ewkino/TreeReader/src/setOutputTree. 
    
    // event id variables
    outputTree->Branch("_runNb", &_runNb, "_runNb/l");
    outputTree->Branch("_lumiBlock", &_lumiBlock, "_lumiBlock/l");
    outputTree->Branch("_eventNb", &_eventNb, "_eventNb/l");
    
    // event BDT variables
    outputTree->Branch("_abs_eta_recoil", &_abs_eta_recoil, "_abs_eta_recoil/D");
    outputTree->Branch("_Mjj_max", &_Mjj_max, "_Mjj_max/D");
    outputTree->Branch("_lW_asymmetry", &_lW_asymmetry, "_lW_asymmetry/D");
    outputTree->Branch("_deepCSV_max", &_deepCSV_max, "_deepCSV_max/D");
    outputTree->Branch("_lT", &_lT, "_lT/D");
    outputTree->Branch("_MT", &_MT, "_MT/D");
    outputTree->Branch("_pTjj_max", &_pTjj_max, "_pTjj_max/D");
    outputTree->Branch("_dRlb_max", &_dRlb_max, "_dRlb_max/D");
    outputTree->Branch("_dPhill_max", &_dPhill_max, "_dPhill_max/D");
    outputTree->Branch("_HT", &_HT, "_HT/D");
    outputTree->Branch("_nJets", &_nJets, "_nJets/i");
    outputTree->Branch("_dRlWrecoil", &_dRlWrecoil, "_dRlWrecoil/D");
    outputTree->Branch("_dRlWbtagged", &_dRlWbtagged, "_dRlWbtagged/D");
    outputTree->Branch("_M3l", &_M3l, "_M3l/D");
    outputTree->Branch("_abs_eta_max", &_abs_eta_max, "_abs_eta_max/D");
} 


void entryFromEvent(Event& event, TTree* outputTree){
    // fill one entry in outputTree (initialized with initOutputTree), based on the info of one event.
    // note that it is assumed the event has been passed through a signal region selection!
    // this is important for e.g. the number of leptons in the event.
    // later find a way to extend this to control regions; not enirely clear how this is done in the AN.
    
    // event id variables 
    _runNb = event.runNumber();
    _lumiBlock = event.luminosityBlock();
    _eventNb = event.eventNumber();
    
    // event BDT variables
    // sort jets and leptons by pT
    event.sortJetsByPt();
    event.sortLeptonsByPt();
    // (note that leptons are in principle already sorted implicitly in passES!)
    // find lW
    int lW = event.WLeptonIndex();
    std::pair< int, int > temp = event.bestZBosonCandidateIndices();
    int lZ1 = temp.first; int lZ2 = temp.second;
    std::cout<<"lZ1, lZ2, lW: "<<lZ1<<" "<<lZ2<<" "<<lW<<std::endl;
    // loop over jets and find relevant quantities
    double abs_eta_max = 0;
    int recoilindex = -1;
    for(JetCollection::const_iterator jIt = event.jetCollection().cbegin();
        jIt != event.jetCollection().cend(); jIt++){
	Jet& jet = **jIt;
        if(std::abs(jet.eta())>abs_eta_max) abs_eta_max = std::abs(jet.eta());
	std::cout<<"max eta: "<<abs_eta_max<<std::endl;
    }
    
}
