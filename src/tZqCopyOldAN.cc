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

#include <chrono>

//include other parts of the code
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"
#include "../interface/tZqTools.h"
#include "../interface/trilepTools.h"
#include "../interface/Reweighter.h"
#include "../interface/HistInfo.h"
#include "../interface/HistCollectionDist.h"
#include "../interface/HistCollectionSample.h"
#include "../interface/kinematicTools.h"
#include "../interface/TrainingTree.h"
#include "../interface/BDTReader.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"

//include TMVA classes
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"

void treeReader::setup(){
    //Set CMS plotting style
    setTDRStyle();
    gROOT->SetBatch(kTRUE);
    //read samples and cross sections from txt file
    readSamples("sampleLists/samples.txt");
    //info on kinematic distributions to plot
    //name      xlabel    nBins,  min, max
    histInfo = {
        HistInfo("bdtG", "BDT output", 30, -1, 1),

        HistInfo("asymmetryWLep", "asymmetry (lepton from W)",30, -2.5, 2.5),
        HistInfo("etaWLep",  "|#eta| (lepton from W)", 20, 0, 2.5),
        HistInfo("highestCSVv2", "highest CSVv2", 20, 0, 1),
        HistInfo("deltaPhiWlepTaggedbJet", "#DeltaR(lepton from W, tagged b-jet)", 20, 0, 3.15),
        HistInfo("deltaPhiWlepZ", "#DeltaR(lepton from W, Z)", 20, 0, 3.15),
        HistInfo("deltaRWLepClosestJet", "#DeltaR(lepton from W, closest jet)", 20, 0, 5), 
        HistInfo("deltaRWlepRecoilingJet", "#DeltaR(lepton from W, recoiling jet)", 20, 0, 5),
        HistInfo("etaTaggedRecoilJet", "|#eta| (recoiling jet) (GeV)", 20, 0, 4.5),
        HistInfo("etaLeadingJet", "|#eta| (leading jet)", 20, 0, 4.5),
        HistInfo("mtop", "M_{(W + b)} (GeV)", 20, 0, 500),
        HistInfo("nJets", "number of jets", 6, 0, 6),
        HistInfo("pTTaggedRecoilJet", "P_{T} (recoiling jet) (GeV)", 20, 0, 250), 
        HistInfo("etaZboson", "|#eta|(Z)", 20, 0, 4),
        HistInfo("deltaRZTop", "#DeltaR(top, Z)", 20, 0, 5),
        HistInfo("pTZboson", "P_{T}(Z) (GeV)", 30, 0, 300),

        HistInfo("mll", "M_{ll} (GeV)", 60, 12, 200),
        HistInfo("mt", "M_{T} (GeV)", 30, 0, 300),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 30, 25, 200),
        HistInfo("subPt", "P_{T}^{subleading} (GeV)", 30, 15, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 30, 10, 200),
        HistInfo("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8),
        HistInfo("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
    };
}

void treeReader::Analyze(const std::string& sampName, const long unsigned begin, const long unsigned end){
    auto samIt = std::find_if(samples.cbegin(), samples.cend(), [sampName](const Sample& s) { return s.getFileName() == sampName; } );
    Analyze(*samIt, begin, end);
}

void treeReader::Analyze(const Sample& samp, const long unsigned begin, const long unsigned end){

    auto start = std::chrono::high_resolution_clock::now();

    //categorization
    Category categorization({ {"mllInclusive", "onZ", "offZ", "noOSSF"}, {"nJetsInclusive", "0bJets", "1bJet01jets", "1bJet23Jets", "1bJet4Jets", "2bJets"} });

    //set up histogram collection for particular sample
    HistCollectionSample histCollection(histInfo, samp, categorization);

    //store relevant info from histCollection
    const unsigned nDist = histInfo.size();                              //number of distributions to plot
    const unsigned nCat = histCollection.categoryRange(1);                //Several categories enriched in different processes
    const unsigned nMll = histCollection.categoryRange(0);                //categories based on dilepton Mass

    //variables to write to tree for bdt training and used for bdt computation
    std::map < std::string, float > bdtVariableMap =
        {   
            {"asymmetryWlep", 0.},
            {"etaWLep", 0.},
            {"highestCSVv2", 0.},
            {"deltaPhiWlepTaggedbJet", 0.},
            {"deltaPhiWlepZ", 0.},
            {"deltaRWLepClosestJet", 0.},
            {"deltaRWlepRecoilingJet", 0.},
            {"etaTaggedRecoilJet", 0.},
            {"etaLeadingJet", 0.},
            {"mtop", 0.},
            {"nJets", 0.},
            {"pTTaggedRecoilJet", 0.},
            {"etaZboson", 0.},
            {"deltaRZTop", 0.},
            {"pTZboson", 0.}  
            {"eventWeight", 0.}
        };
    
    //training tree writer
    TrainingTree trainingTree("trainingTrees/" + std::to_string(begin) + "_" + std::to_string(end) + "_TOP16020_",samp, categorization, bdtVariableMap, samp.isSMSignal() ); 

    //loop over all sample
    initSample(samp, 0);          //Use 2016 lumi
    for(long unsigned it = begin; it < end; ++it){
        GetEntry(samp, it);


        for(unsigned j = 0; j < _nJets; ++j){
            TLorentzVector jet;
            jet.SetPtEtaPhiE(_jetPt[j], _jetEta[j], _jetPhi[j], _jetE[j]);
            if(jet.M() < 0){
                std::cout << "jet of negative mass = " << jet.M() << std::endl;
            }
        }

        //vector containing good lepton indices
        std::vector<unsigned> ind;

        //select leptons
        const unsigned lCount = selectLep_TOP16_020(ind);
        if(lCount != 3) continue;

        //find presence of fourth veto lepton
        unsigned vetoLeptonCount = 0;
        for(unsigned l = 0; l < _nLight; ++l){
            if(lepIsVeto_TOP16_020(l)) ++vetoLeptonCount;
        }
        if(vetoLeptonCount > 3) continue;


        //require presence of OSSF pair
        //if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 

        //remove overlap between samples
        if(photonOverlap(samp)) continue;

        //make lorentzvectors for leptons
        TLorentzVector lepV[lCount];
        for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);


        //make ordered jet and bjet collections
        std::vector<unsigned> jetInd, bJetInd;
        unsigned jetCount = nJets(jetInd);
        unsigned bJetCount = nBJets(bJetInd);

        //find highest eta jet
        unsigned highestEtaJ = (jetCount == 0) ? 99 : jetInd[0];
        for(unsigned j = 1; j < jetCount; ++j){
            if(fabs(_jetEta[jetInd[j]]) > fabs(_jetEta[highestEtaJ]) ) highestEtaJ = jetInd[j];
        }

        //determine tZq analysis category
        unsigned tzqCat = tzq::cat(jetCount, bJetCount);

        //check if OSSF pair is present 
        bool isOSSF = (trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) == 0);

        //find best Z candidate
        std::pair<unsigned, unsigned> bestZ;
        if(isOSSF){
           bestZ = trilep::bestZ(lepV, ind, _lFlavor, _lCharge, lCount);
        } else {
            //pick leading leptons if there is no OSSF pair present 
            bestZ = {ind[0], ind[1]};
        }

        //determine best Z mass
        double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();

        //veto events with mll below 30 GeV in agreement with the tZq sample
        if(isOSSF && ( mll < 30) ) continue;

        unsigned mllCat = 99;
        //determine mll/flavor category 
        if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) == 0){
            if( fabs(mll - 91.1876) < 15){
                //onZ
                mllCat = 0;
            } else{
                //offZ
                mllCat = 1;
            }
        //no OSSF pair present
        } else{
            mllCat = 2;
        }

        //apply event weight
        if(!samp.isData() ){
            weight*=sfWeight();
            //std::cout << "sfWeight() = " << sfWeight() << std::endl;
        }

        //make LorentzVector for all jets 
        TLorentzVector jetV[(const unsigned) _nJets];
        for(unsigned j = 0; j < _nJets; ++j){
            jetV[j].SetPtEtaPhiE(_jetPt[j], _jetEta[j], _jetPhi[j], _jetE[j]);
            /*
            if(jetV[j].M() < 0){
                std::cout << "jet has negative mass = " << jetV[j].M() << std::endl;
            }
            */
        }

        //find W lepton 
        unsigned lw = 999;
        for(unsigned l = 0; l < lCount; ++l){
            if( l != bestZ.first && l != bestZ.second ) lw = l;
        }

        //make met vector 
        TLorentzVector met;
        met.SetPtEtaPhiE(_met, 0, _metPhi, _met);
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
            if( _jetCsvV2[jetInd[j]] > _jetCsvV2[highestCSVv2I] ) highestCSVv2I = jetInd[j];
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
            if(jetCount > 1) trailingJet = jetV[jetInd[jetInd.size() - 1]];
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
        //initialize lepton indices
        std::vector<unsigned> lepVecInd;

        for(unsigned l = 0; l < lCount; ++l) lepVecInd.push_back(l); 
        //lepton Jet 
        double minMLeptonJet = kinematics::minMass(lepV, lepVecInd, jetV, jetInd);
        double maxMLeptonJet = kinematics::maxMass(lepV, lepVecInd, jetV, jetInd);
        double minDeltaRLeptonJet = kinematics::minDeltaR(lepV, lepVecInd, jetV, jetInd);
        double maxDeltaRLeptonJet = kinematics::maxDeltaR(lepV, lepVecInd, jetV, jetInd);
        double minDeltaPhiLeptonJet = kinematics::minDeltaR(lepV, lepVecInd, jetV, jetInd);
        double maxDeltaPhiLeptonJet = kinematics::maxDeltaR(lepV, lepVecInd, jetV, jetInd);
        double minpTLeptonJet = kinematics::minPT(lepV, lepVecInd, jetV, jetInd);
        double maxpTLeptonJet = kinematics::maxPT(lepV, lepVecInd, jetV, jetInd);

        //lepton bjet
        double minMLeptonbJet = kinematics::minMass(lepV, lepVecInd, jetV, bJetInd);
        double maxMLeptonbJet = kinematics::maxMass(lepV, lepVecInd, jetV, bJetInd);
        double minDeltaRLeptonbJet = kinematics::minDeltaR(lepV, lepVecInd, jetV, bJetInd);
        double maxDeltaRLeptonbJet = kinematics::maxDeltaR(lepV, lepVecInd, jetV, bJetInd);
        double minDeltaPhiLeptonbJet = kinematics::minDeltaR(lepV, lepVecInd, jetV, bJetInd);
        double maxDeltaPhiLeptonbJet = kinematics::maxDeltaR(lepV, lepVecInd, jetV, bJetInd);
        double minpTLeptonbJet = kinematics::minPT(lepV, lepVecInd, jetV, bJetInd);
        double maxpTLeptonbJet = kinematics::maxPT(lepV, lepVecInd, jetV, bJetInd);

        //jet jet
        double minMJetJet = kinematics::minMass(jetV, jetInd);
        double maxMJetJet = kinematics::maxMass(jetV, jetInd);
        double minDeltaRJetJet = kinematics::minDeltaR(jetV, jetInd);
        double maxDeltaRJetJet = kinematics::maxDeltaR(jetV, jetInd);
        double minDeltaPhiJetJet = kinematics::minDeltaPhi(jetV, jetInd);
        double maxDeltaPhiJetJet = kinematics::maxDeltaPhi(jetV, jetInd);
        double minpTJetJet = kinematics::minPT(jetV, jetInd);
        double maxpTJetJet = kinematics::maxPT(jetV, jetInd);
        
        //lepton lepton 
        double minMLeptonLepton = kinematics::minMass(lepV, lepVecInd);
        double maxMLeptonLepton = kinematics::maxMass(lepV, lepVecInd);
        double minDeltaRLeptonLepton = kinematics::minDeltaR(lepV, lepVecInd);
        double maxDeltaRLeptonLepton = kinematics::maxDeltaR(lepV, lepVecInd);
        double minDeltaPhiLeptonLepton = kinematics::minDeltaPhi(lepV, lepVecInd);
        double maxDeltaPhiLeptonLepton = kinematics::maxDeltaPhi(lepV, lepVecInd);
        double minpTLeptonLepton = kinematics::minPT(lepV, lepVecInd);
        double maxpTLeptonLepton = kinematics::maxPT(lepV, lepVecInd);
         
        //set met index
        std::vector<unsigned> metIndex = {0};

        //lepton + MET 
        double minDeltaPhiLeptonMET = kinematics::minDeltaPhi(lepV, lepVecInd, &met, metIndex);
        double maxDeltaPhiLeptonMET = kinematics::maxDeltaPhi(lepV, lepVecInd, &met, metIndex);
        double minmTLeptonMET = kinematics::minMT(lepV, lepVecInd, &met, metIndex);
        double maxmTLeptonMET = kinematics::maxMT(lepV, lepVecInd, &met, metIndex);
        double minpTLeptonMET = kinematics::minPT(lepV, lepVecInd, &met, metIndex);
        double maxpTLeptonMET = kinematics::maxPT(lepV, lepVecInd, &met, metIndex);

        //jet + MET
        double minDeltaPhiJetMET = kinematics::minDeltaPhi(jetV, jetInd, &met, metIndex);
        double maxDeltaPhiJetMET = kinematics::maxDeltaPhi(jetV, jetInd, &met, metIndex);
        double minmTJetMET = kinematics::minMT(jetV, jetInd, &met, metIndex);
        double maxmTJetMET = kinematics::maxMT(jetV, jetInd, &met, metIndex);
        double minpTJetMET = kinematics::minPT(jetV, jetInd, &met, metIndex);
        double maxpTJetMET = kinematics::maxPT(jetV, jetInd, &met, metIndex);

        //bjet + MET 
        double minDeltaPhiBJetMET = kinematics::minDeltaPhi(jetV, bJetInd, &met, metIndex);
        double maxDeltaPhiBJetMET = kinematics::maxDeltaPhi(jetV, bJetInd, &met, metIndex);
        double minmTBJetMET = kinematics::minMT(jetV, bJetInd, &met, metIndex);
        double maxmTBJetMET = kinematics::maxMT(jetV, bJetInd, &met, metIndex);
        double minpTBJetMET = kinematics::minPT(jetV, bJetInd, &met, metIndex);
        double maxpTBJetMET = kinematics::maxPT(jetV, bJetInd, &met, metIndex);

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
        
        bdtVariableMap["asymmetryWlep"] = _lEta[ind[lw]]*_lCharge[ind[lw]];
        bdtVariableMap["topMass"] =  std::max(topV.M(), 0.);
        bdtVariableMap["etaMostForward"] = fabs(highestEtaJet.Eta());
        bdtVariableMap["mTW"] = kinematics::mt(lepV[lw], met);
        bdtVariableMap["highestDeepCSV"] = (jetCount == 0) ? 0. : _jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI];
        bdtVariableMap["numberOfJets"] = jetCount;
        bdtVariableMap["numberOfBJets"] = bJetCount;
        bdtVariableMap["pTLeadingLepton"] = _lPt[ind[0]];
        bdtVariableMap["pTLeadingBJet"] = leadingBJet.Pt();
        bdtVariableMap["pTMostForwardJet"] = highestEtaJet.Pt();
        bdtVariableMap["mAllJets"] = std::max(jetSystem.M(), 0.);
        bdtVariableMap["maxDeltaPhijj"] = maxDeltaPhiJetJet;
        bdtVariableMap["maxDeltaRjj"] = maxDeltaRJetJet;
        bdtVariableMap["maxMjj"] = std::max(maxMJetJet, 0.);
        bdtVariableMap["maxMlb"] = std::max(maxMLeptonbJet, 0.);
        bdtVariableMap["minMlb"] = std::max(minMLeptonbJet, 0.);
        bdtVariableMap["pTMaxlb"] = maxpTLeptonbJet;
        bdtVariableMap["maxmTbmet"] = maxmTBJetMET;
        bdtVariableMap["maxpTlmet"] = maxpTLeptonMET;
        bdtVariableMap["pTMax2l"] = maxpTLeptonLepton;
        bdtVariableMap["m3l"] = (lepV[0] + lepV[1] + lepV[2]).M();
        bdtVariableMap["pT3l"] = (lepV[0] + lepV[1] + lepV[2]).Pt();
        bdtVariableMap["ht"] = HT;
        bdtVariableMap["mZ"] = mll;
        bdtVariableMap["deltaRWLeptonTaggedbJet"] = lepV[lw].DeltaR(taggedBJet);
        bdtVariableMap["etaZ"] = fabs((lepV[bestZ.first] + lepV[bestZ.second]).Eta());
        bdtVariableMap["maxDeltaPhibmet"] = maxDeltaPhiBJetMET;
        bdtVariableMap["minDeltaPhibmet"] = minDeltaPhiBJetMET;
        bdtVariableMap["minDeltaPhilb"] = minDeltaPhiLeptonbJet;
        bdtVariableMap["pTmin2l"] = minpTLeptonLepton;
        bdtVariableMap["minmTbmet"] = minmTBJetMET;
        bdtVariableMap["minmTlmet"] = minmTLeptonMET;
        bdtVariableMap["missingEt"] = _met;
        bdtVariableMap["maxmTjmet"] = maxmTJetMET;
        bdtVariableMap["eventWeight"] = weight;

        double bdt = 0, bdt2 = 0;
        if(tzqCat != 0){
            //bdt = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTG method");
            //bdt2 = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTG method 1000 Trees");
        }

        double fill[nDist] = {
            bdt, bdt, bdt2, bdt2, 
            _met, mll, kinematics::mt(lepV[lw], met),  _lPt[ind[0]], _lPt[ind[1]], _lPt[ind[2]], (double) nJets(), (double) nBJets(), 
        (double) nBJets(0, false), fabs(highestEtaJet.Eta()), fabs(leadingJet.Eta()), leadingJet.Pt(), trailingJet.Pt(), leadingBJet.Pt(), trailingBJet.Pt(),
         highestEtaJet.Pt(), std::max(topV.M(), 0.), (lepV[0] + lepV[1] + lepV[2]).M(), taggedBJet.Pt(), fabs(taggedBJet.Eta()), recoilingJet.Pt(), fabs(recoilingJet.Eta()),

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

        std::max(0., forwardJets.M()),
        std::max(0., notSoForwardJets.M()),
        std::max(0., superForwardJets.M()),

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

        deltaRWLepClosestJet, fabs(lepV[lw].DeltaPhi( (lepV[bestZ.first] + lepV[bestZ.second]) ) ), fabs(lepV[lw].DeltaPhi(taggedBJet)), lepV[lw].DeltaR(recoilingJet), topV.DeltaR( (lepV[bestZ.first] + lepV[bestZ.second]) ),
        topV.Pt(), (topV + recoilingJet + lepV[bestZ.first] + lepV[bestZ.second]).Pt(),

        kinematics::mt(lepV[0] + lepV[1] + lepV[2], met), kinematics::mt(lepV[bestZ.first] + lepV[bestZ.second], met),
        minDeltaPhiLeptonMET, maxDeltaPhiLeptonMET, minDeltaPhiJetMET, maxDeltaPhiJetMET, minDeltaPhiBJetMET, maxDeltaPhiBJetMET,
        minmTLeptonMET, maxmTLeptonMET, minmTJetMET, maxmTJetMET, minmTBJetMET, maxmTBJetMET,
        minpTLeptonMET, maxpTLeptonMET, minpTJetMET, maxpTJetMET, minpTBJetMET, maxpTBJetMET,
        kinematics::mt(lepV[0], met), kinematics::mt(lepV[1], met), kinematics::mt(lepV[2], met),
        kinematics::mt(jetSystem, met), kinematics::mt(jetSystem + lepV[0] + lepV[1] + lepV[2], met), 
        (jetSystem + lepV[0] + lepV[1] + lepV[2] + neutrino).M(), jetSystem.M(),
        kinematics::mt(leadingJet, met), kinematics::mt(trailingJet, met)
        };

        for(unsigned m = 0; m < nMll; ++m){
            //m = 0 is onZ + offZ
            if( (m == 0 && mllCat != 2) || m == (mllCat + 1) ){
                for(unsigned cat = 0; cat < nCat; ++cat){
                    if(cat == 0 || cat == (tzqCat + 1) ){
                        //Fill training tree
                        if(samp.getProcessName() != "DY" ) trainingTree.fill({m, cat}, bdtVariableMap); //fluctuations on DY sample too big for training
                        for(unsigned dist = 0; dist < nDist; ++dist){
                            histCollection.access(dist, {m, cat})->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight);
                        }
                    }
                }
            }
        }
    }

    //write histcollection to file
    histCollection.store("tempHists_tZq/", begin, end);

    //timer
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << std::endl;
}

void treeReader::splitJobs(){
    for(unsigned sam = 0; sam < samples.size(); ++sam){
        initSample(1);
        //split samples per 200k events
        for(long unsigned it = 0; it < nEntries; it+=200000){
            long unsigned begin = it;
            long unsigned end = std::min(nEntries, it + 200000);
            //make temporary job script 
            std::ofstream script("runTuples.sh");
            tools::initScript(script);
            script << "./tZqAllPlots " << samples[currentSample].getFileName() << " " << std::to_string(begin) << " " << std::to_string(end);
            script.close();
            //submit job
            tools::submitScript("runTuples.sh", "00:20:00");
         }
    }
}

void treeReader::plot(const std::string& distName){
    //loop over all distributions and find the one to plot
    for(size_t d = 0; d < histInfo.size(); ++d){
        if(histInfo[d].name() == distName){
            std::cout << "making hist collection for: " << histInfo[d].name() << std::endl;
            //read collection for this distribution from files
            HistCollectionDist col("inputList.txt", histInfo[d], samples, { {"mllInclusive", "onZ", "offZ", "noOSSF"}, {"nJetsInclusive", "0bJets01Jets", "0bJets2Jets", "1bJet01jets", "1bJet23Jets", "1bJet4Jets", "2bJets"} });
            //blind onZ categories
            col.blindData("onZ_1bJet23Jets"); 
            col.blindData("onZ_1bJet4Jets"); 
            col.blindData("onZ_2bJets"); 
            col.blindData("onZ_nJetsInclusive"); 
            col.blindData("mllInclusive_1bJet23Jets");
            col.blindData("mllInclusive_1bJet4Jets");
            col.blindData("mllInclusive_2bJets");
            col.blindData("mllInclusive_nJetsInclusive"); 
            //print plots for collection
            bool is2016 = true;
            col.printPlots("plots/tZq/2016", is2016, "tzq", false);
            col.printPlots("plots/tZq/2016", is2016, "tzq", true);
        }
    }
}

void treeReader::splitPlots(){
    tools::makeFileList("tempHists_tZq", "inputList.txt");
    for(auto& h: histInfo){
        std::ofstream script("printPlots.sh");
        tools::initScript(script);
        script << "./tZqAllPlots plot " << h.name();
        script.close();
        tools::submitScript("printPlots.sh", "00:15:00");
    }
}

int main(int argc, char* argv[]){
    treeReader reader;
    reader.setup();
    //convert all input to std::string format for easier handling
    std::vector<std::string> argvStr;
    for(int i = 0; i < argc; ++i){
        argvStr.push_back(std::string(argv[i]));
    }
    //no arguments given: full workflow of program
    if(argc == 1){
        std::cout << "Step 1: Distributing jobs on T2 grid" << std::endl;
        reader.splitJobs();
        std::cout << "Step 2: sleeping until jobs are finished" << std::endl;
        if(tools::runningJobs()) std::cout << "jobs are running!" << std::endl;
        while(tools::runningJobs("runTuples.sh")){
            tools::sleep(60);
        }
        std::cout << "Step 3: submitting plot jobs" << std::endl;
        reader.splitPlots();
        std::cout << "Program closing, plots will be dumped in specified directory soon" << std::endl;
    }
    //single argument "run" given will do all computations and write output histograms to file
    else if(argc == 2 && argvStr[1] == "run"){
        reader.splitJobs();
    }
    //single argument "plot" given will submit all jobs for plotting from existing output files
    else if(argc == 2 && argvStr[1] == "plot"){
        reader.splitPlots();
    }
    //arguments "plot" and distribution name given plots just this particular distribution
    else if(argc == 3 && argvStr[1] == "plot"){
        reader.plot(argvStr[2]);
    }
    //submit single histogram computation job
    else if(argc == 4){
        long unsigned begin = std::stoul(argvStr[2]);
        long unsigned end = std::stoul(argvStr[3]);
        //sample, first entry, last entry:
        reader.Analyze(argvStr[1], begin, end);
    }
    //invalid input given
    else{
        std::cerr << "Invalid input given to program: terminating!" << std::endl;
    }
    return 0;
}
