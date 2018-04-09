#include "../interface/treeReader.h"

/*
* order objects by their pT
*/

void treeReader::orderByPt(std::vector<unsigned>& ind, const double* pt, const unsigned count) const{
    std::vector<std::pair<double, unsigned>> ptMap;
    for(unsigned p = 0; p < count; ++p){
        ptMap.push_back({pt[ind[p]], ind[p]});
    }
    std::sort(ptMap.begin(), ptMap.end(), [](std::pair<double, unsigned>& p1, std::pair<double, unsigned>& p2){ return p1.first > p2.first; } );
    for(unsigned p = 0; p < count; ++p){
        ind[p] = ptMap[p].second;
    }
}

unsigned treeReader::dilFlavorComb(const std::vector<unsigned>& ind) const{
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


/*
* lepton event selection
*/

//apply cone-correction to leptons
double treeReader::coneCorr(const unsigned ind) const{
    return 1. + std::max(_relIso[ind] - 0.1, 0.);
}

void treeReader::setConePt(){
    for(unsigned l = 0; l < _nLight; ++l){
        double coneC = coneCorr(l);
        _lPt[l] *= coneC;
        _lE[l] *= coneC;
    }
}

//select good leptons and order them by pT 
unsigned treeReader::selectLep(std::vector<unsigned>& ind){
    ind.clear();
    unsigned lCount = 0;
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l)){
            ++lCount;
            ind.push_back(l);
        }
    }
    //setConePt(); REMOVE CONE CORRECTION UNTIL MOVING TO FR
    if(lCount < 2) return 0;
    orderByPt(ind, _lPt, lCount);
    return lCount;	
}

//amount of consecutive tight leptons
unsigned treeReader::tightLepCount(const std::vector<unsigned>& ind, const unsigned lCount) const{
    unsigned tightC = 0; 
    for(unsigned l = 0; l < lCount; ++l){
        if(lepIsTight(ind[l])) ++tightC;
        else return tightC;
    }
    return tightC;
}

//lepton pT thresholds
bool treeReader::passPtCuts(const std::vector<unsigned>& ind) const{
    if(_lPt[ind[0]] <= 25) return false;
    if(_lPt[ind[1]] <= 15) return false;
    return true;
}

//check if leptons are prompt
bool treeReader::promptLeptons() const{

    //don't apply this check to data
    if( isData() ){
        return true;
    }
    
    //check whether every MC lepton is prompt
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l) && !_lIsPrompt[l]) return false;
    }
    return true;
}


/*
* number of jets 
*/

//select and count jets
unsigned treeReader::nJets(const unsigned unc, const bool clean) const{
    unsigned nJets = 0;
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 30, unc, clean)) ++nJets;
    }
    return nJets;
}

//select, count, and order jets by pT
unsigned treeReader::nJets(std::vector<unsigned>& jetInd, const unsigned unc, const bool clean) const{
    unsigned nJets = 0;
    jetInd.clear();
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 30, unc, clean)){
            ++nJets;
            jetInd.push_back(j);
        }
    }
    orderByPt(jetInd, _jetPt, nJets);
    return nJets;
}


/*
* number of b-tagged jets
*/

//select and count b-tagged jets
unsigned treeReader::nBJets(const unsigned unc, const bool deepCSV, const bool clean, const unsigned wp) const{
    unsigned nbJets = 0;
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 25, unc, clean)){
            if(bTagged(j, wp, deepCSV) ) ++nbJets;
        }
    }
    return nbJets;
}

//select, count, and order b-jets by pT
unsigned treeReader::nBJets(std::vector<unsigned>& bJetInd, const unsigned unc, const bool deepCSV, const bool clean, const unsigned wp) const{
    unsigned nbJets = 0;
    bJetInd.clear();
    for(unsigned j = 0; j < _nJets; ++j){
        if(jetIsGood(j, 25, unc, clean)){ 
            if(bTagged(j, wp, deepCSV) ){
                ++nbJets;
                bJetInd.push_back(j);
            }
        }
    }
    orderByPt(bJetInd, _jetPt, nbJets);
    return nbJets;
}


/*
* remove overlapping phase-space between different MC samples
*/

//check if lepton comes from matrix-element conversion
bool treeReader::lepFromMEExtConversion(const unsigned leptonIndex) const{
    bool fromConversion = (_lMatchPdgId[leptonIndex] == 22);
    bool promptConversion = (_lIsPrompt[leptonIndex] && _lProvenanceConversion[leptonIndex] == 0);
    return (fromConversion && promptConversion);
}

//reject events with overlapping photon-production phase-space
bool treeReader::photonOverlap(const Sample& samp) const{

    bool isInclusiveSample = (samp.getFileName().find("DYJetsToLL") != std::string::npos) || 
        (samp.getFileName().find("TTTo2L") != std::string::npos) || 
        (samp.getFileName().find("TTJets") != std::string::npos );

    bool isPhotonSample = (samp.getFileName().find("ZGTo2LG") != std::string::npos) ||
        (samp.getFileName().find("TTGJets") != std::string::npos) ||
        (samp.getFileName().find("WGToLNuG") != std::string::npos);

    //require inclusive sample to contain no external conversions
    if(isInclusiveSample){
       for(unsigned l = 0; l < _nLight; ++l){
            if(lepIsGood(l) && lepFromMEExtConversion(l) ){
                return true;
            }
        } 
    //require photon samples to have atlease one external conversion
    } if(isPhotonSample){
        bool hasConversion = false;
        for(unsigned l = 0; l < _nLight; ++l){
            if(lepIsGood(l) && lepFromMEExtConversion(l) ){
                hasConversion = true;
            }
        }
        return !(hasConversion);
    }
    return false;
}

//OLD function: consider deprecating or refactoring
/*
bool treeReader::photonOverlap() const{
    return photonOverlap(samples[currentSample - 1]);
}
*/

//overlap removal between inclusive and HT-binned samples
bool treeReader::htOverlap(const Sample& samp) const{
    if(samp.getFileName().find("DYJetsToLL_M-50_Tune") != std::string::npos){
        return _gen_HT > 70.;
    } else if(samp.getFileName().find("DYJetsToLL_M-10_50_Tune") != std::string::npos){
        return _gen_HT > 100.;
    }
    return false;
}

//OLD function: consider deprecating or refactoring
/*
bool treeReader::htOverlap() const{
    return htOverlap(samples[currentSample - 1]);
}
*/

/*
* trigger selection
*/

bool treeReader::passSingleLeptonTriggers() const{
    return (_pass_e || _pass_m);
}

bool treeReader::passDileptonTriggers() const{
    return (_pass_ee || _pass_em || _pass_mm);
}

bool treeReader::passTrileptonTriggers() const{
    return (_pass_eee || _pass_eem || _pass_emm || _pass_mmm); 
}

bool treeReader::passTriggerCocktail() const{
    bool pass = passSingleLeptonTriggers() ||
        passDileptonTriggers() ||
        passTrileptonTriggers();
    return pass;
}

/*
bool treeReader::lepIsLoose(const unsigned ind) const{
    return _lEwkLoose[ind];
}

bool treeReader::lepIsGood(const unsigned l) const{
    //temporary selection for dileptonCR, update later
    if(!_lEwkLoose[l]) return false;
    if(_lFlavor[l] == 0 && !_lElectronPassEmu[l]) return false;
    if(_lFlavor[l] == 1 && !_lPOGMedium[l]) return false;
    return true;
}

bool treeReader::lepIsTight(const unsigned l) const{
    return _lEwkTight[l];
}

//remove electrons in a cone of DeltaR = 0.05 around a loose muon
bool treeReader::eleIsClean(const unsigned ind) const{
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

bool treeReader::jetIsClean(const unsigned ind) const{
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

bool treeReader::jetIsGood(const unsigned ind, const unsigned ptCut, const unsigned unc, const bool clean) const{
    //only select loose jets:
    if(!_jetIsLoose[ind]) return false;

    if(fabs(_jetEta[ind]) >= 2.4) return false;

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

bool treeReader::bTaggedDeepCSV(const unsigned ind, const unsigned wp) const{
    static const double bTagWP[3] = {0.2219, 0.6324,  0.8958};
    return (_jetDeepCsv_b[ind] + _jetDeepCsv_bb[ind]) > bTagWP[wp];
}

bool treeReader::bTaggedCSVv2(const unsigned ind, const unsigned wp) const{
    static const double bTagWP[3] = {0.5426, 0.8484, 0.9535};
    return _jetCsvV2[ind] > bTagWP[wp];
}

bool treeReader::bTagged(const unsigned ind, const unsigned wp, const bool deepCSV) const{
    if(deepCSV) return bTaggedDeepCSV(ind, wp);
    else return bTaggedCSVv2(ind, wp);
}
*/
