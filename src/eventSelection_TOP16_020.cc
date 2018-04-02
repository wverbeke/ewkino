//include ROOT classes
#include "TLorentzVector.h"
#include "TH1D.h"
#include "TFile.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "TROOT.h"

//include C++ library classes
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <tuple>

#include <chrono>

//include other parts of the code
#include "../interface/treeReader.h"

//selection functions for TOP_16_020
bool treeReader::lepIsVeto_TOP16_020(const unsigned l) const{
    if(_lPt[l] <= 10) return false;
    if(fabs(_lEta[l]) >= (2.5 - 0.1*_lFlavor[l])) return false;
    if(_lFlavor[l] == 0){
        return _lPOGVeto[l];
    } else if(_lFlavor[l] == 1){
        if(!_lPOGLoose[l]) return false;
        //return _relIso0p4Mu[l] < 0.25;
        //THIS IS WRONG: replace with deltaBeta corrected iso
        return _relIso0p4[l] < 0.25;
    }
    return false;
}

bool treeReader::lepIsGood_TOP16_020(const unsigned l) const{
    if(_lPt[l] <= 25) return false;
    if(fabs(_lEta[l]) >= (2.5 - 0.1*_lFlavor[l]) ) return false;
    if(!_lPOGTight[l]) return false;
    //if(_lFlavor[l] == 1 && _relIso0p4Mu[l] >= 0.15) return false;
    //THIS IS WRONG: replace with deltaBeta corrected iso
    if(_lFlavor[l] == 1 && _relIso0p4[l] >= 0.15) return false;
    return true;
}

bool treeReader::lepIsTight_TOP16_020(const unsigned l) const{
    return lepIsGood_TOP16_020(l);
}

unsigned treeReader::selectLep_TOP16_020(std::vector<unsigned>& ind) const{
    ind.clear();
    unsigned lCount = 0;
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood_TOP16_020(l)){
            ++lCount;
            ind.push_back(l);
        }
    }
    if(lCount < 2) return 0;
    orderByPt(ind, _lPt, lCount);
    return lCount;
}

bool treeReader::jetIsClean_TOP16_020(const unsigned ind) const{
    TLorentzVector jet;
    jet.SetPtEtaPhiE(_jetPt[ind], _jetEta[ind], _jetPhi[ind], _jetE[ind]);
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood_TOP16_020(l)){
            TLorentzVector lep;
            lep.SetPtEtaPhiE(_lPt[l], _lEta[l], _lPhi[l], _lE[l]);
            if(lep.DeltaR(jet) <= 0.4) return false;
        }
    }
    return true;
}

bool treeReader::jetIsGood_TOP16_020(const unsigned ind, const unsigned ptCut, const unsigned unc, const bool clean) const{

    if( fabs(_jetEta[ind]) >= 4.5) return false;

    //only select loose jets:
    if(!_jetIsLoose[ind]) return false;

    switch(unc){
        case 0: if(_jetPt[ind] < ptCut) return false; break;
        case 1: if(_jetPt_JECDown[ind] < ptCut) return false; break;
        case 2: if(_jetPt_JECUp[ind] < ptCut) return false; break;
        case 3: if(_jetPt_JERDown[ind] < ptCut) return false; break;
        case 4: if(_jetPt_JERUp[ind] < ptCut) return false; break;
        default: ;
    }
    return !clean || jetIsClean_TOP16_020(ind);
}

unsigned treeReader::nJets_TOP16_020(std::vector<unsigned>& jetInd, const unsigned unc, const bool clean) const{
    unsigned nJets = 0;
    jetInd.clear();
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood_TOP16_020(j, 30, unc, clean)){
            ++nJets;
            jetInd.push_back(j);
        }
    }
    orderByPt(jetInd, _jetPt, nJets);
    return nJets;
}

unsigned treeReader::nBJets_TOP16_020(std::vector<unsigned>& bJetInd, const unsigned unc, const bool deepCSV, const bool clean, const unsigned wp) const{
    unsigned nbJets = 0;
    bJetInd.clear();
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood_TOP16_020(j, 30, unc, clean) && fabs(_jetEta[j]) < 2.4){
            if(bTagged(j, wp, deepCSV) ){
                ++nbJets;
                bJetInd.push_back(j);
            }
        }
    }
    orderByPt(bJetInd, _jetPt, nbJets);
    return nbJets;
}
