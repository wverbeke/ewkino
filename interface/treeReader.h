#ifndef treeReader_h
#define treeReader_h

//include ROOT classes
#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TLorentzVector.h"

//include other parts of code
#include "Reweighter.h"
#include "Sample.h"
#include "HistInfo.h"

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
        UChar_t         _ttgEventType;
        UChar_t         _zgEventType;
        Double_t        _gen_HT;
        Bool_t          _passTrigger_e;
        Bool_t          _passTrigger_ee;
        Bool_t          _passTrigger_eee;
        Bool_t          _passTrigger_em;
        Bool_t          _passTrigger_m;
        Bool_t          _passTrigger_eem;
        Bool_t          _passTrigger_mm;
        Bool_t          _passTrigger_emm;
        Bool_t          _passTrigger_mmm;
        Bool_t          _passTrigger_t;
        Bool_t          _passTrigger_et;
        Bool_t          _passTrigger_mt;
        Bool_t          _passMETFilters;
        //TEMPORARY FOR TEST, CAN BE REMOVED LATER
        Bool_t          _Flag_BadPFMuonFilter;
        Bool_t          _Flag_BadChargedCandidateFilter;
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
        Float_t         _lElectronMvaHZZ[nL_max];
        Float_t         _lElectronMvaFall17Iso[nL_max];
        Float_t         _lElectronMvaFall17NoIso[nL_max];
        Bool_t          _lElectronPassEmu[nL_max];   
        Bool_t          _lElectronPassConvVeto[nL_max];
        Bool_t          _lElectronChargeConst[nL_max];
        UInt_t          _lElectronMissingHits[nL_max];
        Double_t        _leptonMvaSUSY16[nL_max];   
        Double_t        _leptonMvaTTH16[nL_max];
        Double_t        _leptonMvatZqTTV16[nL_max];
        Double_t        _leptonMvaSUSY17[nL_max];
        Double_t        _leptonMvaTTH17[nL_max];
        Double_t        _leptonMvatZqTTV17[nL_max];
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
        Double_t        _tauAgainstElectronMVA6Raw[nL_max];
        Double_t        _tauCombinedIsoDBRaw3Hits[nL_max];
        Double_t        _tauIsoMVAPWdR03oldDMwLT[nL_max];
        Double_t        _tauIsoMVADBdR03oldDMwLT[nL_max];
        Double_t        _tauIsoMVADBdR03newDMwLT[nL_max];
        Double_t        _tauIsoMVAPWnewDMwLT[nL_max];
        Double_t        _tauIsoMVAPWoldDMwLT[nL_max];
        Double_t        _relIso[nL_max];   
        Double_t        _relIso0p4[nL_max];
        Double_t        _relIso0p4MuDeltaBeta[nL_max];
        Double_t        _miniIso[nL_max];   
        Double_t        _miniIsoCharged[nL_max];   
        Double_t        _ptRel[nL_max];   
        Double_t        _ptRatio[nL_max];   
        Double_t        _closestJetCsvV2[nL_max];
        Double_t        _closestJetDeepCsv_b[nL_max];
        Double_t        _closestJetDeepCsv_bb[nL_max];
        UInt_t          _selectedTrackMult[nL_max];   
        Double_t        _lMuonSegComp[nL_max];   
        Double_t        _lMuonTrackPt[nL_max];
        Double_t        _lMuonTrackPtErr[nL_max];
        Bool_t          _lIsPrompt[nL_max];   
        Int_t           _lMatchPdgId[nL_max];   
        Int_t           _lMomPdgId[nL_max];
        UInt_t          _lProvenance[nL_max];
        UInt_t          _lProvenanceCompressed[nL_max];
        UInt_t          _lProvenanceConversion[nL_max];
        UChar_t         _nJets;
        Double_t        _jetPt[nJets_max];   
        Double_t        _jetPt_JECUp[nJets_max];   
        Double_t        _jetPt_JECDown[nJets_max];   
        Double_t        _jetPt_Uncorrected[nJets_max];
        Double_t        _jetPt_L1[nJets_max];
        Double_t        _jetPt_L2[nJets_max];
        Double_t        _jetPt_L3[nJets_max];
        Double_t        _jetPt_L2L3[nJets_max];
        Double_t        _jetEta[nJets_max];   
        Double_t        _jetPhi[nJets_max];   
        Double_t        _jetE[nJets_max];   
        Double_t        _jetCsvV2[nJets_max];   
        Double_t        _jetDeepCsv_udsg[nJets_max];   
        Double_t        _jetDeepCsv_b[nJets_max];   
        Double_t        _jetDeepCsv_c[nJets_max];   
        Double_t        _jetDeepCsv_bb[nJets_max];   
        UInt_t          _jetHadronFlavor[nJets_max];   
        Bool_t          _jetIsLoose[nJets_max];
        Bool_t          _jetIsTight[nJets_max];
        Bool_t          _jetIsTightLepVeto[nJets_max];
        Double_t        _jetNeutralHadronFraction[nJets_max];
        Double_t        _jetChargedHadronFraction[nJets_max];
        Double_t        _jetNeutralEmFraction[nJets_max];
        Double_t        _jetChargedEmFraction[nJets_max];
        Double_t        _jetHFHadronFraction[nJets_max];
        Double_t        _jetHFEmFraction[nJets_max];
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
        Double_t        _metSignificance;


        //Constructor
        treeReader(TTree *tree = nullptr);

        //set up tree for reading and writing
        void initTree(TTree *tree, const bool isData = false);
        void setOutputTree(TTree*, const bool isData = false);

        //skim tree
        void skimTree(const std::string&, std::string outputDirectory = "", const bool isData = false);
        void combinePD(std::vector<std::string>& datasets, const bool is2017, std::string outputDirectory = "");

        //set up tree for analysis
        void readSamples(const std::string& list); //read sample list from file
        void readSamples2016(const std::string&);
        void readSamples2017(const std::string&);

        void initSample();                              //event weights will be set according to is2016() ( or equally is2017() ) flag
        void initSample(const Sample&);  

        //functions to analyze tree
        void GetEntry(long unsigned entry);
        void GetEntry(const Sample&, long unsigned entry);
        void Analyze();
        void Analyze(const std::string&, long unsigned, long unsigned);
        void Analyze(const Sample&, long unsigned, long unsigned);
        void Analyze(const std::string&);
        void Analyze(const Sample&);
        void setup();
        void splitJobs();
        void Loop(const std::string& sample, const double xSection);

        //new functions for parallel plotting
        void plot(const std::string&);
        void splitPlots();

        //functions for event selection
        void orderByPt(std::vector<unsigned>&, const double*, const unsigned) const;
        unsigned dilFlavorComb(const std::vector<unsigned>&) const;
        double coneCorr(const unsigned) const;
        void setConePt();
        bool lepIsLoose(const unsigned) const;
        bool lepIsGood(const unsigned) const;
        bool lepIsTight(const unsigned) const;
        bool lepFromMEExtConversion(const unsigned) const;
        bool eleIsClean(const unsigned) const;
        double closestJetDeepCsv(const unsigned) const;

        unsigned selectLep(std::vector<unsigned>&);
        unsigned tightLepCount(const std::vector<unsigned>&, const unsigned) const;

        bool passPtCuts(const std::vector<unsigned>&) const;
        bool jetIsClean(const unsigned) const;
        bool jetIsGood(const unsigned, const unsigned ptCut = 25, const unsigned unc = 0, const bool clean = true, const bool allowForward = false) const;
        unsigned nJets(const unsigned unc = 0, const bool clean = true) const;                                   //without jet pt ordering
        unsigned nJets(std::vector<unsigned>& jetInd, const unsigned unc = 0, const bool clean = true) const;    //with jet pt ordering
        bool bTagged(const unsigned ind, const unsigned wp = 1, const bool deepCSV = true) const;
        unsigned nBJets(const unsigned unc = 0, const bool deepCSV = true, const bool clean = true, const unsigned wp = 1) const;
        unsigned nBJets(std::vector<unsigned>& bJetInd, const unsigned unc = 0, const bool deepCSV = true, const bool clean = true, const unsigned wp = 1) const;

        //baseline selection for leptonMva training
        bool lepPassBaseline(const unsigned) const;

        //trigger decitions
        bool passSingleLeptonTriggers() const;
        bool passDileptonTriggers() const;
        bool passTrileptonTriggers() const;
        bool passTriggerCocktail() const;
        bool passMETFilters() const;

        //overlap removal between samples
        bool photonOverlap() const;                                                                          //sample overlap due to photons
        bool photonOverlap(const Sample&) const;
        bool htOverlap() const;                                                                              //sample overlap due to HT binning
        bool htOverlap(const Sample&) const;

        //check if leptons are prompt in MC
        bool promptLeptons() const;

        //compute b-tagging efficiency
        void computeBTagEff(const std::string& analysis, const bool clean, const bool deepCSV, const bool is2016);

        //event weights
        double puWeight(const unsigned unc = 0) const;
        double bTagWeight(const unsigned jetFlavor, const unsigned unc = 0) const;
        double bTagWeight(const std::vector<unsigned>& jetInd, const unsigned jetFlavor, const unsigned unc = 0) const; //more efficient version if jets were already selected 
        double bTagWeight_udsg(const unsigned unc = 0) const;
        double bTagWeight_c(const unsigned unc = 0) const;
        double bTagWeight_b(const unsigned unc = 0) const;
        double bTagWeight(const unsigned unc = 0) const;
        double leptonWeight() const;
        double sfWeight();
        double fakeRateWeight(const unsigned unc = 0);

    private:
        TTree* fChain;                                                          //current Tree
        std::shared_ptr<TFile> sampleFile;                                      //current sample
        std::vector<Sample> samples;                                            //combined list of samples
        std::vector<Sample> samples2016;                                        //2016 data and MC samples
        std::vector<Sample> samples2017;                                        //2017 data and MC samples
        Sample currentSample;                                                   //reference to current sample, needed to check what era sample belongs to
        std::vector<HistInfo> histInfo;                                         //histogram info
        int currentSampleIndex = -1;                                                 //current index in list
        //bool isData = false;
        double scale = 0;
        double weight = 1;                                                      //weight of given event
        unsigned long nEntries = 0;
        const double lumi2017 = 41.53;                                          //in units of 1/fb
        const double lumi2016 = 35.867;                 
        std::shared_ptr<Reweighter> reweighter;                                 //instance of reweighter class used for reweighting functions

        //check whether sample is 2017 or not
        bool is2017() const { return currentSample.is2017(); }
        bool is2016() const { return currentSample.is2016(); }                  //if sample is not 2017 it is automatically 2016
        bool isData() const { return currentSample.isData(); }
        bool isMC() const { return isMC(); } 

        //check lepton flavors 
        bool isElectron(const unsigned leptonIndex) const { return (_lFlavor[leptonIndex] == 0); }
        bool isMuon(const unsigned leptonIndex) const { return (_lFlavor[leptonIndex] == 1); }
        bool isTau(const unsigned leptonIndex) const { return (_lFlavor[leptonIndex] == 2); }

        //era-specific event selection functions
        bool lepIsLooseBase(const unsigned) const;
        bool lepIsLoose2016(const unsigned) const;
        bool lepIsLoose2017(const unsigned) const;

        bool lepIsGoodBase(const unsigned) const;
        bool lepIsGood2016(const unsigned) const;
        bool lepIsGood2017(const unsigned) const;

        bool lepIsTightBase(const unsigned) const;
        bool lepIsTight2016(const unsigned) const;
        bool lepIsTight2017(const unsigned) const;

        bool eleIsCleanBase(const unsigned, bool (treeReader::*looseMuon)(const unsigned) const) const;
        bool eleIsClean2016(const unsigned) const;
        bool eleIsClean2017(const unsigned) const;

        bool jetIsCleanBase(const unsigned, bool (treeReader::*leptonIsFO)(const unsigned) const) const;

        bool bTaggedDeepCSVBase(const unsigned, const unsigned wp, const double cuts[3]) const;
        bool bTaggedDeepCSV2016(const unsigned, const unsigned wp = 1) const;
        bool bTaggedDeepCSV2017(const unsigned, const unsigned wp = 1) const;
        bool bTaggedDeepCSV(const unsigned, const unsigned wp = 1) const;

        bool bTaggedCSVv2Base(const unsigned, const unsigned wp, const double cuts[3]) const;
        bool bTaggedCSVv22016(const unsigned, const unsigned wp = 1) const;
        bool bTaggedCSVv22017(const unsigned, const unsigned wp = 1) const;
        bool bTaggedCSVv2(const unsigned, const unsigned wp = 1) const;


        //lepton selection for different parts of tZq and ttV analysis (to compute bTag efficiencies for everyone)
        bool lepIsGoodtZq(const unsigned) const;

        bool lepIsGoodttZ3l2016(const unsigned) const;
        bool lepIsGoodttZ3l2017(const unsigned) const;
        bool lepIsGoodttZ3l(const unsigned) const;

        bool lepIsGoodttZ4l2016(const unsigned) const;
        bool lepIsGoodttZ4l2017(const unsigned) const;
        bool lepIsGoodttZ4l(const unsigned) const;

        bool lepIsGoodttW2016(const unsigned) const;
        bool lepIsGoodttW2017(const unsigned) const;
        bool lepIsGoodttW(const unsigned) const;

        bool lepIsGoodMultiAnalysis(const std::string&, const unsigned) const;

        //initialize SF weights
        void initializeWeights();
        
        //some safety-checks for errors 
        void checkSampleEraConsistency() const;  //make sure a sample is not is2016() AND 2017() 
        void checkEraOrthogonality() const;        //make sure no sample from the wrong era is being used (i.e. no 2016 sample in the list of 2017 samples) 

        //general function to read a list of samples
        void readSamples(const std::string&, std::vector<Sample>&);


        //list of branches
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
        TBranch        *b__ttgEventType;
        TBranch        *b__zgEventType;
        TBranch        *b__gen_HT;
        TBranch        *b__passTrigger_e;   
        TBranch        *b__passTrigger_ee;   
        TBranch        *b__passTrigger_eee;   
        TBranch        *b__passTrigger_em;   
        TBranch        *b__passTrigger_m;   
        TBranch        *b__passTrigger_eem;   
        TBranch        *b__passTrigger_mm;   
        TBranch        *b__passTrigger_emm;   
        TBranch        *b__passTrigger_mmm;   
        TBranch        *b__passTrigger_t;
        TBranch        *b__passTrigger_et;
        TBranch        *b__passTrigger_mt;
        TBranch        *b__passMETFilters;   
        //TEMPORARY FOR CHECK, CAN BE REMOVED LATER
        TBranch        *b__Flag_BadPFMuonFilter;
        TBranch        *b__Flag_BadChargedCandidateFilter;
        //////////////////////////////////////////
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
        TBranch        *b__lElectronMvaHZZ;
        TBranch        *b__lElectronMvaFall17Iso;
        TBranch        *b__lElectronMvaFall17NoIso;
        TBranch        *b__lElectronPassEmu;   
        TBranch        *b__lElectronPassConvVeto;
        TBranch        *b__lElectronChargeConst;
        TBranch        *b__lElectronMissingHits;
        TBranch        *b__leptonMvaSUSY16;
        TBranch        *b__leptonMvaTTH16;
        TBranch        *b__leptonMvatZqTTV16;
        TBranch        *b__leptonMvaSUSY17;
        TBranch        *b__leptonMvaTTH17;
        TBranch        *b__leptonMvatZqTTV17;
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
        TBranch        *b__tauAgainstElectronMVA6Raw;
        TBranch        *b__tauCombinedIsoDBRaw3Hits;
        TBranch        *b__tauIsoMVAPWdR03oldDMwLT;
        TBranch        *b__tauIsoMVADBdR03oldDMwLT;
        TBranch        *b__tauIsoMVADBdR03newDMwLT;
        TBranch        *b__tauIsoMVAPWnewDMwLT;
        TBranch        *b__tauIsoMVAPWoldDMwLT;
        TBranch        *b__relIso;   
        TBranch        *b__relIso0p4;
        TBranch        *b__relIso0p4MuDeltaBeta;
        TBranch        *b__miniIso;   
        TBranch        *b__miniIsoCharged;   
        TBranch        *b__ptRel;   
        TBranch        *b__ptRatio;   
        TBranch        *b__closestJetCsvV2;
        TBranch        *b__closestJetDeepCsv_b;
        TBranch        *b__closestJetDeepCsv_bb;
        TBranch        *b__selectedTrackMult;   
        TBranch        *b__lMuonSegComp;   
        TBranch        *b__lMuonTrackPt;
        TBranch        *b__lMuonTrackPtErr;
        TBranch        *b__lIsPrompt;   
        TBranch        *b__lMatchPdgId;   
        TBranch        *b__lMomPdgId;
        TBranch        *b__lProvenance;
        TBranch        *b__lProvenanceCompressed;
        TBranch        *b__lProvenanceConversion;
        TBranch        *b__nJets;   
        TBranch        *b__jetPt;   
        TBranch        *b__jetPt_JECUp;   
        TBranch        *b__jetPt_JECDown;   
        TBranch        *b__jetPt_Uncorrected;
        TBranch        *b__jetPt_L1;
        TBranch        *b__jetPt_L2;
        TBranch        *b__jetPt_L3;
        TBranch        *b__jetPt_L2L3;
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
        TBranch        *b__jetIsLoose;
        TBranch        *b__jetIsTight;
        TBranch        *b__jetIsTightLepVeto;
        TBranch        *b__jetNeutralHadronFraction;
        TBranch        *b__jetChargedHadronFraction;
        TBranch        *b__jetNeutralEmFraction;
        TBranch        *b__jetChargedEmFraction;
        TBranch        *b__jetHFHadronFraction;
        TBranch        *b__jetHFEmFraction;
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
        TBranch        *b__metSignificance;
};
#endif
