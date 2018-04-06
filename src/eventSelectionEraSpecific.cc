#include "../interface/treeReader.h"
#include "../interface/kinematicTools.h"

//lepton selection

//remove electrons overlapping with muons
bool eleIsCleanBase(const unsigned electronIndex, bool (&looseMuon)(const unsigned) ) const{
    //make sure this lepton is an electron
    if(_lFlavor[electronIndex] != 0){
        std::cerr << "Error: trying to clean non-electron object from muon overlap." << std::cerr;
        return 999;
    }
    //check separation with every muon
    for(unsigned m = 0; m < _nMu; ++m){
        if( looseMuon(m) ){
            if( kinematics::deltaR(_lPhi[m], _lEta[m], _lPhi[electronIndex], _lEta[electronIndex]) < 0.05 ){
                return false;
            }
        }
    }
    return true;
}

bool eleIsClean2016(const unsigned electronIndex) const{
    return eleIsCleanBase(const unsigned electronIndex, lepIsLoose2016);
}

bool eleIsClean2017(const unsigned electronIndex) const{
    return eleIsCleanBase(const unsigned electronIndex, lepIsLoose2017);
}

bool eleIsClean(const unsigned electronIndex) const{
    if( is2016() ){
        return eleIsClean2016(electronIndex);
    } else if( is2017() ){
        return eleIsClean2017(electronIndex);
    }
} 

bool lepIsLooseBase(const unsigned leptonIndex) const{
    if(_lFlavor[leptonIndex] == 2) return false; 
    if( ( _lFlavor[leptonIndex] == 0 ) && !eleIsClean(leptonIndex) ) return false;
	return _lEwkLoose[leptonIndex];
}

bool lepIsLoose2016(const unsigned leptonIndex) const{
	return lepIsLooseBase(leptonIndex);
}

bool lepIsLoose2017(const unsigned leptonIndex) const{
	return lepIsLooseBase(leptonIndex);
}

bool lepIsLoose(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsLoose2016(leptonIndex);
    } else if( is2017() ){
        return lepIsLoose2017(leptonIndex);
    }
}

bool lepIsGoodBase(const unsigned leptonIndex) const{
    if( !lepIsLoose(leptonIndex) ) return false;
    if(_lFlavor[leptonIndex] == 0 && !_lElectronPassEmu[leptonIndex]) return false;
    if(_lFlavor[leptonIndex] == 1 && !_lPOGMedium[leptonIndex]) return false;
    return true;
} 

bool lepIsGood2016(const unsigned leptonIndex) const{
    return lepIsGoodBase(leptonIndex);   
} 

bool lepIsGood2017(const unsigned leptonIndex) const{
    return lepIsGoodBase(leptonIndex);
}

bool lepIsGood(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsLoose2016(leptonIndex);
    } else {
        return lepIsLoose2017(leptonIndex);
    }
}

bool lepIsTightBase(const unsigned leptonIndex) const{
    return lepIsGood(leptonIndex);
}

bool lepIsTight2016(const unsigned leptonIndex) const{
    return lepIsTightBase(leptonIndex);
}

bool lepIsTight2017(const unsigned leptonIndex) const{
    return lepIsTightBase(leptonIndex);
}

bool lepIsTight(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsTight2016(leptonIndex);
    } else {
        return lepIsTight2017(leptonIndex);
    }
}

//jet selection

//jet cleaning depends on era-specific lepton selection!

//jet b-tagging
bool bTaggedDeepCSVBase(const unsigned jetIndex, const unsigned wp, const double bTagWP[3]) const{
	if(wp > 2){
		std::cerr << "Error: trying to evaluate deepCSV b-tagging WP that is out of range." << std::endl;
	}
	return ( (jetDeepCsv_b[jetIndex] + _jetDeepCsv_bb[jetIndex]) > bTagWP[wp] );
}

bool bTaggedDeepCSV2016(const unsigned jetIndex, const unsigned wp = 1) const{
	static const double bTagDeepCSVWP2016[3] = {0.2219, 0.6324,  0.8958}; 
	return bTaggedDeepCSVBase(jetIndex, wp, bTagDeepCSVWP2016);
}

bool bTaggedDeepCSV2017(const unsigned jetIndex, const unsigned wp = 1) const{
	static const double bTagDeepCSVWP2017[3] = {0.1522,  0.4941,  0.8001};
	return bTaggedDeepCSVBase(jetIndex, wp, bTagDeepCSVWP2017); 
}

bool bTaggedCSVv2Base(const unsigned ind, const unsigned wp, const double bTagWP[3]) const{
	if(wp > 2){
		std::cerr << "Error: trying to evaluate CSVv2 b-tagging WP that is out of range." << std::endl;
	}
	return ( _jetCsvV2[jetIndex] > bTagWP[wp] );
}

bool bTaggedCSVv22016(const unsigned ind, const unsigned wp) const{
	static const double bTagCSVv2WP2016[3] = {0.5426, 0.8484, 0.9535};
	return bTaggedCSVv2Base(ind, wp, bTagCSVv2WP2016);
} 

bool bTaggedCSVv22017(const unsigned ind, const unsigned wp) const{
	static const double bTagCSVv2WP2017[3] = {0.5803, 0.8838, 0.9693};
	return bTaggedCSVv2Base(ind, wp, bTagCSVv2WP2017);
}   
