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
        HistInfo("nBJets_CSVv2", "number of b-jets (CSVv2)", 8, 0, 8)
    };
}

void treeReader::Analyze(const std::string& sampName, const long unsigned begin, const long unsigned end){
    auto samIt = std::find_if(samples.cbegin(), samples.cend(), [sampName](const Sample& s) { return s.getFileName() == sampName; } );
    Analyze(*samIt, begin, end);
}

void treeReader::Analyze(const Sample& samp, const long unsigned begin, const long unsigned end){

    //categorization
    Category categorization({ {"onZ"}, {"nJetsInclusive", "0bJets", "1bJet01jets", "1bJet23Jets", "1bJet4Jets", "2bJets"} });

    //set up histogram collection for particular sample
    HistCollectionSample histCollection(histInfo, samp, categorization);

    //store relevant info from histCollection
    const unsigned nDist = histInfo.size();                              //number of distributions to plot
    const unsigned nCat = histCollection.categoryRange(1);                //Several categories enriched in different processes

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
            {"topMass", 0.},
            {"nJets", 0.},
            {"pTTaggedRecoilJet", 0.},
            {"etaZboson", 0.},
            {"deltaRZTop", 0.},
            {"pTZboson", 0.},
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
                //std::cout << "jet of negative mass = " << jet.M() << std::endl;
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
        if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue; 

        //remove overlap between samples
        if(photonOverlap(samp)) continue;

        //make lorentzvectors for leptons
        TLorentzVector lepV[lCount];
        for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);

        //make ordered jet and bjet collections
        std::vector<unsigned> jetInd, bJetInd;
        unsigned jetCount = nJets_TOP16_020(jetInd);
        unsigned bJetCount = nBJets_TOP16_020(bJetInd, 0, false, true, 0); 

        //veto events which have a jet in eta 2.69-3 with pt below 50 GeV
        bool badJetFound = false;
        for(unsigned j = 0; j < jetCount; ++j){
            bool inEE = ( fabs(_jetEta[j]) > 2.69 ) && ( fabs(_jetEta[j]) < 3. );
            bool lowpT = ( _jetPt[j] > 30 ) && (_jetPt[j] < 50);
            if(inEE && lowpT){
                badJetFound = true;
                break;
            }
        }
        if(badJetFound) continue;

        //find best Z candidate
        std::pair<unsigned, unsigned> bestZ;
        bestZ = trilep::bestZ(lepV, ind, _lFlavor, _lCharge, lCount);

        //determine best Z mass
        double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();

        //require presence of Z mass
        if(fabs(mll - 91.1876) >= 15) continue;

        //find W lepton 
        unsigned lw = 999;
        for(unsigned l = 0; l < lCount; ++l){
            if( l != bestZ.first && l != bestZ.second ) lw = l;
        }

        //make met vector 
        TLorentzVector met;
        met.SetPtEtaPhiE(_met, 0, _metPhi, _met);
        //MET and MT Cuts 
        if(_met <= 10) continue;
        if(kinematics::mt(lepV[lw], met) <= 10) continue;

        //only retain useful categories 
        unsigned tZqOldANCategory = 999;
        if(bJetCount == 0){
            tZqOldANCategory = 0;
        } else if(bJetCount == 1){
            if(jetCount == 2 || jetCount == 3){
                tZqOldANCategory = 1;
             } else if(jetCount > 3){
                 tZqOldANCategory = 2;
             }
        } else{
            tZqOldANCategory = 3;
        }
        //don't consider uncategorized events
        if(tZqOldANCategory == 999) continue;

        //apply event weight
        if(!samp.isData() ){
            //APPLY PU AND BTAG WEIGHTS HERE 
            //weight*=sfWeight();
        }

        //make LorentzVector for all jets 
        TLorentzVector jetV[(const unsigned) _nJets];
        for(unsigned j = 0; j < _nJets; ++j){
            jetV[j].SetPtEtaPhiE(_jetPt[j], _jetEta[j], _jetPhi[j], _jetE[j]);
        }

        //reconstruct top mass and tag jets
        std::vector<unsigned> taggedJetI; //0 -> b jet from tZq, 1 -> forward recoiling jet
        TLorentzVector neutrino = tzq::findBestNeutrinoAndTop(lepV[lw], met, taggedJetI, jetInd, bJetInd, jetV);

        //find jets with highest DeepCSV and CSVv2 values
        unsigned highestDeepCSVI = (jetCount == 0) ? 0 : jetInd[0], highestCSVv2I = (jetCount == 0) ? 0 : jetInd[0];
        for(unsigned j = 1; j < jetCount; ++j){
            if( (_jetDeepCsv_b[jetInd[j]] + _jetDeepCsv_bb[jetInd[j]]) > (_jetDeepCsv_b[highestDeepCSVI] + _jetDeepCsv_bb[highestDeepCSVI]) ) highestDeepCSVI = jetInd[j];
            if( _jetCsvV2[jetInd[j]] > _jetCsvV2[highestCSVv2I] ) highestCSVv2I = jetInd[j];
        }
    
        //initialize new vectors to make sure everything is defined for 0 jet events!
        TLorentzVector leadingJet(0,0,0,0);
        TLorentzVector recoilingJet(0,0,0,0);
        TLorentzVector taggedBJet(0,0,0,0);
        TLorentzVector leadingBJet(0,0,0,0);
        TLorentzVector highestCSVv2Jet(0,0,0,0);
        if(taggedJetI[0] != 99) taggedBJet = jetV[taggedJetI[0]];
        if(taggedJetI[1] != 99) recoilingJet = jetV[taggedJetI[1]];
        if(jetCount != 0){
            leadingJet = jetV[jetInd[0]];
            highestCSVv2Jet = jetV[highestCSVv2I]; 
        }
        if(bJetCount != 0){
            leadingBJet = jetV[bJetInd[0]];
        } else if(jetCount > 1){
            leadingBJet = jetV[jetInd[1]];
        }
        //compute top vector
        TLorentzVector topV = (neutrino + lepV[lw] + taggedBJet);

        //find closest jet to W lepton
        double deltaRWLepClosestJet = 99999.;
        for(unsigned j = 0; j < jetCount; ++j){
            if(jetV[jetInd[j]].DeltaR(lepV[lw]) < deltaRWLepClosestJet) deltaRWLepClosestJet = jetV[jetInd[j]].DeltaR(lepV[lw]);
        }
        bdtVariableMap["asymmetryWlep"] = _lEta[ind[lw]]*_lCharge[ind[lw]];
        bdtVariableMap["etaWLep"] = _lEta[ind[lw]];
        bdtVariableMap["highestCSVv2"] = _jetCsvV2[highestCSVv2I];
        bdtVariableMap["deltaPhiWlepTaggedbJet"] = fabs( lepV[lw].DeltaPhi(taggedBJet) );
        bdtVariableMap["deltaPhiWlepZ"] = fabs(lepV[lw].DeltaPhi( (lepV[bestZ.first] + lepV[bestZ.second]) ) );
        bdtVariableMap["deltaRWLepClosestJet"] = deltaRWLepClosestJet;
        bdtVariableMap["deltaRWlepRecoilingJet"] = lepV[lw].DeltaR(recoilingJet);
        bdtVariableMap["etaTaggedRecoilJet"] = fabs(recoilingJet.Eta());
        bdtVariableMap["etaLeadingJet"] = fabs(leadingJet.Eta());
        bdtVariableMap["topMass"] =  std::max(topV.M(), 0.);
        bdtVariableMap["nJets"] = jetCount;
        bdtVariableMap["pTTaggedRecoilJet"] = recoilingJet.Pt();
        bdtVariableMap["etaZboson"] = ( (lepV[bestZ.first] + lepV[bestZ.second]).Eta() );
        bdtVariableMap["deltaRZTop"] = topV.DeltaR( (lepV[bestZ.first] + lepV[bestZ.second]) );
        bdtVariableMap["pTZboson"] = (lepV[bestZ.first] + lepV[bestZ.second]).Pt();
        bdtVariableMap["eventWeight"] = weight;

        double bdt = 0;
        //bdt = mvaReader[mllCat][tzqCat]->EvaluateMVA("BDTG method");
        double fill[nDist] = {
            bdt, 
            _lEta[ind[lw]]*_lCharge[ind[lw]],
            _lEta[ind[lw]],
            _jetCsvV2[highestCSVv2I],
            fabs( lepV[lw].DeltaPhi(taggedBJet) ),
            fabs(lepV[lw].DeltaPhi( (lepV[bestZ.first] + lepV[bestZ.second]) ) ),   
            deltaRWLepClosestJet,
            lepV[lw].DeltaR(recoilingJet),
            fabs(recoilingJet.Eta()),
            fabs(leadingJet.Eta()),
            std::max(topV.M(), 0.),
            (double) jetCount,
            recoilingJet.Pt(),
            ( (lepV[bestZ.first] + lepV[bestZ.second]).Eta() ),
            topV.DeltaR( (lepV[bestZ.first] + lepV[bestZ.second]) ),
            (lepV[bestZ.first] + lepV[bestZ.second]).Pt(),
            mll,
            kinematics::mt(lepV[lw], met),
            _lPt[ind[0]], _lPt[ind[1]], _lPt[ind[2]],
            (double) nBJets(),
            (double) nBJets(0, false)
        };

        for(unsigned cat = 0; cat < nCat; ++cat){
            if(cat == 0 || cat == (tZqOldANCategory + 1) ){
                //Fill training tree
                if(samp.getProcessName() != "DY" ) trainingTree.fill({0, cat}, bdtVariableMap); //fluctuations on DY sample too big for training
                for(unsigned dist = 0; dist < nDist; ++dist){
                    histCollection.access(dist, {0, cat})->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter()), weight);
                }
            }
        }
    }
    //write histcollection to file
    histCollection.store("tempHists_tZq_TOP_16_020/", begin, end);
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
            script << "./tZqCopyOldAN " << samples[currentSample].getFileName() << " " << std::to_string(begin) << " " << std::to_string(end);
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
            HistCollectionDist col("inputList.txt", histInfo[d], samples, { {"onZ"}, {"nJetsInclusive", "0bJets", "1bJet23Jets", "1bJet4Jets", "2bJets"} });
            //print plots for collection
            bool is2016 = true;
            col.printPlots("plots/tZq/TOP-16-020", is2016, "tzq", false);
            col.printPlots("plots/tZq/TOP-16-020", is2016, "tzq", true);
        }
    }
}

void treeReader::splitPlots(){
    tools::makeFileList("tempHists_tZq_TOP_16_020", "inputList.txt");
    for(auto& h: histInfo){
        std::ofstream script("printPlots.sh");
        tools::initScript(script);
        script << "./tZqCopyOldAN plot " << h.name();
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
