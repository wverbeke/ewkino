//include c++ library classes
#include <iostream>

//include other parts of code
#include "../interface/treeReader.h"

//pu SF 
inline double treeReader::puWeight(const unsigned period, const unsigned unc) const{
    return reweighter->puWeight(_nTrueInt, period, unc);
}

//b-tagging SF for given flavor
double treeReader::bTagWeight(const unsigned jetFlavor, const unsigned unc) const{
    double pMC = 1.;
    double pData = 1.;
    for(unsigned j = 0; j < _nJets; ++j){
        if(_jetHadronFlavor[j] == jetFlavor){
            //QUESTION: should JEC and b-tag weights also be varied up and down at the same time when computing systematics?
            if(jetIsGood(j, 25., 0, true) && fabs(_jetEta[j]) < 2.4){
                double sf = reweighter->bTagWeight(_jetHadronFlavor[j], _jetPt[j], _jetEta[j], _jetDeepCsv_b[j] + _jetDeepCsv_bb[j], unc);
                double eff = reweighter->bTagEff(_jetHadronFlavor[j], _jetPt[j], _jetEta[j]);
                if(bTagged(j, 1, true)){
                    pMC *= eff;
                    pData *= eff*sf;
                } else{
                    pMC *= (1 - eff);
                    pData *= (1 - eff*sf);
                }
            }
        }
    }
    return pData/pMC;
}

//light flavor b-tagging SF
double treeReader::bTagWeight_udsg(const unsigned unc) const{
    return bTagWeight(0, unc);
}

//heavy flavor b-tagging SF
double treeReader::bTagWeight_c(const unsigned unc) const{
    return bTagWeight(4, unc);
}

//beauty flavor b-tagging SF
double treeReader::bTagWeight_b(const unsigned unc) const{
    return bTagWeight(5, unc);
}

//total b-tagging SF
double treeReader::bTagWeight(const unsigned unc) const{
    return bTagWeight_udsg(unc)*bTagWeight_c(unc)*bTagWeight_b(unc); 
}

//total lepton SF
double treeReader::leptonWeight() const{
    double sf = 1.;
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsTight(l)){
            if(_lFlavor[l] == 1) sf*= reweighter->muonWeight(_lPt[l], _lEta[l]);
            else if(_lFlavor[l] == 0) sf *= reweighter->electronWeight(_lPt[l], _lEta[l], _lEtaSC[l]);
        }        
    }
    return sf;
}
    
double treeReader::sfWeight(){
    if(reweighter.use_count() == 0 ){
        //reweighter = std::make_shared<Reweighter>();
        reweighter.reset(new Reweighter); 
    }
    double sf = puWeight();
    sf *= bTagWeight();
    sf *= leptonWeight();
    return sf;
}

//fake rate
double treeReader::fakeRateWeight(const unsigned unc) const{
    if(reweighter.use_count() == 0 ){
        reweighter.reset(new Reweighter);
    }
    double sf = -1.;
    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l) && !lepIsTight(l) ){
            double fr = 0;
            if(_lFlavor[l] == 1)        fr = reweighter->muonFakeRate(_lPt[l], _lEta[l], unc);
            else if(_lFlavor[l] == 0)   fr = reweighter->electronFakeRate(_lPt[l], _lEta[l], unc);
            sf *= -fr/(1 - fr);
        }
    }
    return sf;
}
