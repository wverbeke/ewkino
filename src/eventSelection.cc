#include "../interface/treeReader.h"

unsigned treeReader::dilFlavorComb(const std::vector<unsigned>& ind){
    unsigned flavCount[3] = {0,0,0};
    for(unsigned l = 0; l < 2; ++l) ++flavCount[_lFlavor[ind[l]]];
    if(flavCount[2] == 0){
        if(flavCount[1] == 0) return 0; //ee
        else if(flavCount[1] == 1) return 1; //em
        else return 2; //mm
    } else if(flavCount[2] == 1){
        if(flavCount[1] == 0) return 3; //et
        else return 4; //mt
    }
    return 5; //tt
}

double treeReader::coneCorr(const unsigned ind){
    return 1. + std::max(_relIso[ind] - 0.1, 0.);
}

void treeReader::setConePt(){
    for(unsigned l = 0; l < _nLight; ++l){
        double coneC = coneCorr(l);
        _lPt[l] *= coneC;
        _lE[l] *= coneC;
    }
}

unsigned treeReader::selectLep(std::vector<unsigned>& ind){
    setConePt();
    unsigned lCount = 0;
    std::vector<std::pair<double, unsigned>> ptMap;
    for(unsigned l = 0; l < _nLight; ++l){
        if(_lEwkLoose[l]){
            if(_lFlavor[l] == 0 && !_lElectronPassEmu[l]) continue;
            ++lCount;
            ptMap.push_back({_lPt[l], l});
        }
    }
    if(lCount < 2) return 0;
    std::sort(ptMap.begin(), ptMap.end(), [](std::pair<double, unsigned>& p1, std::pair<double, unsigned>& p2){return p1.first > p2.first;} );
    for(unsigned l = 0; l < lCount; ++l){
        ind.push_back(ptMap[l].second);
    }
    return lCount;	
}

unsigned treeReader::tightLepCount(const std::vector<unsigned>& ind, const unsigned lCount){
    unsigned tightC = 0; 
    for(unsigned l = 0; l < lCount; ++l){
        if(_lEwkTight[ind[l]]) ++tightC;
        else return tightC;
    }
    return tightC;
}

bool treeReader::passPtCuts(const std::vector<unsigned>& ind){
    if(_lPt[ind[0]] < 25) return false;
    if(_lPt[ind[1]] < 20) return false;
    return true;
}

bool treeReader::jetIsClean(const unsigned ind){
    TLorentzVector jet;	
    jet.SetPtEtaPhiE(_jetPt[ind], _jetEta[ind], _jetPhi[ind], _jetE[ind]);
    for(unsigned l = 0; l < _nLight; ++l){
        if(_lEwkFO[l]){
            TLorentzVector lep;
            lep.SetPtEtaPhiE(_lPt[l], _lEta[l], _lPhi[l], _lE[l]);
            if(lep.DeltaR(jet) < 0.4) return false;
        }
    }
    return true;
}

bool treeReader::jetIsGood(const unsigned ind, const unsigned unc){
    if(fabs(_jetEta[ind]) < 2.4) return false;
    switch(unc){
        case 0: if(_jetPt[ind] < 30) return false;
        case 1: if(_jetPt_JECDown[ind] < 30) return false;
        case 2: if(_jetPt_JECUp[ind] < 30) return false;
        case 3: if(_jetPt_JERDown[ind] < 30) return false;
        case 4: if(_jetPt_JERUp[ind] < 30) return true;
        default: ;
    }
    return jetIsClean(ind);
}

unsigned treeReader::nJets(const unsigned unc){
    unsigned nJets = 0;
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, unc)) ++nJets;
    }
    return nJets;
}

bool treeReader::bTagged(const unsigned ind){
    static const unsigned bTagWP = 0.6324;
    return (_jetDeepCsv_b[ind] + _jetDeepCsv_bb[ind]) > bTagWP;
}

unsigned treeReader::nBJets(const unsigned unc){
    unsigned nbJets = 0;
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, unc)){
            if(bTagged(j)) ++nbJets;
        }
    }
    return nbJets;
}
