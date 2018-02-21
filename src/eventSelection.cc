#include "../interface/treeReader.h"


void treeReader::orderByPt(std::vector<unsigned>& ind, const double* pt, const unsigned count){
    std::vector<std::pair<double, unsigned>> ptMap;
    for(unsigned p = 0; p < count; ++p){
        ptMap.push_back({pt[ind[p]], ind[p]});
    }
    std::sort(ptMap.begin(), ptMap.end(), [](std::pair<double, unsigned>& p1, std::pair<double, unsigned>& p2){return p1.first > p2.first;} );
    for(unsigned p = 0; p < count; ++p){
        ind[p] = ptMap[p].second;
    }
}

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
    //return 1. + std::max(_relIso[ind] - 0.1, 0.);
    double corr = 1.;
    if(!lepIsTight(ind)){
        corr *= 0.9/_ptRatio[ind];
    }
    return corr;
}

void treeReader::setConePt(){
    for(unsigned l = 0; l < _nLight; ++l){
        double coneC = coneCorr(l);
        _lPt[l] *= coneC;
        _lE[l] *= coneC;
    }
}

bool treeReader::elePassVLooseMvaIDSUSY(const unsigned ind){
    if(!_lFlavor[ind] != 0) return true;
    static const double gpCuts[3][2] = { {-0.48,-0.85}, {-0.67, -0.91}, {-0.49, -0.83} };
    static const double hzzCuts[3] = {0.46, -0.03, 0.06};
    unsigned eta = (fabs(_lEta[ind]) >= 0.8) + (fabs(_lEta[ind]) > 1.479);
    if(_lPt[ind] > 10){
        return _lElectronMva[ind] > std::min( gpCuts[eta][0], std::max(gpCuts[eta][1], gpCuts[eta][0] + (gpCuts[eta][1] - gpCuts[eta][0])*0.1*(_lPt[ind] - 15.) ) );
    } else{
        return _lElectronMvaHZZ[ind] > hzzCuts[eta];
    }
}    

bool treeReader::lepIsLoose(const unsigned ind){
    if(_lFlavor[ind] == 2) return false;  //don't consider taus here
    if(_lPt[ind] <= 7 - 2*_lFlavor[ind]) return false;
    if(fabs(_lEta[ind]) >= (2.5 - 0.1*_lFlavor[ind])) return false;
    if(fabs(_dxy[ind]) >= 0.05) return false;
    if(fabs(_dz[ind]) >= 0.1) return false;
    if(_3dIPSig[ind] >= 8) return false;
    if(_miniIso[ind] >= 0.4) return false;
    if(_lFlavor[ind] == 1){
        if(!_lPOGLoose[ind]) return false;
    } else if(_lFlavor[ind] == 0){
        if(_lElectronMissingHits[ind] > 1) return false;
        if(!elePassVLooseMvaIDSUSY(ind)) return false;
        if(!eleIsClean(ind)) return false;
    }
    return true;
}

//remove electrons in a cone of DeltaR = 0.05 around a loose muon
bool treeReader::eleIsClean(const unsigned ind){
    TLorentzVector ele;
    ele.SetPtEtaPhiE(_lPt[ind], _lEta[ind], _lPhi[ind], _lE[ind]);
    for(unsigned m = 0; m < _nMu; ++m){
        if(lepIsLoose(m)){
            TLorentzVector mu;
            mu.SetPtEtaPhiE(_lPt[m], _lEta[m], _lPhi[m], _lE[m]);
            if(ele.DeltaR(mu) < 0.05) return false;
        }
    }
    return true;
}

bool treeReader::lepIsGood(const unsigned l){
    //ttH FO definition
    if(!lepIsLoose(l)) return false;
    if(_lPt[l] <= 15) return false;
    if(_closestJetCsvV2[l] >= 0.8484) return false;
    if(_leptonMvaTTH[l] <= 0.9){
        if(_ptRatio[l] <= 0.5) return false;
        if(_closestJetCsvV2[l] >= 0.3) return false;
        if(_lFlavor[l] == 1 && _lMuonSegComp[l] <= 0.3) return false; 
        if(_lFlavor[l] == 0 && _lElectronMvaHZZ[l] <= 0.0 + (fabs(_lEta[l]) >= 1.479)*0.7) return false;
    }
    if(_lFlavor[l] == 0){
        if(!_lElectronPassEmu[l]) return false;
    }
    return true;
}

bool treeReader::lepIsTight(const unsigned l){
    if(_lFlavor[l] == 2) return false;
    else if(_lFlavor[l] == 1){
        if(!_lPOGMedium[l]) return false;
    } 
    return _leptonMvaTTH[l] > 0.9;
}

unsigned treeReader::selectLep(std::vector<unsigned>& ind){
    ind.clear();
    unsigned lCount = 0;
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l)){
            ++lCount;
            ind.push_back(l);
        }
    }
    if(lCount < 2) return 0;

    //set cone pt's after baseline object selection
    setConePt();

    //order particles by pT
    orderByPt(ind, _lPt, lCount);
    return lCount;	
}

unsigned treeReader::tightLepCount(const std::vector<unsigned>& ind, const unsigned lCount){
    unsigned tightC = 0; 
    for(unsigned l = 0; l < lCount; ++l){
        if(lepIsTight(ind[l])) ++tightC;
        else return tightC;
    }
    return tightC;
}

bool treeReader::lepIsVeto_TOP16_020(const unsigned l){
    if(_lPt[l] <= 10) return false;
    if(fabs(_lEta[l]) >= (2.5 - 0.1*_lFlavor[l])) return false;
    if(_lFlavor[l] == 0){
        return _lPOGVeto[l];
    } else if(_lFlavor[l] == 1){
        if(!_lPOGLoose[l]) return false;
        return _relIso0p4Mu[l] < 0.25;
    }
    return false;
}

bool treeReader::lepIsGood_TOP16_020(const unsigned l){
    if(_lPt[l] <= 25) return false;
    if(fabs(_lEta[l]) > (2.5 - 0.1*_lFlavor[l]) ) return false;
    if(!_lPOGTight[l]) return false;
    if(_lFlavor[l] == 1 && _relIso0p4Mu[l] > 0.15) return false;
    return true;
}

bool treeReader::lepIsTight_TOP16_020(const unsigned l){
    return lepIsGood_TOP16_020(l);
}

unsigned treeReader::selectLep_TOP16_020(std::vector<unsigned>& ind){
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

bool treeReader::passPtCuts(const std::vector<unsigned>& ind){
    if(_lPt[ind[0]] <= 25) return false;
    if(_lPt[ind[1]] <= 15) return false;
    if(_lPt[ind[0]] <= 10) return false;
    return true;
}

bool treeReader::jetIsClean(const unsigned ind){
    TLorentzVector jet;	
    jet.SetPtEtaPhiE(_jetPt[ind], _jetEta[ind], _jetPhi[ind], _jetE[ind]);
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l)){
            TLorentzVector lep;
            lep.SetPtEtaPhiE(_lPt[l], _lEta[l], _lPhi[l], _lE[l]);
            if(lep.DeltaR(jet) <= 0.4) return false;
        }
    }
    return true;
}

bool treeReader::jetIsGood(const unsigned ind, const unsigned ptCut, const unsigned unc, const bool clean){
    //No eta cut applied for jets in this analysis!

    //only select loose jets:
    //0: no id, 1 : loose id, 2 : tight id
    if(_jetId[ind] < 1 ) return false;

    switch(unc){
        case 0: if(_jetPt[ind] < ptCut) return false; break;
        case 1: if(_jetPt_JECDown[ind] < ptCut) return false; break;
        case 2: if(_jetPt_JECUp[ind] < ptCut) return false; break;
        case 3: if(_jetPt_JERDown[ind] < ptCut) return false; break;
        case 4: if(_jetPt_JERUp[ind] < ptCut) return false; break;
        default: ;
    }
    return !clean || jetIsClean(ind);
}

unsigned treeReader::nJets(const unsigned unc, const bool clean){
    unsigned nJets = 0;
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 25, unc, clean)) ++nJets;
    }
    return nJets;
}

unsigned treeReader::nJets(std::vector<unsigned>& jetInd, const unsigned unc, const bool clean){
    unsigned nJets = 0;
    jetInd.clear();
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 25, unc, clean)){
            ++nJets;
            jetInd.push_back(j);
        }
    }
    orderByPt(jetInd, _jetPt, nJets);
    return nJets;
}

bool treeReader::bTaggedDeepCSV(const unsigned ind, const unsigned wp){
    static const double bTagWP[3] = {0.2219, 0.6324,  0.8958};
    return (_jetDeepCsv_b[ind] + _jetDeepCsv_bb[ind]) > bTagWP[wp];
}

bool treeReader::bTaggedCSVv2(const unsigned ind, const unsigned wp){
    static const double bTagWP[3] = {0.5426, 0.8484, 0.9535};
    return _jetCsvV2[ind] > bTagWP[wp];
}

bool treeReader::bTagged(const unsigned ind, const unsigned wp, const bool deepCSV){
    if(deepCSV) return bTaggedDeepCSV(ind, wp);
    else        return bTaggedCSVv2(ind, wp);
}

unsigned treeReader::nBJets(const unsigned unc, const bool deepCSV, const bool clean, const unsigned wp){
    unsigned nbJets = 0;
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 25, unc, clean) && fabs(_jetEta[j]) < 2.4){
            if(bTagged(j, wp, deepCSV) ) ++nbJets;
        }
    }
    return nbJets;
}

unsigned treeReader::nBJets(std::vector<unsigned>& bJetInd, const unsigned unc, const bool deepCSV, const bool clean, const unsigned wp){
    unsigned nbJets = 0;
    bJetInd.clear();
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 25, unc, clean) && fabs(_jetEta[j]) < 2.4){ 
            if(bTagged(j, wp, deepCSV) ){
                ++nbJets;
                bJetInd.push_back(j);
            }
        }
    }
    orderByPt(bJetInd, _jetPt, nbJets);
    return nbJets;
}

//check if leptons are prompt
bool treeReader::promptLeptons(){
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l) && !_lIsPrompt[l]) return false;
    }
    return true;
}

//overlap removal between events

bool treeReader::photonOverlap(const Sample& samp){
    /*
    if(samp.getFileName().find("DYJetsToLL") != std::string::npos){
        return _zgEventType > 2; 
    } else if(samp.getFileName().find("TTTo2L") != std::string::npos || samp.getFileName().find("TTJets") != std::string::npos ){
        return _ttgEventType > 2;
    }
    return false;
    */
    if( (samp.getFileName().find("DYJetsToLL") != std::string::npos ) || (samp.getFileName().find("TTJets") != std::string::npos ) ){
        for(unsigned l = 0; l < _nLight; ++l){
            if(lepIsGood(l) && _lMatchPdgId[l] == 22){
                return true;
            }
        }
    } else if( ( samp.getFileName().find("ZGTo2LG") != std::string::npos ) || ( samp.getFileName().find("TTGJets") != std::string::npos ) || ( samp.getFileName().find("TGJets") != std::string::npos )
            || ( samp.getFileName().find("WGToLNuG") != std::string::npos) ){
        bool ret = true;
        for(unsigned l = 0; l < _nLight; ++l){
            if(lepIsGood(l) && _lMatchPdgId[l] == 22){
                ret = false;
            }
        }
        return ret;
    }
    return false;
}

bool treeReader::photonOverlap(){
    return photonOverlap(samples[currentSample - 1]);
}

bool treeReader::htOverlap(const Sample& samp){
    if(samp.getFileName().find("DYJetsToLL_M-50_Tune") != std::string::npos){
        return _gen_HT > 70.;
    } else if(samp.getFileName().find("DYJetsToLL_M-10_50_Tune") != std::string::npos){
        return _gen_HT > 100.;
    }
    return false;
}

bool treeReader::htOverlap(){
    return htOverlap(samples[currentSample - 1]);
}
