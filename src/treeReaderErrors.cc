//functions to do some automatic and optional prints for error catching and debugging

//include c++ library classes

//include other parts of code
#include "../interface/treeReader.h"

void treeReader::checkSampleEraConsistency() const{
    for(auto& sample : samples2016){
        if( sample.is2017() ){
            std::cerr << "Error: 2017 sample detected in list of 2016 samples, this will lead to inconsistent lumi-scaling and cuts being applied!" << std::endl;
        }
    }
    for(auto& sample : samples2017){
        if( sample.is2016() ){
            std::cerr << "Error: 2016 sample detected in list of 2017 samples, this will lead to inconsistent lumi-scaling and cuts being applied!" << std::endl;
        }
    }
}

void treeReader::checkEraOrthogonality() const{
    bool bothTrue = is2017() && is2016();
    if(bothTrue){
        std::cerr << "Error: both is2016 and is2017 flags are returning TRUE for current sample! Sample has to be either one." << std::endl;
    }
    bool bothFalse = !( is2017() || is2016() );
    if(bothFalse){
        std::cerr << "Error: both is2016 and is2017 flags are returning FALSE for current sample! Sample has to be either one." << std::endl;
    }
}

void treeReader::printEventTags( std::ostream& os) const{
    os << "######################################\n";
    os << ( is2016() ? "2016" : "2017" ) << " event\n";
    os << ( isData() ? "data" : "MC" ) << " event\n";
    os << "RunNb LumiBlock EventNb : " << _runNb << " " << _lumiBlock << " " << _eventNb << "\n";
}

void treeReader::printLeptonContent( std::ostream& os ) const{
    
    //event tags
    printEventTags( os );

    for(unsigned l = 0; l < _nLight; ++l){
        
        //lepton flavor 
        os << "lepton " << l << " -> ";
        if( isElectron(l) ){
            os << "electron :";
        } else if( isMuon(l) ){
            os << "muon :";
        } else{
            os << "tau :";
        }
        
        //lepton charge and kinematics
        os << "pT = " << _lPt[l] << "\t eta = " << _lEta[l] << "\t";
        os << "charge = " << _lCharge[l];

        //leptin ID decisions
        if( lepIsLoose(l) ){
            os << "\tloose\t";
        } else {
            os << "\tfail_loose\t";
        }

        if( lepIsGood(l) ){
            os << "FO\t";
        } else {
            os << "fail_FO\t";
        }

        if( lepIsTight(l) ){
            os << "tight\t";
        } else {
            os << "fail_tight\t";
        }

        os << "leptonMVA16 = " << _leptonMvatZqTTV16[l];
        os << "\tleptonMVA17 = " << _leptonMvatZqTTV17[l];
        os << "\n";

        os << "lepton MVA inputs : selectedTrackMult = " << _selectedTrackMult[l]
            << "\tminiIsoCharged = " << _miniIsoCharged[l]
            << "\tminiIsoNeutral = " << _miniIso[l] - _miniIsoCharged[l]
            << "\tptRel = " << _ptRel[l]
            << "\tptRatio = " << _ptRatio[l]
            << "\trelIso = " << _relIso[l]
            << "\tdeepCsvClosestJet = " << closestJetDeepCSV(l)
            << "\tsip3d = " << _3dIPSig[l]
            << "\tdxy = " << _dxy[l]
            << "\tdz = " << _dz[l];
        if( _lFlavor[l] == 0 ){
            os << "\telectronMvaSpring16GP = " << _lElectronMva[l] << "\telectronMvaFall17NoIso = " << _lElectronMvaFall17NoIso[l];
        } else {
            os << "\tsegmentCompatibility = " << _lMuonSegComp[l];
        }
        os << "\n";
    }
    os << std::flush; 
}

void treeReader::printLeptonPairing( std::ostream& os ) const{
    if( _nLight < 2){
        std::cerr << "Error : trying to print lepton pairing info for event with less than 2 light leptons." << std::endl;
        return;
    }
    for(unsigned l = 0; l < (unsigned)_nLight - 1; ++l){
        TLorentzVector lep1;
        lep1.SetPtEtaPhiE(_lPt[l], _lEta[l], _lPhi[l], _lE[l]);
        for(unsigned k = l + 1; k < _nLight; ++k){

            //print information on flavor and charge combination
            os << "lepton " << l << " + lepton " << k << " : ";
            if( _lCharge[l] != _lCharge[k] ){
                if(_lFlavor[l] == _lFlavor[k]){
                    os << "OSSF ";
                } else {
                    os << "OSOF ";
                }
            } else {
                os << "SS ";
            }

            //compute and print dilepton mass
            TLorentzVector lep2;
            lep2.SetPtEtaPhiE(_lPt[k], _lEta[k], _lPhi[k], _lE[k]);
            os << "mass = " << (lep1 + lep2).M();
            os << "\n";
        }
    }
    os << std::flush;
}

std::string jetFlavor( const unsigned jetHadronFlavor ){
    if( jetHadronFlavor == 0 ){
        return "light-jet";
    } else if( jetHadronFlavor == 4){
        return "c-jet";
    } else {
        return "b-jet";
    }
}

void treeReader::printJetContent( std::ostream& os) const{
    
    //event tags
    printEventTags( os );

    for(unsigned j = 0; j < _nJets; ++j){
        
        os << "jet " << j;
    
        //jet flavor 
        os << "\t" << jetFlavor( _jetHadronFlavor[j] );

        //jet id decisions 
        os << "\t";
        if( _jetIsLoose[j]){
            os << "loose\t";
        } else {
            os << "fail loose\t";
        }

        if( _jetIsTight[j]){
            os << "tight\t";
        } else {
            os << "fail tight\t";
        }

        if( _jetIsTightLepVeto[j]){
            os << "tightLepVeto";
        } else {
            os << "fail tightLepVeto";
        }

        //jet pt , eta, phi
        os << "\tpT = " << _jetPt[j] << "\teta = " << _jetEta[j] << "\tphi = " << _jetPhi[j];

        //jet mass
        TLorentzVector jet; 
        jet.SetPtEtaPhiE(_jetPt[j], _jetEta[j], _jetPhi[j], _jetE[j]);
        os << "\tmass = " << jet.M();

        //jet CSV values 
        os << "\tdeepCSV = " << deepCSV(j) << "\tCSVv2 = " << _jetCsvV2[j];
        
        os << "\n";
    }
    os << std::flush;
}
