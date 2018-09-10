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
#include "../interface/systemTools.h"
#include "../interface/trilepTools.h"
#include "../interface/HistCollectionSample.h"
#include "../interface/HistCollectionDist.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"
#include "../interface/Reweighter_old.h"


void treeReader::setup(){

    //Set CMS plotting style
    setTDRStyle();
    gROOT->SetBatch(kTRUE);

    //read samples and cross sections from txt file
    readSamples2016("sampleLists/samples_dilepCR_2016.txt", "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino");
    readSamples2017("sampleLists/samples_dilepCR_2017.txt", "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino");

    //info on kinematic distributions to plot
    //name      xlabel    nBins,  min, max
    histInfo = {
        HistInfo("met", "E_{T}^{miss} (GeV)", 100, 0, 300),
        HistInfo("mll", "M_{ll} (GeV)", 200, 12, 200),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 100, 25, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 100, 15, 150),
        HistInfo("muonPt", "P_{T}^{muon} (GeV)", 100, 15, 200),
        HistInfo("electronPt", "P_{T}^{electron} (GeV)", 100, 15, 200),
        HistInfo("leadinEta", "|#eta|^{leading}", 100, 0, 2.5),
        HistInfo("trailingEta", "|#eta|^{trailing}", 100, 0, 2.5),
        HistInfo("nVertex", "number of vertices", 100, 0, 100),
        HistInfo("nJets", "number of jets", 10, 0, 10),
        HistInfo("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8),
        HistInfo("nBJets_DeepCSV", "number of b-jets (Deep CSV)", 8, 0, 8),
        HistInfo("flavors", "flavors", 3, 0, 3, {"ee", "e#mu", "#mu#mu"} )
    };
}

void treeReader::Analyze(const std::string& sampName, const long unsigned begin, const long unsigned end){
    auto samIt = std::find_if(samples.cbegin(), samples.cend(), [sampName](const Sample& s) { return s.getUniqueName() == sampName; } );
    if(samIt == samples.cend()){
        std::cerr << "Error : Given sample name not found in list of samples!" << std::endl;
        return;
    }
    Analyze(*samIt, begin, end);
}

void treeReader::Analyze(const Sample& samp, const long unsigned begin, const long unsigned end){
    //initialize sample 
    initSample(samp);

    //set up histogram collection for particular sample
    HistCollectionSample histCollection(histInfo, samp, { {"inclusive", "ee", "em", "mm", "same-sign-ee", "same-sign-em", "same-sign-mm"} });

    const unsigned nDist = histInfo.size();

    for(long unsigned it = begin; it < end; ++it){
        GetEntry(samp, it);

        //vector containing good lepton indices
        std::vector<unsigned> ind;

        //require met filters and triggers
        if( !passMETFilters() ) continue;
        if( !(passSingleLeptonTriggers() || passDileptonTriggers() ) ) continue;

        //select leptons
        const unsigned lCount = selectLep(ind);
        if(lCount != 2) continue;

        //require leptons to be tight 
        const unsigned tightCount = tightLepCount( ind, lCount);
        if( tightCount != lCount ) continue;

        //require pt cuts (25, 20) to be passed
        if( _lPt[ind[0]] < 40 ) continue;
        if( _lPt[ind[1]] < 20 ) continue;

        //make lorentzvectors for leptons
        TLorentzVector lepV[lCount];
        for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);

        //Cut of Mll at 12 GeV
        if((lepV[0] + lepV[1]).M() < 12) continue;

        //determine flavor compositions
        unsigned flav = dilFlavorComb(ind) + 1; //reserve 0 for inclusive

        //determine whether there is an OS lepton pair 
        bool hasOS = ( _lCharge[ind[0]] != _lCharge[ind[1]] );

        //order jets and check the number of jets
        std::vector<unsigned> jetInd;
        unsigned jetCount = nJets(jetInd);

        //number of jets for categorization (not forward)
        unsigned jetCountCategorization =  nJets(0, true, false);

        //Extra category selection: for DY select onZ, for ttbar select 1 b-jet 2-jets
        if((flav == 1 || flav == 3) && hasOS){ //OSSF
            if( fabs((lepV[0] + lepV[1]).M() - 91) > 10 ) continue;
            if(nBJets(0,  true, true, 0) != 0) continue;    //veto cleaned jets, maybe this fixes csv distribution of close jets
        } else if(flav == 2 || !hasOS){
            if(_met < 50) continue;
            if(jetCountCategorization < 2 || nBJets() < 1) continue;
            if(!hasOS && (jetCountCategorization < 3 || jetCountCategorization > 4)) continue;
        }

        //new flavor categorization if the leptons are SS
        if(!hasOS){
            flav += 3;
        }

        //apply reweighting
        if( isMC() ){
            weight *= sfWeight();
        }

        //pt of leading muon and electron
        double muonPt = 0;
        double electronPt = 0;
        for( unsigned l = 0; l < lCount; ++l){
            if( isMuon(ind[l]) && muonPt == 0){
                muonPt = _lPt[ind[l]];
            } else if( isElectron(ind[l]) && electronPt == 0){
                electronPt = _lPt[ind[l]];
            }
        }

        double fill[nDist] = {_met, (lepV[0] + lepV[1]).M(), _lPt[ind[0]], _lPt[ind[1]], muonPt, electronPt, fabs(_lEta[ind[0]]), fabs(_lEta[ind[1]]), (double) _nVertex, 
            (double) jetCount, (double) nBJets(0, false), (double) nBJets(), (double) (flav - 1)}; 

        for(unsigned dist = 0; dist < nDist; ++dist){
            if( flav == 1 && dist == 4) continue;
            if( flav == 3 && dist == 5) continue;
            if(dist == 0){
                std::cout << "filling met, weight = " << weight << std::endl;
            }
            histCollection.access(dist, std::vector<size_t>(1, flav) )->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight);
            histCollection.access(dist, std::vector<size_t>(1, 0) )->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight);
        }
    }

    //store histograms for later merging
    histCollection.store("tempHists", begin, end);
}

void treeReader::splitJobs(){

    //clear previous histograms
    systemTools::system("rm tempHists/*");

    for(unsigned sam = 0; sam < samples.size(); ++sam){
        initSample();
        for(long unsigned it = 0; it < nEntries; it+=500000){
            long unsigned begin = it;
            long unsigned end = std::min(nEntries, it + 500000);

            //make temporary job script 
            std::ofstream script("runTuples.sh");
            systemTools::initScript(script);
            script << "./dilepCR " << samples[currentSampleIndex].getUniqueName() << " " << std::to_string(begin) << " " << std::to_string(end);
            script.close();

            //submit job
            systemTools::submitScript("runTuples.sh", "02:00:00");
         }
    }
}

void treeReader::plot(const std::string& distName){

    //loop over all distributions and find the one to plot
    for(size_t d = 0; d < histInfo.size(); ++d){
        if(histInfo[d].name() == distName){
        
            //read collection for this distribution from files
            HistCollectionDist col2016("tempHists", histInfo[d], samples2016, { {"inclusive", "ee", "em", "mm", "same-sign-ee", "same-sign-em", "same-sign-mm"} });
            HistCollectionDist col2017("tempHists", histInfo[d], samples2017, { {"inclusive", "ee", "em", "mm", "same-sign-ee", "same-sign-em", "same-sign-mm"} });

            //rebin same-sign category because of low statistics
            std::vector<std::string> notToRebin = {"nJets", "nBJets"};//distributions not  to rebin
            bool doNotRebin = false;
            for(auto& name : notToRebin){
                if(distName.find(name) != std::string::npos){
                    doNotRebin = true;
                    break;
                }
            }
            if(!doNotRebin){
                col2016.rebin("same-sign", 5);
                col2017.rebin("same-sign", 5);
            }

            unsigned is2016 = 0;
            unsigned is2017 = 1;

            //print plots for collection
            /*
            col2016.printPlots("plots/tZq/2016/controlR/dilepCR/", is2016, "ewkinoDilep", true, true);     //log
            col2016.printPlots("plots/tZq/2016/controlR/dilepCR/", is2016, "ewkinoDilep", false, true);    //linear

            col2017.printPlots("plots/tZq/2017/controlR/dilepCR/", is2017, "ewkinoDilep", true, true);     //log
            col2017.printPlots("plots/tZq/2017/controlR/dilepCR/", is2017, "ewkinoDilep", false, true);    //linear
            */
            col2016.printPlots("plots/tZq/2016/controlR/dilepCR/", is2016, "ewkinoDilep", true);     //log
            col2016.printPlots("plots/tZq/2016/controlR/dilepCR/", is2016, "ewkinoDilep", false);    //linear

            col2017.printPlots("plots/tZq/2017/controlR/dilepCR/", is2017, "ewkinoDilep", true);     //log
            col2017.printPlots("plots/tZq/2017/controlR/dilepCR/", is2017, "ewkinoDilep", false);    //linear
        }
    }
}

void treeReader::splitPlots(){
    //tools::makeFileList("tempHists", "inputList.txt");
    for(auto& h: histInfo){
        std::ofstream script("printPlots.sh");
        systemTools::initScript(script);
        script << "./dilepCR plot " << h.name();
        script.close();
        systemTools::submitScript("printPlots.sh", "00:30:00");
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
        if( systemTools::runningJobs() ) std::cout << "jobs are running!" << std::endl;
        while( systemTools::runningJobs("runTuples.sh") ){
            systemTools::sleep(60);
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
