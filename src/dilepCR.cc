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
#include "../interface/ewkinoTools.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"

void treeReader::Analyze(){
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
        HistInfo("leptonMvaSUSY", "SUSY lepton MVA value", 100, -1, 1),
        HistInfo("leptonMvaTTH", "TTH lepton MVA value", 100, -1, 1),
        HistInfo("ptRel", "P_{T}^{rel} (GeV)", 100, 0, 200),
        HistInfo("ptRatio", "P_{T}^{ratio}", 100, 0, 2),
        HistInfo("closestJetCsv", "closest jet CSV", 100, 0, 1),
        HistInfo("chargedTrackMult", "closest jet track multiplicity", 20, 0, 20),
        HistInfo("electronMvaGP", "electron GP MVA value", 100, -1, 1),
        HistInfo("muonSegComp", "muon segment compatibility", 100, 0, 1),
        HistInfo("met", "E_{T}^{miss} (GeV)", 100, 0, 300),
        HistInfo("mll", "M_{ll} (GeV)", 200, 12, 200),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 100, 25, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 100, 15, 150),
        HistInfo("nVertex", "number of vertices", 100, 0, 100),
        HistInfo("nJets", "number of jets", 10, 0, 10),
        HistInfo("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
        HistInfo("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8)
    };

    std::vector< std::tuple < std::string, std::string, unsigned, double , double > > histInfos;
    //name      xlabel    nBins,  min, max
    histInfos = {
        std::make_tuple("sip3d", "SIP_{3D}", 100, 0, 8),
        std::make_tuple("dxy", "|d_{xy}| (cm)", 100, 0, 0.05),
        std::make_tuple("dz", "|d_{z}| (cm)", 100, 0, 0.1),
        std::make_tuple("miniIso", "miniIso", 100, 0, 0.4),
        std::make_tuple("leptonMvaSUSY", "SUSY lepton MVA value", 100, -1, 1),
        std::make_tuple("leptonMvaTTH", "TTH lepton MVA value", 100, -1, 1),
        std::make_tuple("ptRel", "P_{T}^{rel} (GeV)", 100, 0, 200),
        std::make_tuple("ptRatio", "P_{T}^{ratio}", 100, 0, 2),
        std::make_tuple("closestJetCsv", "closest jet CSV", 100, 0, 1),
        std::make_tuple("chargedTrackMult", "closest jet track multiplicity", 20, 0, 20),
        std::make_tuple("electronMvaGP", "electron GP MVA value", 100, -1, 1),
        std::make_tuple("muonSegComp", "muon segment compatibility", 100, 0, 1),
        std::make_tuple("met", "E_{T}^{miss} (GeV)", 100, 0, 300),
        std::make_tuple("mll", "M_{ll} (GeV)", 200, 12, 200),
        std::make_tuple("leadPt", "P_{T}^{leading} (GeV)", 100, 25, 200),
        std::make_tuple("trailPt", "P_{T}^{trailing} (GeV)", 100, 15, 150),
        std::make_tuple("nVertex", "number of vertices", 100, 0, 100),
        std::make_tuple("nJets", "number of jets", 10, 0, 10),
        std::make_tuple("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
        std::make_tuple("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8)
    };
    //read pu weights for every period
    TFile* puFile = TFile::Open("weights/puWeights2017.root");
    const std::string eras[6] = {"Inclusive", "B", "C", "D", "E", "F"};
    TH1D* puWeights[6];
    for(unsigned e = 0; e < 6; ++e){
        puWeights[e] = (TH1D*) puFile->Get( (const TString&) "puw_Run" + eras[e]);
    }

    histCollection = HistCollection(histInfo, samples, { {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"}, {"inclusive", "ee", "em", "mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });
    const unsigned nDist = histCollection.infoRange();
    const unsigned nRuns = histCollection.catRange(0);
    const unsigned nFlav = histCollection.catRange(1);
    const unsigned nJetCat = histCollection.catRange(2);
    const unsigned nPuRew = histCollection.catRange(3);
    //tweakable options
    //const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        std::cout << "sam = " << sam << std::endl;
        initSample(1);  //use 2017 lumi
        std::cout<<"Entries in "<< samples[currentSample] << " " << nEntries << std::endl;
        double progress = 0; 	//for printing progress bar
        for(long unsigned it = 0; it < nEntries/1000; ++it){
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
            if(lCount != 2) continue;
            //require pt cuts (25, 20) to be passed
            if(!passPtCuts(ind)) continue;
            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
            //Cut of Mll at 12 GeV
            if((lepV[0] + lepV[1]).M() < 12) continue;
            //reject SS events
            if(_lCharge[ind[0]] == _lCharge[ind[1]]) continue;
            //determine flavor compositions
            unsigned flav = dilFlavorComb(ind) + 1; //reserve 0 for inclusive
            //order jets and check the number of jets
            std::vector<unsigned> jetInd;
            unsigned jetCount = nJets(jetInd);

            //Extra category selection: for DY select onZ, for ttbar select 1 b-jet 2-jets
            if(flav == 1 || flav == 3){ //OSSF
                if(fabs((lepV[0] + lepV[1]).M() - 91) > 10) continue;
                if(nBJets(0,  true, false, 0) != 0) continue;
            } else if(flav == 2){
                if(_met < 50) continue;
                if(jetCount < 2 || nBJets() < 1) continue;
            }
            //determine run perios
            unsigned run;
            run = ewk::runPeriod(_runNb) + 1 - 1; //reserve 0 for inclusive // -1 because we start at run B 
            //max pu to extract
            float max = (run == 0 || run > 3) ? 70 : 60;
            //float max = 60;
            //loop over leading leptons
            for(unsigned l = 0; l < 2; ++l){
                double fill[12] = {_3dIPSig[ind[l]], fabs(_dxy[ind[l]]), fabs(_dz[ind[l]]), _miniIso[ind[l]], _leptonMvaSUSY[ind[l]], _leptonMvaTTH[ind[l]], _ptRel[ind[l]], _ptRatio[ind[l]], _closestJetCsvV2[ind[l]], (double) _selectedTrackMult[ind[l]], (_lFlavor[ind[l]] == 0) ? _lElectronMva[ind[l]] : 0,  (_lFlavor[ind[l]] == 1) ? _lMuonSegComp[ind[l]] : 0};
                //fill histograms
                for(unsigned j = 0; j < nJetCat; ++j){
                    if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
                    for(unsigned pu = 0; pu < nPuRew; ++pu){
                        for(unsigned dist = 0; dist < 12; ++dist){
                            for(unsigned r = 0; r < nRuns; ++r){
                                if(sam != 0 || r == run || r == 0){
                                    double puw = 1.;
                                    if(sam != 0 && pu == 1){
                                        puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, max) ) );
                                    }
                                    histCollection.access(sam, dist, {r, flav, j, pu})->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight*puw); 
                                    histCollection.access(sam, dist, {r, 0,    j, pu})->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight*puw);

                                }
                            }
                        }
                    }
                }
            }
            double fill[nDist - 12] = {_met, (lepV[0] + lepV[1]).M(), _lPt[ind[0]], _lPt[ind[1]], (double) _nVertex, (double) jetCount, (double) nBJets(0, false), (double) nBJets()}; //replace 0 by _met for correct trees
            for(unsigned j = 0; j < nJetCat; ++j){
                if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
                for(unsigned pu = 0; pu < nPuRew; ++pu){
                    for(unsigned dist = 12; dist < nDist; ++dist){
                        for(unsigned r = 0; r < nRuns; ++r){
                            if(sam != 0 || r == run || r == 0){
                                double puw = 1.;
                                if(sam != 0 && pu == 1){
                                    puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, max) ) );
                                }
                                histCollection.access(sam, dist, {r, flav, j, pu})->Fill(std::min(fill[dist - 12], histInfo[dist].maxBinCenter()), weight*puw);
                                histCollection.access(sam, dist, {r, 0,    j, pu})->Fill(std::min(fill[dist - 12], histInfo[dist].maxBinCenter()), weight*puw);
                            }
                        }
                    }
                } 
            }
        }
    }
    std::cout << "merging processes" << std::endl;
    HistCollection col = histCollection.mergeProcesses();    
    std::cout << "processes merged" << std::endl;

    for(unsigned d = 0; d < histInfo.size(); ++d){
        if(d > 3) break;
        for(unsigned c = 0; c < histCollection.catSize(); ++c){
            if(c > 3) break;
            col.getPlot(d,c).draw();
        } 
    }
    /*
    //merge histograms with the same physical background
    std::vector<std::string> proc = {"obs.", "DY", "TT + Jets", "WJets", "VV", "TT + X", "T + X"};
    std::vector< std::vector <std::vector < std::vector< std::vector < std::vector < TH1D*> > > > > > mergedHists(nJetCat);
    for(unsigned j = 0; j < nJetCat; ++j){
        mergedHists[j] = std::vector <std::vector < std::vector< std::vector < std::vector < TH1D*> > > > >(nPuRew);
        for(unsigned pu = 0; pu < nPuRew; ++pu){
            mergedHists[j][pu] = std::vector <std::vector < std::vector< std::vector < TH1D*> > > >(nRuns);
            for(unsigned run = 0; run < nRuns; ++run){
                mergedHists[j][pu][run] = std::vector <std::vector < std::vector < TH1D*> > > (nFlav);
                for(unsigned flav = 0; flav < nFlav; ++flav){
                    mergedHists[j][pu][run][flav] = std::vector <std::vector < TH1D*> > (nDist);
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        mergedHists[j][pu][run][flav][dist] = std::vector<TH1D*>(proc.size());
                        for(size_t m = 0, sam = 0; m < proc.size(); ++m){
                            mergedHists[j][pu][run][flav][dist][m] = (TH1D*) hists[j][pu][run][flav][dist][sam]->Clone();
                            while(sam < samples.size() - 1 && std::get<0>(samples[sam]) == std::get<0>(samples[sam + 1]) ){
                                mergedHists[j][pu][run][flav][dist][m]->Add(hists[j][pu][run][flav][dist][sam + 1]);
                                ++sam;
                            }
                            ++sam;
                        }
                    }
                }
            }
        }
    }
    //plot all distributions
    //const std::string runString[nRuns] = {"32.5 fb^{-1} (13 TeV)", "2017 Run A", "2017 Run B", "2017 Run C", "2017 Run D", "2017 Run E", "2017 Run F"};
    const std::string runString[nRuns] = {"42 fb^{-1} (13 TeV)", "2017 Run B", "2017 Run C", "2017 Run D", "2017 Run E", "2017 Run F"};
    const std::string flavString[nFlav] = {"", "ee : ", "e#mu : ", "#mu#mu : "};
    for(unsigned j = 0; j < nJetCat; ++j){
        for(unsigned pu = 0; pu < nPuRew; ++pu){
            for(unsigned run = 0; run < nRuns; ++run){
                for(unsigned flav = 0; flav < nFlav; ++flav){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        plotDataVSMC(mergedHists[j][pu][run][flav][dist][0], &mergedHists[j][pu][run][flav][dist][1], &proc[0], mergedHists[j][pu][run][flav][dist].size() - 1, "ewkino/dilepCR/" + runNames[run] + "/" + flavNames[flav] + "/" + jetNames[j] + "/" + puNames[pu] + "/" + std::get<0>(histInfo[dist]) + "_" + flavNames[flav] + "_" + runNames[run], "ewkinoDilep", false, true, flavString[flav] + runString[run]);             //linear plots
                        plotDataVSMC(mergedHists[j][pu][run][flav][dist][0], &mergedHists[j][pu][run][flav][dist][1], &proc[0], mergedHists[j][pu][run][flav][dist].size() - 1, "ewkino/dilepCR/" + runNames[run] + "/" + flavNames[flav] + "/" + jetNames[j] + "/" + puNames[pu] + "/" + std::get<0>(histInfo[dist]) + "_" + flavNames[flav] + "_" + runNames[run] + "_log", "ewkinoDilep", true, true, flavString[flav] + runString[run]);     //log plots
                    }
                }
            }
        }
    }
    */
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
