#ifndef treeReader_h
#define treeReader_h

#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TLorentzVector.h"

#include <TF1.h>
#include <TH1.h>


class treeReader {
    public :
        //Declare leaf types
        static const unsigned nL_max = 20;
        static const unsigned nJets_max = 20;
        static const unsigned gen_nL_max = 20;
        static const unsigned gen_nPh_max = 10;
        ULong_t       _runNb;
        ULong_t       _lumiBlock;
        ULong_t       _eventNb;
        UChar_t         _nVertex;
        Double_t        _weight;
        UChar_t         _nLheWeights;
        Double_t        _lheWeight[110];
        Float_t         _nTrueInt;
        Double_t        _gen_met;
        Double_t        _gen_metPhi;
        UChar_t         _gen_nL;
        Double_t        _gen_lPt[gen_nL_max];   
        Double_t        _gen_lEta[gen_nL_max];   
        Double_t        _gen_lPhi[gen_nL_max];   
        Double_t        _gen_lE[gen_nL_max];   
        UInt_t          _gen_lFlavor[gen_nL_max];   
        Int_t           _gen_lCharge[gen_nL_max];   
        Int_t           _gen_lMomPdg[gen_nL_max];   
        Bool_t          _gen_lIsPrompt[gen_nL_max];   
        Bool_t          _2017_e;
        Bool_t          _HLT_Ele35_WPTight_Gsf;
        Int_t           _HLT_Ele35_WPTight_Gsf_prescale;
        Bool_t          _HLT_Ele40_WPTight_Gsf;
        Int_t           _HLT_Ele40_WPTight_Gsf_prescale;
        Bool_t          _2017_ee;
        Bool_t          _HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350;
        Int_t           _HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350_prescale;
        Bool_t          _HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL;
        Int_t           _HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_prescale;
        Bool_t          _HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ;
        Int_t           _HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale;
        Bool_t          _2017_eee;
        Bool_t          _HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL;
        Int_t           _HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_prescale;
        Bool_t          _2017_em;
        Bool_t          _HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ;
        Int_t           _HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ_prescale;
        Bool_t          _HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ;
        Int_t           _HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale;
        Bool_t          _HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ;
        Int_t           _HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_prescale;
        Bool_t          _2017_m;
        Bool_t          _HLT_IsoMu27;
        Int_t           _HLT_IsoMu27_prescale;
        Bool_t          _HLT_IsoMu30;
        Int_t           _HLT_IsoMu30_prescale;
        Bool_t          _2017_mee;
        Bool_t          _HLT_Mu8_DiEle12_CaloIdL_TrackIdL;
        Int_t           _HLT_Mu8_DiEle12_CaloIdL_TrackIdL_prescale;
        Bool_t          _HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ;
        Int_t           _HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_prescale;
        Bool_t          _2017_mm;
        Bool_t          _HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL;
        Int_t           _HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_prescale;
        Bool_t          _HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ;
        Int_t           _HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_prescale;
        Bool_t          _HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8;
        Int_t           _HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8_prescale;
        Bool_t          _HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8;
        Int_t           _HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_prescale;
        Bool_t          _HLT_DoubleMu4_Mass8_DZ_PFHT350;
        Int_t           _HLT_DoubleMu4_Mass8_DZ_PFHT350_prescale;
        Bool_t          _HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8;
        Int_t           _HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8_prescale;
        Bool_t          _2017_mme;
        Bool_t          _HLT_DiMu9_Ele9_CaloIdL_TrackIdL;
        Int_t           _HLT_DiMu9_Ele9_CaloIdL_TrackIdL_prescale;
        Bool_t          _HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ;
        Int_t           _HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_prescale;
        Bool_t          _2017_mmm;
        Bool_t          _HLT_TripleMu_10_5_5_DZ;
        Int_t           _HLT_TripleMu_10_5_5_DZ_prescale;
        Bool_t          _HLT_TripleMu_5_3_3_Mass3p8to60_DZ;
        Int_t           _HLT_TripleMu_5_3_3_Mass3p8to60_DZ_prescale;
        Bool_t          _TripleMu_12_10_5;
        Bool_t          _passMETFilters;
        UChar_t         _nL;
        UChar_t         _nMu;
        UChar_t         _nEle;
        UChar_t         _nLight;
        UChar_t         _nTau;
        Double_t        _lPt[nL_max];   
        Double_t        _lEta[nL_max];   
        Double_t        _lEtaSC[nL_max];   
        Double_t        _lPhi[nL_max];   
        Double_t        _lE[nL_max];   
        UInt_t          _lFlavor[nL_max];   
        Int_t           _lCharge[nL_max];   
        Double_t        _dxy[nL_max];   
        Double_t        _dz[nL_max];   
        Double_t        _3dIP[nL_max];   
        Double_t        _3dIPSig[nL_max];   
        Float_t         _lElectronMva[nL_max];   
        Bool_t          _lElectronPassEmu[nL_max];   
        Double_t        _leptonMva[nL_max];   
        Bool_t          _lHNLoose[nL_max];   
        Bool_t          _lHNFO[nL_max];   
        Bool_t          _lHNTight[nL_max];   
        Bool_t          _lEwkLoose[nL_max];   
        Bool_t          _lEwkFO[nL_max];   
        Bool_t          _lEwkTight[nL_max];   
        Bool_t          _lPOGVeto[nL_max];   
        Bool_t          _lPOGLoose[nL_max];   
        Bool_t          _lPOGMedium[nL_max];   
        Bool_t          _lPOGTight[nL_max];   
        Bool_t          _tauMuonVeto[nL_max];   
        Bool_t          _tauEleVeto[nL_max];   
        Bool_t          _decayModeFindingNew[nL_max];   
        Bool_t          _tauVLooseMvaNew[nL_max];   
        Bool_t          _tauLooseMvaNew[nL_max];   
        Bool_t          _tauMediumMvaNew[nL_max];   
        Bool_t          _tauTightMvaNew[nL_max];   
        Bool_t          _tauVTightMvaNew[nL_max];   
        Bool_t          _tauVTightMvaOld[nL_max];   
        Double_t        _relIso[nL_max];   
        Double_t        _miniIso[nL_max];   
        Double_t        _miniIsoCharged[nL_max];   
        Double_t        _ptRel[nL_max];   
        Double_t        _ptRatio[nL_max];   
        Double_t        _closestJetCsv[nL_max];   
        UInt_t          _selectedTrackMult[nL_max];   
        Double_t        _muonSegComp[nL_max];   
        Bool_t          _lIsPrompt[nL_max];   
        Int_t           _lMatchPdgId[nL_max];   
        UChar_t         _nJets;
        Double_t        _jetPt[nJets_max];   
        Double_t        _jetPt_JECUp[nJets_max];   
        Double_t        _jetPt_JECDown[nJets_max];   
        Double_t        _jetPt_JERUp[nJets_max];   
        Double_t        _jetPt_JERDown[nJets_max];   
        Double_t        _jetEta[nJets_max];   
        Double_t        _jetPhi[nJets_max];   
        Double_t        _jetE[nJets_max];   
        Double_t        _jetCsvV2[nJets_max];   
        Double_t        _jetDeepCsv_udsg[nJets_max];   
        Double_t        _jetDeepCsv_b[nJets_max];   
        Double_t        _jetDeepCsv_c[nJets_max];   
        Double_t        _jetDeepCsv_bb[nJets_max];   
        UInt_t          _jetHadronFlavor[nJets_max];   
        UInt_t          _jetId[nJets_max];   
        Double_t        _met;
        Double_t        _metJECDown;
        Double_t        _metJECUp;
        Double_t        _metUnclDown;
        Double_t        _metUnclUp;
        Double_t        _metPhi;
        Double_t        _metPhiJECDown;
        Double_t        _metPhiJECUp;
        Double_t        _metPhiUnclDown;
        Double_t        _metPhiUnclUp;       


        //Constructor
        treeReader(TTree *tree = nullptr);

        //set up tree for reading and writing
        void initTree(TTree *tree, const bool isData = false);
        void setOutputTree(TTree*, const bool isData = false);

        //skim tree
        void skimTree(const std::string&, std::string outputDirectory = "", const bool isData = false);
        void combinePD(const std::vector<std::string>& datasets, std::string outputDirectory = "");

        //set up tree for analysis
        void readSamples(const std::string& list = ""); //read sample list from file
        void initSample();

        //functions to analyze tree
        void GetEntry(long unsigned entry);
        void Analyze();
        void Loop(const std::string& sample, const double xSection);

        //functions for event selection
        unsigned dilFlavorComb(const std::vector<unsigned>&);
        double coneCorr(const unsigned);
        void setConePt();
        bool lepIsGood(const unsigned);
        bool lepIsTight(const unsigned);
        unsigned selectLep(std::vector<unsigned>&);
        unsigned tightLepCount(const std::vector<unsigned>&, const unsigned);
        bool passPtCuts(const std::vector<unsigned>&);
        bool jetIsClean(const unsigned);
        bool jetIsGood(const unsigned, const unsigned ptCut = 25, const unsigned unc = 0, const bool clean = true);
        unsigned nJets(const unsigned unc = 0, const bool clean = true);
        bool bTaggedDeepCSV(const unsigned unc, const unsigned wp = 1);
        bool bTaggedCSVv2(const unsigned uncm, const unsigned wp = 1);
        unsigned nBJets(const unsigned unc = 0, const bool deepCSV = true, const bool clean = true, const unsigned wp = 1);
    private:
        TTree* fChain;                                                          //current Tree
        std::shared_ptr<TFile> sampleFile;                                      //current sample
        std::vector<std::tuple<std::string, std::string, double> > samples;     //list of samples
        unsigned currentSample = 0;                                             //current index in list
        bool isData = false;
        double scale = 0;
        double weight = 1;                                                      //weight of given event
        unsigned long nEntries = 0;
        const double dataLumi = 29.55;                                          //in units of 1/fb

        // List of branches
        TBranch        *b__runNb;   
        TBranch        *b__lumiBlock;   
        TBranch        *b__eventNb;   
        TBranch        *b__nVertex;   
        TBranch        *b__weight;   
        TBranch        *b__nLheWeights;   
        TBranch        *b__lheWeight;   
        TBranch        *b__nTrueInt;   
        TBranch        *b__gen_met;   
        TBranch        *b__gen_metPhi;   
        TBranch        *b__gen_nL;   
        TBranch        *b__gen_lPt;   
        TBranch        *b__gen_lEta;   
        TBranch        *b__gen_lPhi;   
        TBranch        *b__gen_lE;   
        TBranch        *b__gen_lFlavor;   
        TBranch        *b__gen_lCharge;   
        TBranch        *b__gen_lMomPdg;   
        TBranch        *b__gen_lIsPrompt;   
        TBranch        *b__2017_e;   
        TBranch        *b__HLT_Ele35_WPTight_Gsf;   
        TBranch        *b__HLT_Ele35_WPTight_Gsf_prescale;   
        TBranch        *b__HLT_Ele40_WPTight_Gsf;   
        TBranch        *b__HLT_Ele40_WPTight_Gsf_prescale;   
        TBranch        *b__2017_ee;   
        TBranch        *b__HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350;   
        TBranch        *b__HLT_DoubleEle8_CaloIdM_TrackIdM_Mass8_DZ_PFHT350_prescale;   
        TBranch        *b__HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL;   
        TBranch        *b__HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_prescale;   
        TBranch        *b__HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ;   
        TBranch        *b__HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale;   
        TBranch        *b__2017_eee;   
        TBranch        *b__HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL;   
        TBranch        *b__HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_prescale;   
        TBranch        *b__2017_em;   
        TBranch        *b__HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ;   
        TBranch        *b__HLT_Mu8_Ele8_CaloIdM_TrackIdM_Mass8_PFHT350_DZ_prescale;   
        TBranch        *b__HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ;   
        TBranch        *b__HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_prescale;   
        TBranch        *b__HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ;   
        TBranch        *b__HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_prescale;   
        TBranch        *b__2017_m;   
        TBranch        *b__HLT_IsoMu27;   
        TBranch        *b__HLT_IsoMu27_prescale;   
        TBranch        *b__HLT_IsoMu30;   
        TBranch        *b__HLT_IsoMu30_prescale;   
        TBranch        *b__2017_mee;   
        TBranch        *b__HLT_Mu8_DiEle12_CaloIdL_TrackIdL;   
        TBranch        *b__HLT_Mu8_DiEle12_CaloIdL_TrackIdL_prescale;   
        TBranch        *b__HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ;   
        TBranch        *b__HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_prescale;   
        TBranch        *b__2017_mm;   
        TBranch        *b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL;   
        TBranch        *b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_prescale;   
        TBranch        *b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ;   
        TBranch        *b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_prescale;   
        TBranch        *b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8;   
        TBranch        *b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_Mass8_prescale;   
        TBranch        *b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8;   
        TBranch        *b__HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_prescale;   
        TBranch        *b__HLT_DoubleMu4_Mass8_DZ_PFHT350;   
        TBranch        *b__HLT_DoubleMu4_Mass8_DZ_PFHT350_prescale;   
        TBranch        *b__HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8;   
        TBranch        *b__HLT_Mu19_TrkIsoVVL_Mu9_TrkIsoVVL_DZ_Mass3p8_prescale;   
        TBranch        *b__2017_mme;   
        TBranch        *b__HLT_DiMu9_Ele9_CaloIdL_TrackIdL;   
        TBranch        *b__HLT_DiMu9_Ele9_CaloIdL_TrackIdL_prescale;   
        TBranch        *b__HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ;   
        TBranch        *b__HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_prescale;   
        TBranch        *b__2017_mmm;   
        TBranch        *b__HLT_TripleMu_10_5_5_DZ;   
        TBranch        *b__HLT_TripleMu_10_5_5_DZ_prescale;   
        TBranch        *b__HLT_TripleMu_5_3_3_Mass3p8to60_DZ;   
        TBranch        *b__HLT_TripleMu_5_3_3_Mass3p8to60_DZ_prescale;   
        TBranch        *b__TripleMu_12_10_5;   
        TBranch        *b__passMETFilters;   
        TBranch        *b__nL;   
        TBranch        *b__nMu;   
        TBranch        *b__nEle;   
        TBranch        *b__nLight;   
        TBranch        *b__nTau;   
        TBranch        *b__lPt;   
        TBranch        *b__lEta;   
        TBranch        *b__lEtaSC;   
        TBranch        *b__lPhi;   
        TBranch        *b__lE;   
        TBranch        *b__lFlavor;   
        TBranch        *b__lCharge;   
        TBranch        *b__dxy;   
        TBranch        *b__dz;   
        TBranch        *b__3dIP;   
        TBranch        *b__3dIPSig;   
        TBranch        *b__lElectronMva;   
        TBranch        *b__lElectronPassEmu;   
        TBranch        *b__leptonMva;   
        TBranch        *b__lHNLoose;   
        TBranch        *b__lHNFO;   
        TBranch        *b__lHNTight;   
        TBranch        *b__lEwkLoose;   
        TBranch        *b__lEwkFO;   
        TBranch        *b__lEwkTight;   
        TBranch        *b__lPOGVeto;   
        TBranch        *b__lPOGLoose;   
        TBranch        *b__lPOGMedium;   
        TBranch        *b__lPOGTight;   
        TBranch        *b__tauMuonVeto;   
        TBranch        *b__tauEleVeto;   
        TBranch        *b__decayModeFindingNew;   
        TBranch        *b__tauVLooseMvaNew;   
        TBranch        *b__tauLooseMvaNew;   
        TBranch        *b__tauMediumMvaNew;   
        TBranch        *b__tauTightMvaNew;   
        TBranch        *b__tauVTightMvaNew;   
        TBranch        *b__tauVTightMvaOld;   
        TBranch        *b__relIso;   
        TBranch        *b__miniIso;   
        TBranch        *b__miniIsoCharged;   
        TBranch        *b__ptRel;   
        TBranch        *b__ptRatio;   
        TBranch        *b__closestJetCsv;   
        TBranch        *b__selectedTrackMult;   
        TBranch        *b__muonSegComp;   
        TBranch        *b__lIsPrompt;   
        TBranch        *b__lMatchPdgId;   
        TBranch        *b__nJets;   
        TBranch        *b__jetPt;   
        TBranch        *b__jetPt_JECUp;   
        TBranch        *b__jetPt_JECDown;   
        TBranch        *b__jetPt_JERUp;   
        TBranch        *b__jetPt_JERDown;   
        TBranch        *b__jetEta;   
        TBranch        *b__jetPhi;   
        TBranch        *b__jetE;   
        TBranch        *b__jetCsvV2;   
        TBranch        *b__jetDeepCsv_udsg;   
        TBranch        *b__jetDeepCsv_b;   
        TBranch        *b__jetDeepCsv_c;   
        TBranch        *b__jetDeepCsv_bb;   
        TBranch        *b__jetHadronFlavor;   
        TBranch        *b__jetId;   
        TBranch        *b__met;   
        TBranch        *b__metJECDown;   
        TBranch        *b__metJECUp;   
        TBranch        *b__metUnclDown;   
        TBranch        *b__metUnclUp;   
        TBranch        *b__metPhi;   
        TBranch        *b__metPhiJECDown;   
        TBranch        *b__metPhiJECUp;   
        TBranch        *b__metPhiUnclDown;   
        TBranch        *b__metPhiUnclUp;   
};
#endif
