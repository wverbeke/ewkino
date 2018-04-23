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

bool treeReader::lepIsLooseTTH(const unsigned leptonIndex) const{
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

bool treeReader::lepIsGoodTTH(const unsigned leptonIndex) const{
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

bool treeReader::lepIsTightTTH(const unsigned leptonIndex) const{
    //TO DO: make this era dependent
    if(_lFlavor[leptonIndex] == 2) return false;
    if(!lepIsGood(leptonIndex) ) return false;
    else if(_lFlavor[leptonIndex] == 1){
        if(!_lPOGMedium[leptonIndex]) return false;
    }
    return _leptonMvaTTH16[leptonIndex] > 0.9;
}




