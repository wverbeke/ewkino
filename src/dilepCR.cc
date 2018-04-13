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
#include <stdlib.h>

//include other parts of the code
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"
#include "../interface/ewkinoTools.h"
#include "../interface/trilepTools.h"
#include "../interface/HistCollectionSample.h"
#include "../interface/HistCollectionDist.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"


void treeReader::setup(){
    //Set CMS plotting style
    setTDRStyle();
    gROOT->SetBatch(kTRUE);
    //read samples and cross sections from txt file
    readSamples("sampleLists/samples_dilepCR.txt");
    //info on kinematic distributions to plot
    //name      xlabel    nBins,  min, max
    histInfo = {
        HistInfo("sip3d", "SIP_{3D}", 100, 0, 8),
        HistInfo("dxy", "|d_{xy}| (cm)", 100, 0, 0.05),
        HistInfo("dz", "|d_{z}| (cm)", 100, 0, 0.1),
        HistInfo("miniIso", "miniIso", 100, 0, 0.4),
        HistInfo("relIso", "relIso_{#DeltaR 0.3}", 100, 0, 1),
        HistInfo("leptonMvaSUSY16", "SUSY lepton MVA 2016", 100, -1, 1),
        HistInfo("leptonMvaTTH16", "TTH lepton MVA 2016", 100, -1, 1),
        HistInfo("leptonMvatZqTTV", "tZq/TTV lepton MVA 2016", 100, -1, 1),
        HistInfo("leptonMvaSUSY17", "SUSY lepton MVA 2017", 100, -1, 1),
        HistInfo("leptonMvaTTH17", "TTH lepton MVA 2017", 100, -1, 1),
        HistInfo("ptRel", "P_{T}^{rel} (GeV)", 100, 0, 200),
        HistInfo("ptRatio", "P_{T}^{ratio}", 100, 0, 2),
        HistInfo("closestJetCsvV2", "closest jet CSVv2", 100, 0, 1),
        HistInfo("closestJetDeepCsv", "closest jet DeepCSV", 100, 0, 1),
        HistInfo("chargedTrackMult", "closest jet track multiplicity", 20, 0, 20),
        HistInfo("electronMvaGP", "electron GP Spring 16 MVA value", 100, -1, 1),
        HistInfo("electronMvaHZZ", "electron HZZ Spring 16 MVA value", 100, -1, 1),
        HistInfo("electronMvaFall17NoIso", "electron Fall 17 no iso MVA value", 100, -1, 1),
        HistInfo("electronMvaFall17Iso", "electron HZZ Fall 17 iso MVA value", 100, -1, 1),
        HistInfo("muonSegComp", "muon segment compatibility", 100, 0, 1),
        HistInfo("met", "E_{T}^{miss} (GeV)", 100, 0, 300),
        HistInfo("mll", "M_{ll} (GeV)", 200, 12, 200),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 100, 25, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 100, 15, 150),
        HistInfo("leadinEta", "|#eta|^{leading}", 100, 0, 2.5),
        HistInfo("trailingEta", "|#eta|^{trailing}", 100, 0, 2.5),
        HistInfo("nVertex", "number of vertices", 100, 0, 100),
        HistInfo("nJets", "number of jets", 10, 0, 10),
        HistInfo("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
        HistInfo("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8),
        HistInfo("jetPt_higheEtaJet", "p_{T} (most forward jet)", 100, 0, 500),
        HistInfo("jetEta_highestEtaJet_pTCut20", "|#eta|(most forward jet) (P_{T} > 20 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut20", "#eta (most forward jet) (P_{T} > 20 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut20", "number of |eta| > 2.4, P_{T} > 20 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut25", "|#eta|(most forward jet) (P_{T} > 25 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut25", "#eta(most forward jet) (P_{T} > 25 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut25", "number of |eta| > 2.4, P_{T} > 25 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut30", "|#eta|(most forward jet) (P_{T} > 30 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut30", "#eta(most forward jet) (P_{T} > 30 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut30", "number of |eta| > 2.4, P_{T} > 30 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut35", "|#eta|(most forward jet) (P_{T} > 35 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut35", "#eta(most forward jet) (P_{T} > 35 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut35", "number of |eta| > 2.4, P_{T} > 35 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut40", "|#eta|(most forward jet) (P_{T} > 40 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut40", "#eta(most forward jet) (P_{T} > 40 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut40", "number of |eta| > 2.4, P_{T} > 40 GeV", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut45", "|#eta|(most forward jet) (P_{T} > 45 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut45", "#eta(most forward jet) (P_{T} > 45 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut45", "number of |eta| > 2.4, P_{T} > 45 GeV", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut50", "|#eta|(most forward jet) (P_{T} > 50 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut50", "#eta(most forward jet) (P_{T} > 50 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut50", "number of |eta| > 2.4, P_{T} > 50 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut55", "|#eta|(most forward jet) (P_{T} > 55 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut55", "#eta(most forward jet) (P_{T} > 55 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut55", "number of |eta| > 2.4, P_{T} > 55 GeV jets", 6, 0, 6),
        HistInfo("jetEta_highestEtaJet_pTCut60", "|#eta|(most forward jet) (P_{T} > 60 GeV)", 100, 0, 5),
        HistInfo("jetSignedEta_highestEtaJet_pTCut60", "#eta (most forward jet) (P_{T} > 60 GeV)", 100, -5, 5),
        HistInfo("nForwardJets_pTCut60", "number of |eta| > 2.4, P_{T} > 60 GeV jets", 6, 0, 6)
    };
}

void treeReader::Analyze(const std::string& sampName, const long unsigned begin, const long unsigned end){
    auto samIt = std::find_if(samples.cbegin(), samples.cend(), [sampName](const Sample& s) { return s.getFileName() == sampName; } );
    Analyze(*samIt, begin, end);
}

void treeReader::Analyze(const Sample& samp, const long unsigned begin, const long unsigned end){
    //set up histogram collection for particular sample
    HistCollectionSample histCollection(histInfo, samp, { {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"}, {"inclusive", "ee", "em", "mm", "same-sign-ee", "same-sign-em", "same-sign-mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });

    //read pu weights for every period
    TFile* puFile = TFile::Open("weights/puWeights2017.root");
    const std::string eras[6] = {"Inclusive", "B", "C", "D", "E", "F"};
    TH1D* puWeights[6];
    for(unsigned e = 0; e < 6; ++e){
        puWeights[e] = (TH1D*) puFile->Get( (const TString&) "puw_Run" + eras[e]);
    }

    const unsigned nDist = histInfo.size();
    const unsigned nRuns = histCollection.categoryRange(0);
    const unsigned nJetCat = histCollection.categoryRange(2);
    const unsigned nPuRew = histCollection.categoryRange(3);

    initSample(samp);  //use 2017 lumi
    for(long unsigned it = begin; it < end; ++it){
        GetEntry(samp, it);
        //vector containing good lepton indices
        std::vector<unsigned> ind;

        //require met filters and triggers
        //bool fail2016Trigger = ( (!_2016_e) && (!_2016_m) && (!_2016_em) && (!_2016_ee) && (!_2016_mm) );
        /*
        bool fail2017Trigger = ( (!_2017_e) && (!_2017_m) && (!_2017_em) && (!_2017_ee) && (!_2017_mm) );
        if( (samp.isData() || samp.is2017() ) && fail2017Trigger) continue;
        */
        //if( ! _passMETFilters ) continue;

        //select leptons
        const unsigned lCount = selectLep(ind);
        if(lCount != 2) continue;
        //require pt cuts (25, 20) to be passed
        if(!passPtCuts(ind)) continue;
        //make lorentzvectors for leptons
        TLorentzVector lepV[lCount];
        for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
        //Cut of Mll at 12 GeV
        if((lepV[0] + lepV[1]).M() < 12) continue;
        //reject SS events
        //if(_lCharge[ind[0]] == _lCharge[ind[1]]) continue;
        //determine flavor compositions
        unsigned flav = dilFlavorComb(ind) + 1; //reserve 0 for inclusive
        //determine whether there is an OS lepton pair 
        bool hasOS = ( _lCharge[ind[0]] != _lCharge[ind[1]] );
        //order jets and check the number of jets
        std::vector<unsigned> jetInd;
        unsigned jetCount = nJets(jetInd);

        //Extra category selection: for DY select onZ, for ttbar select 1 b-jet 2-jets
        if((flav == 1 || flav == 3) && hasOS){ //OSSF
            if( fabs((lepV[0] + lepV[1]).M() - 91) > 10 ) continue;
            if(nBJets(0,  true, true, 0) != 0) continue;    //veto cleaned jets, maybe this fixes csv distribution of close jets
            //if(!hasOS) continue;
        } else if(flav == 2 || !hasOS){
            if(_met < 50) continue;
            if(jetCount < 2 || nBJets() < 1) continue;
            if(!hasOS && (jetCount < 3 || jetCount > 4)) continue;
        }

        //new flavor categorization if the leptons are SS
        if(!hasOS){
            flav += 3;
        }
        //determine run perios
        unsigned run;
        run = ewk::runPeriod2017(_runNb) + 1 - 1; //reserve 0 for inclusive // -1 because we start at run B 
        //max pu to extract
        float max = (run == 0 || run > 3) ? 80 : 80;
        //float max = 100;
        //loop over leading leptons
        for(unsigned l = 0; l < 2; ++l){
            double fill[20] = {_3dIPSig[ind[l]], fabs(_dxy[ind[l]]), fabs(_dz[ind[l]]), _miniIso[ind[l]], _relIso[ind[l]],
                _leptonMvaSUSY16[ind[l]], 
                _leptonMvaTTH16[ind[l]], 
                _leptonMvatZqTTV16[ind[l]],
                _leptonMvaSUSY17[ind[l]], 
                _leptonMvaTTH17[ind[l]], 
                _ptRel[ind[l]], 
                _ptRatio[ind[l]], 
                _closestJetCsvV2[ind[l]], 
                _closestJetDeepCsv_b[ind[l]] + _closestJetDeepCsv_bb[ind[l]], 
                (double) _selectedTrackMult[ind[l]], 
                (_lFlavor[ind[l]] == 0) ? _lElectronMva[ind[l]] : 0,  
                (_lFlavor[ind[l]] == 0) ? _lElectronMvaHZZ[ind[l]] : 0,
                (_lFlavor[ind[l]] == 0) ? _lElectronMvaFall17NoIso[ind[l]] : 0,
                (_lFlavor[ind[l]] == 0) ? _lElectronMvaFall17Iso[ind[l]] : 0,
                (_lFlavor[ind[l]] == 1) ? _lMuonSegComp[ind[l]] : 0
            };
            //fill histograms
            for(unsigned j = 0; j < nJetCat; ++j){
                if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
                for(unsigned pu = 0; pu < nPuRew; ++pu){
                    for(unsigned dist = 0; dist < 20; ++dist){
                        if(_lFlavor[ind[l]] == 0 && dist == 19) continue;  //do not plot muonSegComp for electrons
                        if(_lFlavor[ind[l]] == 1 && (dist == 15 || dist == 16 || dist == 17 || dist == 18) ) continue;  //do not plot electronMva for muons
                        for(unsigned r = 0; r < nRuns; ++r){
                            if(!samp.isData() || r == run || r == 0){
                                double puw = 1.;
                                if(!samp.isData() && pu == 1){
                                    puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, max) ) );
                                }
                                histCollection.access(dist, {r, flav, j, pu})->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight*puw); 
                                histCollection.access(dist, {r, 0,    j, pu})->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight*puw);

                            }
                        }
                    }
                }
            }
        }
        //compute number of forward jets and find the highest eta jet
        std::vector<double> forwardJetPtCuts = {20, 25, 30, 35, 40, 45, 50, 55, 60};
        std::vector<unsigned> mostForwardJetIndices(9, 99); 
        std::vector<unsigned> highPtJetCount(9, 0);
        std::vector<unsigned> nForwardJets(9, 0);
        for(unsigned p = 0; p < forwardJetPtCuts.size(); ++p){
            for(unsigned j = 0; j < _nJets; ++j){
                if(jetIsGood(j, forwardJetPtCuts[p], 0, true, true) ){     //last "true" allows jets to have any eta value
                    if( (highPtJetCount[p] == 0) || ( fabs(_jetEta[j]) > fabs(_jetEta[mostForwardJetIndices[p]]) ) ){
                        mostForwardJetIndices[p] = j;
                    }
                    if(fabs(_jetEta[j]) >= 2.4){
                        ++nForwardJets[p];
                    }
                    ++highPtJetCount[p];
                }
            }
        }

        double fill[nDist - 20] = {_met, (lepV[0] + lepV[1]).M(), _lPt[ind[0]], _lPt[ind[1]], fabs(_lEta[ind[0]]), fabs(_lEta[ind[1]]), (double) _nVertex, (double) jetCount, (double) nBJets(0, false), (double) nBJets(),
           (highPtJetCount[1] > 0) ? _jetPt[mostForwardJetIndices[1]] : -9999., //pT of most forward jet after 25 GeV pT cut selection`
           (highPtJetCount[0] > 0) ? fabs(_jetEta[mostForwardJetIndices[0]]) : -9999., (highPtJetCount[0] > 0) ? _jetEta[mostForwardJetIndices[0]] : -9999., (double) nForwardJets[0], 
           (highPtJetCount[1] > 0) ? fabs(_jetEta[mostForwardJetIndices[1]]) : -9999., (highPtJetCount[1] > 0) ? _jetEta[mostForwardJetIndices[1]] : -9999., (double) nForwardJets[1], 
           (highPtJetCount[2] > 0) ? fabs(_jetEta[mostForwardJetIndices[2]]) : -9999., (highPtJetCount[2] > 0) ? _jetEta[mostForwardJetIndices[2]] : -9999., (double) nForwardJets[2], 
           (highPtJetCount[3] > 0) ? fabs(_jetEta[mostForwardJetIndices[3]]) : -9999., (highPtJetCount[3] > 0) ? _jetEta[mostForwardJetIndices[3]] : -9999., (double) nForwardJets[3], 
           (highPtJetCount[4] > 0) ? fabs(_jetEta[mostForwardJetIndices[4]]) : -9999., (highPtJetCount[4] > 0) ? _jetEta[mostForwardJetIndices[4]] : -9999., (double) nForwardJets[4], 
           (highPtJetCount[5] > 0) ? fabs(_jetEta[mostForwardJetIndices[5]]) : -9999., (highPtJetCount[5] > 0) ? _jetEta[mostForwardJetIndices[5]] : -9999., (double) nForwardJets[5], 
           (highPtJetCount[6] > 0) ? fabs(_jetEta[mostForwardJetIndices[6]]) : -9999., (highPtJetCount[6] > 0) ? _jetEta[mostForwardJetIndices[6]] : -9999., (double) nForwardJets[6], 
           (highPtJetCount[7] > 0) ? fabs(_jetEta[mostForwardJetIndices[7]]) : -9999., (highPtJetCount[7] > 0) ? _jetEta[mostForwardJetIndices[7]] : -9999., (double) nForwardJets[7], 
           (highPtJetCount[8] > 0) ? fabs(_jetEta[mostForwardJetIndices[8]]) : -9999., (highPtJetCount[8] > 0) ? _jetEta[mostForwardJetIndices[8]] : -9999., (double) nForwardJets[8]
        }; 

        for(unsigned j = 0; j < nJetCat; ++j){
            if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
            for(unsigned pu = 0; pu < nPuRew; ++pu){
                for(unsigned dist = 20; dist < nDist; ++dist){
                    //if(jetCount == 0 && dist > (nDist - 16) ) continue; //don't plot high eta-jet variables if there are no jets 
                    for(unsigned r = 0; r < nRuns; ++r){
                        if(!samp.isData() || r == run || r == 0){
                            double puw = 1.;
                            if(!samp.isData() && pu == 1){
                                puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, max) ) );
                            }
                            histCollection.access(dist, {r, flav, j, pu})->Fill(std::min(fill[dist - 20], histInfo[dist].maxBinCenter()), weight*puw);
                            histCollection.access(dist, {r, 0,    j, pu})->Fill(std::min(fill[dist - 20], histInfo[dist].maxBinCenter()), weight*puw);
                        }
                    }
                }
            } 
        }
    }
    //store histograms for later merging
    histCollection.store("tempHists/", begin, end);
}

void treeReader::splitJobs(){
    //clear previous histograms
    tools::system("rm tempHists/*");

    for(unsigned sam = 0; sam < samples.size(); ++sam){
        initSample();
        for(long unsigned it = 0; it < nEntries; it+=500000){
            long unsigned begin = it;
            long unsigned end = std::min(nEntries, it + 500000);
            //make temporary job script 
            std::ofstream script("runTuples.sh");
            tools::initScript(script);
            script << "./dilepCR " << samples[currentSampleIndex].getFileName() << " " << std::to_string(begin) << " " << std::to_string(end);
            script.close();
            //submit job
            tools::submitScript("runTuples.sh", "01:00:00");
         }
    }
}

void treeReader::plot(const std::string& distName){

    //loop over all distributions and find the one to plot
    for(size_t d = 0; d < histInfo.size(); ++d){
        if(histInfo[d].name() == distName){

            //read collection for this distribution from files
            HistCollectionDist col("inputList.txt", histInfo[d], samples, { {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"}, {"inclusive", "ee", "em", "mm", "same-sign-ee", "same-sign-em", "same-sign-mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });

            //print plots for collection
            bool is2016 = false;

            //rebin same-sign category because of low statistics
            std::vector<std::string> notToRebin = {"nJets", "nForwardJets", "nBJets"};//distributions not  to rebin
            bool doNotRebin = false;
            for(auto& name : notToRebin){
                if(distName.find(name) != std::string::npos){
                    doNotRebin = true;
                    break;
                }
            }
            if(!doNotRebin){
                col.rebin("same-sign", 5);
            }
            col.printPlots("plots/ewkino/dilepCR", is2016, "ewkinoDilep", true, true);
            col.printPlots("plots/ewkino/dilepCR", is2016, "ewkinoDilep", false, true);
        }
    }
}

void treeReader::splitPlots(){
    tools::makeFileList("tempHists", "inputList.txt");
    for(auto& h: histInfo){
        std::ofstream script("printPlots.sh");
        tools::initScript(script);
        script << "./dilepCR plot " << h.name();
        script.close();
        tools::submitScript("printPlots.sh", "00:30:00");
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
