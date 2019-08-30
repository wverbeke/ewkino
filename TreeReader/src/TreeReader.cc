#include "../interface/TreeReader.h"

//include c++ library classes 
#include <fstream>
#include <iostream>

//include other parts of analysis framework
#include "../../Tools/interface/analysisTools.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"


TreeReader::TreeReader( const std::string& sampleListFile, const std::string& sampleDirectory ){
    readSamples( sampleListFile, sampleDirectory );
}


void TreeReader::readSamples( const std::string& list, const std::string& directory, std::vector<Sample>& sampleVector ){

    //clean current sample list 
    sampleVector.clear();

    //read list of samples from file
    sampleVector = readSampleList(list, directory);

    //print sample information
    for(auto& sample : sampleVector){
        std::cout << sample << std::endl;
    }
}


void TreeReader::readSamples( const std::string& list, const std::string& directory ){
    readSamples( list, directory, this->samples );
}


void TreeReader::readSamples2016( const std::string& list, const std::string& directory ){
    std::cout << "########################################" << std::endl;
    std::cout << "         2016 samples                   " << std::endl;
    std::cout << "########################################" << std::endl;

    readSamples( list, directory, this->samples2016 );

    //add the 2016 samples to the total sample list 
    this->samples.insert( samples.end(), samples2016.begin(), samples2016.end() );

    //check for errors
    checkSampleEraConsistency();

}


void TreeReader::readSamples2017( const std::string& list, const std::string& directory ){
    std::cout << "########################################" << std::endl;
    std::cout << "         2017 samples                   " << std::endl;
    std::cout << "########################################" << std::endl;

    readSamples( list, directory, this->samples2017 );

    //add the 2017 samples to the total sample list
    this->samples.insert( samples.end(), samples2017.begin(), samples2017.end() );

    //check for errors 
    checkSampleEraConsistency();
}


std::pair< std::map< std::string, bool >, std::map< std::string, TBranch* > > buildBranchMap( TTree* treePtr, const std::string& nameIdentifier, const std::string& antiIdentifier = "" ){   
    std::map< std::string, bool > decisionMap;
    std::map< std::string, TBranch* > branchMap;
    TObjArray* branch_list = treePtr->GetListOfBranches();
    for( const auto& branchPtr : *branch_list ){
        std::string branchName = branchPtr->GetName();
        if( stringTools::stringContains( branchName, nameIdentifier ) ){
            if( antiIdentifier != "" && stringTools::stringContains( branchName, antiIdentifier ) ) continue;
            decisionMap[ branchName ] = false;
            branchMap[ branchName ] = nullptr;
        }
    }
    return { decisionMap, branchMap };
}


void TreeReader::initializeTriggerMap( TTree* treePtr ){
    auto triggerMaps = buildBranchMap( treePtr, "HLT", "prescale" );
    _triggerMap = triggerMaps.first;
    b__triggerMap = triggerMaps.second;
}


void TreeReader::initializeMetFilterMap( TTree* treePtr ){

    //WARNING: Currently one MET filter contains 'updated' rather than 'Flag' in the name. If this changes, make sure to modify the code here!
    auto filterMaps = buildBranchMap( treePtr, "Flag" );
    auto filterMaps_part2 = buildBranchMap( treePtr , "updated" );
    filterMaps.first.insert( filterMaps_part2.first.cbegin(), filterMaps_part2.first.cend() );
    filterMaps.second.insert( filterMaps_part2.second.cbegin(), filterMaps_part2.second.cend() );

    _MetFilterMap = filterMaps.first;
    b__MetFilterMap = filterMaps.second;
}


bool treeHasBranchWithName( TTree* treePtr, const std::string& nameToFind ){
	TObjArray* branch_list = treePtr->GetListOfBranches();
    for( const auto& branchPtr : *branch_list ){
        std::string branchName = branchPtr->GetName();
		if( stringTools::stringContains( branchName, nameToFind ) ){
			return true;
		}
	}
	return false;
}


bool TreeReader::containsGeneratorInfo() const{
    return treeHasBranchWithName( currentTreePtr.get(), "_gen_" );
}


bool TreeReader::containsSUSYMassInfo() const{
    return treeHasBranchWithName( currentTreePtr.get(), "_mChi" );
}


bool TreeReader::isData() const{
    if( currentSamplePtr != nullptr ){
        return currentSamplePtr->isData();
    } else {
        return !containsGeneratorInfo();
    }
}


bool TreeReader::isMC() const{
    return !isData();
}


void TreeReader::checkCurrentSample() const{
    if( currentSamplePtr == nullptr ){
        throw std::domain_error( "pointer to current Sample is nullptr." );
    }
}


void TreeReader::checkCurrentTree() const{
    if( !currentTreePtr ){
        throw std::domain_error( "pointer to current TTree is nullptr." );
    }
}


bool TreeReader::is2016() const{
    checkCurrentSample();
    return currentSamplePtr->is2016();
}


bool TreeReader::is2017() const{
    checkCurrentSample();
    return currentSamplePtr->is2017();
}


bool TreeReader::is2018() const{
    checkCurrentSample();
    return currentSamplePtr->is2018();
}


bool TreeReader::isSMSignal() const{
    checkCurrentSample();
    return currentSamplePtr->isSMSignal();
}


bool TreeReader::isNewPhysicsSignal() const{
    checkCurrentSample();
    return currentSamplePtr->isNewPhysicsSignal();
}


long unsigned TreeReader::numberOfEntries() const{
    checkCurrentTree();
    return currentTreePtr->GetEntries();
}


void TreeReader::initSample( const Sample& samp ){ 

    //update current sample
    currentSamplePtr = &samp;
    currentFilePtr = samp.filePtr();
    currentTreePtr = std::shared_ptr< TTree >( (TTree*) currentFilePtr->Get( "blackJackAndHookers/blackJackAndHookersTree" ) );
    initTree();
    if( !samp.isData() ){

        //read sum of simulated event weights
        TH1D* hCounter = new TH1D( "hCounter", "Events counter", 1, 0, 1 );
        currentFilePtr->cd( "blackJackAndHookers" );
        hCounter->Read( "hCounter" ); 
        double sumSimulatedEventWeights = hCounter->GetBinContent(1);
        delete hCounter;

        //event weights set with lumi depending on sample's era 
        double dataLumi;
        if( is2016() ){
            dataLumi = lumi2016;
        } else if( is2017() ){
            dataLumi = lumi2017;
        } else {
            dataLumi = lumi2018;
        }
        scale = samp.xSec()*dataLumi*1000 / sumSimulatedEventWeights;
    }
}


//initialize the next sample in the list
void TreeReader::initSample(){
    initSample( samples[ ++currentSampleIndex ] );
}


//initialize the current Sample directly from a root file, this is used when skimming
void TreeReader::initSampleFromFile( const std::string& pathToFile, const bool is2017, const bool is2018 ){

    currentFilePtr = std::shared_ptr< TFile >( new TFile( pathToFile.c_str() ) );
    currentTreePtr = std::shared_ptr< TTree >( (TTree*) currentFilePtr->Get( "blackJackAndHookers/blackJackAndHookersTree" ) );

    //make a new sample, and make sure the pointer remains valid
    //new is no option here since this would also require a destructor for the class which does not work for the other initSample case
    static Sample samp;
    samp = Sample( pathToFile, is2017, is2018, isData() );
    currentSamplePtr = &samp;

    //initialize tree
    initTree();
}


void TreeReader::GetEntry( const Sample& samp, long unsigned entry ){
    checkCurrentTree();

    currentTreePtr->GetEntry( entry );

    //Set up correct event weight
    if( !samp.isData() ){
        _scaledWeight = _weight*scale;
    } else{
        _scaledWeight = 1;
    }
}


//use the currently initialized sample when running in serial
void TreeReader::GetEntry( long unsigned entry ){
    GetEntry( *currentSamplePtr, entry );
}


Event TreeReader::buildEvent( const Sample& samp, long unsigned entry, const bool readIndividualTriggers, const bool readIndividualMetFilters ){
    GetEntry( samp, entry );
    return Event( *this, readIndividualTriggers, readIndividualMetFilters );
}


Event TreeReader::buildEvent( long unsigned entry, const bool readIndividualTriggers, const bool readIndividualMetFilters ){
    GetEntry( entry );
    return Event( *this, readIndividualTriggers, readIndividualMetFilters );
}


template< typename T > void setMapBranchAddresses( TTree* treePtr, std::map< std::string, T >& variableMap, std::map< std::string, TBranch* > branchMap ){
    for( const auto& variable : variableMap ){
        treePtr->SetBranchAddress( variable.first.c_str(), &variableMap[ variable.first ], &branchMap[ variable.first ] );
    }
}


//WARNING: this piece of code has not been generalized to other types than 'bool' because the correct ROOT identifier string needs to be used
void setMapOutputBranches( TTree* treePtr, std::map< std::string, bool >& variableMap ){
    for( const auto& variable : variableMap ){
        treePtr->Branch( variable.first.c_str(), &variableMap[ variable.first ], ( variable.first + "/O" ).c_str() );
    }    
}


void TreeReader::initTree(){

    // Set branch addresses and branch pointers
    checkCurrentTree();

    currentTreePtr->SetMakeClass(1);

    currentTreePtr->SetBranchAddress("_runNb", &_runNb, &b__runNb);
    currentTreePtr->SetBranchAddress("_lumiBlock", &_lumiBlock, &b__lumiBlock);
    currentTreePtr->SetBranchAddress("_eventNb", &_eventNb, &b__eventNb);
    currentTreePtr->SetBranchAddress("_nVertex", &_nVertex, &b__nVertex);    
    currentTreePtr->SetBranchAddress("_passTrigger_e", &_passTrigger_e, &b__passTrigger_e);
    currentTreePtr->SetBranchAddress("_passTrigger_ee", &_passTrigger_ee, &b__passTrigger_ee);
    currentTreePtr->SetBranchAddress("_passTrigger_eee", &_passTrigger_eee, &b__passTrigger_eee);
    currentTreePtr->SetBranchAddress("_passTrigger_em", &_passTrigger_em, &b__passTrigger_em);
    currentTreePtr->SetBranchAddress("_passTrigger_m", &_passTrigger_m, &b__passTrigger_m);
    currentTreePtr->SetBranchAddress("_passTrigger_eem", &_passTrigger_eem, &b__passTrigger_eem);
    currentTreePtr->SetBranchAddress("_passTrigger_mm", &_passTrigger_mm, &b__passTrigger_mm);
    currentTreePtr->SetBranchAddress("_passTrigger_emm", &_passTrigger_emm, &b__passTrigger_emm);
    currentTreePtr->SetBranchAddress("_passTrigger_mmm", &_passTrigger_mmm, &b__passTrigger_mmm);
    currentTreePtr->SetBranchAddress("_passTrigger_et", &_passTrigger_et, &b__passTrigger_et);
    currentTreePtr->SetBranchAddress("_passTrigger_mt", &_passTrigger_mt, &b__passTrigger_mt);
    currentTreePtr->SetBranchAddress("_passMETFilters", &_passMETFilters, &b__passMETFilters);
    currentTreePtr->SetBranchAddress("_nL", &_nL, &b__nL);
    currentTreePtr->SetBranchAddress("_nMu", &_nMu, &b__nMu);
    currentTreePtr->SetBranchAddress("_nEle", &_nEle, &b__nEle);
    currentTreePtr->SetBranchAddress("_nLight", &_nLight, &b__nLight);
    currentTreePtr->SetBranchAddress("_nTau", &_nTau, &b__nTau);
    currentTreePtr->SetBranchAddress("_lPt", _lPt, &b__lPt);
    currentTreePtr->SetBranchAddress("_lEta", _lEta, &b__lEta);
    currentTreePtr->SetBranchAddress("_lEtaSC", _lEtaSC, &b__lEtaSC);
    currentTreePtr->SetBranchAddress("_lPhi", _lPhi, &b__lPhi);
    currentTreePtr->SetBranchAddress("_lE", _lE, &b__lE);
    currentTreePtr->SetBranchAddress("_lFlavor", _lFlavor, &b__lFlavor);
    currentTreePtr->SetBranchAddress("_lCharge", _lCharge, &b__lCharge);
    currentTreePtr->SetBranchAddress("_dxy", _dxy, &b__dxy);
    currentTreePtr->SetBranchAddress("_dz", _dz, &b__dz);
    currentTreePtr->SetBranchAddress("_3dIP", _3dIP, &b__3dIP);
    currentTreePtr->SetBranchAddress("_3dIPSig", _3dIPSig, &b__3dIPSig);
    currentTreePtr->SetBranchAddress("_lElectronSummer16MvaGP", _lElectronSummer16MvaGP, &b__lElectronSummer16MvaGP);
    currentTreePtr->SetBranchAddress("_lElectronSummer16MvaHZZ", _lElectronSummer16MvaHZZ, &b__lElectronSummer16MvaHZZ);
    currentTreePtr->SetBranchAddress("_lElectronMvaFall17Iso", _lElectronMvaFall17Iso, &b__lElectronMvaFall17Iso);
    currentTreePtr->SetBranchAddress("_lElectronMvaFall17NoIso", _lElectronMvaFall17NoIso, &b__lElectronMvaFall17NoIso);
    currentTreePtr->SetBranchAddress("_lElectronPassEmu", _lElectronPassEmu, &b__lElectronPassEmu);
    currentTreePtr->SetBranchAddress("_lElectronPassConvVeto", _lElectronPassConvVeto, &b__lElectronPassConvVeto);
    currentTreePtr->SetBranchAddress("_lElectronChargeConst", _lElectronChargeConst, &b__lElectronChargeConst);
    currentTreePtr->SetBranchAddress("_lElectronMissingHits", _lElectronMissingHits, &b__lElectronMissingHits);
    currentTreePtr->SetBranchAddress("_leptonMvaTTH", _leptonMvaTTH, &b__leptonMvaTTH);
    currentTreePtr->SetBranchAddress("_leptonMvatZq", _leptonMvatZq, &b__leptonMvatZq);
    currentTreePtr->SetBranchAddress("_lPOGVeto", _lPOGVeto, &b__lPOGVeto);
    currentTreePtr->SetBranchAddress("_lPOGLoose", _lPOGLoose, &b__lPOGLoose);
    currentTreePtr->SetBranchAddress("_lPOGMedium", _lPOGMedium, &b__lPOGMedium);
    currentTreePtr->SetBranchAddress("_lPOGTight", _lPOGTight, &b__lPOGTight);

    currentTreePtr->SetBranchAddress("_tauDecayMode", _tauDecayMode, &b__tauDecayMode);
    currentTreePtr->SetBranchAddress("_decayModeFinding", _decayModeFinding, &b__decayModeFinding);
    currentTreePtr->SetBranchAddress("_decayModeFindingNew", _decayModeFindingNew, &b__decayModeFindingNew);
    currentTreePtr->SetBranchAddress("_tauMuonVetoLoose", _tauMuonVetoLoose, &b__tauMuonVetoLoose);
    currentTreePtr->SetBranchAddress("_tauMuonVetoTight", _tauMuonVetoTight, &b__tauMuonVetoTight);
    currentTreePtr->SetBranchAddress("_tauEleVetoVLoose", _tauEleVetoVLoose, &b__tauEleVetoVLoose);
    currentTreePtr->SetBranchAddress("_tauEleVetoLoose", _tauEleVetoLoose, &b__tauEleVetoLoose);
    currentTreePtr->SetBranchAddress("_tauEleVetoMedium", _tauEleVetoMedium, &b__tauEleVetoMedium);
    currentTreePtr->SetBranchAddress("_tauEleVetoTight", _tauEleVetoTight, &b__tauEleVetoTight);
    currentTreePtr->SetBranchAddress("_tauEleVetoVTight", _tauEleVetoVTight, &b__tauEleVetoVTight);
    currentTreePtr->SetBranchAddress("_tauPOGVLoose2015", _tauPOGVLoose2015, &b__tauPOGVLoose2015);
    currentTreePtr->SetBranchAddress("_tauPOGLoose2015", _tauPOGLoose2015, &b__tauPOGLoose2015);
    currentTreePtr->SetBranchAddress("_tauPOGMedium2015", _tauPOGMedium2015, &b__tauPOGMedium2015);
    currentTreePtr->SetBranchAddress("_tauPOGTight2015", _tauPOGTight2015, &b__tauPOGTight2015);
    currentTreePtr->SetBranchAddress("_tauPOGVTight2015", _tauPOGVTight2015, &b__tauPOGVTight2015);
    currentTreePtr->SetBranchAddress("_tauVLooseMvaNew2015", _tauVLooseMvaNew2015, &b__tauVLooseMvaNew2015);
    currentTreePtr->SetBranchAddress("_tauLooseMvaNew2015", _tauLooseMvaNew2015, &b__tauLooseMvaNew2015);
    currentTreePtr->SetBranchAddress("_tauMediumMvaNew2015", _tauMediumMvaNew2015, &b__tauMediumMvaNew2015);
    currentTreePtr->SetBranchAddress("_tauTightMvaNew2015", _tauTightMvaNew2015, &b__tauTightMvaNew2015);
    currentTreePtr->SetBranchAddress("_tauVTightMvaNew2015", _tauVTightMvaNew2015, &b__tauVTightMvaNew2015);
    currentTreePtr->SetBranchAddress("_tauPOGVVLoose2017v2", _tauPOGVVLoose2017v2, &b__tauPOGVVLoose2017v2);
    currentTreePtr->SetBranchAddress("_tauPOGVTight2017v2", _tauPOGVTight2017v2, &b__tauPOGVTight2017v2);
    currentTreePtr->SetBranchAddress("_tauPOGVVTight2017v2", _tauPOGVVTight2017v2, &b__tauPOGVVTight2017v2);
    currentTreePtr->SetBranchAddress("_tauVLooseMvaNew2017v2", _tauVLooseMvaNew2017v2, &b__tauVLooseMvaNew2017v2);
    currentTreePtr->SetBranchAddress("_tauLooseMvaNew2017v2", _tauLooseMvaNew2017v2, &b__tauLooseMvaNew2017v2);
    currentTreePtr->SetBranchAddress("_tauMediumMvaNew2017v2", _tauMediumMvaNew2017v2, &b__tauMediumMvaNew2017v2);
    currentTreePtr->SetBranchAddress("_tauTightMvaNew2017v2", _tauTightMvaNew2017v2, &b__tauTightMvaNew2017v2);
    currentTreePtr->SetBranchAddress("_tauVTightMvaNew2017v2", _tauVTightMvaNew2017v2, &b__tauVTightMvaNew2017v2);

    currentTreePtr->SetBranchAddress("_relIso", _relIso, &b__relIso);
    currentTreePtr->SetBranchAddress("_relIso0p4", _relIso0p4, &b__relIso0p4);
    currentTreePtr->SetBranchAddress("_relIso0p4MuDeltaBeta", _relIso0p4MuDeltaBeta, &b__relIso0p4MuDeltaBeta);
    currentTreePtr->SetBranchAddress("_miniIso", _miniIso, &b__miniIso);
    currentTreePtr->SetBranchAddress("_miniIsoCharged", _miniIsoCharged, &b__miniIsoCharged);
    currentTreePtr->SetBranchAddress("_ptRel", _ptRel, &b__ptRel);
    currentTreePtr->SetBranchAddress("_ptRatio", _ptRatio, &b__ptRatio);
    currentTreePtr->SetBranchAddress("_closestJetCsvV2", _closestJetCsvV2, &b__closestJetCsvV2);
    currentTreePtr->SetBranchAddress("_closestJetDeepCsv_b", _closestJetDeepCsv_b, &b__closestJetDeepCsv_b);
    currentTreePtr->SetBranchAddress("_closestJetDeepCsv_bb", _closestJetDeepCsv_bb, &b__closestJetDeepCsv_bb);
    currentTreePtr->SetBranchAddress("_closestJetDeepFlavor_b", _closestJetDeepFlavor_b, &b__closestJetDeepFlavor_b);
    currentTreePtr->SetBranchAddress("_closestJetDeepFlavor_bb", _closestJetDeepFlavor_bb, &b__closestJetDeepFlavor_bb);
    currentTreePtr->SetBranchAddress("_closestJetDeepFlavor_lepb", _closestJetDeepFlavor_lepb, &b__closestJetDeepFlavor_lepb);
    currentTreePtr->SetBranchAddress("_selectedTrackMult", _selectedTrackMult, &b__selectedTrackMult);
    currentTreePtr->SetBranchAddress("_lMuonSegComp", _lMuonSegComp, &b__lMuonSegComp);
    currentTreePtr->SetBranchAddress("_lMuonTrackPt", _lMuonTrackPt, &b__lMuonTrackPt);
    currentTreePtr->SetBranchAddress("_lMuonTrackPtErr", _lMuonTrackPtErr, &b__lMuonTrackPtErr);
    currentTreePtr->SetBranchAddress("_nJets", &_nJets, &b__nJets);
    currentTreePtr->SetBranchAddress("_jetPt", _jetPt, &b__jetPt);
    currentTreePtr->SetBranchAddress("_jetSmearedPt", _jetSmearedPt, &b__jetSmearedPt);
    currentTreePtr->SetBranchAddress("_jetSmearedPt_JECDown", _jetSmearedPt_JECDown, &b__jetSmearedPt_JECDown);
    currentTreePtr->SetBranchAddress("_jetSmearedPt_JECUp", _jetSmearedPt_JECUp, &b__jetSmearedPt_JECUp);
    currentTreePtr->SetBranchAddress("_jetSmearedPt_JERDown", _jetSmearedPt_JERDown, &b__jetSmearedPt_JERDown);
    currentTreePtr->SetBranchAddress("_jetSmearedPt_JERUp", _jetSmearedPt_JERUp, &b__jetSmearedPt_JERUp);
    currentTreePtr->SetBranchAddress("_jetPt_JECUp", _jetPt_JECUp, &b__jetPt_JECUp);
    currentTreePtr->SetBranchAddress("_jetPt_JECDown", _jetPt_JECDown, &b__jetPt_JECDown);
    currentTreePtr->SetBranchAddress("_jetEta", _jetEta, &b__jetEta);
    currentTreePtr->SetBranchAddress("_jetPhi", _jetPhi, &b__jetPhi);
    currentTreePtr->SetBranchAddress("_jetE", _jetE, &b__jetE);
    currentTreePtr->SetBranchAddress("_jetPt_Uncorrected",_jetPt_Uncorrected, &b__jetPt_Uncorrected);
    currentTreePtr->SetBranchAddress("_jetPt_L1", _jetPt_L1, &b__jetPt_L1);
    currentTreePtr->SetBranchAddress("_jetPt_L2", _jetPt_L2, &b__jetPt_L2);
    currentTreePtr->SetBranchAddress("_jetPt_L3", _jetPt_L3, &b__jetPt_L3);
    currentTreePtr->SetBranchAddress("_jetCsvV2", _jetCsvV2, &b__jetCsvV2);
    currentTreePtr->SetBranchAddress("_jetDeepCsv_udsg", _jetDeepCsv_udsg, &b__jetDeepCsv_udsg);
    currentTreePtr->SetBranchAddress("_jetDeepCsv_b", _jetDeepCsv_b, &b__jetDeepCsv_b);
    currentTreePtr->SetBranchAddress("_jetDeepCsv_c", _jetDeepCsv_c, &b__jetDeepCsv_c);
    currentTreePtr->SetBranchAddress("_jetDeepCsv_bb", _jetDeepCsv_bb, &b__jetDeepCsv_bb);
    currentTreePtr->SetBranchAddress("_jetDeepFlavor_b", _jetDeepFlavor_b, &b__jetDeepFlavor_b);
    currentTreePtr->SetBranchAddress("_jetDeepFlavor_bb", _jetDeepFlavor_bb, &b__jetDeepFlavor_bb);
    currentTreePtr->SetBranchAddress("_jetDeepFlavor_lepb", _jetDeepFlavor_lepb, &b__jetDeepFlavor_lepb);
    currentTreePtr->SetBranchAddress("_jetHadronFlavor", _jetHadronFlavor, &b__jetHadronFlavor);
    currentTreePtr->SetBranchAddress("_jetIsTight", _jetIsTight, &b__jetIsTight);
    currentTreePtr->SetBranchAddress("_jetIsTightLepVeto", _jetIsTightLepVeto, &b__jetIsTightLepVeto);
    currentTreePtr->SetBranchAddress("_jetNeutralHadronFraction", _jetNeutralHadronFraction, &b__jetNeutralHadronFraction);
    currentTreePtr->SetBranchAddress("_jetChargedHadronFraction", _jetChargedHadronFraction, &b__jetChargedHadronFraction);
    currentTreePtr->SetBranchAddress("_jetNeutralEmFraction", _jetNeutralEmFraction, &b__jetNeutralEmFraction);
    currentTreePtr->SetBranchAddress("_jetChargedEmFraction", _jetChargedEmFraction, &b__jetChargedEmFraction);
    currentTreePtr->SetBranchAddress("_jetHFHadronFraction", _jetHFHadronFraction, &b__jetHFHadronFraction);
    currentTreePtr->SetBranchAddress("_jetHFEmFraction", _jetHFEmFraction, &b__jetHFEmFraction);
    currentTreePtr->SetBranchAddress("_met", &_met, &b__met);
    currentTreePtr->SetBranchAddress("_metJECDown", &_metJECDown, &b__metJECDown);
    currentTreePtr->SetBranchAddress("_metJECUp", &_metJECUp, &b__metJECUp);
    currentTreePtr->SetBranchAddress("_metUnclDown", &_metUnclDown, &b__metUnclDown);
    currentTreePtr->SetBranchAddress("_metUnclUp", &_metUnclUp, &b__metUnclUp);
    currentTreePtr->SetBranchAddress("_metPhi", &_metPhi, &b__metPhi);
    currentTreePtr->SetBranchAddress("_metPhiJECDown", &_metPhiJECDown, &b__metPhiJECDown);
    currentTreePtr->SetBranchAddress("_metPhiJECUp", &_metPhiJECUp, &b__metPhiJECUp);
    currentTreePtr->SetBranchAddress("_metPhiUnclDown", &_metPhiUnclDown, &b__metPhiUnclDown);
    currentTreePtr->SetBranchAddress("_metPhiUnclUp", &_metPhiUnclUp, &b__metPhiUnclUp);
    currentTreePtr->SetBranchAddress("_metSignificance", &_metSignificance, &b__metSignificance);

    if( containsGeneratorInfo() ){
        currentTreePtr->SetBranchAddress("_weight", &_weight, &b__weight);
        currentTreePtr->SetBranchAddress("_nLheWeights", &_nLheWeights, &b__nLheWeights);
        currentTreePtr->SetBranchAddress("_lheWeight", _lheWeight, &b__lheWeight);
        currentTreePtr->SetBranchAddress("_nPsWeights", &_nPsWeights, &b__nPsWeights);
        currentTreePtr->SetBranchAddress("_psWeight", _psWeight, &b__psWeight);
        currentTreePtr->SetBranchAddress("_nTrueInt", &_nTrueInt, &b__nTrueInt);
        currentTreePtr->SetBranchAddress("_lheHTIncoming", &_lheHTIncoming, &b__lheHTIncoming);
        currentTreePtr->SetBranchAddress("_gen_met", &_gen_met, &b__gen_met);
        currentTreePtr->SetBranchAddress("_gen_metPhi", &_gen_metPhi, &b__gen_metPhi);
        currentTreePtr->SetBranchAddress("_gen_nL", &_gen_nL, &b__gen_nL);
        currentTreePtr->SetBranchAddress("_gen_lPt", _gen_lPt, &b__gen_lPt);
        currentTreePtr->SetBranchAddress("_gen_lEta", _gen_lEta, &b__gen_lEta);
        currentTreePtr->SetBranchAddress("_gen_lPhi", _gen_lPhi, &b__gen_lPhi);
        currentTreePtr->SetBranchAddress("_gen_lE", _gen_lE, &b__gen_lE);
        currentTreePtr->SetBranchAddress("_gen_lFlavor", _gen_lFlavor, &b__gen_lFlavor);
        currentTreePtr->SetBranchAddress("_gen_lCharge", _gen_lCharge, &b__gen_lCharge);
        currentTreePtr->SetBranchAddress("_gen_lMomPdg", _gen_lMomPdg, &b__gen_lMomPdg);
        currentTreePtr->SetBranchAddress("_gen_lIsPrompt", _gen_lIsPrompt, &b__gen_lIsPrompt);
        currentTreePtr->SetBranchAddress("_lIsPrompt", _lIsPrompt, &b__lIsPrompt);
        currentTreePtr->SetBranchAddress("_lMatchPdgId", _lMatchPdgId, &b__lMatchPdgId);
        currentTreePtr->SetBranchAddress("_lMatchCharge", _lMatchCharge, &b__lMatchCharge);
        currentTreePtr->SetBranchAddress("_lMomPdgId",  _lMomPdgId, &b__lMomPdgId);
        currentTreePtr->SetBranchAddress("_lProvenance", _lProvenance, &b__lProvenance);
        currentTreePtr->SetBranchAddress("_lProvenanceCompressed", _lProvenanceCompressed, &b__lProvenanceCompressed);
        currentTreePtr->SetBranchAddress("_lProvenanceConversion", _lProvenanceConversion, &b__lProvenanceConversion);
        currentTreePtr->SetBranchAddress("_ttgEventType", &_ttgEventType, &b__ttgEventType);
        currentTreePtr->SetBranchAddress("_zgEventType", &_zgEventType, &b__zgEventType);
    } 

    if( !is2018() ){
        currentTreePtr->SetBranchAddress("_prefireWeight", &_prefireWeight, &b__prefireWeight);
        currentTreePtr->SetBranchAddress("_prefireWeightDown", &_prefireWeightDown, &b__prefireWeightDown);
        currentTreePtr->SetBranchAddress("_prefireWeightUp", &_prefireWeightUp, &b__prefireWeightUp);
    }

	if( containsSUSYMassInfo() ){
		currentTreePtr->SetBranchAddress("_mChi1", &_mChi1, &b__mChi1);
		currentTreePtr->SetBranchAddress("_mChi2", &_mChi2, &b__mChi2);
	}

    //add all individually stored triggers 
    initializeTriggerMap( currentTreePtr.get() );
    setMapBranchAddresses( currentTreePtr.get(), _triggerMap, b__triggerMap );

    //add all individually stored MET filters
    initializeMetFilterMap( currentTreePtr.get() );
    setMapBranchAddresses( currentTreePtr.get(), _MetFilterMap, b__MetFilterMap );
}


void TreeReader::setOutputTree( TTree* outputTree ){
    outputTree->Branch("_runNb",                        &_runNb,                        "_runNb/l");
    outputTree->Branch("_lumiBlock",                    &_lumiBlock,                    "_lumiBlock/l");
    outputTree->Branch("_eventNb",                      &_eventNb,                      "_eventNb/l");
    outputTree->Branch("_nVertex",                      &_nVertex,                      "_nVertex/b");
    outputTree->Branch("_met",                          &_met,                          "_met/D");
    outputTree->Branch("_metJECDown",                   &_metJECDown,                   "_metJECDown/D");
    outputTree->Branch("_metJECUp",                     &_metJECUp,                     "_metJECUp/D");
    outputTree->Branch("_metUnclDown",                  &_metUnclDown,                  "_metUnclDown/D");
    outputTree->Branch("_metUnclUp",                    &_metUnclUp,                    "_metUnclUp/D");
    outputTree->Branch("_metPhi",                       &_metPhi,                       "_metPhi/D");
    outputTree->Branch("_metPhiJECDown",                &_metPhiJECDown,                "_metPhiJECDown/D");
    outputTree->Branch("_metPhiJECUp",                  &_metPhiJECUp,                  "_metPhiJECUp/D");
    outputTree->Branch("_metPhiUnclDown",               &_metPhiUnclDown,               "_metPhiUnclDown/D");
    outputTree->Branch("_metPhiUnclUp",                 &_metPhiUnclUp,                 "_metPhiUnclUp/D");
    outputTree->Branch("_metSignificance",              &_metSignificance,              "_metSignificance/D");
    outputTree->Branch("_passTrigger_e", &_passTrigger_e, "_passTrigger_e/O");
    outputTree->Branch("_passTrigger_ee", &_passTrigger_ee, "_passTrigger_ee/O");
    outputTree->Branch("_passTrigger_eee", &_passTrigger_eee, "_passTrigger_eee/O");
    outputTree->Branch("_passTrigger_em", &_passTrigger_em, "_passTrigger_em/O");
    outputTree->Branch("_passTrigger_m", &_passTrigger_m, "_passTrigger_m/O");
    outputTree->Branch("_passTrigger_eem", &_passTrigger_eem, "_passTrigger_eem/O");
    outputTree->Branch("_passTrigger_mm", &_passTrigger_mm, "_passTrigger_mm/O");
    outputTree->Branch("_passTrigger_emm", &_passTrigger_emm, "_passTrigger_emm/O");
    outputTree->Branch("_passTrigger_mmm", &_passTrigger_mmm, "_passTrigger_mmm/O");
    outputTree->Branch("_passTrigger_et", &_passTrigger_et, "_passTrigger_et/O");
    outputTree->Branch("_passTrigger_mt", &_passTrigger_mt, "_passTrigger_mt/O");
    outputTree->Branch("_passMETFilters", &_passMETFilters, "_passMETFilters/O");
    outputTree->Branch("_nL",                           &_nL,                           "_nL/i");
    outputTree->Branch("_nMu",                          &_nMu,                          "_nMu/i");
    outputTree->Branch("_nEle",                         &_nEle,                         "_nEle/i");
    outputTree->Branch("_nLight",                       &_nLight,                       "_nLight/i");
    outputTree->Branch("_nTau",                         &_nTau,                         "_nTau/i");
    outputTree->Branch("_lPt",                          &_lPt,                          "_lPt[_nL]/D");
    outputTree->Branch("_lEta",                         &_lEta,                         "_lEta[_nL]/D");
    outputTree->Branch("_lEtaSC",                       &_lEtaSC,                       "_lEtaSC[_nLight]/D");
    outputTree->Branch("_lPhi",                         &_lPhi,                         "_lPhi[_nL]/D");
    outputTree->Branch("_lE",                           &_lE,                           "_lE[_nL]/D");
    outputTree->Branch("_lFlavor",                      &_lFlavor,                      "_lFlavor[_nL]/i");
    outputTree->Branch("_lCharge",                      &_lCharge,                      "_lCharge[_nL]/I");
    outputTree->Branch("_dxy",                          &_dxy,                          "_dxy[_nL]/D");
    outputTree->Branch("_dz",                           &_dz,                           "_dz[_nL]/D");
    outputTree->Branch("_3dIP",                         &_3dIP,                         "_3dIP[_nL]/D");
    outputTree->Branch("_3dIPSig",                      &_3dIPSig,                      "_3dIPSig[_nL]/D");
    outputTree->Branch("_lElectronSummer16MvaGP",       &_lElectronSummer16MvaGP,       "_lElectronSummer16MvaGP[_nLight]/F");
    outputTree->Branch("_lElectronSummer16MvaHZZ",      &_lElectronSummer16MvaHZZ,      "_lElectronSummer16MvaHZZ[_nLight]/F");
    outputTree->Branch("_lElectronMvaFall17Iso",        &_lElectronMvaFall17Iso,        "_lElectronMvaFall17Iso[_nLight]/F");
    outputTree->Branch("_lElectronMvaFall17NoIso",      &_lElectronMvaFall17NoIso,      "_lElectronMvaFall17NoIso[_nLight]/F");
    outputTree->Branch("_lElectronPassEmu",             &_lElectronPassEmu,             "_lElectronPassEmu[_nLight]/O");
    outputTree->Branch("_lElectronPassConvVeto",        &_lElectronPassConvVeto,        "_lElectronPassConvVeto[_nLight]/O");
    outputTree->Branch("_lElectronChargeConst",         &_lElectronChargeConst,         "_lElectronChargeConst[_nLight]/O");
    outputTree->Branch("_lElectronMissingHits",         &_lElectronMissingHits,         "_lElectronMissingHits[_nLight]/i");
    outputTree->Branch("_leptonMvaTTH",                 &_leptonMvaTTH,                 "_leptonMvaTTH[_nLight]/D");
    outputTree->Branch("_leptonMvatZq",                 &_leptonMvatZq,                 "_leptonMvatZq[_nLight]/D");
    outputTree->Branch("_lPOGVeto",                     &_lPOGVeto,                     "_lPOGVeto[_nL]/O");
    outputTree->Branch("_lPOGLoose",                    &_lPOGLoose,                    "_lPOGLoose[_nL]/O");
    outputTree->Branch("_lPOGMedium",                   &_lPOGMedium,                   "_lPOGMedium[_nL]/O");
    outputTree->Branch("_lPOGTight",                    &_lPOGTight,                    "_lPOGTight[_nL]/O");

    outputTree->Branch("_tauDecayMode",                 &_tauDecayMode,                 "_tauDecayMode[_nL]/i");
    outputTree->Branch("_decayModeFinding",             &_decayModeFinding,             "_decayModeFinding[_nL]/O");
   	outputTree->Branch("_decayModeFindingNew",          &_decayModeFindingNew,          "_decayModeFindingNew[_nL]/O");
    outputTree->Branch("_tauPOGVLoose2015",             &_tauPOGVLoose2015,             "_tauPOGVLoose2015[_nL]/O");
    outputTree->Branch("_tauPOGLoose2015",              &_tauPOGLoose2015,              "_tauPOGLoose2015[_nL]/O");
    outputTree->Branch("_tauPOGMedium2015",             &_tauPOGMedium2015,             "_tauPOGMedium2015[_nL]/O");
    outputTree->Branch("_tauPOGTight2015",              &_tauPOGTight2015,              "_tauPOGTight2015[_nL]/O");
    outputTree->Branch("_tauPOGVTight2015",             &_tauPOGVTight2015,             "_tauPOGVTight2015[_nL]/O");
    outputTree->Branch("_tauVLooseMvaNew2015",          &_tauVLooseMvaNew2015,          "_tauVLooseMvaNew2015[_nL]/O");
    outputTree->Branch("_tauLooseMvaNew2015",           &_tauLooseMvaNew2015,           "_tauLooseMvaNew2015[_nL]/O");
    outputTree->Branch("_tauMediumMvaNew2015",          &_tauMediumMvaNew2015,          "_tauMediumMvaNew2015[_nL]/O");
    outputTree->Branch("_tauTightMvaNew2015",           &_tauTightMvaNew2015,           "_tauTightMvaNew2015[_nL]/O");
    outputTree->Branch("_tauVTightMvaNew2015",          &_tauVTightMvaNew2015,          "_tauVTightMvaNew2015[_nL]/O");
    outputTree->Branch("_tauPOGVVLoose2017v2",          &_tauPOGVVLoose2017v2,          "_tauPOGVVLoose2017v2[_nL]/O");
    outputTree->Branch("_tauPOGVTight2017v2",           &_tauPOGVTight2017v2,           "_tauPOGVTight2017v2[_nL]/O");
    outputTree->Branch("_tauPOGVVTight2017v2",          &_tauPOGVVTight2017v2,          "_tauPOGVVTight2017v2[_nL]/O");
    outputTree->Branch("_tauVLooseMvaNew2017v2",        &_tauVLooseMvaNew2017v2,        "_tauVLooseMvaNew2017v2[_nL]/O");
    outputTree->Branch("_tauLooseMvaNew2017v2",         &_tauLooseMvaNew2017v2,         "_tauLooseMvaNew2017v2[_nL]/O");
    outputTree->Branch("_tauMediumMvaNew2017v2",        &_tauMediumMvaNew2017v2,        "_tauMediumMvaNew2017v2[_nL]/O");
    outputTree->Branch("_tauTightMvaNew2017v2",         &_tauTightMvaNew2017v2,         "_tauTightMvaNew2017v2[_nL]/O");
    outputTree->Branch("_tauVTightMvaNew2017v2",        &_tauVTightMvaNew2017v2,        "_tauVTightMvaNew2017v2[_nL]/O");
	outputTree->Branch("_tauMuonVetoLoose",             &_tauMuonVetoLoose,             "_tauMuonVetoLoose[_nL]/O");
    outputTree->Branch("_tauMuonVetoTight",             &_tauMuonVetoTight,             "_tauMuonVetoTight[_nL]/O");
    outputTree->Branch("_tauEleVetoVLoose",             &_tauEleVetoVLoose,             "_tauEleVetoVLoose[_nL]/O");
    outputTree->Branch("_tauEleVetoLoose",              &_tauEleVetoLoose,              "_tauEleVetoLoose[_nL]/O");
    outputTree->Branch("_tauEleVetoMedium",             &_tauEleVetoMedium,             "_tauEleVetoMedium[_nL]/O");
    outputTree->Branch("_tauEleVetoTight",              &_tauEleVetoTight,              "_tauEleVetoTight[_nL]/O");
    outputTree->Branch("_tauEleVetoVTight",             &_tauEleVetoVTight,             "_tauEleVetoVTight[_nL]/O"); 

    outputTree->Branch("_relIso",                       &_relIso,                       "_relIso[_nLight]/D");
    outputTree->Branch("_relIso0p4",                    &_relIso0p4,                    "_relIso0p4[_nLight]/D");
    outputTree->Branch("_relIso0p4MuDeltaBeta",         &_relIso0p4MuDeltaBeta,         "_relIso0p4MuDeltaBeta[_nMu]/D");
    outputTree->Branch("_miniIso",                      &_miniIso,                      "_miniIso[_nLight]/D");
    outputTree->Branch("_miniIsoCharged",               &_miniIsoCharged,               "_miniIsoCharged[_nLight]/D");
    outputTree->Branch("_ptRel",                        &_ptRel,                        "_ptRel[_nLight]/D");
    outputTree->Branch("_ptRatio",                      &_ptRatio,                      "_ptRatio[_nLight]/D");
    outputTree->Branch("_closestJetCsvV2",              &_closestJetCsvV2,              "_closestJetCsvV2[_nLight]/D");
    outputTree->Branch("_closestJetDeepCsv_b",          &_closestJetDeepCsv_b,          "_closestJetDeepCsv_b[_nLight]/D");
    outputTree->Branch("_closestJetDeepCsv_bb",         &_closestJetDeepCsv_bb,         "_closestJetDeepCsv_bb[_nLight]/D");
	outputTree->Branch("_closestJetDeepFlavor_b",       &_closestJetDeepFlavor_b,       "_closestJetDeepFlavor_b[_nLight]/D");
    outputTree->Branch("_closestJetDeepFlavor_bb",      &_closestJetDeepFlavor_bb,      "_closestJetDeepFlavor_bb[_nLight]/D");
    outputTree->Branch("_closestJetDeepFlavor_lepb",    &_closestJetDeepFlavor_lepb,    "_closestJetDeepFlavor_lepb[_nLight]/D");
    outputTree->Branch("_selectedTrackMult",            &_selectedTrackMult,            "_selectedTrackMult[_nLight]/i");
    outputTree->Branch("_lMuonSegComp",                 &_lMuonSegComp,                 "_lMuonSegComp[_nMu]/D");
    outputTree->Branch("_lMuonTrackPt",                 &_lMuonTrackPt,                 "_lMuonTrackPt[_nMu]/D");
    outputTree->Branch("_lMuonTrackPtErr",              &_lMuonTrackPtErr,              "_lMuonTrackPtErr[_nMu]/D");
    outputTree->Branch("_nJets",                     &_nJets,                    "_nJets/i");
    outputTree->Branch("_jetPt",                     &_jetPt,                    "_jetPt[_nJets]/D");
    outputTree->Branch("_jetPt_JECUp",               &_jetPt_JECUp,              "_jetPt_JECUp[_nJets]/D");
    outputTree->Branch("_jetPt_JECDown",             &_jetPt_JECDown,            "_jetPt_JECDown[_nJets]/D");
    outputTree->Branch("_jetSmearedPt",              &_jetSmearedPt,             "_jetSmearedPt[_nJets]/D");
    outputTree->Branch("_jetSmearedPt_JECDown",      &_jetSmearedPt_JECDown,     "_jetSmearedPt_JECDown[_nJets]/D");
    outputTree->Branch("_jetSmearedPt_JECUp",        &_jetSmearedPt_JECUp,       "_jetSmearedPt_JECUp[_nJets]/D");
    outputTree->Branch("_jetSmearedPt_JERDown",      &_jetSmearedPt_JERDown,     "_jetSmearedPt_JERDown[_nJets]/D");
    outputTree->Branch("_jetSmearedPt_JERUp",        &_jetSmearedPt_JERUp,       "_jetSmearedPt_JERUp[_nJets]/D");
    outputTree->Branch("_jetEta",                    &_jetEta,                   "_jetEta[_nJets]/D");
    outputTree->Branch("_jetPhi",                    &_jetPhi,                   "_jetPhi[_nJets]/D");
    outputTree->Branch("_jetE",                      &_jetE,                     "_jetE[_nJets]/D");
    outputTree->Branch("_jetPt_Uncorrected",         &_jetPt_Uncorrected,        "_jetPt_Uncorrected[_nJets]/D");
    outputTree->Branch("_jetPt_L1",                  &_jetPt_L1,                 "_jetPt_L1[_nJets]/D");
    outputTree->Branch("_jetPt_L2",                  &_jetPt_L2,                 "_jetPt_L2[_nJets]/D");
    outputTree->Branch("_jetPt_L3",                  &_jetPt_L3,                 "_jetPt_L3[_nJets]/D");
    outputTree->Branch("_jetCsvV2",                  &_jetCsvV2,                 "_jetCsvV2[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_udsg",           &_jetDeepCsv_udsg,          "_jetDeepCsv_udsg[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_b",              &_jetDeepCsv_b,             "_jetDeepCsv_b[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_c",              &_jetDeepCsv_c,             "_jetDeepCsv_c[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_bb",             &_jetDeepCsv_bb,            "_jetDeepCsv_bb[_nJets]/D");
	outputTree->Branch("_jetDeepFlavor_b",           &_jetDeepFlavor_b,          "_jetDeepFlavor_b[_nJets]/D");
    outputTree->Branch("_jetDeepFlavor_bb",          &_jetDeepFlavor_bb,         "_jetDeepFlavor_bb[_nJets]/D");
    outputTree->Branch("_jetDeepFlavor_lepb",        &_jetDeepFlavor_lepb,       "_jetDeepFlavor_lepb[_nJets]/D");
    outputTree->Branch("_jetHadronFlavor",           &_jetHadronFlavor,          "_jetHadronFlavor[_nJets]/i");
    outputTree->Branch("_jetIsTight",                &_jetIsTight,               "_jetIsTight[_nJets]/O");
    outputTree->Branch("_jetIsTightLepVeto",         &_jetIsTightLepVeto,        "_jetIsTightLepVeto[_nJets]/O");
    outputTree->Branch("_jetNeutralHadronFraction",  &_jetNeutralHadronFraction, "_jetNeutralHadronFraction[_nJets]/D");
    outputTree->Branch("_jetChargedHadronFraction",  &_jetChargedHadronFraction, "_jetChargedHadronFraction[_nJets]/D");
    outputTree->Branch("_jetNeutralEmFraction",      &_jetNeutralEmFraction,     "_jetNeutralEmFraction[_nJets]/D");
    outputTree->Branch("_jetChargedEmFraction",      &_jetChargedEmFraction,     "_jetChargedEmFraction[_nJets]/D");
    outputTree->Branch("_jetHFHadronFraction",       &_jetHFHadronFraction,      "_jetHFHadronFraction[_nJets]/D");
    outputTree->Branch("_jetHFEmFraction",           &_jetHFEmFraction,          "_jetHFEmFraction[_nJets]/D");


    if( containsGeneratorInfo() ){
        outputTree->Branch("_nLheWeights",               &_nLheWeights,               "_nLheWeights/i");
        outputTree->Branch("_lheWeight",                 &_lheWeight,                 "_lheWeight[_nLheWeights]/D");
        outputTree->Branch("_weight",                    &_weight,                    "_weight/D");
        outputTree->Branch("_nPsWeights",                &_nPsWeights,                "_nPsWeights/i");
        outputTree->Branch("_psWeight",                  &_psWeight,                  "_psWeight/D");
        outputTree->Branch("_nTrueInt",                  &_nTrueInt,                  "_nTrueInt/F");
        outputTree->Branch("_lheHTIncoming",             &_lheHTIncoming,             "_lheHTIncoming/D");
        outputTree->Branch("_lIsPrompt",                 &_lIsPrompt,                 "_lIsPrompt[_nL]/O");
        outputTree->Branch("_lMatchPdgId",               &_lMatchPdgId,               "_lMatchPdgId[_nL]/I");
        outputTree->Branch("_lMatchCharge",              &_lMatchCharge,              "_lMatchCharge[_nL]/I");
        outputTree->Branch("_lMomPdgId",                 &_lMomPdgId,                 "_lMomPdgId[_nL]/I");
        outputTree->Branch("_lProvenance",               &_lProvenance,               "_lProvenance[_nL]/i");
        outputTree->Branch("_lProvenanceCompressed",     &_lProvenanceCompressed,     "_lProvenanceCompressed[_nL]/i");
        outputTree->Branch("_lProvenanceConversion",     &_lProvenanceConversion,     "_lProvenanceConversion[_nL]/i");
        outputTree->Branch("_gen_met",                   &_gen_met,                   "_gen_met/D");
        outputTree->Branch("_gen_metPhi",                &_gen_metPhi,                "_gen_metPhi/D");
        outputTree->Branch("_gen_nL",                    &_gen_nL,                    "_gen_nL/b");
        outputTree->Branch("_gen_lPt",                   &_gen_lPt,                   "_gen_lPt[_gen_nL]/D");
        outputTree->Branch("_gen_lEta",                  &_gen_lEta,                  "_gen_lEta[_gen_nL]/D");
        outputTree->Branch("_gen_lPhi",                  &_gen_lPhi,                  "_gen_lPhi[_gen_nL]/D");
        outputTree->Branch("_gen_lE",                    &_gen_lE,                    "_gen_lE[_gen_nL]/D");
        outputTree->Branch("_gen_lFlavor",               &_gen_lFlavor,               "_gen_lFlavor[_gen_nL]/i");
        outputTree->Branch("_gen_lCharge",               &_gen_lCharge,               "_gen_lCharge[_gen_nL]/I");
        outputTree->Branch("_gen_lMomPdg",               &_gen_lMomPdg,               "_gen_lMomPdg[_gen_nL]/I");
        outputTree->Branch("_gen_lIsPrompt",             &_gen_lIsPrompt,             "_gen_lIsPrompt[_gen_nL]/O");
        outputTree->Branch("_ttgEventType",              &_ttgEventType,              "_ttgEventType/b");
        outputTree->Branch("_zgEventType",               &_zgEventType,               "_zgEventType/b");
    } 

    if( !is2018() ){
       	outputTree->Branch("_prefireWeight",             &_prefireWeight,             "_prefireWeight/F");
        outputTree->Branch("_prefireWeightUp",           &_prefireWeightUp,           "_prefireWeightUp/F");
        outputTree->Branch("_prefireWeightDown",         &_prefireWeightDown,         "_prefireWeightDown/F"); 
    }

    if( containsSUSYMassInfo() ){
		outputTree->Branch("_mChi1", &_mChi1, "_mChi1/D");
		outputTree->Branch("_mChi2", &_mChi2, "_mChi2/D");
    }

    //write individual trigger decisions to output tree 
    setMapOutputBranches( outputTree, _triggerMap );

    //write individual MET filters to output tree
    setMapOutputBranches( outputTree, _MetFilterMap );
}
