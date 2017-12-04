//include ROOT classes
#include "TLorentzVector.h"
#include "TH1D.h"
#include "TFile.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"
#include "TROOT.h"

//include C++ library classes
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <tuple>

//include other parts of the code
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"
#include "../interface/tZqTools.h"
#include "../interface/trilepTools.h"
#include "../interface/Reweighter.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"

//include TMVA classes
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"

void treeReader::Analyze(){
    //Set CMS plotting style
    setTDRStyle();
    gROOT->SetBatch(kTRUE);
    //read samples and cross sections from txt file
    readSamples("sampleLists/samples.txt");
    //info on kinematic distributions to plot
    std::vector< std::tuple < std::string, std::string, unsigned, double , double > > histInfo;
    //name      xlabel    nBins,  min, max
    histInfo = {
        //new BDT distribution
        /*
        std::make_tuple("bdt", "BDT output", 30, -0.35, 0.35),
        std::make_tuple("bdtG", "BDTG output", 30, -1, 1),
        std::make_tuple("bdtD", "BDTD output", 30, -0.5, 0.5),
        std::make_tuple("bdtB", "BDTB output", 30, -1, 1),
        std::make_tuple("mlp", "MLP output", 30, 0, 1),
        std::make_tuple("deepNN", "deep neural network output", 30, 0, 1),
        std::make_tuple("bdtGAlt", "alternate BDTG output", 30, -1, 1),
        std::make_tuple("bdtG_200trees", "BDTG output (200 trees)", 30, -1, 1),
        std::make_tuple("bdtG_shrinkage04", "BDTG output (shrinkage 0.4)", 30, -1, 1),
        std::make_tuple("bdtG_minNode005", "BDTG output (min node size 5%)", 30, -1, 1),
        std::make_tuple("bdtG_negWeights", "BDTG output (include neg. weights)", 30, -1, 1),
        std::make_tuple("bdtG_MaxDepth4", "BDTG output (max depth 4)", 30, -1, 1),
        std::make_tuple("bdtG_20Cuts", "BDTG output (20 points cut grid)", 30, -1, 1),
        std::make_tuple("bdtG_20Cuts_10bins", "BDTG output (20 points cut grid)", 10, -1, 1),
        std::make_tuple("BDTGAlt_negWeights_SDivSqrtSPlusB", "BDTG output (neg. weights, s/sqrt(s + b) )", 30, -1, 1),
        std::make_tuple("BDTGAlt_shrinkage02_20Cuts", "BDTG output (shrinkage 0.2, 20 cuts)", 30, -1, 1),
        std::make_tuple("BDTGAlt_shrinkage04_20Cuts",  "BDTG output (shrinkage 0.4, 20 cuts)", 30, -1, 1),
        std::make_tuple("BDTGAlt_shrinkage02_20Cuts_depth3", "BDTG output (shrinkage 0.2, 20 cuts, depth 3)", 30, -1, 1),
        std::make_tuple("BDTGAlt_200trees_MaxDepth3_shrinkage02", "BDTG output (200 trees, shrinkage 0.2, depth 3)", 30, -1, 1),
        std::make_tuple("BDTGAlt_MaxDepth3", "BDTG output (depth 3)", 30, -1, 1),
        std::make_tuple("BDTGAlt_Decorrelate", "BDTG output (decorrelated)", 30, -1, 1),
        std::make_tuple("BDTAda_200Cuts",  "BDTAda output (200 cuts)", 30, -0.35, 0.35),
        std::make_tuple("BDTAda_200Cuts_Depth2", "BDTAda output (depth 2)", 30, -0.35, 0.35),
        std::make_tuple("BDTAda_200Cuts_Depth2_Beta01", "BDTAda output (200 cuts, depth 2, beta 0.1)", 30, -0.35, 0.35),
        std::make_tuple("BDTAda_200Cuts_Depth2_200Trees", "BDTAda output (200 cuts, 200 trees, depth 2)", 30, -0.35, 0.35),
        std::make_tuple("BDTAda_200Cuts_SDivSqrtSPlusB", "BDTAda output (200 cuts, s/sqrt(s + b) )", 30, -0.35, 0.35),
        std::make_tuple("BDTAda_200Cuts_ignoreNegative", "BDTAda output (200 cuts, ignora neg. weights)", 30, -0.35, 0.35),
        std::make_tuple("BDTAda_200Cuts_Decorrelate", "BDTAda output (200 cuts, decorrelated)", 30, -0.35, 0.35),
        std::make_tuple("BDTB_200Cuts", "BDTB output (200 cuts)", 30, -1, 1),
        std::make_tuple("BDTB_200Cuts_Depth3", "BDTB output (200 cuts, depth 3)", 30, -1, 1),
        std::make_tuple("BDTB_200Cuts_1000Trees", "BDTB output (200 cuts, 1000 trees)", 30, -1, 1),
        */
        ////
        std::make_tuple("met", "E_{T}^{miss} (GeV)", 30, 0, 300),
        std::make_tuple("mll", "M_{ll} (GeV)", 60, 12, 200),
        std::make_tuple("mt", "M_{T} (GeV)", 30, 0, 300),
        std::make_tuple("leadPt", "P_{T}^{leading} (GeV)", 30, 25, 200),
        std::make_tuple("subPt", "P_{T}^{subleading} (GeV)", 30, 15, 200),
        std::make_tuple("trailPt", "P_{T}^{trailing} (GeV)", 30, 10, 200),
        std::make_tuple("nJets", "number of jets", 10, 0, 10),
        std::make_tuple("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8),
        std::make_tuple("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
        std::make_tuple("jetEta_highestEta", "|#eta| (most forward jet)", 30, 0, 5),
        std::make_tuple("jetEta_leading", "|#eta| (leading jet)", 30, 0, 5),
        std::make_tuple("jetLeadPt", "P_{T} (leading jet) (GeV)", 30, 0, 300),
        std::make_tuple("trailingJetPt", "P_{T} (trailing jet) (GeV)", 30, 0, 200), 
        std::make_tuple("leadinBJetPt", "P_{T} (leading b-jet) (GeV)", 30, 0, 200),
        std::make_tuple("trailingBJetPt", "P_{T} (trailing b-jet) (GeV)", 30, 0, 200),
        std::make_tuple("jetHighestEtaPt", "P_{T} (most forward jet) (GeV)", 30, 0, 300),
        std::make_tuple("mtop", "M_{(W + b)} (GeV)", 30, 0, 400),
        std::make_tuple("m3l", "M_{3l} (GeV)", 30, 0, 600),
        std::make_tuple("taggedBJetPt", "P_{T} (b-jet from top) (GeV)", 30, 0, 300),
        std::make_tuple("taggedBJetEta", "|#eta| (b-jet from top) (GeV)", 30, 0, 2.5),
        std::make_tuple("taggedRecoilJetPt", "P_{T} (recoiling jet) (GeV)", 30, 0, 300), 
        std::make_tuple("taggedRecoilJetEta", "|#eta| (recoiling jet) (GeV)", 30, 0, 5),
        std::make_tuple("m_highestEta_leadingB_W", "M_{(most forward jet + leading b-jet + W)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_highestEta_leadingB_Wlep", "M_{(most forward jet + leading b-jet + lepton)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_highestEta_leadingB_WZ", "M_{(most forward jet + leading b-jet + WZ)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_highestEta_leadingB_WlepZ", "M_{(most forward jet + leading b-jet + lepton + Z)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_taggedRecoil_taggedB_W", "M_{(recoiling jet + tagged b-jet + W)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_taggedRecoil_taggedB_Wlep", "M_{(recoiling jet + tagged b-jet + lepton)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_taggedRecoil_taggedB_WZ", "M_{(recoiling jet + tagged b-jet + WZ)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_taggedRecoil_taggedB_WlepZ", "M_{(recoiling jet + tagged b-jet + lepton + Z)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_forwardJets_leadingB_W", "M_{(forward jets + leading b-jet + W)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_forwardJets_leadingB_Wlep", "M_{(forward jets + leading b-jet + lepton)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_forwardJets_leadingB_WZ", "M_{(forward jets + leading b-jet + WZ)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_forwardJets_leadingB_WlepZ", "M_{(forward jets + leading b-jet + lepton + Z)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_forwardJets", "M_{(|#eta| > 2.4 jets)} (GeV)", 30, 0, 600),
        std::make_tuple("m_notSoForwardJets", "M_{(|#eta| > 0.8 jets)} (GeV)", 30, 0, 1000),
        std::make_tuple("m_superForwardJets", "M_{(|#eta| > 3 jets)} (GeV)", 30, 0, 600),

        std::make_tuple("pT_highestEta_leadingB_W", "P_{T}^{most forward jet + leading b-jet + W} (GeV)", 30, 0, 600),
        std::make_tuple("pT_highestEta_leadingB_Wlep", "P_{T}^{most forward jet + leading b-jet + lepton} (GeV)", 30, 0, 600),
        std::make_tuple("pT_highestEta_leadingB_WZ", "P_{T}^{most forward jet + leading b-jet + WZ} (GeV)", 30, 0, 600),
        std::make_tuple("pT_highestEta_leadingB_WlepZ", "P_{T}^{most forward jet + leading b-jet + lepton + Z} (GeV)", 30, 0, 600),
        std::make_tuple("pT_taggedRecoil_taggedB_W", "P_{T}^{recoiling jet + tagged b-jet + W} (GeV)", 30, 0, 600),
        std::make_tuple("pT_taggedRecoil_taggedB_Wlep", "P_{T}^{recoiling jet + tagged b-jet + lepton} (GeV)", 30, 0, 600),
        std::make_tuple("pT_taggedRecoil_taggedB_WZ", "P_{T}^{recoiling jet + tagged b-jet + WZ} (GeV)", 30, 0, 600),
        std::make_tuple("pT_taggedRecoil_taggedB_WlepZ", "P_{T}^{recoiling jet + tagged b-jet + lepton + Z} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets_leadingB_W", "P_{T}^{forward jets + leading b-jet + W} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets_leadingB_Wlep", "P_{T}^{forward jets + leading b-jet + lepton} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets_leadingB_WZ", "P_{T}^{forward jets + leading b-jet + WZ} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets_leadingB_WlepZ", "P_{T}^{forward jets + leading b-jet + lepton + Z} (GeV)", 30, 0, 600),
        std::make_tuple("pT_forwardJets", "P_{T}^{|#eta| > 2.4 jets} (GeV)", 30, 0, 600),
        std::make_tuple("pT_notSoForwardJets", "P_{T}^{|#eta| > 0.8 jets} (GeV)", 30, 0, 600),
        std::make_tuple("pT_superForwardJets", "P_{T}^{|#eta| > 3 jets} (GeV)", 30, 0, 600),

        std::make_tuple("eta_forwardJets", "|#eta|^{|#eta| > 2.4 jets}", 30, 0, 5),
        std::make_tuple("eta_notSoForwardJets", "|#eta|^{|#eta| > 0.8 jets}", 30, 0, 5),
        std::make_tuple("eta_superForwardJets", "|#eta|^{|#eta| > 3 jets}", 30, 0, 5),

        std::make_tuple("highestDeepCSV", "highest deepCSV (b + bb)", 30, 0, 1), 
        std::make_tuple("highestDeepCSVb", "highest deepCSV (b)", 30, 0, 1), 
        std::make_tuple("highestDeepCSVbb", "highest deepCSV( bb)", 30, 0, 1), 
        std::make_tuple("highestCSVv2", "highest CSVv2", 30, 0, 1),
        std::make_tuple("highestDeepCSVJetPt", "P_{T}(highest DeepCSV jet) (GeV)", 30, 0, 300),
        std::make_tuple("deltaRTaggedBJetRecoilingJet", "#DeltaR(tagged b-jet, recoiling jet)", 30, 0, 10),
        std::make_tuple("deltaRleadingBJet_highestEtaJet", "#DeltaR(leading b-jet, most forward jet)", 30, 0, 10),
        std::make_tuple("deltaRhighestDeepCSVJet_highestEtaJet", "#DeltaR(highest DeepCSV jet, most forward jet)", 30, 0, 10),

        std::make_tuple("minMLeptonJet", "M_{l + jet}^{min} (GeV)", 30, 0, 200),
        std::make_tuple("maxMLeptonJet", "M_{l + jet}^{max} (GeV)", 30, 0, 800),
        std::make_tuple("minMLeptonBJet", "M_{l + bjet}^{min} (GeV)", 30, 0, 300),
        std::make_tuple("maxMLeptonBJet", "M_{l + bjet}^{max} (GeV)", 30, 0, 800),
        std::make_tuple("minMJetJet", "M_{jet + jet}^{min} (GeV)", 30, 0, 500),
        std::make_tuple("maxMJetJet", "M_{jet + jet}^{max} (GeV)", 30, 0, 800),
        std::make_tuple("minMLeptonLepton", "M_{2l}^{min} (GeV)", 30, 0, 200),
        std::make_tuple("maxMLeptonLepton", "M_{2l}^{max} (GeV)", 30, 0, 500),

        std::make_tuple("minDeltaPhiLeptonJet", "min(#Delta#Phi(l, jet))", 30, 0, 3.15),
        std::make_tuple("maxDeltaPhiLeptonJet", "max(#Delta#Phi(l, jet))", 30, 0, 3.15),
        std::make_tuple("minDeltaPhiLeptonBJet", "min(#Delta#Phi(l, bjet))", 30, 0, 3.15),
        std::make_tuple("maxDeltaPhiLeptonBJet", "max(#Delta#Phi(l, bjet))", 30, 0, 3.15),
        std::make_tuple("minDeltaPhiJetJet", "min(#Delta#Phi(jet, jet))", 30, 0, 3.15),
        std::make_tuple("maxDeltaPhiJetJet", "max(#Delta#Phi(jet, jet))", 30, 0, 3.15),
        std::make_tuple("minDeltaPhiLeptonLepton", "min(#Delta#Phi(l, l))", 30, 0, 3.15),
        std::make_tuple("maxDeltaPhiLeptonLepton", "max(#Delta#Phi(l, l))", 30, 0, 3.15),

        std::make_tuple("minDeltaRLeptonJet", "min(#DeltaR(l, jet))", 30, 0, 7),
        std::make_tuple("maxDeltaRLeptonJet", "max(#DeltaR(l, jet))", 30, 0, 10),
        std::make_tuple("minDeltaRLeptonBJet", "min(#DeltaR(l, bjet))", 30, 0, 7),
        std::make_tuple("maxDeltaRLeptonBJet", "max(#DeltaR(l, bjet))", 30, 0, 10),
        std::make_tuple("minDeltaRJetJet", "min(#DeltaR(jet, jet))", 30, 0, 7),
        std::make_tuple("maxDeltaRJetJet", "max(#DeltaR(jet, jet))", 30, 0, 10),
        std::make_tuple("minDeltaRLeptonLepton", "min(#DeltaR(l, l))", 30, 0, 7),
        std::make_tuple("maxDeltaRLeptonLepton", "max(#DeltaR(l, l))", 30, 0, 10),

        std::make_tuple("minpTLeptonJet", "P_{T}^{min}(l + jet) (GeV)", 30, 0, 200),
        std::make_tuple("maxpTLeptonJet", "P_{T}^{max}(l + jet) (GeV)", 30, 0, 300),
        std::make_tuple("minpTLeptonBJet", "P_{T}^{min}(l + bjet) (GeV)", 30, 0, 200),
        std::make_tuple("maxpTLeptonBJet", "P_{T}^{max}(l + bjet) (GeV)", 30, 0, 300),
        std::make_tuple("minpTJetJet", "P_{T}^{min}(jet + jet) (GeV)", 30, 0, 200),
        std::make_tuple("maxpTJetJet", "P_{T}^{max}(jet + jet) (GeV)", 30, 0, 300),
        std::make_tuple("minpTLeptonLepton", "P_{T}^{min}(2l) (GeV)", 30, 0, 200),
        std::make_tuple("maxpTLeptonLepton", "P_{T}^{max}(2l) (GeV)", 30, 0, 300),

        std::make_tuple("HT", "H_{T} (GeV)", 30, 0, 800),
        std::make_tuple("pTZboson", "P_{T}(Z) (GeV)", 30, 0, 300),
        std::make_tuple("etaZboson", "|#eta|(Z)", 30, 0, 2.5),

        std::make_tuple("etaMostForwardLepton", "|#eta| (most forward lepton)", 30, 0, 2.5),
        std::make_tuple("etaLeading", "|#eta| (leading lepton)", 30, 0, 2.5),
        std::make_tuple("etaSubLeading", "|#eta| (subleading lepton)", 30, 0, 2.5),
        std::make_tuple("etaTrailing", "|#eta| (trailing lepton)", 30, 0, 2.5),
        std::make_tuple("etaWLep",  "|#eta| (lepton from W)", 30, 0, 2.5),
        std::make_tuple("pT3l", "P_{T}^{3l} (GeV)", 30, 0, 300),
        std::make_tuple("assymmetryLeading", "asymmetry (leading lepton)", 30, -2.5, 2.5),
        std::make_tuple("asymmetrySubleading", "asymmetry (subleading lepton)",30, -2.5, 2.5),
        std::make_tuple("asymmetryTrailing", "asymmetry (trailing lepton)",30, -2.5, 2.5),
        std::make_tuple("asymmetryWLep", "asymmetry (lepton from W)",30, -2.5, 2.5),
        std::make_tuple("deltaRWLepClosestJet", "#DeltaR(lepton from W, closest jet)",30, 0, 7), 
        std::make_tuple("deltaPhiWlepZ", "#DeltaR(lepton from W, Z)", 30, 0, 3.15),
        std::make_tuple("deltaPhiWlepTaggedbJet", "#DeltaR(lepton from W, tagged b-jet)", 30, 0, 3.15),
        std::make_tuple("deltaRWlepRecoilingJet", "#DeltaR(lepton from W, recoiling jet)", 30, 0, 7),
        std::make_tuple("deltaRZTop", "#DeltaR(top, Z)", 30, 0, 7),
        std::make_tuple("pTTop", "P_{T}^{top} (GeV)", 30, 0, 300), 
        std::make_tuple("pTtzq", "P_{T}^{tZq} (GeV)", 30, 0, 300),

        std::make_tuple("mt3l", "M_{T}(3l + MET) (GeV)", 30, 0, 500),
        std::make_tuple("mtZlep", "M_{T}(Z + MET) (GeV)", 30, 0, 400),
        std::make_tuple("minDeltaPhiLepMET", "min(#Delta#Phi(l,MET))", 30, 0, 3.15),
        std::make_tuple("maxDeltaPhiLepMET", "max(#Delta#Phi(l,MET))", 30, 0, 3.15),
        std::make_tuple("minDeltaPhiJetMET", "min(#Delta#Phi(jet,MET))", 30, 0, 3.15),
        std::make_tuple("maxDeltaPhiJetMET", "max(#Delta#Phi(jet,MET))", 30, 0, 3.15),
        std::make_tuple("minDeltaPhiBJetMET", "min(#Delta#Phi(bjet,MET))", 30, 0, 3.15),
        std::make_tuple("maxDeltaPhiBJetMET", "max(#Delta#Phi(bjet,MET))", 30, 0, 3.15),
        std::make_tuple("mtMinLepMET", "min(M_{T}(l + MET)) (GeV)", 30, 0, 300),
        std::make_tuple("mtMaxLepMET", "max(M_{T}(l + MET)) (GeV)", 30, 0, 400),
        std::make_tuple("mtMinJetMET", "min(M_{T}(jet + MET)) (GeV)", 30, 0, 300),
        std::make_tuple("mtMaxJetMET", "max(M_{T}(jet + MET)) (GeV)", 30, 0, 400),
        std::make_tuple("mtMinBJet", "min(M_{T}(bjet + MET)) (GeV)", 30, 0, 300),
        std::make_tuple("mtMaxBJet", "max(M_{T}(bjet + MET)) (GeV)", 30, 0, 400),
        std::make_tuple("pTMinLeptonMET", "min(P_{T}(lepton + MET)) (GeV)", 30, 0, 300),
        std::make_tuple("pTMaxLeptonMET", "max(P_{T}(lepton + MET)) (GeV)", 30, 0, 400),
        std::make_tuple("pTMinJetMET", "min(P_{T}(lepton + MET)) (GeV)", 30, 0, 300), 
        std::make_tuple("pTMaxJetMET", "max(P_{T}(lepton + MET)) (GeV)", 30, 0, 400),
        std::make_tuple("pTMinBJetMET", "min(P_{T}(lepton + MET)) (GeV)", 30, 0, 300),
        std::make_tuple("pTMaxBJetMET", "max(P_{T}(lepton + MET)) (GeV)", 30, 0, 400),
        std::make_tuple("mtLeadingLepMET", "M_{T}(leading lepton, MET) (GeV)", 30, 0, 300),
        std::make_tuple("mtSubLeadingLepMET", "M_{T}(subleading lepton, MET) (GeV)", 30, 0, 300),
        std::make_tuple("mtTrainingLepMET", "M_{T}(trailing lepton, MET) (GeV)", 30, 0, 300),
        std::make_tuple("mtJetsMET", "M_{T}(Jets + MET) (GeV)", 30, 0, 700),
        std::make_tuple("mtJetsLeptonsMET", "M_{T}(Jets leptons + MET) (GeV)", 30, 0, 700), 
        std::make_tuple("mSystem", "M_{(Jets + leptons + neutrino)} (GeV)", 30, 0, 1000), 
        std::make_tuple("mJets", "M_{(all Jets)} (GeV)", 30, 0, 1000), 
        std::make_tuple("mtLeadingJetMET", "M_{T}(leading jet + MET) (GeV)", 30, 0, 300),
        std::make_tuple("mtTrailingJetMET", "M_{T}(trailing jet + MET) (GeV)", 30, 0, 300)
    };

    const unsigned nDist = histInfo.size(); //number of distributions to plot
    const unsigned nCat = 7;                //Several categories enriched in different processes
    const unsigned nMll = 3;                //categories based on dilepton Mass
    const std::string mllNames[nMll] = {"mllInclusive", "onZ", "offZ"};
    const std::string catNames[nCat] = {"nJetsInclusive", "0bJets_01Jets", "0bJets_2Jets", "1bJet_01jets", "1bJet_23Jets", "1bJet_4Jets", "2bJets"};
    //initialize vector holding all histograms
    std::vector< std::vector < std::vector< std::vector< TH1D* > > > > hists(nMll);
    for(unsigned m = 0; m < nMll; ++m){
        hists[m] = std::vector< std::vector < std::vector< TH1D* > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                hists[m][cat].push_back(std::vector < TH1D* >() );
                for(size_t sam = 0; sam < samples.size(); ++sam){
                    hists[m][cat][dist].push_back(nullptr);
                    hists[m][cat][dist][sam] = new TH1D( (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) + catNames[cat] + mllNames[m]), (const TString&) (std::get<1>(samples[sam]) + std::get<0>(histInfo[dist]) + catNames[cat] + mllNames[m] + ";" + std::get<1>(histInfo[dist]) + ";Events" ),  std::get<2>(histInfo[dist]), std::get<3>(histInfo[dist]), std::get<4>(histInfo[dist]) );
                }
            }
        }
    }

    //store maxima of histograms for overflow bins
    double maxBin[nDist];
    for(unsigned dist = 0; dist < nDist; ++dist){
        maxBin[dist] = std::get<4>(histInfo[dist]) - 0.5*(std::get<4>(histInfo[dist]) - std::get<3>(histInfo[dist]) )/std::get<2>(histInfo[dist]);
    }

    //tweakable options
    const TString extra = ""; //for plot names

    Float_t topMass, pTForwardJets, etaMostForward, eventWeight, missingET, pTLeadingBJet;
    Float_t numberOfJets, numberOfBJets, pTLeadingJet, mNotSoForwardJets, pTLeadingLepton, highestDeepCSV;
    Float_t maxMjj, minMlb, asymmetryWlep, etaZ, m3l, maxDeltaPhijj, maxDeltaRjj, maxDeltaPhill, maxM2l;
    Float_t pTMaxlb, pTMax2l, pT3l, mForwardJetsLeadinBJetW, ht;
    /*
    //tree for BDT training
    TFile treeFile("trainingTrees/bdtTrainingTree.root","RECREATE");
    TTree *tree[2][nCat][nMll];
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            tree[0][cat][m] = new TTree((const TString&) "signalTree" + catNames[cat] + mllNames[m],(const TString&) "tZq signal tree" + catNames[cat] + mllNames[m]);
            tree[1][cat][m] = new TTree((const TString&) "backgroundTree" + catNames[cat] + mllNames[m], (const TString&) "tZq background tree" + catNames[cat] + mllNames[m]);
            for(unsigned t = 0; t < 2; ++t){
                tree[t][cat][m]->Branch("topMass", &topMass, "topMass/F");
                tree[t][cat][m]->Branch("pTForwardJets", &pTForwardJets, "pTForwardJets/F");
                tree[t][cat][m]->Branch("etaMostForward", &etaMostForward, "etaMostForward/F");
                tree[t][cat][m]->Branch("numberOfBJets", &numberOfBJets, "numberOfBJets/F");
                tree[t][cat][m]->Branch("numberOfJets", &numberOfJets, "numberOfJets/F");
                tree[t][cat][m]->Branch("pTLeadingJet", &pTLeadingJet, "pTLeadingJet/F");
                tree[t][cat][m]->Branch("pTLeadingLepton", &pTLeadingLepton, "pTLeadingLepton/F");
                tree[t][cat][m]->Branch("mNotSoForwardJets", &mNotSoForwardJets, "mNotSoForwardJets/F");
                tree[t][cat][m]->Branch("pTLeadingBJet", &pTLeadingBJet, "pTLeadingBJet/F");
                tree[t][cat][m]->Branch("missingET", &missingET, "missingET/F");
                tree[t][cat][m]->Branch("highestDeepCSV", &highestDeepCSV, "highestDeepCSV/F");
                tree[t][cat][m]->Branch("maxMjj", &maxMjj, "maxMjj/F");
                tree[t][cat][m]->Branch("minMlb", &minMlb, "minMlb/F");
                tree[t][cat][m]->Branch("asymmetryWlep", &asymmetryWlep, "asymmetryWlep/F");
                tree[t][cat][m]->Branch("etaZ", &etaZ, "etaZ/F");
                tree[t][cat][m]->Branch("m3l", &m3l, "m3l/F");
                tree[t][cat][m]->Branch("maxDeltaPhijj", &maxDeltaPhijj, "maxDeltaPhijj/F");
                tree[t][cat][m]->Branch("maxDeltaRjj", &maxDeltaRjj, "maxDeltaRjj/F");
                tree[t][cat][m]->Branch("maxDeltaPhill", &maxDeltaPhill, "maxDeltaPhill/F");
                tree[t][cat][m]->Branch("maxM2l", &maxM2l, "maxM2l/F");
                tree[t][cat][m]->Branch("pTMaxlb", &pTMaxlb, "pTMaxlb/F");
                tree[t][cat][m]->Branch("pTMax2l", &pTMax2l, "pTMax2l/F");
                tree[t][cat][m]->Branch("pT3l", &pT3l, "pT3l/F");
                tree[t][cat][m]->Branch("mForwardJetsLeadinBJetW", &mForwardJetsLeadinBJetW, "mForwardJetsLeadinBJetW/F");
                tree[t][cat][m]->Branch("ht", &ht, "ht/F");
                //event weights
                tree[t][cat][m]->Branch("eventWeight", &eventWeight, "eventWeight/F");
            }
        }
    }
    */
    const std::vector<std::string> mvaMethods = {"BDT", "BDTG", "BDTD", "BDTB", "MLP", "DNN", "BDTGAlt", "BDTGAlt_200trees", "BDTGAlt_shrinkage04", "BDTGAlt_minNode005", "BDTGAlt_negWeights", "BDTGAlt_MaxDepth4", "BDTGAlt_20Cuts",
                                     "BDTGAlt_negWeights_SDivSqrtSPlusB", "BDTGAlt_shrinkage02_20Cuts", "BDTGAlt_shrinkage04_20Cuts", "BDTGAlt_shrinkage02_20Cuts_depth3", "BDTGAlt_200trees_MaxDepth3_shrinkage02",
                                     "BDTGAlt_MaxDepth3", "BDTGAlt_Decorrelate",
                                     "BDTAda_200Cuts", "BDTAda_200Cuts_Depth2", "BDTAda_200Cuts_Depth2_Beta01", "BDTAda_200Cuts_Depth2_200Trees", "BDTAda_200Cuts_SDivSqrtSPlusB", "BDTAda_200Cuts_ignoreNegative",
                                     "BDTAda_200Cuts_Decorrelate",
                                     "BDTB_200Cuts", "BDTB_200Cuts_Depth3", "BDTB_200Cuts_1000Trees"}; 

    //MVA reader 
    TMVA::Reader *mvaReader[nMll][nCat]; //one BDT for every category
    for(unsigned m = 0; m < nMll - 1; ++m){
        for(unsigned cat = 0; cat < nCat - 1; ++cat){
            if(catNames[cat + 1] == "0bJets_01Jets") continue;
            mvaReader[m][cat] = new TMVA::Reader( "!Color:!Silent" );
            mvaReader[m][cat]->AddVariable("topMass", &topMass);
            if(catNames[cat + 1] != "1bJet_01jets") mvaReader[m][cat]->AddVariable("pTForwardJets", &pTForwardJets);
            mvaReader[m][cat]->AddVariable("etaMostForward", &etaMostForward);
            if(catNames[cat + 1]  == "1bJet_4Jets" || catNames[cat + 1] == "2bJets") mvaReader[m][cat]->AddVariable("numberOfJets", &numberOfJets);
            if(catNames[cat + 1]  != "1bJet_4Jets" && catNames[cat + 1] != "2bJets") mvaReader[m][cat]->AddVariable("pTLeadingJet", &pTLeadingJet);
            mvaReader[m][cat]->AddVariable("pTLeadingLepton", &pTLeadingLepton);
            mvaReader[m][cat]->AddVariable("mNotSoForwardJets", &mNotSoForwardJets);
            if(catNames[cat + 1] != "1bJet_01jets" && catNames[cat + 1] != "0bJets_01Jets" && catNames[cat + 1]  != "1bJet_4Jets" && catNames[cat + 1] != "2bJets") mvaReader[m][cat]->AddVariable("pTLeadingBJet", &pTLeadingBJet);
            mvaReader[m][cat]->AddVariable("missingET", &missingET);
            mvaReader[m][cat]->AddVariable("highestDeepCSV", &highestDeepCSV);
            if(catNames[cat + 1] != "1bJet_01jets" && catNames[cat + 1] != "0bJets_01Jets")  mvaReader[m][cat]->AddVariable("maxMjj", &maxMjj);
            if(catNames[cat + 1] != "0bJets_2Jets" && catNames[cat + 1] != "0bJets_01Jets")  mvaReader[m][cat]->AddVariable("minMlb", &minMlb);
            if(catNames[cat + 1] != "1bJet_01jets") mvaReader[m][cat]->AddVariable("asymmetryWlep", &asymmetryWlep);
            mvaReader[m][cat]->AddVariable("etaZ", &etaZ);
            mvaReader[m][cat]->AddVariable("m3l", &m3l);
            if(catNames[cat + 1] != "1bJet_01jets" && catNames[cat + 1] != "0bJets_01Jets" && catNames[cat + 1]  != "1bJet_4Jets" && catNames[cat + 1] != "2bJets") mvaReader[m][cat]->AddVariable("maxDeltaPhijj", &maxDeltaPhijj);
            if(catNames[cat + 1] != "1bJet_01jets" && catNames[cat + 1] != "0bJets_01Jets") mvaReader[m][cat]->AddVariable("maxDeltaRjj", &maxDeltaRjj);
            mvaReader[m][cat]->AddVariable("maxDeltaPhill", &maxDeltaPhill);
            if(catNames[cat + 1] != "0bJets_2Jets" && catNames[cat + 1] != "0bJets_01Jets") mvaReader[m][cat]->AddVariable("pTMaxlb", &pTMaxlb);
            mvaReader[m][cat]->AddVariable("pT3l", &pT3l);
            mvaReader[m][cat]->AddVariable("mForwardJetsLeadinBJetW", &mForwardJetsLeadinBJetW);
            mvaReader[m][cat]->AddVariable("ht", &ht);
            /*
            mvaReader[m][cat]->BookMVA("BDT method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDT.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTG method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTG.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTD method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTD.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTB method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTB.weights.xml");
            mvaReader[m][cat]->BookMVA("MLP method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_MLP.weights.xml");
            mvaReader[m][cat]->BookMVA("DNN method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_DNN.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTGAlt method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTGAlt.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTGAlt_200trees method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTGAlt_200trees.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTGAlt_shrinkage04 method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTGAlt_shrinkage04.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTGAlt_minNode005 method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTGAlt_minNode005.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTGAlt_negWeights method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTGAlt_negWeights.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTGAlt_MaxDepth4 method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTGAlt_MaxDepth4.weights.xml");
            mvaReader[m][cat]->BookMVA("BDTGAlt_20Cuts method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_BDTGAlt_20Cuts.weights.xml");
            */
            for(auto it = mvaMethods.cbegin(); it != mvaMethods.cend(); ++it){
                //mvaReader[m][cat]->BookMVA(*it + " method", "bdtTraining/bdtWeights/" + catNames[cat + 1] + "_" + mllNames[m + 1] + "_" + *it + ".weights.xml");
            }
        }
    }
    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        if(sam == 0){                   //skip data for now
            ++currentSample;
            continue;
        }
        initSample(0);          //Use combined 2016 + 2017 luminosity
        std::cout<<"Entries in "<< std::get<1>(samples[sam]) << " " << nEntries << std::endl;
        double progress = 0; 	//for printing progress bar
        for(long unsigned it = 0; it < nEntries; ++it){
            //print progress bar	
            if(it%100 == 0 && it != 0){
                progress += (double) (100./nEntries);
                tools::printProgress(progress);
            } else if(it == nEntries -1){
                progress = 1.;
                tools::printProgress(progress);
            }
            GetEntry(it);
            //vector containing good lepton indices
            std::vector<unsigned> ind;
            //select leptons
            const unsigned lCount = selectLep(ind);
            if(lCount != 3) continue;
            //require pt cuts (25, 15, 10) to be passed
            if(tightLepCount(ind, lCount) != 3) continue; //require 3 tight leptons
            if(!passPtCuts(ind)) continue;
            //require presence of OSSF pair
            if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 
            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
            //require best Z mass to be onZ
            std::pair<unsigned, unsigned> bestZ = trilep::bestZ(lepV, ind, _lFlavor, _lCharge, lCount);
            //make ordered jet and bjet collections
            std::vector<unsigned> jetInd, bJetInd;
            unsigned jetCount = nJets(jetInd);
            unsigned bJetCount = nBJets(bJetInd);
            //find highest eta jet
            unsigned highestEtaJ = (jetCount == 0) ? 99 : jetInd[0];
            for(unsigned j = 1; j < jetCount; ++j){
                if(fabs(_jetEta[jetInd[j]]) > fabs(_jetEta[highestEtaJ]) ) highestEtaJ = jetInd[j];
            }
            //Determine tZq analysis category
            unsigned tzqCat = tzq::cat(jetCount, bJetCount);
            //Determine mll category
            unsigned mllCat = 1;                      //offZ by default
            double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();
            if( fabs(mll - 91.1876) < 15) mllCat = 0; //offZ    
            //make LorentzVector for all jets 
            TLorentzVector jetV[(const unsigned) _nJets];
            for(unsigned j = 0; j < _nJets; ++j){
                jetV[j].SetPtEtaPhiE(_jetPt[j], _jetEta[j], _jetPhi[j], _jetE[j]);
            }
            //find W lepton 
            unsigned lw;
            for(unsigned l = 0; l < lCount; ++l){
                if( l != bestZ.first && l != bestZ.second ) lw = l;
            }
            //make met vector 
            TLorentzVector met;
            met.SetPtEtaPhiE(_met, _metPhi, 0, _met);
            //reconstruct top mass and tag jets
            std::vector<unsigned> taggedJetI; //0 -> b jet from tZq, 1 -> forward recoiling jet
            TLorentzVector neutrino = tzq::findBestNeutrinoAndTop(lepV[lw], met, taggedJetI, jetInd, bJetInd, jetV);

            //forward jet sum
            TLorentzVector forwardJets(0,0,0,0);
            //not so forward jet sum
            TLorentzVector notSoForwardJets(0,0,0,0);
            //super forward jet sum
            TLorentzVector superForwardJets(0,0,0,0);
            //jet sum 
            TLorentzVector jetSystem(0,0,0,0);
            for(unsigned j = 0; j < jetCount; ++j){
                jetSystem += jetV[jetInd[j]];
                if(fabs(_jetEta[jetInd[j]]) >= 2.4) forwardJets += jetV[jetInd[j]];
                if(fabs(_jetEta[jetInd[j]]) >= 0.8) notSoForwardJets += jetV[jetInd[j]];
                if(fabs(_jetEta[jetInd[j]]) >= 3.0) superForwardJets += jetV[jetInd[j]];
            } 
            //find jets with highest DeepCSV and CSVv2 values
            unsigned highestDeepCSVI = (jetCount == 0) ? 0 : jetInd[0], highestCSVv2I = (jetCount == 0) ? 0 : jetInd[0];
            for(unsigned j = 1; j < jetCount; ++j){
                if( (_jetDeepCsv_b[jetInd[j]] + _jetDeepCsv_bb[jetInd[j]]) > (_jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI]) ) highestDeepCSVI = jetInd[j];
                if( _jetCsvV2[jetInd[j]] > _jetCsvV2[jetInd[j]] ) highestCSVv2I = jetInd[j];
            }
        
            //initialize new vectors to make sure everything is defined for 0 jet events!
            TLorentzVector leadingJet(0,0,0,0);
            TLorentzVector trailingJet(0,0,0,0);
            TLorentzVector highestEtaJet(0,0,0,0);
            TLorentzVector recoilingJet(0,0,0,0);
            TLorentzVector taggedBJet(0,0,0,0);
            TLorentzVector leadingBJet(0,0,0,0);
            TLorentzVector trailingBJet(0,0,0,0);
            TLorentzVector highestDeepCSVJet(0,0,0,0);
            TLorentzVector highestCSVv2Jet(0,0,0,0);
            if(taggedJetI[0] != 99) taggedBJet = jetV[taggedJetI[0]];
            if(taggedJetI[1] != 99) recoilingJet = jetV[taggedJetI[1]];
            if(jetCount != 0){
                leadingJet = jetV[jetInd[0]];
                trailingJet = jetV[jetInd[jetInd.size() - 1]];
                highestEtaJet = jetV[highestEtaJ];
                highestDeepCSVJet = jetV[highestDeepCSVI];
                highestCSVv2Jet = jetV[highestCSVv2I]; 
            }
            if(bJetCount != 0){
                leadingBJet = jetV[bJetInd[0]];
                if(bJetCount > 1) trailingBJet = jetV[bJetInd[bJetInd.size() - 1]];
            } else if(jetCount > 1){
                leadingBJet = jetV[jetInd[1]];
            }
            //compute top vector
            TLorentzVector topV = (neutrino + lepV[lw] + taggedBJet);
        
            //Compute minimum and maximum masses and separations for several objects
            //lepton Jet 
            double minMLeptonJet = 99999.;
            double maxMLeptonJet = 0.;
            double minDeltaRLeptonJet = 99999.;
            double maxDeltaRLeptonJet = 0.;
            double minDeltaPhiLeptonJet = 99999.;
            double maxDeltaPhiLeptonJet = 0.;
            double minpTLeptonJet = 99999.;
            double maxpTLeptonJet = 0.;
            for(unsigned l = 0; l < lCount; ++l){
                for(unsigned j = 0; j < jetCount; ++j){
                    if( (lepV[l] + jetV[jetInd[j]]).M() < minMLeptonJet) minMLeptonJet = (lepV[l] + jetV[jetInd[j]]).M();
                    if( (lepV[l] + jetV[jetInd[j]]).M() > maxMLeptonJet) maxMLeptonJet = (lepV[l] + jetV[jetInd[j]]).M();
                    if( lepV[l].DeltaR(jetV[jetInd[j]]) < minDeltaRLeptonJet) minDeltaRLeptonJet = lepV[l].DeltaR(jetV[jetInd[j]]);
                    if( lepV[l].DeltaR(jetV[jetInd[j]]) > maxDeltaRLeptonJet) maxDeltaRLeptonJet = lepV[l].DeltaR(jetV[jetInd[j]]);
                    if( fabs(lepV[l].DeltaPhi(jetV[jetInd[j]]) ) < minDeltaPhiLeptonJet) minDeltaPhiLeptonJet = fabs(lepV[l].DeltaPhi(jetV[jetInd[j]]));
                    if( fabs(lepV[l].DeltaPhi(jetV[jetInd[j]]) ) > maxDeltaPhiLeptonJet) maxDeltaPhiLeptonJet = fabs(lepV[l].DeltaPhi(jetV[jetInd[j]]));
                    if( (lepV[l] + jetV[jetInd[j]]).Pt()  < minpTLeptonJet) minpTLeptonJet = (lepV[l] + jetV[jetInd[j]]).Pt();
                    if( (lepV[l] + jetV[jetInd[j]]).Pt()  > maxpTLeptonJet) maxpTLeptonJet = (lepV[l] + jetV[jetInd[j]]).Pt();
                }
            }
            if(minDeltaRLeptonJet == 99999.) minDeltaRLeptonJet = 0.;
            if(minDeltaPhiLeptonJet == 99999.) minDeltaPhiLeptonJet = 0.;
            if(minMLeptonJet == 99999.) minMLeptonJet = 0.;
            if(minpTLeptonJet == 99999.) minpTLeptonJet = 0.;
            //lepton bjet
            double minMLeptonbJet = 99999.;
            double maxMLeptonbJet = 0.;
            double minDeltaRLeptonbJet = 99999.;
            double maxDeltaRLeptonbJet = 0.;
            double minDeltaPhiLeptonbJet = 99999.;
            double maxDeltaPhiLeptonbJet = 0.;
            double minpTLeptonbJet = 99999.;
            double maxpTLeptonbJet = 0.;
            for(unsigned l = 0; l < lCount; ++l){
                for(unsigned j = 0; j < bJetCount; ++j){
                    if( (lepV[l] + jetV[bJetInd[j]]).M() < minMLeptonbJet) minMLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).M();
                    if( (lepV[l] + jetV[bJetInd[j]]).M() > maxMLeptonbJet) maxMLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).M();
                    if( lepV[l].DeltaR(jetV[bJetInd[j]]) < minDeltaRLeptonbJet) minDeltaRLeptonbJet = lepV[l].DeltaR(jetV[bJetInd[j]]);
                    if( lepV[l].DeltaR(jetV[bJetInd[j]]) > maxDeltaRLeptonbJet) maxDeltaRLeptonbJet = lepV[l].DeltaR(jetV[bJetInd[j]]);
                    if( fabs(lepV[l].DeltaPhi(jetV[bJetInd[j]]) ) < minDeltaPhiLeptonbJet) minDeltaPhiLeptonbJet = fabs(lepV[l].DeltaPhi(jetV[bJetInd[j]]));
                    if( fabs(lepV[l].DeltaPhi(jetV[bJetInd[j]]) ) > maxDeltaPhiLeptonbJet) maxDeltaPhiLeptonbJet = fabs(lepV[l].DeltaPhi(jetV[bJetInd[j]]));
                    if( (lepV[l] + jetV[bJetInd[j]]).Pt() < minpTLeptonbJet ) minpTLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).Pt();
                    if( (lepV[l] + jetV[bJetInd[j]]).Pt() > maxpTLeptonbJet ) maxpTLeptonbJet = (lepV[l] + jetV[bJetInd[j]]).Pt();
                }
            }
            if(minDeltaRLeptonbJet == 99999.) minDeltaRLeptonbJet = 0.;
            if(minDeltaPhiLeptonbJet == 99999.) minDeltaPhiLeptonbJet = 0.;
            if(minMLeptonbJet == 99999.) minMLeptonbJet = 0.;
            if(minpTLeptonbJet ==  99999.) minpTLeptonbJet = 0.;
            //jet jet
            double minMJetJet = 99999.;
            double maxMJetJet = 0.;
            double minDeltaRJetJet = 99999.;
            double maxDeltaRJetJet = 0.;
            double minDeltaPhiJetJet = 99999.;
            double maxDeltaPhiJetJet = 0.;
            double minpTJetJet = 99999.;
            double maxpTJetJet = 0.;
            if(jetCount != 0){
                for(unsigned l = 0; l < jetCount - 1; ++l){
                    for(unsigned j = l + 1; j < jetCount; ++j){
                        if( (jetV[jetInd[l]] + jetV[jetInd[j]]).M() < minMJetJet) minMJetJet = (jetV[jetInd[l]] + jetV[jetInd[j]]).M();
                        if( (jetV[jetInd[l]] + jetV[jetInd[j]]).M() > maxMJetJet) maxMJetJet = (jetV[jetInd[l]] + jetV[jetInd[j]]).M();
                        if( jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]) < minDeltaRJetJet) minDeltaRJetJet = jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]);
                        if( jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]) > maxDeltaRJetJet) maxDeltaRJetJet = jetV[jetInd[l]].DeltaR(jetV[jetInd[j]]);
                        if( fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]) ) < minDeltaPhiJetJet) minDeltaPhiJetJet = fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]));
                        if( fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]) ) > maxDeltaPhiJetJet) maxDeltaPhiJetJet = fabs(jetV[jetInd[l]].DeltaPhi(jetV[jetInd[j]]));
                        if( (jetV[jetInd[l]] + jetV[jetInd[j]]).Pt() < minpTJetJet ) minpTJetJet = (jetV[jetInd[l]] + jetV[jetInd[j]]).Pt();
                        if( (jetV[jetInd[l]] + jetV[jetInd[j]]).Pt() > maxpTJetJet ) maxpTJetJet = (jetV[jetInd[l]] + jetV[jetInd[j]]).Pt();
                    }
                }
            }
            if(minDeltaRJetJet == 99999.) minDeltaRJetJet = 0.;
            if(minDeltaPhiJetJet == 99999.) minDeltaPhiJetJet = 0.;
            if(minMJetJet == 99999.) minMJetJet = 0.;
            if(minpTJetJet == 99999.) minpTJetJet = 0.;
            //lepton lepton 
            double minMLeptonLepton = 99999.;
            double maxMLeptonLepton = 0.;
            double minDeltaRLeptonLepton = 99999.;
            double maxDeltaRLeptonLepton = 0.;
            double minDeltaPhiLeptonLepton = 99999.;
            double maxDeltaPhiLeptonLepton = 0.;
            double minpTLeptonLepton = 99999.;
            double maxpTLeptonLepton = 0.;
            for(unsigned l = 0; l < lCount - 1; ++l){
                for(unsigned j = l + 1; j < lCount; ++j){
                    if( (lepV[l] + lepV[j]).M() < minMLeptonLepton) minMLeptonLepton = (lepV[l] + lepV[j]).M();
                    if( (lepV[l] + lepV[j]).M() > maxMLeptonLepton) maxMLeptonLepton = (lepV[l] + lepV[j]).M();
                    if( lepV[l].DeltaR(lepV[j]) < minDeltaRLeptonLepton) minDeltaRLeptonLepton = lepV[l].DeltaR(lepV[j]);
                    if( lepV[l].DeltaR(lepV[j]) > maxDeltaRLeptonLepton) maxDeltaRLeptonLepton = lepV[l].DeltaR(lepV[j]);
                    if( fabs(lepV[l].DeltaPhi(lepV[j])) < minDeltaPhiLeptonLepton) minDeltaPhiLeptonLepton = fabs(lepV[l].DeltaPhi(lepV[j]));
                    if( fabs(lepV[l].DeltaPhi(lepV[j])) > maxDeltaPhiLeptonLepton) maxDeltaPhiLeptonLepton = fabs(lepV[l].DeltaPhi(lepV[j]));
                    if( (lepV[l] + lepV[j]).Pt() < minpTLeptonLepton) minpTLeptonLepton = (lepV[l] + lepV[j]).Pt();
                    if( (lepV[l] + lepV[j]).Pt() > maxpTLeptonLepton) maxpTLeptonLepton = (lepV[l] + lepV[j]).Pt();
                }
            }
            if(minDeltaRLeptonLepton == 99999.) minDeltaRLeptonLepton = 0.;
            if(minDeltaPhiLeptonLepton == 99999.) minDeltaPhiLeptonLepton = 0.;
            if(minMLeptonLepton == 99999.) minMLeptonLepton = 0.;
            if(minpTLeptonLepton == 99999.) minpTLeptonLepton = 0.;
            //compute HT
            double HT = 0;
            for(unsigned j = 0; j < jetCount; ++j){
                HT += _jetPt[jetInd[j]];
            }
            //find most forward lepton
            unsigned mostForwardLepInd = ind[0];
            for(unsigned l = 1; l < lCount; ++l){
                if(fabs(_lEta[ind[l]]) > fabs(_lEta[mostForwardLepInd]) ) mostForwardLepInd = ind[l];
            }
            //find closest jet to W lepton
            double deltaRWLepClosestJet = 99999.;
            for(unsigned j = 0; j < jetCount; ++j){
                if(jetV[jetInd[j]].DeltaR(lepV[lw]) < deltaRWLepClosestJet) deltaRWLepClosestJet = jetV[jetInd[j]].DeltaR(lepV[lw]);
            }
             
            unsigned forwardJetCount = 0;
            for(unsigned j = 0; j < jetCount; ++j){
                if( fabs(_jetEta[jetInd[j]]) > 0.8) ++forwardJetCount;
            }
            //lepton + MET 
            double minDeltaPhiLeptonMET = 99999.;
            double maxDeltaPhiLeptonMET = 0.;
            double minmTLeptonMET = 99999.;
            double maxmTLeptonMET = 0.;
            double minpTLeptonMET = 99999.;
            double maxpTLeptonMET = 0.;
            for(unsigned l = 0; l < lCount; ++l){
                if(fabs(lepV[l].DeltaPhi(met)) < minDeltaPhiLeptonMET) minDeltaPhiLeptonMET = fabs(lepV[l].DeltaPhi(met));
                if(fabs(lepV[l].DeltaPhi(met)) > maxDeltaPhiLeptonMET) maxDeltaPhiLeptonMET = fabs(lepV[l].DeltaPhi(met));
                if(tools::mt(lepV[l], met) < minmTLeptonMET) minmTLeptonMET = tools::mt(lepV[l], met);
                if(tools::mt(lepV[l], met) < maxmTLeptonMET) maxmTLeptonMET = tools::mt(lepV[l], met);
                if((lepV[l] + met).Pt() > minpTLeptonMET) minpTLeptonMET = (lepV[l] + met).Pt();
                if((lepV[l] + met).Pt() > maxpTLeptonMET) maxpTLeptonMET = (lepV[l] + met).Pt();
            }
            if(minDeltaPhiLeptonMET == 99999.) minDeltaPhiLeptonMET = 0;
            if(minmTLeptonMET == 99999.) minmTLeptonMET = 0.;
            if(minpTLeptonMET == 99999.) minpTLeptonMET = 0.;
            //jet + MET
            double minDeltaPhiJetMET = 99999.;
            double maxDeltaPhiJetMET = 0.;
            double minmTJetMET = 99999.;
            double maxmTJetMET = 0.;
            double minpTJetMET = 99999.;
            double maxpTJetMET = 0.;
            for(unsigned j = 0; j < jetCount; ++j){
                if(fabs(jetV[jetInd[j]].DeltaPhi(met)) < minDeltaPhiJetMET) minDeltaPhiJetMET = fabs(jetV[jetInd[j]].DeltaPhi(met));
                if(fabs(jetV[jetInd[j]].DeltaPhi(met)) > maxDeltaPhiJetMET) maxDeltaPhiJetMET = fabs(jetV[jetInd[j]].DeltaPhi(met));
                if(tools::mt(jetV[jetInd[j]], met) < minmTJetMET) minmTJetMET = tools::mt(jetV[jetInd[j]], met);
                if(tools::mt(jetV[jetInd[j]], met) < maxmTJetMET) maxmTJetMET = tools::mt(jetV[jetInd[j]], met);
                if((jetV[jetInd[j]] + met).Pt() > minpTJetMET) minpTJetMET = (jetV[jetInd[j]] + met).Pt();
                if((jetV[jetInd[j]] + met).Pt() > maxpTJetMET) maxpTJetMET = (jetV[jetInd[j]] + met).Pt();
            }
            if(minDeltaPhiJetMET == 99999.) minDeltaPhiJetMET = 0;
            if(minmTJetMET == 99999.) minmTJetMET = 0.;
            if(minpTLeptonMET == 99999.) minpTLeptonMET = 0.;
            //bjet + MET 
            double minDeltaPhiBJetMET = 99999.;
            double maxDeltaPhiBJetMET = 0.;
            double minmTBJetMET = 99999.;
            double maxmTBJetMET = 0.;
            double minpTBJetMET = 99999.;
            double maxpTBJetMET = 0.;
            for(unsigned j = 0; j < bJetCount; ++j){
                if(fabs(jetV[bJetInd[j]].DeltaPhi(met)) < minDeltaPhiBJetMET) minDeltaPhiBJetMET = fabs(jetV[bJetInd[j]].DeltaPhi(met));
                if(fabs(jetV[bJetInd[j]].DeltaPhi(met)) > maxDeltaPhiBJetMET) maxDeltaPhiBJetMET = fabs(jetV[bJetInd[j]].DeltaPhi(met));
                if(tools::mt(jetV[bJetInd[j]], met) < minmTBJetMET) minmTBJetMET = tools::mt(jetV[bJetInd[j]], met);
                if(tools::mt(jetV[bJetInd[j]], met) < maxmTBJetMET) maxmTBJetMET = tools::mt(jetV[bJetInd[j]], met);
                if((jetV[bJetInd[j]] + met).Pt() > minpTBJetMET) minpTBJetMET = (jetV[bJetInd[j]] + met).Pt();
                if((jetV[bJetInd[j]] + met).Pt() > maxpTBJetMET) maxpTBJetMET = (jetV[bJetInd[j]] + met).Pt();
            }
            if(minDeltaPhiBJetMET == 99999.) minDeltaPhiBJetMET = 0;
            if(minmTBJetMET == 99999.) minmTBJetMET = 0.;
            if(minpTBJetMET == 99999.) minpTBJetMET = 0.;

            //Fill tree for BDT training
            /*
            topMass = std::max(topV.M(), 0.);
            pTForwardJets = forwardJets.Pt();
            etaMostForward = fabs(highestEtaJet.Eta());
            numberOfBJets = bJetCount;
            numberOfJets = jetCount;
            pTLeadingJet = leadingJet.Pt();
            pTLeadingLepton = _lPt[ind[0]];
            mNotSoForwardJets = std::max(notSoForwardJets.M(), 0.);
            pTLeadingBJet = leadingBJet.Pt();
            missingET = _met;
            highestDeepCSV = (jetCount == 0) ? 0. : _jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI];
            maxMjj = maxMJetJet;
            minMlb = minMLeptonbJet;
            asymmetryWlep = _lEta[ind[lw]]*_lCharge[ind[lw]];
            etaZ = fabs((lepV[bestZ.first] + lepV[bestZ.second]).Eta());
            m3l = (lepV[0] + lepV[1] + lepV[2]).M();
            maxDeltaPhijj = maxDeltaPhiJetJet;
            maxDeltaRjj = maxDeltaRJetJet;
            maxDeltaPhill = maxDeltaPhiLeptonLepton;
            maxM2l = maxMLeptonLepton;
            pTMaxlb = maxpTLeptonbJet;
            pTMax2l = maxpTLeptonLepton;
            pT3l = (lepV[0] + lepV[1] + lepV[2]).Pt();
            mForwardJetsLeadinBJetW = std::max((forwardJets + leadingBJet + lepV[lw] + neutrino).M(), 0.);
            ht = HT;
            eventWeight = weight;
            //double bdt = 0, bdtG = 0, bdtD = 0, bdtB = 0, mlp = 0, deepNN = 0, bdtGAlt = 0;
            //double bdtGAlt_200trees = 0, bdtGAlt_shrinkage04 = 0, bdtGAlt_minNode005 = 0, bdtGAlt_negWeights = 0, bdtGAlt_MaxDepth4 = 0, bdtGAlt_20cuts = 0;
            std::vector<double> mvaVals(30);
            for(unsigned i = 0; i < mvaMethods.size(); ++i){
               if(tzqCat != 0 ) mvaVals[i] =  mvaReader[mllCat][tzqCat]->EvaluateMVA(mvaMethods[i] + " method");
               else mvaVals[i] = 0;
            }
            */
            /*
            if(catNames[tzqCat + 1] != "0bJets_01Jets"){
                bdt = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDT method");
                bdtG = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTG method");
                bdtD = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTD method");
                bdtB = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTB method");
                mlp = mvaReader[mllCat][tzqCat]->EvaluateMVA("MLP method");
                deepNN = mvaReader[mllCat][tzqCat]->EvaluateMVA("DNN method");
                bdtGAlt = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTGAlt method");
                bdtGAlt_200trees = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTGAlt_200trees method");
                bdtGAlt_shrinkage04 = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTGAlt_shrinkage04 method");
                bdtGAlt_minNode005 = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTGAlt_minNode005 method");
                bdtGAlt_negWeights = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTGAlt_negWeights method");
                bdtGAlt_MaxDepth4 = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTGAlt_MaxDepth4 method");
                bdtGAlt_20cuts = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTGAlt_20Cuts method");
            }
            */
            //distributions to plot
            //double fill[nDist] = {bdt, bdtG, bdtD, bdtB, mlp, deepNN, bdtGAlt, 
            //    bdtGAlt_200trees, bdtGAlt_shrinkage04, bdtGAlt_minNode005, bdtGAlt_negWeights, bdtGAlt_MaxDepth4, bdtGAlt_20cuts,
            double fill[nDist] = {/*mvaVals[0], mvaVals[1], mvaVals[2], mvaVals[3], mvaVals[4], mvaVals[5], mvaVals[6], mvaVals[7], mvaVals[8], mvaVals[9], 
                mvaVals[10], mvaVals[11], mvaVals[12], mvaVals[12], mvaVals[13], mvaVals[14], mvaVals[15], mvaVals[16], mvaVals[17], mvaVals[18], mvaVals[19], mvaVals[20], mvaVals[21],
                mvaVals[22], mvaVals[23], mvaVals[24], mvaVals[25], mvaVals[26], mvaVals[27], mvaVals[28], mvaVals[29],
                */
                _met, mll, tools::mt(lepV[lw], met),  _lPt[ind[0]], _lPt[ind[1]], _lPt[ind[2]], (double) nJets(), (double) nBJets(), 
            (double) nBJets(0, false), fabs(highestEtaJet.Eta()), fabs(leadingJet.Eta()), leadingJet.Pt(), trailingJet.Pt(), leadingBJet.Pt(), trailingBJet.Pt(),
             highestEtaJet.Pt(), topV.M(), (lepV[0] + lepV[1] + lepV[2]).M(), taggedBJet.Pt(), fabs(taggedBJet.Eta()), recoilingJet.Pt(), fabs(recoilingJet.Eta()),

            (highestEtaJet + leadingBJet + lepV[lw] + neutrino).M(),
            (highestEtaJet + leadingBJet + lepV[lw]).M(),
            (highestEtaJet + leadingBJet + lepV[lw] + neutrino + lepV[bestZ.first] + lepV[bestZ.second] ).M(),
            (highestEtaJet + leadingBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).M(),

            (recoilingJet + taggedBJet + lepV[lw] + neutrino).M(),
            (recoilingJet + taggedBJet + lepV[lw]).M(),
            (recoilingJet + taggedBJet + lepV[lw] + neutrino + lepV[bestZ.first] + lepV[bestZ.second]).M(),
            (recoilingJet + taggedBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).M(),

            (forwardJets + leadingBJet + lepV[lw] + neutrino).M(),
            (forwardJets + leadingBJet + lepV[lw]).M(),
            (forwardJets + leadingBJet + lepV[lw] + neutrino + lepV[bestZ.first] + lepV[bestZ.second]).M(),
            (forwardJets + leadingBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).M(),

            forwardJets.M(),
            notSoForwardJets.M(),
            superForwardJets.M(),

            (highestEtaJet + leadingBJet + lepV[lw] + neutrino).Pt(),
            (highestEtaJet + leadingBJet + lepV[lw]).Pt(),
            (highestEtaJet + leadingBJet + lepV[lw] + neutrino + lepV[bestZ.first] + lepV[bestZ.second] ).Pt(),
            (highestEtaJet + leadingBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),

            (recoilingJet + taggedBJet + lepV[lw] + neutrino).Pt(),
            (recoilingJet + taggedBJet + lepV[lw]).Pt(),
            (recoilingJet + taggedBJet + lepV[lw] + neutrino + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),
            (recoilingJet + taggedBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),

            (forwardJets + leadingBJet + lepV[lw] + neutrino).Pt(),
            (forwardJets + leadingBJet + lepV[lw]).Pt(),
            (forwardJets + leadingBJet + lepV[lw] + neutrino + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),
            (forwardJets + leadingBJet + lepV[lw] + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),

            forwardJets.Pt(),
            notSoForwardJets.Pt(),
            superForwardJets.Pt(),
    
            fabs(forwardJets.Eta()),
            fabs(notSoForwardJets.Eta()),
            fabs(superForwardJets.Eta()),

            _jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI],
            _jetDeepCsv_b[highestDeepCSVI],
            _jetDeepCsv_bb[highestDeepCSVI],
            _jetCsvV2[highestCSVv2I],
            highestDeepCSVJet.Pt(),
            taggedBJet.DeltaR(recoilingJet),
            leadingBJet.DeltaR(highestEtaJet),
            highestDeepCSVJet.DeltaR(highestEtaJet),

            minMLeptonJet, maxMLeptonJet, minMLeptonbJet, maxMLeptonbJet, minMJetJet, maxMJetJet, minMLeptonLepton, maxMLeptonLepton,
            minDeltaPhiLeptonJet, maxDeltaPhiLeptonJet, minDeltaPhiLeptonbJet, maxDeltaPhiLeptonbJet, minDeltaPhiJetJet, maxDeltaPhiJetJet, minDeltaPhiLeptonLepton, maxDeltaPhiLeptonLepton,
            minDeltaRLeptonJet, maxDeltaRLeptonJet, minDeltaRLeptonbJet, maxDeltaRLeptonbJet, minDeltaRJetJet, maxDeltaRJetJet, minDeltaRLeptonLepton, maxDeltaRLeptonLepton,
            minpTLeptonJet, maxpTLeptonJet, minpTLeptonbJet, maxpTLeptonbJet, minpTJetJet, maxpTJetJet, minpTLeptonLepton, maxpTLeptonLepton,
            HT,
            (lepV[bestZ.first] + lepV[bestZ.second]).Pt(),
            fabs((lepV[bestZ.first] + lepV[bestZ.second]).Eta()),
            fabs(_lEta[mostForwardLepInd]), fabs(_lEta[ind[0]]), fabs(_lEta[ind[1]]), fabs(_lEta[ind[2]]), fabs(lepV[lw].Eta()),
            (lepV[0] + lepV[1] + lepV[2]).Pt(), fabs(_lEta[ind[0]])*_lCharge[ind[0]], fabs(_lEta[ind[1]])*_lCharge[ind[1]], fabs(_lEta[ind[2]])*_lCharge[ind[2]], fabs(_lEta[ind[lw]])*_lCharge[ind[lw]],

            deltaRWLepClosestJet, lepV[lw].DeltaPhi( (lepV[bestZ.first] + lepV[bestZ.second]) ), lepV[lw].DeltaPhi(taggedBJet), lepV[lw].DeltaR(recoilingJet), topV.DeltaR( (lepV[bestZ.first] + lepV[bestZ.second]) ),
            topV.Pt(), (topV + recoilingJet + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),

            tools::mt(lepV[0] + lepV[1] + lepV[2], met), tools::mt(lepV[bestZ.first] + lepV[bestZ.second], met),
            minDeltaPhiLeptonMET, maxDeltaPhiLeptonMET, minDeltaPhiJetMET, maxDeltaPhiJetMET, minDeltaPhiBJetMET, maxDeltaPhiBJetMET,
            minmTLeptonMET, maxmTLeptonMET, minmTJetMET, maxmTJetMET, minmTBJetMET, maxmTBJetMET,
            minpTLeptonMET, maxpTLeptonMET, minpTJetMET, maxpTJetMET, minpTBJetMET, maxpTBJetMET,
            tools::mt(lepV[0], met), tools::mt(lepV[1], met), tools::mt(lepV[2], met),
            tools::mt(jetSystem, met), tools::mt(jetSystem + lepV[0] + lepV[1] + lepV[2], met), 
            (jetSystem + lepV[0] + lepV[1] + lepV[2] + neutrino).M(), jetSystem.M(),
            tools::mt(leadingJet, met), tools::mt(trailingJet, met)
            };
            for(unsigned m = 0; m < nMll; ++m){
                if(m == 0 || m == (mllCat + 1) ){
                    for(unsigned cat = 0; cat < nCat; ++cat){
                        if(cat == 0 || cat == (tzqCat + 1) ){
                            //Fill training tree
                            //if(currentSample == 2) tree[0][cat][m]->Fill();
                            //else if(currentSample > 2 && std::get<0>(samples[sam]) != "DY" ) tree[1][cat][m]->Fill(); //fluctuations on DY sample too big for training
                            for(unsigned dist = 0; dist < nDist; ++dist){
                                hists[m][cat][dist][sam]->Fill(std::min(fill[dist], maxBin[dist]), weight);
                            }
                        }
                    }
                }
            }
        }
        //set histograms to 0 if negative
        for(unsigned m = 0; m < nMll; ++m){
            for(unsigned cat = 0; cat < nCat; ++cat){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    tools::setNegativeZero(hists[m][cat][dist][sam]);
                }	
            }
        }
    }
    
    //Save training tree
    //treeFile.Write();
    //treeFile.Close();
    //merge histograms with the same physical background
    std::vector<std::string> proc = {"total bkg.", "tZq", "DY", "TT + Jets", "WJets", "WZ", "multiboson", "TT + Z", "TT/T + X", "X + #gamma", "ZZ/H"};
    std::vector< std::vector< std::vector< std::vector< TH1D* > > > > mergedHists(nMll);
    for(unsigned mll = 0; mll < nMll; ++mll){
        mergedHists[mll] = std::vector< std::vector < std::vector < TH1D* > > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                mergedHists[mll][cat].push_back(std::vector<TH1D*>(proc.size() ) );
                for(size_t m = 0, sam = 0; m < proc.size(); ++m){
                    mergedHists[mll][cat][dist][m] = (TH1D*) hists[mll][cat][dist][sam]->Clone();
                    while(sam < samples.size() - 1 && std::get<0>(samples[sam]) == std::get<0>(samples[sam + 1]) ){
                        mergedHists[mll][cat][dist][m]->Add(hists[mll][cat][dist][sam + 1]);
                        ++sam;
                    }
                    ++sam;
                }
            }
        }
    }
    //TEMPORARY//////
    //replace data with sum of all backgrounds
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                mergedHists[m][cat][dist][0] = (TH1D*) mergedHists[m][cat][dist][1]->Clone(); 
                for(unsigned p = 2; p < proc.size(); ++p){
                    mergedHists[m][cat][dist][0]->Add(mergedHists[m][cat][dist][p]);
                }
            }
        }
    }
    ////////////////
    //Make 81 fb copy of all histograms
    TH1D* mergedHists_81fb[nMll][nCat][nDist][(const size_t) proc.size()];
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                for(unsigned p = 0; p < proc.size(); ++p){
                    mergedHists_81fb[m][cat][dist][p] = (TH1D*) mergedHists[m][cat][dist][p]->Clone();
                    mergedHists_81fb[m][cat][dist][p]->Scale((lumi2017 + lumi2016)/lumi2016);
                }
            }
        }
    }
  
    const std::string sigNames[1] = {"tZq"};
    std::vector< std::vector< std::vector< TH1D* > > >  signal(nMll);
    for(unsigned m = 0; m < nMll; ++m){
        signal[m] = std::vector< std::vector < TH1D* > >(nCat);
        for(unsigned cat = 0; cat < nCat; ++cat){
            signal[m][cat] = std::vector<TH1D*>(nDist);
            for(unsigned dist = 0; dist < nDist; ++dist){
                signal[m][cat][dist] = (TH1D*) mergedHists[m][cat][dist][1]->Clone();
            }
        }
    }
    //make 81 fb copy of signal
    TH1D* signal_81fb[nMll][nCat][nDist];
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                signal_81fb[m][cat][dist] = (TH1D*) mergedHists_81fb[m][cat][dist][1]->Clone();
            }
       } 
    }
    //plot all distributions
    const bool isSMSignal[(const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
    for(unsigned m = 0; m < nMll; ++m){
        for(unsigned cat = 0; cat < nCat; ++cat){
            for(unsigned dist = 0; dist < nDist; ++dist){
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/36fb/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_" + catNames[cat] + "_" + mllNames[m], "tzq", false, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);             //linear plots
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/36fb/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_" + catNames[cat] + "_" + mllNames[m] + "_withSignal", "tzq", false, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal, &signal[m][cat][dist], sigNames, 1);             //linear plots with signal

                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/36fb/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_"  + catNames[cat] + "_" + mllNames[m] + "_log", "tzq", true, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal);    //log plots
                plotDataVSMC(mergedHists[m][cat][dist][0], &mergedHists[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/36fb/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_"  + catNames[cat] + "_" + mllNames[m] + "_withSignal_log", "tzq", true, false, "35.9 fb^{-1} (13 TeV)", nullptr, isSMSignal, &signal[m][cat][dist], sigNames, 1);    //log plots with signal

                plotDataVSMC(mergedHists_81fb[m][cat][dist][0], &mergedHists_81fb[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/81fb/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_" + catNames[cat] + "_" + mllNames[m], "tzq", false, false, "81 fb^{-1} (13 TeV)", nullptr, isSMSignal);             //linear plots
                plotDataVSMC(mergedHists_81fb[m][cat][dist][0], &mergedHists_81fb[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/81fb/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_" + catNames[cat] + "_" + mllNames[m] + "_withSignal", "tzq", false, false, "81 fb^{-1} (13 TeV)", nullptr, isSMSignal, &signal[m][cat][dist], sigNames, 1);             //linear plots with signal

                plotDataVSMC(mergedHists_81fb[m][cat][dist][0], &mergedHists_81fb[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/81fb/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_"  + catNames[cat] + "_" + mllNames[m] + "_log", "tzq", true, false, "81 fb^{-1} (13 TeV)", nullptr, isSMSignal);    //log plots
                plotDataVSMC(mergedHists_81fb[m][cat][dist][0], &mergedHists_81fb[m][cat][dist][1], &proc[0], mergedHists[m][cat][dist].size() - 1, "tZq/81fb/" + mllNames[m] + "/" + catNames[cat] + "/" + std::get<0>(histInfo[dist]) + "_"  + catNames[cat] + "_" + mllNames[m] + "_withSignal_log", "tzq", true, false, "81 fb^{-1} (13 TeV)", nullptr, isSMSignal, &signal[m][cat][dist], sigNames, 1);    //log plots with signal
            }
        }
    }
}
int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
