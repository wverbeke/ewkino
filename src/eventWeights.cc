//include c++ library classes
#include <iostream>

//include other parts of code
#include "../interface/treeReader.h"

//pu SF 
double treeReader::puWeight(const unsigned unc) const{
    return reweighter->puWeight(_nTrueInt, currentSample, unc);
}

//b-tagging SF for given flavor
double treeReader::bTagWeight(const unsigned jetFlavor, const unsigned unc) const{
    //WARNING: reactivate this code once the b-tag efficiencies have been computed 
    double pMC = 1.;
    double pData = 1.;
    for(unsigned j = 0; j < _nJets; ++j){
        if(_jetHadronFlavor[j] == jetFlavor){
            //QUESTION: should JEC and b-tag weights also be varied up and down at the same time when computing systematics?
            if(jetIsGood(j, 25., 0, true) && fabs(_jetEta[j]) < 2.4){
                //std::cout << "jet Pt = " << _jetPt[j] << "\teta = " << _jetEta[j] << "\t";
                double sf = reweighter->bTagWeight(_jetHadronFlavor[j], _jetPt[j], _jetEta[j], deepCSV(j), unc);
                double eff = reweighter->bTagEff(_jetHadronFlavor[j], _jetPt[j], _jetEta[j]);
                if(bTagged(j, 1, true)){
                    pMC *= eff;
                    pData *= eff*sf;
                    //std::cout << "sf = " << eff*sf/eff << std::endl;
                } else {
                    pMC *= (1 - eff);
                    pData *= (1 - eff*sf);
                    //std::cout << "sf = " << (1 - eff*sf)/(1 - eff) << std::endl;
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
        if( lepIsTight(l) ){
            if( isMuon(l) ){
                //std::cout << "muon with pT " << _lPt[l] << " ";
                sf *= reweighter->muonTightWeight(_lPt[l], _lEta[l]);
            } else if( isElectron(l) ){
                //std::cout << "electron with pT " << _lPt[l] << " ";
                sf *= reweighter->electronTightWeight(_lPt[l], _lEtaSC[l]);
            }
        } else if( lepIsLoose(l) ){
            if( isMuon(l) ){
                sf *= reweighter->muonLooseWeight(_lPt[l], _lEta[l]);
            } else if( isElectron(l) ){
                sf *= reweighter->electronLooseWeight(_lPt[l], _lEtaSC[l]);
            }
        } 
    }
    return sf;
}

//check if scale-factors have to be initialized, and do so if needed
void treeReader::initializeWeights(){
    static bool weightsAre2016 = is2016();
    bool firstTime = ( reweighter.use_count() == 0 );
    bool changedEra = ( weightsAre2016 != is2016() );
    if( firstTime || changedEra){
        weightsAre2016 = is2016();
        reweighter.reset(new Reweighter(samples, is2016() ) );
    } 
}
    
double treeReader::sfWeight(){
    initializeWeights();
    double sf = puWeight();
    sf *= bTagWeight();
    sf *= leptonWeight();
    if( sf == 0){
        std::cerr << "Error: event sf is zero! This has to be debugged!" << std::endl;
    } else if( std::isnan(sf) ){
        std::cerr << "Error: event sf is nan! This has to be debugged" << std::endl;
    }
    return sf;
}


//fake rate
double treeReader::fakeRateWeight(const unsigned unc){
    initializeWeights();
    double sf;
    if( isData() ){
        sf = -1.;

    //subtract prompt MC fakes from data-driven prediction
    } else {
        sf = 1.;
    }

    for(unsigned l = 0; l < _nLight; ++l){
        if(lepIsGood(l) && !lepIsTight(l) ){
            double fr = 1.;
            if( isMuon(l) ){
                fr = reweighter->muonFakeRate(_lPt[l], _lEta[l], unc);
            } else if( isElectron(l) ){
                fr = reweighter->electronFakeRate(_lPt[l], _lEta[l], unc);
            }
            sf *= -fr/(1 - fr);
        }
    }
    return sf;
}
