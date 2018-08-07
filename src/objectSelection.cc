#include "../interface/treeReader.h"
#include "../interface/kinematicTools.h"

//lepton selection


//remove electrons overlapping with muons
bool treeReader::eleIsCleanBase(const unsigned electronIndex, bool (treeReader::*looseMuon)(const unsigned) const) const{
    //make sure this lepton is an electron
    if( !isElectron(electronIndex) ){
        std::cerr << "Error: trying to clean non-electron object from muon overlap." << std::endl;
        return false;
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

double treeReader::closestJetDeepCSV(const unsigned leptonIndex) const{
    double closestJetDeepCSVVal = _closestJetDeepCsv_b[leptonIndex] + _closestJetDeepCsv_bb[leptonIndex];
    bool isNan = std::isnan(closestJetDeepCSVVal);
    if( isNan ){
        return 0.;
    } else {
        return std::max(closestJetDeepCSVVal, 0.);
    }
}

bool treeReader::lepIsLooseBase(const unsigned leptonIndex) const{
    if( isTau(leptonIndex) ) return false;
    if(_lPt[leptonIndex] <= 10) return false;
    if(fabs(_lEta[leptonIndex]) >= (2.5 - 0.1*isMuon(leptonIndex) ) ) return false;
    if(fabs(_dxy[leptonIndex]) >= 0.05) return false;
    if(fabs(_dz[leptonIndex]) >= 0.1) return false;
    if(_3dIPSig[leptonIndex] >= 8) return false;
    if(_miniIso[leptonIndex] >= 0.4) return false;
    if( isMuon(leptonIndex) ){
        if(!_lPOGLoose[leptonIndex]) return false;
    } else if( isElectron(leptonIndex) ){
        if(_lElectronMissingHits[leptonIndex] > 1) return false;
        if(!_lElectronPassEmu[leptonIndex]) return false;
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

bool treeReader::passLeptonMva2016(const unsigned leptonIndex, const double mvaCut) const{
    return (_leptonMvatZqTTV16[leptonIndex] > mvaCut);
}

bool treeReader::passLeptonMva2017(const unsigned leptonIndex, const double mvaCut) const{
    return (_leptonMvatZqTTV17[leptonIndex] > mvaCut);
}

bool treeReader::passLeptonMva(const unsigned leptonIndex, const double mvaCut) const{
    if( is2016() ){
        return passLeptonMva2016(leptonIndex, mvaCut);
    } else {
        return passLeptonMva2017(leptonIndex, mvaCut);
    }
}

bool treeReader::lepIsGoodBase(const unsigned leptonIndex) const{
    if(!lepIsLoose(leptonIndex)) return false;
    if(_lPt[leptonIndex] <= 10) return false;
    if( isMuon(leptonIndex) ){
        if( !_lPOGMedium[leptonIndex] ) return false;
    }
    return true;
}

bool treeReader::lepIsGood2016(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;
    if( closestJetDeepCSV(leptonIndex) >  0.8958) return false;
    if( !passLeptonMva( leptonIndex, 0.8) ){
        if( _ptRatio[leptonIndex] <= 0.6) return false;         //used to be 0.7 with old FR tune
        if( closestJetDeepCSV( leptonIndex ) >= 0.3) return false; //used to be 0.4 with old FR tune
        if( isElectron(leptonIndex) ){
            if( _lElectronMva[leptonIndex] <= ( ( fabs(_lEta[leptonIndex]) < 1.479 ) ? 0.4 : 0.7 ) ) return false; //used to be 0.4 : 0.9 with old FR tune
        }
    }
    return true;
} 

bool treeReader::lepIsGood2017(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;
    if( closestJetDeepCSV(leptonIndex) >  0.8001) return false;
    if( !passLeptonMva( leptonIndex, 0.8) ){
        if( _ptRatio[leptonIndex] <= 0.6) return false;
        if( closestJetDeepCSV( leptonIndex ) >= 0.4) return false;  //used to be 0.4 in old FR maps
        if( isElectron(leptonIndex) ){
            if( _lElectronMvaFall17NoIso[leptonIndex] <= ( ( fabs(_lEta[leptonIndex]) < 1.479 ) ? 0.3 : 0.6 ) ) return false;
        } 
    }
    return true;   
}

bool treeReader::lepIsGood(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsGood2016(leptonIndex);
    } else {
        return lepIsGood2017(leptonIndex);
    }
}

bool treeReader::lepIsTightBase(const unsigned leptonIndex) const{
    if(!lepIsGood(leptonIndex) ) return false;
    if( isMuon(leptonIndex) ){
        if(!_lPOGMedium[leptonIndex]) return false;
    }
    return passLeptonMva(leptonIndex, 0.8); 
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
/*
 * The general version of this function was specifically declared to facilitate an easier b-tagging efficiency determination for a number of different lepton ID's (ttW, ttZ)
 */

bool treeReader::jetIsCleanBase(const unsigned jetIndex, bool (treeReader::*leptonIsFO)(const unsigned) const) const{
    for(unsigned l = 0; l < _nLight; ++l){
        if( (this->*leptonIsFO)(l)){
            double deltaR = kinematics::deltaR(_lPhi[l], _lEta[l], _jetPhi[jetIndex], _jetEta[jetIndex]);
            if(deltaR < 0.4){
                return false;
            }
        }
    }
    return true;
}

bool treeReader::jetIsClean(const unsigned jetIndex) const{
    return jetIsCleanBase(jetIndex, &treeReader::lepIsGood);
}


bool treeReader::jetIsGood(const unsigned jetIndex, const double ptCut, const unsigned unc, const bool clean, const bool allowForward) const{

    //only select loose jets in 2016 and tight jets in 2017
    if( is2016() ){
        if( !_jetIsLoose[jetIndex] ) return false;
    } else {
        if( !_jetIsTight[jetIndex] ) return false;
    }

    //only select jets in tracker volume
    if( (!allowForward) && ( fabs(_jetEta[jetIndex]) >= 2.4 ) ) return false;

    //pT cut that can be modified for certain eta regions
    double ptThreshold = ptCut;

    //put harsh pT cuts  on jetc in problematic region for 2017
    if( is2017() ){
        if( fabs(_jetEta[jetIndex]) > 2.7 && fabs(_jetEta[jetIndex]) < 3 ){
            ptThreshold = 60;
        }
    }   

    //apply jet pT cuts
    switch(unc){
        case 0: if(_jetPt[jetIndex] < ptThreshold) return false; break;
        case 1: if(_jetPt_JECDown[jetIndex] < ptThreshold) return false; break;
        case 2: if(_jetPt_JECUp[jetIndex] < ptThreshold) return false; break;
        default: {
                     std::cerr << "Error: uncertainty case larger than 2 given to jetIsGood, option not recognized" << std::endl;
                     return false; 
                 }
    }
    return !clean || jetIsClean(jetIndex);
}


//jet b-tagging
double treeReader::deepCSV(const unsigned jetIndex) const{
    return _jetDeepCsv_b[jetIndex] + _jetDeepCsv_bb[jetIndex];
}

bool treeReader::bTaggedDeepCSVBase(const unsigned jetIndex, const unsigned wp, const double bTagWP[3]) const{
    if(wp > 2){
        std::cerr << "Error: trying to evaluate deepCSV b-tagging WP that is out of range." << std::endl;
    }
    return ( deepCSV(jetIndex) > bTagWP[wp] );
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
