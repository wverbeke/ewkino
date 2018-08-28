//include c++ library classes
#include <iostream>

//include other parts of code
#include "../interface/treeReader.h"

//pu SF 
double treeReader::puWeight(const unsigned unc) const{
    return reweighter->puWeight(_nTrueInt, currentSample, unc);
}

//b-tag reweighting

//extract the weight for a single jet 
//WARNING: This function should be refactored!
double treeReader::bTagWeight_cut_singleJet(const unsigned jetIndex, const unsigned unc) const{
    double sf = reweighter->bTagWeight(_jetHadronFlavor[jetIndex], _jetPt[jetIndex], _jetEta[jetIndex], deepCSV(jetIndex), unc);
    if(bTagged(jetIndex, 1, true)){
        return sf;
    } else {
        double eff = reweighter->bTagEff(_jetHadronFlavor[jetIndex], _jetPt[jetIndex], _jetEta[jetIndex]);
        return (1 - eff*sf)/(1 - eff);
    }
}

double treeReader::bTagWeight_reshaping_singleJet(const unsigned jetIndex, const unsigned unc) const{
    return reweighter->bTagWeight(_jetHadronFlavor[jetIndex], _jetPt[jetIndex], _jetEta[jetIndex], deepCSV(jetIndex), unc);
}

//b-tagging SF for given flavor
double treeReader::bTagWeight_base(const unsigned jetFlavor, const unsigned unc, double (treeReader::*jetWeight)(const unsigned, const unsigned) const ) const{
    double sf = 1.;
    for(unsigned j = 0; j < _nJets; ++j){
        if(_jetHadronFlavor[j] == jetFlavor){
            //QUESTION: should JEC and b-tag weights also be varied up and down at the same time when computing systematics?
            if(jetIsGood(j, 25., 0, true) && fabs(_jetEta[j]) < 2.4){
                sf *= (this->*jetWeight)(j, unc);
            }
        }
    }
    return sf;
}

//WARNING: This function should be refactored!
double treeReader::bTagWeight_cut(const unsigned jetFlavor, const unsigned unc) const{
    return bTagWeight_base( jetFlavor, unc, &treeReader::bTagWeight_cut_singleJet );
}

double treeReader::bTagWeight_reshaping(const unsigned jetFlavor, const unsigned unc) const{   
    return bTagWeight_base( jetFlavor, unc, &treeReader::bTagWeight_reshaping_singleJet );
}

double treeReader::bTagWeight(const unsigned jetFlavor, const unsigned unc) const{
    
    //currently we will always use b-tag reshaping
    return bTagWeight_reshaping( jetFlavor, unc );
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
                sf *= reweighter->muonTightWeight(_lPt[l], _lEta[l]);
            } else if( isElectron(l) ){
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

        //automatically use b-tag reshaping for now
        reweighter.reset(new Reweighter(samples, is2016(), "reshaping") );
    } 
}
    
double treeReader::sfWeight(){
    initializeWeights();
    double sf = puWeight();
    if( _nTrueInt < 0){
        std::cerr << "Error: event with negative pileup, returning SF weight 0." << std::endl;
        return 0.;
    }
    sf *= bTagWeight();
    sf *= leptonWeight();
    if( sf == 0){
        std::cerr << "Error: event sf is zero! This has to be debugged!" << std::endl;
    } else if( std::isnan(sf) ){
        std::cerr << "Error: event sf is nan! This has to be debugged!" << std::endl;
    } else if( std::isinf(sf) ){
        std::cerr << "Error: event sf is inf! This has to be debugged!" << std::endl;
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
            sf *= -fr/(1. - fr);
        }
    }
    return sf;
}

//jet prefiring probability 
double treeReader::jetPrefiringWeight() const{
    double sf = 1.;
    for(unsigned j = 0; j < _nJets; ++j){
        if( jetIsGood(j) ){
            double prefiringProbability = reweighter->jetPrefiringProbability( _jetPt[j], _jetEta[j] );
            sf *= ( 1. - prefiringProbability );
        }
    }
    return sf;
}
