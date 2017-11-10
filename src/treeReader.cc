#include <iostream>
#include <fstream>

#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"

treeReader::treeReader(TTree *tree) : fChain(nullptr) 
{
    if (tree != nullptr){
        initTree(tree);
    }
}

void treeReader::readSamples(const std::string& list){
    samples.clear();    //clear current sample list
    //read samples and cross sections from txt file
    std::ifstream file(list);
    std::string line;
    while(std::getline(file, line)){
        samples.push_back(tools::readSampleLine(line));
    }
    file.close();       //close file after usage
    for( auto it = samples.cbegin(); it != samples.cend(); ++it){
        std::cout << std::get<0>(*it) << "     " << std::get<1>(*it) << "      " << std::get<2>(*it) << std::endl;
    }
}

void treeReader::initSample(const unsigned period){                             //0 = 2016, 1 = 2017, > 1 = combined
    isData = (currentSample == 0);
    sampleFile = std::make_shared<TFile>("../../ntuples_ewkino/"+ (const TString&) std::get<1>(samples[currentSample]),"read"); 
    sampleFile->cd("blackJackAndHookers");
    fChain = (TTree*) sampleFile->Get("blackJackAndHookers/blackJackAndHookersTree");
    initTree(fChain, isData);
    nEntries = fChain->GetEntries();
    if(!isData){
        TH1D* hCounter = new TH1D("hCounter", "Events counter", 1, 0, 1);
        hCounter->Read("hCounter"); 
        double dataLumi;
        if(period == 0) dataLumi = lumi2016;
        else if(period == 1) dataLumi = lumi2017;
        else dataLumi = lumi2016 + lumi2017;
        scale = std::get<2>(samples[currentSample])*dataLumi*1000/hCounter->GetBinContent(1);       //xSec*lumi divided by number of events
        delete hCounter;
    }
    ++currentSample;    //increment the current sample for the next iteration
}

void treeReader::GetEntry(long unsigned entry)
{
    if (!fChain) return;
    fChain->GetEntry(entry);
    //Set up correct weights
    if(!isData) weight = _weight*scale; //MC
    else weight = 1;                               //data
}

void treeReader::initTree(TTree *tree, const bool isData)
{
    // Set branch addresses and branch pointers
    if (!tree) return;
    fChain = tree;
    fChain->SetMakeClass(1);

    fChain->SetBranchAddress("_runNb", &_runNb, &b__runNb);
    fChain->SetBranchAddress("_lumiBlock", &_lumiBlock, &b__lumiBlock);
    fChain->SetBranchAddress("_eventNb", &_eventNb, &b__eventNb);
    fChain->SetBranchAddress("_nVertex", &_nVertex, &b__nVertex);    
    if(isData){         //Temporarily only store 2017 triggers for data, to be updated when 2017 MC is available
        fChain->SetBranchAddress("_2017_e", &_2017_e, &b__2017_e);
        fChain->SetBranchAddress("_HLT_Ele35_WPTight_Gsf", &_HLT_Ele35_WPTight_Gsf, &b__HLT_Ele35_WPTight_Gsf);
        fChain->SetBranchAddress("_HLT_Ele35_WPTight_Gsf_prescale", &_HLT_Ele35_WPTight_Gsf_prescale, &b__HLT_Ele35_WPTight_Gsf_prescale);
        fChain->SetBranchAddress("_HLT_Ele40_WPTight_Gsf", &_HLT_Ele40_WPTight_Gsf, &b__HLT_Ele40_WPTight_Gsf);
        fChain->SetBranchAddress("_HLT_Ele40_WPTight_Gsf_prescale", &_HLT_Ele40_WPTight_Gsf_prescale, &b__HLT_Ele40_WPTight_Gsf_prescale);
        fChain->SetBranchAddress("_2017_ee", &_2017_ee, &b__2017_ee);
        fChain->SetBranchAddress("_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350", &_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350, &b__HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350);
        fChain->SetBranchAddress("_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350_prescale", &_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350_prescale, &b__HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350_prescale);
        fChain->SetBranchAddress("_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL", &_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL, &b__HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL);
        fChain->SetBranchAddress("_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_prescale", &_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_prescale, &b__HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_prescale);
        fChain->SetBranchAddress("_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ", &_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ, &b__HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ);
        fChain->SetBranchAddress("_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale", &_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale, &b__HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale);
        fChain->SetBranchAddress("_2017_eee", &_2017_eee, &b__2017_eee);
        fChain->SetBranchAddress("_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL", &_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL, &b__HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL);
        fChain->SetBranchAddress("_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_prescale", &_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_prescale, &b__HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_prescale);
        fChain->SetBranchAddress("_2017_em", &_2017_em, &b__2017_em);
        fChain->SetBranchAddress("_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ", &_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ, &b__HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ);
        fChain->SetBranchAddress("_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ_prescale", &_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ_prescale, &b__HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ_prescale);
        fChain->SetBranchAddress("_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ", &_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ, &b__HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ);
        fChain->SetBranchAddress("_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale", &_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale, &b__HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale);
        fChain->SetBranchAddress("_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ", &_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ, &b__HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ);
        fChain->SetBranchAddress("_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_prescale", &_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_prescale, &b__HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_prescale);
        fChain->SetBranchAddress("_2017_m", &_2017_m, &b__2017_m);
        fChain->SetBranchAddress("_HLT_IsoMu27", &_HLT_IsoMu27, &b__HLT_IsoMu27);
        fChain->SetBranchAddress("_HLT_IsoMu27_prescale", &_HLT_IsoMu27_prescale, &b__HLT_IsoMu27_prescale);
        fChain->SetBranchAddress("_HLT_IsoMu30", &_HLT_IsoMu30, &b__HLT_IsoMu30);
        fChain->SetBranchAddress("_HLT_IsoMu30_prescale", &_HLT_IsoMu30_prescale, &b__HLT_IsoMu30_prescale);
        fChain->SetBranchAddress("_2017_mee", &_2017_mee, &b__2017_mee);
        fChain->SetBranchAddress("_HLT_Mu8_DiEle12_CaloIdL_TrackIdL", &_HLT_Mu8_DiEle12_CaloIdL_TrackIdL, &b__HLT_Mu8_DiEle12_CaloIdL_TrackIdL);
        fChain->SetBranchAddress("_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_prescale", &_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_prescale, &b__HLT_Mu8_DiEle12_CaloIdL_TrackIdL_prescale);
        fChain->SetBranchAddress("_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ", &_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ, &b__HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ);
        fChain->SetBranchAddress("_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_prescale", &_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_prescale, &b__HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_prescale);
        fChain->SetBranchAddress("_2017_mm", &_2017_mm, &b__2017_mm);
        fChain->SetBranchAddress("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL, &b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL);
        fChain->SetBranchAddress("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_prescale", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_prescale, &b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_prescale);
        fChain->SetBranchAddress("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ, &b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ);
        fChain->SetBranchAddress("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_prescale", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_prescale, &b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_prescale);
        fChain->SetBranchAddress("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8, &b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8);
        fChain->SetBranchAddress("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8_prescale", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8_prescale, &b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8_prescale);
        fChain->SetBranchAddress("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8, &b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8);
        fChain->SetBranchAddress("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_prescale", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_prescale, &b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_prescale);
        fChain->SetBranchAddress("_HLT_DoubleMu4_Mass8_DZ_PFHT350", &_HLT_DoubleMu4_Mass8_DZ_PFHT350, &b__HLT_DoubleMu4_Mass8_DZ_PFHT350);
        fChain->SetBranchAddress("_HLT_DoubleMu4_Mass8_DZ_PFHT350_prescale", &_HLT_DoubleMu4_Mass8_DZ_PFHT350_prescale, &b__HLT_DoubleMu4_Mass8_DZ_PFHT350_prescale);
        fChain->SetBranchAddress("_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8", &_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8, &b__HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8);
        fChain->SetBranchAddress("_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8_prescale", &_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8_prescale, &b__HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8_prescale);
        fChain->SetBranchAddress("_2017_mme", &_2017_mme, &b__2017_mme);
        fChain->SetBranchAddress("_HLT_DiMu9_Ele9_CaloIdL_TrackIdL", &_HLT_DiMu9_Ele9_CaloIdL_TrackIdL, &b__HLT_DiMu9_Ele9_CaloIdL_TrackIdL);
        fChain->SetBranchAddress("_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_prescale", &_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_prescale, &b__HLT_DiMu9_Ele9_CaloIdL_TrackIdL_prescale);
        fChain->SetBranchAddress("_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ", &_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ, &b__HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ);
        fChain->SetBranchAddress("_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_prescale", &_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_prescale, &b__HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_prescale);
        fChain->SetBranchAddress("_2017_mmm", &_2017_mmm, &b__2017_mmm);
        fChain->SetBranchAddress("_HLT_TripleMu_10_5_5_DZ", &_HLT_TripleMu_10_5_5_DZ, &b__HLT_TripleMu_10_5_5_DZ);
        fChain->SetBranchAddress("_HLT_TripleMu_10_5_5_DZ_prescale", &_HLT_TripleMu_10_5_5_DZ_prescale, &b__HLT_TripleMu_10_5_5_DZ_prescale);
        fChain->SetBranchAddress("_HLT_TripleMu_5_3_3_Mass3p8to60_DZ", &_HLT_TripleMu_5_3_3_Mass3p8to60_DZ, &b__HLT_TripleMu_5_3_3_Mass3p8to60_DZ);
        fChain->SetBranchAddress("_HLT_TripleMu_5_3_3_Mass3p8to60_DZ_prescale", &_HLT_TripleMu_5_3_3_Mass3p8to60_DZ_prescale, &b__HLT_TripleMu_5_3_3_Mass3p8to60_DZ_prescale);
        fChain->SetBranchAddress("_TripleMu_12_10_5", &_TripleMu_12_10_5, &b__TripleMu_12_10_5);
    }
    fChain->SetBranchAddress("_passMETFilters", &_passMETFilters, &b__passMETFilters);
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
    fChain->SetBranchAddress("_lElectronPassEmu", _lElectronPassEmu, &b__lElectronPassEmu);
    fChain->SetBranchAddress("_lElectronPassConvVeto", _lElectronPassConvVeto, &b__lElectronPassConvVeto);
    fChain->SetBranchAddress("_lElectronChargeConst", _lElectronChargeConst, &b__lElectronChargeConst);
    fChain->SetBranchAddress("_lElectronMissingHits", _lElectronMissingHits, &b__lElectronMissingHits);
    fChain->SetBranchAddress("_leptonMvaSUSY", _leptonMvaSUSY, &b__leptonMvaSUSY);
    fChain->SetBranchAddress("_leptonMvaTTH", _leptonMvaTTH, &b__leptonMvaTTH);
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
    fChain->SetBranchAddress("_relIso", _relIso, &b__relIso);
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
    fChain->SetBranchAddress("_jetPt_JERUp", _jetPt_JERUp, &b__jetPt_JERUp);
    fChain->SetBranchAddress("_jetPt_JERDown", _jetPt_JERDown, &b__jetPt_JERDown);
    fChain->SetBranchAddress("_jetEta", _jetEta, &b__jetEta);
    fChain->SetBranchAddress("_jetPhi", _jetPhi, &b__jetPhi);
    fChain->SetBranchAddress("_jetE", _jetE, &b__jetE);
    fChain->SetBranchAddress("_jetCsvV2", _jetCsvV2, &b__jetCsvV2);
    fChain->SetBranchAddress("_jetDeepCsv_udsg", _jetDeepCsv_udsg, &b__jetDeepCsv_udsg);
    fChain->SetBranchAddress("_jetDeepCsv_b", _jetDeepCsv_b, &b__jetDeepCsv_b);
    fChain->SetBranchAddress("_jetDeepCsv_c", _jetDeepCsv_c, &b__jetDeepCsv_c);
    fChain->SetBranchAddress("_jetDeepCsv_bb", _jetDeepCsv_bb, &b__jetDeepCsv_bb);
    fChain->SetBranchAddress("_jetHadronFlavor", _jetHadronFlavor, &b__jetHadronFlavor);
    fChain->SetBranchAddress("_jetId", _jetId, &b__jetId);
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
    }
}

void treeReader::setOutputTree(TTree* outputTree, const bool isData){
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
    if(isData){                 //Temporarily only store 2017 triggers for data, to be updated when 2017 MC is available
        outputTree->Branch("_2017_e", &_2017_e, "_2017_e/O");
        outputTree->Branch("_HLT_Ele35_WPTight_Gsf", &_HLT_Ele35_WPTight_Gsf, "_HLT_Ele35_WPTight_Gsf/O");
        outputTree->Branch("_HLT_Ele35_WPTight_Gsf_prescale", &_HLT_Ele35_WPTight_Gsf_prescale, "_HLT_Ele35_WPTight_Gsf_prescale/O");
        outputTree->Branch("_HLT_Ele40_WPTight_Gsf", &_HLT_Ele40_WPTight_Gsf, "_HLT_Ele40_WPTight_Gsf/O");
        outputTree->Branch("_HLT_Ele40_WPTight_Gsf_prescale", &_HLT_Ele40_WPTight_Gsf_prescale, "_HLT_Ele40_WPTight_Gsf_prescale/O");
        outputTree->Branch("_2017_ee", &_2017_ee, "_2017_ee/O");
        outputTree->Branch("_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350", &_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350, "_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350/O");
        outputTree->Branch("_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350_prescale", &_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350_prescale, "_HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350_prescale/O");
        outputTree->Branch("_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL", &_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL, "_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL/O");
        outputTree->Branch("_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_prescale", &_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_prescale, "_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_prescale/O");
        outputTree->Branch("_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ", &_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ, "_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ/O");
        outputTree->Branch("_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale", &_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale, "_HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale/O");
        outputTree->Branch("_2017_eee", &_2017_eee, "_2017_eee/O");
        outputTree->Branch("_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL", &_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL, "_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL/O");
        outputTree->Branch("_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_prescale", &_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_prescale, "_HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_prescale/O");
        outputTree->Branch("_2017_em", &_2017_em, "_2017_em/O");
        outputTree->Branch("_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ", &_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ, "_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ/O");
        outputTree->Branch("_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ_prescale", &_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ_prescale, "_HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ_prescale/O");
        outputTree->Branch("_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ", &_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ, "_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ/O");
        outputTree->Branch("_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale", &_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale, "_HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale/O");
        outputTree->Branch("_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ", &_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ, "_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ/O");
        outputTree->Branch("_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_prescale", &_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_prescale, "_HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_prescale/O");
        outputTree->Branch("_2017_m", &_2017_m, "_2017_m/O");
        outputTree->Branch("_HLT_IsoMu27", &_HLT_IsoMu27, "_HLT_IsoMu27/O");
        outputTree->Branch("_HLT_IsoMu27_prescale", &_HLT_IsoMu27_prescale, "_HLT_IsoMu27_prescale/O");
        outputTree->Branch("_HLT_IsoMu30", &_HLT_IsoMu30, "_HLT_IsoMu30/O");
        outputTree->Branch("_HLT_IsoMu30_prescale", &_HLT_IsoMu30_prescale, "_HLT_IsoMu30_prescale/O");
        outputTree->Branch("_2017_mee", &_2017_mee, "_2017_mee/O");
        outputTree->Branch("_HLT_Mu8_DiEle12_CaloIdL_TrackIdL", &_HLT_Mu8_DiEle12_CaloIdL_TrackIdL, "_HLT_Mu8_DiEle12_CaloIdL_TrackIdL/O");
        outputTree->Branch("_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_prescale", &_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_prescale, "_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_prescale/O");
        outputTree->Branch("_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ", &_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ, "_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ/O");
        outputTree->Branch("_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_prescale", &_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_prescale, "_HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_prescale/O");
        outputTree->Branch("_2017_mm", &_2017_mm, "_2017_mm/O");
        outputTree->Branch("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL, "_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL/O");
        outputTree->Branch("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_prescale", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_prescale, "_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_prescale/O");
        outputTree->Branch("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ, "_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ/O");
        outputTree->Branch("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_prescale", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_prescale, "_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_prescale/O");
        outputTree->Branch("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8, "_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8/O");
        outputTree->Branch("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8_prescale", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8_prescale, "_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8_prescale/O");
        outputTree->Branch("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8, "_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8/O");
        outputTree->Branch("_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_prescale", &_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_prescale, "_HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_prescale/O");
        outputTree->Branch("_HLT_DoubleMu4_Mass8_DZ_PFHT350", &_HLT_DoubleMu4_Mass8_DZ_PFHT350, "_HLT_DoubleMu4_Mass8_DZ_PFHT350/O");
        outputTree->Branch("_HLT_DoubleMu4_Mass8_DZ_PFHT350_prescale", &_HLT_DoubleMu4_Mass8_DZ_PFHT350_prescale, "_HLT_DoubleMu4_Mass8_DZ_PFHT350_prescale/O");
        outputTree->Branch("_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8", &_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8, "_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8/O");
        outputTree->Branch("_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8_prescale", &_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8_prescale, "_HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8_prescale/O");
        outputTree->Branch("_2017_mme", &_2017_mme, "_2017_mme/O");
        outputTree->Branch("_HLT_DiMu9_Ele9_CaloIdL_TrackIdL", &_HLT_DiMu9_Ele9_CaloIdL_TrackIdL, "_HLT_DiMu9_Ele9_CaloIdL_TrackIdL/O");
        outputTree->Branch("_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_prescale", &_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_prescale, "_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_prescale/O");
        outputTree->Branch("_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ", &_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ, "_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ/O");
        outputTree->Branch("_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_prescale", &_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_prescale, "_HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_prescale/O");
        outputTree->Branch("_2017_mmm", &_2017_mmm, "_2017_mmm/O");
        outputTree->Branch("_HLT_TripleMu_10_5_5_DZ", &_HLT_TripleMu_10_5_5_DZ, "_HLT_TripleMu_10_5_5_DZ/O");
        outputTree->Branch("_HLT_TripleMu_10_5_5_DZ_prescale", &_HLT_TripleMu_10_5_5_DZ_prescale, "_HLT_TripleMu_10_5_5_DZ_prescale/O");
        outputTree->Branch("_HLT_TripleMu_5_3_3_Mass3p8to60_DZ", &_HLT_TripleMu_5_3_3_Mass3p8to60_DZ, "_HLT_TripleMu_5_3_3_Mass3p8to60_DZ/O");
        outputTree->Branch("_HLT_TripleMu_5_3_3_Mass3p8to60_DZ_prescale", &_HLT_TripleMu_5_3_3_Mass3p8to60_DZ_prescale, "_HLT_TripleMu_5_3_3_Mass3p8to60_DZ_prescale/O");
        outputTree->Branch("_TripleMu_12_10_5", &_TripleMu_12_10_5, "_TripleMu_12_10_5/O");
    }
    outputTree->Branch("_passMETFilters", &_passMETFilters, "_passMETFilters/O");
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
    outputTree->Branch("_lElectronPassEmu",             &_lElectronPassEmu,             "_lElectronPassEmu[_nLight]/O");
    outputTree->Branch("_lElectronPassConvVeto",        &_lElectronPassConvVeto,        "_lElectronPassConvVeto[_nLight]/O");
    outputTree->Branch("_lElectronChargeConst",         &_lElectronChargeConst,         "_lElectronChargeConst[_nLight]/O");
    outputTree->Branch("_lElectronMissingHits",         &_lElectronMissingHits,         "_lElectronMissingHits[_nLight]/i");
    outputTree->Branch("_leptonMvaSUSY",                &_leptonMvaSUSY,                "_leptonMvaSUSY[_nLight]/D");
    outputTree->Branch("_leptonMvaTTH",                 &_leptonMvaTTH,                 "_leptonMvaTTH[_nLight]/D");
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
    outputTree->Branch("_relIso",                       &_relIso,                       "_relIso[_nLight]/D");
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
    outputTree->Branch("_jetPt_JERUp",               &_jetPt_JERUp,              "_jetPt_JERUp[_nJets]/D");
    outputTree->Branch("_jetPt_JERDown",             &_jetPt_JERDown,            "_jetPt_JERDown[_nJets]/D");
    outputTree->Branch("_jetEta",                    &_jetEta,                   "_jetEta[_nJets]/D");
    outputTree->Branch("_jetPhi",                    &_jetPhi,                   "_jetPhi[_nJets]/D");
    outputTree->Branch("_jetE",                      &_jetE,                     "_jetE[_nJets]/D");
    outputTree->Branch("_jetCsvV2",                  &_jetCsvV2,                 "_jetCsvV2[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_udsg",           &_jetDeepCsv_udsg,          "_jetDeepCsv_udsg[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_b",              &_jetDeepCsv_b,             "_jetDeepCsv_b[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_c",              &_jetDeepCsv_c,             "_jetDeepCsv_c[_nJets]/D");
    outputTree->Branch("_jetDeepCsv_bb",             &_jetDeepCsv_bb,            "_jetDeepCsv_bb[_nJets]/D");
    //outputTree->Branch("_jetDeepCsv_cc",             &_jetDeepCsv_cc,            "_jetDeepCsv_cc[_nJets]/D");
    outputTree->Branch("_jetHadronFlavor",           &_jetHadronFlavor,          "_jetHadronFlavor[_nJets]/i");
    outputTree->Branch("_jetId",                     &_jetId,                    "_jetId[_nJets]/i");

    if(!isData){
        outputTree->Branch("_nLheWeights",               &_nLheWeights,               "_nLheWeights/b");
        outputTree->Branch("_lheWeight",                 &_lheWeight,                 "_lheWeight[_nLheWeights]/D");
        outputTree->Branch("_weight",                    &_weight,                    "_weight/D");
        outputTree->Branch("_lIsPrompt",                 &_lIsPrompt,                 "_lIsPrompt[_nL]/O");
        outputTree->Branch("_lMatchPdgId",               &_lMatchPdgId,               "_lMatchPdgId[_nL]/I");
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

    }
}

