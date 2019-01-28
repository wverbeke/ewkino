#include <iostream>
#include <fstream>

#include "../interface/TreeReader.h"
#include "../../Tools/interface/analysisTools.h"

TreeReader::TreeReader(TTree *tree) : fChain(nullptr) 
{
    if (tree != nullptr){
        initTree(tree);
    }
}

void TreeReader::readSamples(const std::string& list, const std::string& directory, std::vector<Sample>& sampleVector){

    //clean current sample list 
    sampleVector.clear();

    //read list of samples from file
    sampleVector = readSampleList(list, directory);

    //print sample information
    for(auto& sample : sampleVector){
        std::cout << sample << std::endl;
    }
}

void TreeReader::readSamples(const std::string& list, const std::string& directory){
    readSamples(list, directory, this->samples);
}

void TreeReader::readSamples2016(const std::string& list, const std::string& directory){
    std::cout << "########################################" << std::endl;
    std::cout << "         2016 samples                   " << std::endl;
    std::cout << "########################################" << std::endl;

    readSamples(list, directory, this->samples2016);

    //add the 2016 samples to the total sample list 
    this->samples.insert(samples.end(), samples2016.begin(), samples2016.end() );

    //check for errors
    checkSampleEraConsistency();

}

void TreeReader::readSamples2017(const std::string& list, const std::string& directory){
    std::cout << "########################################" << std::endl;
    std::cout << "         2017 samples                   " << std::endl;
    std::cout << "########################################" << std::endl;

    readSamples(list, directory, this->samples2017);

    //add the 2017 samples to the total sample list
    this->samples.insert(samples.end(), samples2017.begin(), samples2017.end() );

    //check for errors 
    checkSampleEraConsistency();
}

void TreeReader::initSample(const Sample& samp){ 

    //update current sample
    currentSample = samp;
    sampleFile = samp.getFile();
    sampleFile->cd("blackJackAndHookers");
    fChain = (TTree*) sampleFile->Get("blackJackAndHookers/blackJackAndHookersTree");
    initTree(fChain, samp.isData());
    nEntries = fChain->GetEntries();
    if(!samp.isData()){

        //read sum of simulated event weights
        TH1D* hCounter = new TH1D("hCounter", "Events counter", 1, 0, 1);
        hCounter->Read("hCounter"); 
        double sumSimulatedEventWeights = hCounter->GetBinContent(1);
        delete hCounter;

        //event weights set with lumi depending on sample's era 
        double dataLumi;
        if( is2016() ){
            dataLumi = lumi2016;
        } else {
            dataLumi = lumi2017;
        } 
        scale = samp.getXSec()*dataLumi*1000/sumSimulatedEventWeights;       //xSec*lumi divided by total sum of simulated event weights
    }
}

void TreeReader::initSample(){ //initialize the next sample in the list 
    initSample(samples[++currentSampleIndex]);
}


void TreeReader::GetEntry(const Sample& samp, long unsigned entry)
{
    if (!fChain) return;
    fChain->GetEntry(entry);
    //Set up correct weights
    if(!samp.isData() ) weight = _weight*scale; //MC
    else weight = 1;                            //data
}

void TreeReader::GetEntry(long unsigned entry){    //currently initialized sample when running serial
    GetEntry(samples[currentSampleIndex], entry);
}

void TreeReader::initTree(TTree *tree, const bool isData)
{
    // Set branch addresses and branch pointers
    if (!tree) return;
    fChain = tree;
    fChain->SetMakeClass(1);

    fChain->SetBranchAddress("_runNb", &_runNb, &b__runNb);
    fChain->SetBranchAddress("_lumiBlock", &_lumiBlock, &b__lumiBlock);
    fChain->SetBranchAddress("_eventNb", &_eventNb, &b__eventNb);
    fChain->SetBranchAddress("_nVertex", &_nVertex, &b__nVertex);    
    fChain->SetBranchAddress("_passTrigger_e", &_passTrigger_e, &b__passTrigger_e);
    fChain->SetBranchAddress("_passTrigger_ee", &_passTrigger_ee, &b__passTrigger_ee);
    fChain->SetBranchAddress("_passTrigger_eee", &_passTrigger_eee, &b__passTrigger_eee);
    fChain->SetBranchAddress("_passTrigger_em", &_passTrigger_em, &b__passTrigger_em);
    fChain->SetBranchAddress("_passTrigger_m", &_passTrigger_m, &b__passTrigger_m);
    fChain->SetBranchAddress("_passTrigger_eem", &_passTrigger_eem, &b__passTrigger_eem);
    fChain->SetBranchAddress("_passTrigger_mm", &_passTrigger_mm, &b__passTrigger_mm);
    fChain->SetBranchAddress("_passTrigger_emm", &_passTrigger_emm, &b__passTrigger_emm);
    fChain->SetBranchAddress("_passTrigger_mmm", &_passTrigger_mmm, &b__passTrigger_mmm);
    fChain->SetBranchAddress("_passTrigger_et", &_passTrigger_et, &b__passTrigger_et);
    fChain->SetBranchAddress("_passTrigger_mt", &_passTrigger_mt, &b__passTrigger_mt);
    fChain->SetBranchAddress("_passMETFilters", &_passMETFilters, &b__passMETFilters);
    //TEMPORARY FOR CHECK, CAN BE REMOVED LATER
    fChain->SetBranchAddress("_Flag_BadPFMuonFilter", &_Flag_BadPFMuonFilter, &b__Flag_BadPFMuonFilter);
    fChain->SetBranchAddress("_Flag_BadPFMuonFilter", &_Flag_BadPFMuonFilter, &b__Flag_BadPFMuonFilter);
    ///////////////////////////////////////////
    fChain->SetBranchAddress("_nL", &_nL, &b__nL);
    fChain->SetBranchAddress("_nMu", &_nMu, &b__nMu);
    fChain->SetBranchAddress("_nEle", &_nEle, &b__nEle);
    fChain->SetBranchAddress("_nLight", &_nLight, &b__nLight);
    fChain->SetBranchAddress("_nTau", &_nTau, &b__nTau);
    fChain->SetBranchAddress("_lPt", _lPt, &b__lPt);
    fChain->SetBranchAddress("_lEta", _lEta, &b__lEta);
    fChain->SetBranchAddress("_lEtaSC", _lEtaSC, &b__lEtaSC);
    fChain->SetBranchAddress("_lPhi", _lPhi, &b__lPhi);
    fChain->SetBranchAddress("_lE", _lE, &b__lE);
    fChain->SetBranchAddress("_lFlavor", _lFlavor, &b__lFlavor);
    fChain->SetBranchAddress("_lCharge", _lCharge, &b__lCharge);
    fChain->SetBranchAddress("_dxy", _dxy, &b__dxy);
    fChain->SetBranchAddress("_dz", _dz, &b__dz);
    fChain->SetBranchAddress("_3dIP", _3dIP, &b__3dIP);
    fChain->SetBranchAddress("_3dIPSig", _3dIPSig, &b__3dIPSig);
    fChain->SetBranchAddress("_lElectronMva", _lElectronMva, &b__lElectronMva);
    fChain->SetBranchAddress("_lElectronMvaHZZ", _lElectronMvaHZZ, &b__lElectronMvaHZZ);
    fChain->SetBranchAddress("_lElectronMvaFall17Iso", _lElectronMvaFall17Iso, &b__lElectronMvaFall17Iso);
    fChain->SetBranchAddress("_lElectronMvaFall17NoIso", _lElectronMvaFall17NoIso, &b__lElectronMvaFall17NoIso);
    fChain->SetBranchAddress("_lElectronPassEmu", _lElectronPassEmu, &b__lElectronPassEmu);
    fChain->SetBranchAddress("_lElectronPassConvVeto", _lElectronPassConvVeto, &b__lElectronPassConvVeto);
    fChain->SetBranchAddress("_lElectronChargeConst", _lElectronChargeConst, &b__lElectronChargeConst);
    fChain->SetBranchAddress("_lElectronMissingHits", _lElectronMissingHits, &b__lElectronMissingHits);
    fChain->SetBranchAddress("_leptonMvaSUSY16", _leptonMvaSUSY16, &b__leptonMvaSUSY16);
    fChain->SetBranchAddress("_leptonMvaTTH16", _leptonMvaTTH16, &b__leptonMvaTTH16);
    fChain->SetBranchAddress("_leptonMvatZqTTV16", _leptonMvatZqTTV16, &b__leptonMvatZqTTV16);
    fChain->SetBranchAddress("_leptonMvaSUSY17", _leptonMvaSUSY17, &b__leptonMvaSUSY17);
    fChain->SetBranchAddress("_leptonMvaTTH17", _leptonMvaTTH17, &b__leptonMvaTTH17);
    fChain->SetBranchAddress("_leptonMvatZqTTV17", _leptonMvatZqTTV17, &b__leptonMvatZqTTV17);
    fChain->SetBranchAddress("_lHNLoose", _lHNLoose, &b__lHNLoose);
    fChain->SetBranchAddress("_lHNFO", _lHNFO, &b__lHNFO);
    fChain->SetBranchAddress("_lHNTight", _lHNTight, &b__lHNTight);
    fChain->SetBranchAddress("_lEwkLoose", _lEwkLoose, &b__lEwkLoose);
    fChain->SetBranchAddress("_lEwkFO", _lEwkFO, &b__lEwkFO);
    fChain->SetBranchAddress("_lEwkTight", _lEwkTight, &b__lEwkTight);
    fChain->SetBranchAddress("_lPOGVeto", _lPOGVeto, &b__lPOGVeto);
    fChain->SetBranchAddress("_lPOGLoose", _lPOGLoose, &b__lPOGLoose);
    fChain->SetBranchAddress("_lPOGMedium", _lPOGMedium, &b__lPOGMedium);
    fChain->SetBranchAddress("_lPOGTight", _lPOGTight, &b__lPOGTight);
    fChain->SetBranchAddress("_tauMuonVeto", _tauMuonVeto, &b__tauMuonVeto);
    fChain->SetBranchAddress("_tauEleVeto", _tauEleVeto, &b__tauEleVeto);
    fChain->SetBranchAddress("_decayModeFindingNew", _decayModeFindingNew, &b__decayModeFindingNew);
    fChain->SetBranchAddress("_tauVLooseMvaNew", _tauVLooseMvaNew, &b__tauVLooseMvaNew);
    fChain->SetBranchAddress("_tauLooseMvaNew", _tauLooseMvaNew, &b__tauLooseMvaNew);
    fChain->SetBranchAddress("_tauMediumMvaNew", _tauMediumMvaNew, &b__tauMediumMvaNew);
    fChain->SetBranchAddress("_tauTightMvaNew", _tauTightMvaNew, &b__tauTightMvaNew);
    fChain->SetBranchAddress("_tauVTightMvaNew", _tauVTightMvaNew, &b__tauVTightMvaNew);
    fChain->SetBranchAddress("_tauVTightMvaOld", _tauVTightMvaOld, &b__tauVTightMvaOld);
    fChain->SetBranchAddress("_tauAgainstElectronMVA6Raw", _tauAgainstElectronMVA6Raw, &b__tauAgainstElectronMVA6Raw);
    fChain->SetBranchAddress("_tauCombinedIsoDBRaw3Hits", _tauCombinedIsoDBRaw3Hits, &b__tauCombinedIsoDBRaw3Hits);
    fChain->SetBranchAddress("_tauIsoMVAPWdR03oldDMwLT", _tauIsoMVAPWdR03oldDMwLT, &b__tauIsoMVAPWdR03oldDMwLT);
    fChain->SetBranchAddress("_tauIsoMVADBdR03oldDMwLT", _tauIsoMVADBdR03oldDMwLT, &b__tauIsoMVADBdR03oldDMwLT);
    fChain->SetBranchAddress("_tauIsoMVADBdR03newDMwLT", _tauIsoMVADBdR03newDMwLT, &b__tauIsoMVADBdR03newDMwLT);
    fChain->SetBranchAddress("_tauIsoMVAPWnewDMwLT", _tauIsoMVAPWnewDMwLT, &b__tauIsoMVAPWnewDMwLT);
    fChain->SetBranchAddress("_tauIsoMVAPWoldDMwLT", _tauIsoMVAPWoldDMwLT, &b__tauIsoMVAPWoldDMwLT);
    fChain->SetBranchAddress("_relIso", _relIso, &b__relIso);
    fChain->SetBranchAddress("_relIso0p4", _relIso0p4, &b__relIso0p4);
    fChain->SetBranchAddress("_relIso0p4MuDeltaBeta", _relIso0p4MuDeltaBeta, &b__relIso0p4MuDeltaBeta);
    fChain->SetBranchAddress("_miniIso", _miniIso, &b__miniIso);
    fChain->SetBranchAddress("_miniIsoCharged", _miniIsoCharged, &b__miniIsoCharged);
    fChain->SetBranchAddress("_ptRel", _ptRel, &b__ptRel);
    fChain->SetBranchAddress("_ptRatio", _ptRatio, &b__ptRatio);
    fChain->SetBranchAddress("_closestJetCsvV2", _closestJetCsvV2, &b__closestJetCsvV2);
    fChain->SetBranchAddress("_closestJetDeepCsv_b", _closestJetDeepCsv_b, &b__closestJetDeepCsv_b);
    fChain->SetBranchAddress("_closestJetDeepCsv_bb", _closestJetDeepCsv_bb, &b__closestJetDeepCsv_bb);
    fChain->SetBranchAddress("_selectedTrackMult", _selectedTrackMult, &b__selectedTrackMult);
    fChain->SetBranchAddress("_lMuonSegComp", _lMuonSegComp, &b__lMuonSegComp);
    fChain->SetBranchAddress("_lMuonTrackPt", _lMuonTrackPt, &b__lMuonTrackPt);
    fChain->SetBranchAddress("_lMuonTrackPtErr", _lMuonTrackPtErr, &b__lMuonTrackPtErr);
    fChain->SetBranchAddress("_nJets", &_nJets, &b__nJets);
    fChain->SetBranchAddress("_jetPt", _jetPt, &b__jetPt);
    fChain->SetBranchAddress("_jetPt_JECUp", _jetPt_JECUp, &b__jetPt_JECUp);
    fChain->SetBranchAddress("_jetPt_JECDown", _jetPt_JECDown, &b__jetPt_JECDown);
    fChain->SetBranchAddress("_jetEta", _jetEta, &b__jetEta);
    fChain->SetBranchAddress("_jetPhi", _jetPhi, &b__jetPhi);
    fChain->SetBranchAddress("_jetE", _jetE, &b__jetE);
    fChain->SetBranchAddress("_jetPt_Uncorrected",_jetPt_Uncorrected, &b__jetPt_Uncorrected);
    fChain->SetBranchAddress("_jetPt_L1", _jetPt_L1, &b__jetPt_L1);
    fChain->SetBranchAddress("_jetPt_L2", _jetPt_L2, &b__jetPt_L2);
    fChain->SetBranchAddress("_jetPt_L3", _jetPt_L3, &b__jetPt_L3);
    fChain->SetBranchAddress("_jetCsvV2", _jetCsvV2, &b__jetCsvV2);
    fChain->SetBranchAddress("_jetDeepCsv_udsg", _jetDeepCsv_udsg, &b__jetDeepCsv_udsg);
    fChain->SetBranchAddress("_jetDeepCsv_b", _jetDeepCsv_b, &b__jetDeepCsv_b);
    fChain->SetBranchAddress("_jetDeepCsv_c", _jetDeepCsv_c, &b__jetDeepCsv_c);
    fChain->SetBranchAddress("_jetDeepCsv_bb", _jetDeepCsv_bb, &b__jetDeepCsv_bb);
    fChain->SetBranchAddress("_jetHadronFlavor", _jetHadronFlavor, &b__jetHadronFlavor);
    fChain->SetBranchAddress("_jetIsLoose", _jetIsLoose, &b__jetIsLoose);
    fChain->SetBranchAddress("_jetIsTight", _jetIsTight, &b__jetIsTight);
    fChain->SetBranchAddress("_jetIsTightLepVeto", _jetIsTightLepVeto, &b__jetIsTightLepVeto);
    fChain->SetBranchAddress("_jetNeutralHadronFraction", _jetNeutralHadronFraction, &b__jetNeutralHadronFraction);
    fChain->SetBranchAddress("_jetChargedHadronFraction", _jetChargedHadronFraction, &b__jetChargedHadronFraction);
    fChain->SetBranchAddress("_jetNeutralEmFraction", _jetNeutralEmFraction, &b__jetNeutralEmFraction);
    fChain->SetBranchAddress("_jetChargedEmFraction", _jetChargedEmFraction, &b__jetChargedEmFraction);
    fChain->SetBranchAddress("_jetHFHadronFraction", _jetHFHadronFraction, &b__jetHFHadronFraction);
    fChain->SetBranchAddress("_jetHFEmFraction", _jetHFEmFraction, &b__jetHFEmFraction);
    fChain->SetBranchAddress("_met", &_met, &b__met);
    fChain->SetBranchAddress("_metJECDown", &_metJECDown, &b__metJECDown);
    fChain->SetBranchAddress("_metJECUp", &_metJECUp, &b__metJECUp);
    fChain->SetBranchAddress("_metUnclDown", &_metUnclDown, &b__metUnclDown);
    fChain->SetBranchAddress("_metUnclUp", &_metUnclUp, &b__metUnclUp);
    fChain->SetBranchAddress("_metPhi", &_metPhi, &b__metPhi);
    fChain->SetBranchAddress("_metPhiJECDown", &_metPhiJECDown, &b__metPhiJECDown);
    fChain->SetBranchAddress("_metPhiJECUp", &_metPhiJECUp, &b__metPhiJECUp);
    fChain->SetBranchAddress("_metPhiUnclDown", &_metPhiUnclDown, &b__metPhiUnclDown);
    fChain->SetBranchAddress("_metPhiUnclUp", &_metPhiUnclUp, &b__metPhiUnclUp);
    fChain->SetBranchAddress("_metSignificance", &_metSignificance, &b__metSignificance);

    if(!isData){
        fChain->SetBranchAddress("_weight", &_weight, &b__weight);
        fChain->SetBranchAddress("_nLheWeights", &_nLheWeights, &b__nLheWeights);
        fChain->SetBranchAddress("_lheWeight", _lheWeight, &b__lheWeight);
        fChain->SetBranchAddress("_nTrueInt", &_nTrueInt, &b__nTrueInt);
        fChain->SetBranchAddress("_gen_met", &_gen_met, &b__gen_met);
        fChain->SetBranchAddress("_gen_metPhi", &_gen_metPhi, &b__gen_metPhi);
        fChain->SetBranchAddress("_gen_nL", &_gen_nL, &b__gen_nL);
        fChain->SetBranchAddress("_gen_lPt", _gen_lPt, &b__gen_lPt);
        fChain->SetBranchAddress("_gen_lEta", _gen_lEta, &b__gen_lEta);
        fChain->SetBranchAddress("_gen_lPhi", _gen_lPhi, &b__gen_lPhi);
        fChain->SetBranchAddress("_gen_lE", _gen_lE, &b__gen_lE);
        fChain->SetBranchAddress("_gen_lFlavor", _gen_lFlavor, &b__gen_lFlavor);
        fChain->SetBranchAddress("_gen_lCharge", _gen_lCharge, &b__gen_lCharge);
        fChain->SetBranchAddress("_gen_lMomPdg", _gen_lMomPdg, &b__gen_lMomPdg);
        fChain->SetBranchAddress("_gen_lIsPrompt", _gen_lIsPrompt, &b__gen_lIsPrompt);
        fChain->SetBranchAddress("_lIsPrompt", _lIsPrompt, &b__lIsPrompt);
        fChain->SetBranchAddress("_lMatchPdgId", _lMatchPdgId, &b__lMatchPdgId);
        fChain->SetBranchAddress("_lMomPdgId",  _lMomPdgId, &b__lMomPdgId);
        fChain->SetBranchAddress("_lProvenance", _lProvenance, &b__lProvenance);
        fChain->SetBranchAddress("_lProvenanceCompressed", _lProvenanceCompressed, &b__lProvenanceCompressed);
        fChain->SetBranchAddress("_lProvenanceConversion", _lProvenanceConversion, &b__lProvenanceConversion);
        fChain->SetBranchAddress("_ttgEventType", &_ttgEventType, &b__ttgEventType);
        fChain->SetBranchAddress("_zgEventType", &_zgEventType, &b__zgEventType);
        fChain->SetBranchAddress("_gen_HT", &_gen_HT, &b__gen_HT);
    }
}

void TreeReader::setOutputTree(TTree* outputTree, const bool isData){
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
    //TEMPORARY FOR CHECK, CAN BE REMOVED LATER
    outputTree->Branch("_Flag_BadPFMuonFilter", &_Flag_BadPFMuonFilter, "_Flag_BadPFMuonFilter/O");
    outputTree->Branch("_Flag_BadChargedCandidateFilter", &_Flag_BadChargedCandidateFilter, "_Flag_BadChargedCandidateFilter/O");
    //////////////////////////////////////////
    outputTree->Branch("_nL",                           &_nL,                           "_nL/b");
    outputTree->Branch("_nMu",                          &_nMu,                          "_nMu/b");
    outputTree->Branch("_nEle",                         &_nEle,                         "_nEle/b");
    outputTree->Branch("_nLight",                       &_nLight,                       "_nLight/b");
    outputTree->Branch("_nTau",                         &_nTau,                         "_nTau/b");
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
    outputTree->Branch("_lElectronMva",                 &_lElectronMva,                 "_lElectronMva[_nLight]/F");
    outputTree->Branch("_lElectronMvaHZZ",              &_lElectronMvaHZZ,              "_lElectronMvaHZZ[_nLight]/F");
    outputTree->Branch("_lElectronMvaFall17Iso",        &_lElectronMvaFall17Iso,        "_lElectronMvaFall17Iso[_nLight]/F");
    outputTree->Branch("_lElectronMvaFall17NoIso",      &_lElectronMvaFall17NoIso,      "_lElectronMvaFall17NoIso[_nLight]/F");
    outputTree->Branch("_lElectronPassEmu",             &_lElectronPassEmu,             "_lElectronPassEmu[_nLight]/O");
    outputTree->Branch("_lElectronPassConvVeto",        &_lElectronPassConvVeto,        "_lElectronPassConvVeto[_nLight]/O");
    outputTree->Branch("_lElectronChargeConst",         &_lElectronChargeConst,         "_lElectronChargeConst[_nLight]/O");
    outputTree->Branch("_lElectronMissingHits",         &_lElectronMissingHits,         "_lElectronMissingHits[_nLight]/i");
    outputTree->Branch("_leptonMvaSUSY16",              &_leptonMvaSUSY16,              "_leptonMvaSUSY16[_nLight]/D");
    outputTree->Branch("_leptonMvaTTH16",               &_leptonMvaTTH16,               "_leptonMvaTTH16[_nLight]/D");
    outputTree->Branch("_leptonMvatZqTTV16",            &_leptonMvatZqTTV16,            "_leptonMvatZqTTV16[_nLight]/D");
    outputTree->Branch("_leptonMvaSUSY17",              &_leptonMvaSUSY17,              "_leptonMvaSUSY17[_nLight]/D");
    outputTree->Branch("_leptonMvaTTH17",               &_leptonMvaTTH17,               "_leptonMvaTTH17[_nLight]/D");
    outputTree->Branch("_leptonMvatZqTTV17",            &_leptonMvatZqTTV17,            "_leptonMvatZqTTV17[_nLight]/D");
    outputTree->Branch("_lHNLoose",                     &_lHNLoose,                     "_lHNLoose[_nLight]/O");
    outputTree->Branch("_lHNFO",                        &_lHNFO,                        "_lHNFO[_nLight]/O");
    outputTree->Branch("_lHNTight",                     &_lHNTight,                     "_lHNTight[_nLight]/O");
    outputTree->Branch("_lEwkLoose",                    &_lEwkLoose,                    "_lEwkLoose[_nL]/O");
    outputTree->Branch("_lEwkFO",                       &_lEwkFO,                       "_lEwkFO[_nL]/O");
    outputTree->Branch("_lEwkTight",                    &_lEwkTight,                    "_lEwkTight[_nL]/O");
    outputTree->Branch("_lPOGVeto",                     &_lPOGVeto,                     "_lPOGVeto[_nL]/O");
    outputTree->Branch("_lPOGLoose",                    &_lPOGLoose,                    "_lPOGLoose[_nL]/O");
    outputTree->Branch("_lPOGMedium",                   &_lPOGMedium,                   "_lPOGMedium[_nL]/O");
    outputTree->Branch("_lPOGTight",                    &_lPOGTight,                    "_lPOGTight[_nL]/O");
    outputTree->Branch("_tauMuonVeto",                  &_tauMuonVeto,                  "_tauMuonVeto[_nL]/O");
    outputTree->Branch("_tauEleVeto",                   &_tauEleVeto,                   "_tauEleVeto[_nL]/O");
    outputTree->Branch("_decayModeFindingNew",          &_decayModeFindingNew,          "_decayModeFindingNew[_nL]/O");
    outputTree->Branch("_tauVLooseMvaNew",              &_tauVLooseMvaNew,              "_tauVLooseMvaNew[_nL]/O");
    outputTree->Branch("_tauLooseMvaNew",               &_tauLooseMvaNew,               "_tauLooseMvaNew[_nL]/O");
    outputTree->Branch("_tauMediumMvaNew",              &_tauMediumMvaNew,              "_tauMediumMvaNew[_nL]/O");
    outputTree->Branch("_tauTightMvaNew",               &_tauTightMvaNew,               "_tauTightMvaNew[_nL]/O");
    outputTree->Branch("_tauVTightMvaNew",              &_tauVTightMvaNew,              "_tauVTightMvaNew[_nL]/O");
    outputTree->Branch("_tauVTightMvaOld",              &_tauVTightMvaOld,              "_tauVTightMvaOld[_nL]/O");
    outputTree->Branch("_tauAgainstElectronMVA6Raw",    &_tauAgainstElectronMVA6Raw,    "_tauAgainstElectronMVA6Raw[_nL]/D");
    outputTree->Branch("_tauCombinedIsoDBRaw3Hits",     &_tauCombinedIsoDBRaw3Hits,     "_tauCombinedIsoDBRaw3Hits[_nL]/D");
    outputTree->Branch("_tauIsoMVAPWdR03oldDMwLT",      &_tauIsoMVAPWdR03oldDMwLT,      "_tauIsoMVAPWdR03oldDMwLT[_nL]/D");
    outputTree->Branch("_tauIsoMVADBdR03oldDMwLT",      &_tauIsoMVADBdR03oldDMwLT,      "_tauIsoMVADBdR03oldDMwLT[_nL]/D");
    outputTree->Branch("_tauIsoMVADBdR03newDMwLT",      &_tauIsoMVADBdR03newDMwLT,      "_tauIsoMVADBdR03newDMwLT[_nL]/D");
    outputTree->Branch("_tauIsoMVAPWnewDMwLT",          &_tauIsoMVAPWnewDMwLT,          "_tauIsoMVAPWnewDMwLT[_nL]/D");
    outputTree->Branch("_tauIsoMVAPWoldDMwLT",          &_tauIsoMVAPWoldDMwLT,          "_tauIsoMVAPWoldDMwLT[_nL]/D"); 
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
    outputTree->Branch("_selectedTrackMult",            &_selectedTrackMult,            "_selectedTrackMult[_nLight]/i");
    outputTree->Branch("_lMuonSegComp",                 &_lMuonSegComp,                 "_lMuonSegComp[_nMu]/D");
    outputTree->Branch("_lMuonTrackPt",                 &_lMuonTrackPt,                 "_lMuonTrackPt[_nMu]/D");
    outputTree->Branch("_lMuonTrackPtErr",              &_lMuonTrackPtErr,              "_lMuonTrackPtErr[_nMu]/D");
    outputTree->Branch("_nJets",                     &_nJets,                    "_nJets/b");
    outputTree->Branch("_jetPt",                     &_jetPt,                    "_jetPt[_nJets]/D");
    outputTree->Branch("_jetPt_JECUp",               &_jetPt_JECUp,              "_jetPt_JECUp[_nJets]/D");
    outputTree->Branch("_jetPt_JECDown",             &_jetPt_JECDown,            "_jetPt_JECDown[_nJets]/D");
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
    outputTree->Branch("_jetHadronFlavor",           &_jetHadronFlavor,          "_jetHadronFlavor[_nJets]/i");
    outputTree->Branch("_jetIsLoose",                &_jetIsLoose,               "_jetIsLoose[_nJets]/O");
    outputTree->Branch("_jetIsTight",                &_jetIsTight,               "_jetIsTight[_nJets]/O");
    outputTree->Branch("_jetIsTightLepVeto",         &_jetIsTightLepVeto,        "_jetIsTightLepVeto[_nJets]/O");
    outputTree->Branch("_jetNeutralHadronFraction",  &_jetNeutralHadronFraction, "_jetNeutralHadronFraction[_nJets]/D");
    outputTree->Branch("_jetChargedHadronFraction",  &_jetChargedHadronFraction, "_jetChargedHadronFraction[_nJets]/D");
    outputTree->Branch("_jetNeutralEmFraction",      &_jetNeutralEmFraction,     "_jetNeutralEmFraction[_nJets]/D");
    outputTree->Branch("_jetChargedEmFraction",      &_jetChargedEmFraction,     "_jetChargedEmFraction[_nJets]/D");
    outputTree->Branch("_jetHFHadronFraction",       &_jetHFHadronFraction,      "_jetHFHadronFraction[_nJets]/D");
    outputTree->Branch("_jetHFEmFraction",           &_jetHFEmFraction,          "_jetHFEmFraction[_nJets]/D");


    if(!isData){
        outputTree->Branch("_nLheWeights",               &_nLheWeights,               "_nLheWeights/b");
        outputTree->Branch("_lheWeight",                 &_lheWeight,                 "_lheWeight[_nLheWeights]/D");
        outputTree->Branch("_weight",                    &_weight,                    "_weight/D");
        outputTree->Branch("_lIsPrompt",                 &_lIsPrompt,                 "_lIsPrompt[_nL]/O");
        outputTree->Branch("_lMatchPdgId",               &_lMatchPdgId,               "_lMatchPdgId[_nL]/I");
        outputTree->Branch("_lMomPdgId",                  &_lMomPdgId,                "_lMomPdgId[_nL]/I");
        outputTree->Branch("_lProvenance",               &_lProvenance,               "_lProvenance[_nL]/i");
        outputTree->Branch("_lProvenanceCompressed",     &_lProvenanceCompressed,     "_lProvenanceCompressed[_nL]/i");
        outputTree->Branch("_lProvenanceConversion",     &_lProvenanceConversion,     "_lProvenanceConversion[_nL]/i");
        outputTree->Branch("_nTrueInt",                  &_nTrueInt,                  "_nTrueInt/F");
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
        outputTree->Branch("_gen_HT",                    &_gen_HT,                    "_gen_HT/D");
    }
}
