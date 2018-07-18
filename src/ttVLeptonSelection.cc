#include "../interface/treeReader.h"
#include "../interface/kinematicTools.h"

/*
 * Implementation of lepton selection from the different analyses from tZq and ttV
 */

bool treeReader::lepIsGoodtZq(const unsigned leptonIndex) const{
    return lepIsGood(leptonIndex);
}

bool treeReader::lepIsGoodttZ3l2016(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;
    if( closestJetDeepCSV(leptonIndex) >  0.8958) return false;
    if( !passLeptonMva( leptonIndex, 0.4) ){
        if( _ptRatio[leptonIndex] <= 0.4) return false;
        if( closestJetDeepCSV( leptonIndex ) >= 0.4) return false;
        if( isElectron(leptonIndex) ){
            if( _lElectronMva[leptonIndex] <= ( ( fabs(_lEta[leptonIndex]) < 1.479 ) ? -0.1 : 0.7 ) ) return false;
        }
    }
    return true;
}

bool treeReader::lepIsGoodttZ3l2017(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;
    if( closestJetDeepCSV(leptonIndex) >  0.8001) return false;
    if( !passLeptonMva( leptonIndex, 0.4) ){
        if( _ptRatio[leptonIndex] <= 0.3) return false;
        if( closestJetDeepCSV( leptonIndex ) >= 0.2) return false;
        if( isElectron(leptonIndex) ){
            if( _lElectronMva[leptonIndex] <= ( ( fabs(_lEta[leptonIndex]) < 1.479 ) ? 0.0 : 0.3 ) ) return false;
        }
    }
    return true;
}

bool treeReader::lepIsGoodttZ3l(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsGoodttZ3l2016(leptonIndex);
    } else {
        return lepIsGoodttZ3l2017(leptonIndex);
    }
}

bool treeReader::lepIsGoodttZ4l2016(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;
    return true;

}

bool treeReader::lepIsGoodttZ4l2017(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;
    return true;
}

bool treeReader::lepIsGoodttZ4l(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsGoodttZ4l2016(leptonIndex);
    } else {
        return lepIsGoodttZ4l2017(leptonIndex);
    }
}

bool treeReader::lepIsGoodttW2016(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;
    if( closestJetDeepCSV(leptonIndex) >  0.8958) return false;
    if( !passLeptonMva( leptonIndex, 0.6) ){
        if( _ptRatio[leptonIndex] <= 0.5) return false;
        if( closestJetDeepCSV( leptonIndex ) >= 0.3) return false;
        if( isElectron(leptonIndex) ){
            if( _lElectronMva[leptonIndex] <= ( ( fabs(_lEta[leptonIndex]) < 1.479 ) ? 0.3 : 0.8 ) ) return false;
        }
    }
    return true;
}

bool treeReader::lepIsGoodttW2017(const unsigned leptonIndex) const{
    if( !lepIsGoodBase(leptonIndex) ) return false;
    if( closestJetDeepCSV(leptonIndex) >  0.8001) return false;
    if( !passLeptonMva( leptonIndex, 0.6) ){
        if( _ptRatio[leptonIndex] <= 0.4) return false;
        if( closestJetDeepCSV( leptonIndex ) >= 0.4) return false;
        if( isElectron(leptonIndex) ){
            if( _lElectronMva[leptonIndex] <= ( ( fabs(_lEta[leptonIndex]) < 1.479 ) ? 0.4 : 0.8 ) ) return false;
        }
    }
    return true;
}

bool treeReader::lepIsGoodttW(const unsigned leptonIndex) const{
    if( is2016() ){
        return lepIsGoodttW2016(leptonIndex);
    } else {
        return lepIsGoodttW2017(leptonIndex);
    }
}

bool treeReader::lepIsGoodMultiAnalysis(const std::string& analysis, const unsigned leptonIndex) const{
    if(analysis == "tZq"){
        return lepIsGoodtZq(leptonIndex);
    } else if(analysis == "ttZ3l"){
        return lepIsGoodttZ3l(leptonIndex);
    } else if(analysis == "ttZ4l"){
        return lepIsGoodttZ4l(leptonIndex);
    } else if(analysis == "ttW"){
        return lepIsGoodttW(leptonIndex);
    } else {
        std::cerr << "Error: unrecognized analysis option in lepton selection" << std::endl;
        return false;
    }
}

