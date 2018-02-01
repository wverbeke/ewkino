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
    histCollection = HistCollection(histInfo, samples, { {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"}, {"inclusive", "ee", "em", "mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });
}

void treeReader::Analyze(const std::string& sampName, const long unsigned begin, const long unsigned end){
    auto samIt = std::find_if(samples.cbegin(), samples.cend(), [sampName](const Sample& s) { return s.getFileName() == sampName; } );
    Analyze(*samIt, begin, end);
}

void treeReader::Analyze(const Sample& samp, const long unsigned begin, const long unsigned end){
    //set up histogram collection for particular sample
    HistCollectionSample histCollection = HistCollectionSample(histInfo, std::make_shared<Sample>(samp), { {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"}, {"inclusive", "ee", "em", "mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });

    //read pu weights for every period
    TFile* puFile = TFile::Open("weights/puWeights2017.root");
    const std::string eras[6] = {"Inclusive", "B", "C", "D", "E", "F"};
    TH1D* puWeights[6];
    for(unsigned e = 0; e < 6; ++e){
        puWeights[e] = (TH1D*) puFile->Get( (const TString&) "puw_Run" + eras[e]);
    }

    const unsigned nDist = histCollection.infoRange();
    const unsigned nRuns = histCollection.catRange(0);
    const unsigned nFlav = histCollection.catRange(1);
    const unsigned nJetCat = histCollection.catRange(2);
    const unsigned nPuRew = histCollection.catRange(3);

    initSample(samp, 1);  //use 2017 lumi
    for(long unsigned it = begin; it < end; ++it){
        GetEntry(samp, it);
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
        //loop over leading leptons
        for(unsigned l = 0; l < 2; ++l){
            double fill[12] = {_3dIPSig[ind[l]], fabs(_dxy[ind[l]]), fabs(_dz[ind[l]]), _miniIso[ind[l]], _leptonMvaSUSY[ind[l]], _leptonMvaTTH[ind[l]], _ptRel[ind[l]], _ptRatio[ind[l]], _closestJetCsvV2[ind[l]], (double) _selectedTrackMult[ind[l]], (_lFlavor[ind[l]] == 0) ? _lElectronMva[ind[l]] : 0,  (_lFlavor[ind[l]] == 1) ? _lMuonSegComp[ind[l]] : 0};
            //fill histograms
            for(unsigned j = 0; j < nJetCat; ++j){
                if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
                for(unsigned pu = 0; pu < nPuRew; ++pu){
                    for(unsigned dist = 0; dist < 12; ++dist){
                        for(unsigned r = 0; r < nRuns; ++r){
                            if(samp.isData() || r == run || r == 0){
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
        double fill[nDist - 12] = {_met, (lepV[0] + lepV[1]).M(), _lPt[ind[0]], _lPt[ind[1]], (double) _nVertex, (double) jetCount, (double) nBJets(0, false), (double) nBJets()}; //replace 0 by _met for correct trees
        for(unsigned j = 0; j < nJetCat; ++j){
            if(j == 1 && ( (jetCount == 0) ? false :_jetPt[jetInd[0]] <= 40 ) ) continue;
            for(unsigned pu = 0; pu < nPuRew; ++pu){
                for(unsigned dist = 12; dist < nDist; ++dist){
                    for(unsigned r = 0; r < nRuns; ++r){
                        if(!samp.isData() || r == run || r == 0){
                            double puw = 1.;
                            if(!samp.isData() && pu == 1){
                                puw = puWeights[run]->GetBinContent(puWeights[run]->FindBin( std::min(_nTrueInt, max) ) );
                            }
                            histCollection.access(dist, {r, flav, j, pu})->Fill(std::min(fill[dist - 12], histInfo[dist].maxBinCenter()), weight*puw);
                            histCollection.access(dist, {r, 0,    j, pu})->Fill(std::min(fill[dist - 12], histInfo[dist].maxBinCenter()), weight*puw);
                        }
                    }
                }
            } 
        }
    }
    //store histograms for later merging
    histCollection.store("tempHists/", begin, end);
}

std::ostream& initScript(std::ostream& os){
     os << "cd /user/wverbeke/CMSSW_9_4_2/src \n";
     os << "source /cvmfs/cms.cern.ch/cmsset_default.sh \n";
     os << "eval \\`scram runtime -sh\\` \n";
     os << "cd /user/wverbeke/Work/AnalysisCode/ewkino/ \n";
     return os;
}


void treeReader::splitJobs(){
    for(unsigned sam = 0; sam < samples.size(); ++sam){
        initSample(1);
        for(long unsigned it = 0; it < nEntries; it+=1000000){
            long unsigned begin = it;
            long unsigned end = std::min(nEntries, it + 1000000);
            //make temporary job script 
            std::ofstream script("runTuples.sh");
            initScript(script);
            script << "./dilepCR " << samples[currentSample].getFileName() << " " << std::to_string(begin) << " " << std::to_string(end);
            script.close();
            //submit job
            //std::system( ("./dilepCR " + samples[currentSample].getFileName() + " " + std::to_string(begin) + " " + std::to_string(end) ).c_str() );
            std::system("qsub runTuples.sh -l walltime=01:00:00");
         }
    }
}

void treeReader::plot(const std::string& distName){
    //readPlots();
    histCollection = HistCollection("tempHists", histInfo, samples, { {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"}, {"inclusive", "ee", "em", "mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });
    HistCollection col = histCollection.mergeProcesses();    
    for(unsigned d = 0; d < histInfo.size(); ++d){
        if(histInfo[d].name() == distName){
            for(unsigned c = 0; c < col.catSize(); ++c){
                col.getPlot(d,c).draw();
            }
        }
    }
}

void treeReader::readPlots(){
    //histCollection.read("tempHists");
}

void treeReader::splitPlots(){
    histCollection = HistCollection("tempHists", histInfo, samples, { {"all2017", "RunB", "RunC", "RunD", "RunE", "RunF"}, {"inclusive", "ee", "em", "mm"}, {"nJetsInclusive", "1pt40Jet"}, {"noPuW", "PuW"} });
    /*
    for(auto& h: histInfo){
        std::ofstream script("printPlots.sh");
        initScript(script);
        script << "./dilepCR plot " << h.name();
        script.close();
        std::system("qsub printPlots.sh -l walltime=00:30:00");
    }
    */
}

int main(int argc, char* argv[]){
    treeReader reader;
    reader.setup();
    std::vector<std::string> argvStr;
    for(unsigned i = 0; i < argc; ++i){
        argvStr.push_back(std::string(argv[i]));
    }
    //submit single job if sample and range given
    if(argc == 4){
        std::string sample(argv[1]);
        std::string beginStr(argv[2]);
        std::string endStr(argv[3]);
        long unsigned begin = std::stoul(beginStr);
        long unsigned end = std::stoul(endStr);
        //MODIFY THIS CALL TO ACCEPT A STRING AS SAMPLE
        //std::cout << sample << "\t" << begin << "\t" << end << std::endl;
        reader.Analyze(sample, begin, end);
    //else if(argc == 2){
    }
    else if(argc > 1 && argvStr[1] == "plot"){
        reader.readPlots();
        if(argc > 2){
            std::string dist(argv[2]);
            reader.plot(dist);
        } else{
            reader.splitPlots();           
        }
    }
    else{
        //Analyze all, or split jobs
        reader.splitJobs();
    }
    return 0;
}
/*
void treeReader::Analyze(){
    //read pu weights for every period
    TFile* puFile = TFile::Open("weights/puWeights2017.root");
    const std::string eras[6] = {"Inclusive", "B", "C", "D", "E", "F"};
    TH1D* puWeights[6];
    for(unsigned e = 0; e < 6; ++e){
        puWeights[e] = (TH1D*) puFile->Get( (const TString&) "puw_Run" + eras[e]);
    }

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
    HistCollection col = histCollection.mergeProcesses();    
    for(unsigned d = 0; d < histInfo.size(); ++d){
        for(unsigned c = 0; c < histCollection.catSize(); ++c){
            col.getPlot(d,c).draw();
        } 
    }
}
*/
