#include "../interface/treeReader.h"
#include "../interface/kinematicTools.h"

//lepton selection


//remove electrons overlapping with muons
bool treeReader::eleIsCleanBase(const unsigned electronIndex, bool (treeReader::*looseMuon)(const unsigned) const) const{
    //make sure this lepton is an electron
    if( !isElectron(electronIndex) ){
        std::cerr << "Error: trying to clean non-electron object from muon overlap." << std::endl;
        return 999;
    }
    //check separation with every muon
    for(unsigned m = 0; m < _nMu; ++m){
        if( !isMuon(m) ){
            std::cerr << "Error trying to clean electron from non-muon object" << std::endl;
        }
        if( ( this->*looseMuon )(m) ){
            if( kinematics::deltaR(_lPhi[m], _lEta[m], _lPhi[electronIndex], _lEta[electronIndex]) < 0.05 ){
                return false;
            }
        }
    }
    return true;
}

bool treeReader::eleIsClean2016(const unsigned electronIndex) const{
    return eleIsCleanBase(electronIndex, &treeReader::lepIsLoose2016);
}

bool treeReader::eleIsClean2017(const unsigned electronIndex) const{
    return eleIsCleanBase(electronIndex, &treeReader::lepIsLoose2017);
}

bool treeReader::eleIsClean(const unsigned electronIndex) const{
    if( is2016() ){
        return eleIsClean2016(electronIndex);
    } else {
        return eleIsClean2017(electronIndex);
    }
} 

bool treeReader::elePassVLooseMvaIDSUSY(const unsigned ind) const{
    //To DO: this function should no longer be used in the future, and replaced by trigger emulation
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


bool treeReader::lepIsLooseBase(const unsigned leptonIndex) const{
    if( isTau(leptonIndex) ) return false;
    if(_lPt[leptonIndex] <= 7 - 2*_lFlavor[leptonIndex]) return false;
    if(fabs(_lEta[leptonIndex]) >= (2.5 - 0.1*_lFlavor[leptonIndex])) return false;
    if(fabs(_dxy[leptonIndex]) >= 0.05) return false;
    if(fabs(_dz[leptonIndex]) >= 0.1) return false;
    if(_3dIPSig[leptonIndex] >= 8) return false;
    if(_miniIso[leptonIndex] >= 0.4) return false;
    if(_lFlavor[leptonIndex] == 1){
        if(!_lPOGLoose[leptonIndex]) return false;
    } else if(_lFlavor[leptonIndex] == 0){
        if(_lElectronMissingHits[leptonIndex] > 1) return false;
        if(!elePassVLooseMvaIDSUSY(leptonIndex)) return false;
        if(!eleIsClean(leptonIndex)) return false;
    }
    return true;
}

bool treeReader::lepIsLoose2016(const unsigned leptonIndex) const{
	return lepIsLooseBase(leptonIndex);
}

bool treeReader::lepIsLoose2017(const unsigned leptonIndex) const{
	return lepIsLooseBase(leptonIndex);
}

bool treeReader::lepIsLoose(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsLoose2016(leptonIndex);
    } else {
        return lepIsLoose2017(leptonIndex);
    }
}

bool treeReader::lepIsGoodBase(const unsigned leptonIndex) const{
    //ttH FO definition
    //TO-DO make this selection era-specifid
    if(!lepIsLoose(leptonIndex)) return false;
    if(_lPt[leptonIndex] <= 15) return false;
    if(_closestJetCsvV2[leptonIndex] >= 0.8484) return false;
    if(_leptonMvaTTH16[leptonIndex] <= 0.9){
        if(_ptRatio[leptonIndex] <= 0.5) return false;
        if(_closestJetCsvV2[leptonIndex] >= 0.3) return false;
        if(_lFlavor[leptonIndex] == 1 && _lMuonSegComp[leptonIndex] <= 0.3) return false;
        if(_lFlavor[leptonIndex] == 0 && _lElectronMvaHZZ[leptonIndex] <= 0.0 + (fabs(_lEta[leptonIndex]) >= 1.479)*0.7) return false;
    }
    if(_lFlavor[leptonIndex] == 0){
        if(!_lElectronPassEmu[leptonIndex]) return false;
    }
    return true;
} 

bool treeReader::lepIsGood2016(const unsigned leptonIndex) const{
    return lepIsGoodBase(leptonIndex);   
} 

bool treeReader::lepIsGood2017(const unsigned leptonIndex) const{
    return lepIsGoodBase(leptonIndex);
}

bool treeReader::lepIsGood(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsLoose2016(leptonIndex);
    } else {
        return lepIsLoose2017(leptonIndex);
    }
}

bool treeReader::lepIsTightBase(const unsigned leptonIndex) const{
    //TO DO: make this era dependent
    if(_lFlavor[leptonIndex] == 2) return false;
    if(!lepIsGood(leptonIndex) ) return false;
    else if(_lFlavor[leptonIndex] == 1){
        if(!_lPOGMedium[leptonIndex]) return false;
    }
    return _leptonMvaTTH16[leptonIndex] > 0.9;
}

bool treeReader::lepIsTight2016(const unsigned leptonIndex) const{
    return lepIsTightBase(leptonIndex);
}

bool treeReader::lepIsTight2017(const unsigned leptonIndex) const{
    return lepIsTightBase(leptonIndex);
}

bool treeReader::lepIsTight(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsTight2016(leptonIndex);
    } else {
        return lepIsTight2017(leptonIndex);
    }
}


//jet selection
//note that jet cleaning depends on era-specific lepton selection, but this is automatically propagated through the "lepIsGood" function
bool treeReader::jetIsClean(const unsigned jetIndex) const{
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l)){
            double deltaR = kinematics::deltaR(_lPhi[l], _lEta[l], _jetPhi[jetIndex], _jetEta[jetIndex]);
            if(deltaR < 0.4){
                return false;
            }
        }
    }
    return true;
}

bool treeReader::jetIsGood(const unsigned jetIndex, const unsigned ptCut, const unsigned unc, const bool clean, const bool allowForward) const{
    //only select loose jets:
    if(!_jetIsLoose[jetIndex]) return false;
    //only select jets in tracker volume
    if( (!allowForward) && ( fabs(_jetEta[jetIndex]) >= 2.4 ) ) return false;
    //apply jet pT cuts
    switch(unc){
        case 0: if(_jetPt[jetIndex] < ptCut) return false; break;
        case 1: if(_jetPt_JECDown[jetIndex] < ptCut) return false; break;
        case 2: if(_jetPt_JECUp[jetIndex] < ptCut) return false; break;
        case 3: if(_jetPt_JERDown[jetIndex] < ptCut) return false; break;
        case 4: if(_jetPt_JERUp[jetIndex] < ptCut) return false; break;
        default: ;
    }
    return !clean || jetIsClean(jetIndex);
}


//jet b-tagging
bool treeReader::bTaggedDeepCSVBase(const unsigned jetIndex, const unsigned wp, const double bTagWP[3]) const{
	if(wp > 2){
		std::cerr << "Error: trying to evaluate deepCSV b-tagging WP that is out of range." << std::endl;
	}
	return ( (_jetDeepCsv_b[jetIndex] + _jetDeepCsv_bb[jetIndex]) > bTagWP[wp] );
}

bool treeReader::bTaggedDeepCSV2016(const unsigned jetIndex, const unsigned wp) const{
	static const double bTagDeepCSVWP2016[3] = {0.2219, 0.6324,  0.8958}; 
	return bTaggedDeepCSVBase(jetIndex, wp, bTagDeepCSVWP2016);
}

bool treeReader::bTaggedDeepCSV2017(const unsigned jetIndex, const unsigned wp) const{
	static const double bTagDeepCSVWP2017[3] = {0.1522,  0.4941,  0.8001};
	return bTaggedDeepCSVBase(jetIndex, wp, bTagDeepCSVWP2017); 
}

bool treeReader::bTaggedDeepCSV(const unsigned jetIndex, const unsigned wp) const{
    if( is2016() ){
        return bTaggedDeepCSV2016(jetIndex, wp);
    } else {
        return bTaggedDeepCSV2017(jetIndex, wp);
    }
}

bool treeReader::bTaggedCSVv2Base(const unsigned jetIndex, const unsigned wp, const double bTagWP[3]) const{
	if(wp > 2){
		std::cerr << "Error: trying to evaluate CSVv2 b-tagging WP that is out of range." << std::endl;
	}
	return ( _jetCsvV2[jetIndex] > bTagWP[wp] );
}

bool treeReader::bTaggedCSVv22016(const unsigned jetIndex, const unsigned wp) const{
	static const double bTagCSVv2WP2016[3] = {0.5426, 0.8484, 0.9535};
	return bTaggedCSVv2Base(jetIndex, wp, bTagCSVv2WP2016);
} 

bool treeReader::bTaggedCSVv22017(const unsigned jetIndex, const unsigned wp) const{
	static const double bTagCSVv2WP2017[3] = {0.5803, 0.8838, 0.9693};
	return bTaggedCSVv2Base(jetIndex, wp, bTagCSVv2WP2017);
}   

bool treeReader::bTaggedCSVv2(const unsigned jetIndex, const unsigned wp) const{
    if( is2016() ){
        return bTaggedCSVv22016(jetIndex, wp);
    } else {
        return bTaggedCSVv22017(jetIndex, wp);
    }
}

bool treeReader::bTagged(const unsigned ind, const unsigned wp, const bool deepCSV) const{
    if(deepCSV){
        return bTaggedDeepCSV(ind, wp);
    } else {
        return bTaggedCSVv2(ind, wp);
    }
}
