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
#include "../interface/PostFitScaler.h"
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
    readSamples2016("sampleLists/samples_nonpromptDataDriven_2016.txt", "../../ntuples_tzq");
    readSamples2017("sampleLists/samples_nonpromptDataDriven_2017.txt", "../../ntuples_tzq");

    //name      xlabel    nBins,  min, max
    histInfo = {
        HistInfo("totalYield", "Total control region yield", 1, 0, 1),
        HistInfo("taggedRecoilJetEta", "|#eta| (recoiling jet) (GeV)", 20, 0, 5),
        HistInfo("maxMJetJet", "M_{jet + jet}^{max} (GeV)", 20, 0, 1200),
        HistInfo("asymmetryWLep", "asymmetry (lepton from W)",20, -2.5, 2.5),
        HistInfo("highestDeepCSV", "highest deepCSV (b + bb)", 20, 0, 1),
        HistInfo("LTPlusMET", "L_{T} + E_{T}^{miss} (GeV)", 20, 0, 800),
        HistInfo("maxDeltaPhiJetJet", "max(#Delta#Phi(jet, jet))", 20, 0, 3.15),
        HistInfo("mt", "M_{T} (GeV)", 20, 0, 300),
        HistInfo("mtop", "M_{(W + b)} (GeV)", 20, 0, 400),
        HistInfo("maxpTJetJet", "P_{T}^{max}(jet + jet) (GeV)", 20, 0, 300),
        HistInfo("minDeltaPhiLeptonBJet", "min(#Delta#Phi(l, bjet))", 20, 0, 3.15),
        HistInfo("maxDeltaPhiLeptonLepton", "max(#Delta#Phi(l, l))", 20, 0, 3.15),
        HistInfo("HT", "H_{T} (GeV)", 20, 0, 800),
        HistInfo("deltaRTaggedBJetRecoilingJet", "#DeltaR(tagged b-jet, recoiling jet)", 20, 0, 10),
        HistInfo("deltaRWlepTaggedbJet", "#DeltaR(lepton from W, tagged b-jet)", 20, 0, 7),
        HistInfo("m3l", "M_{3l} (GeV)", 20, 0, 600),
        HistInfo("jetEta_highestEta", "|#eta| (most forward jet)", 20, 0, 5),
        HistInfo("nJets", "number of jets", 10, 0, 10),
        HistInfo("nbJets", "number of b-jets", 8, 0, 8),
        HistInfo("met", "E_{T}^{miss} (GeV)", 20, 0, 300),
        HistInfo("leadPt", "P_{T}^{leading} (GeV)", 20, 25, 200),
        HistInfo("subPt", "P_{T}^{subleading} (GeV)", 20, 15, 200),
        HistInfo("trailPt", "P_{T}^{trailing} (GeV)", 20, 10, 200),
        HistInfo("etaLeading", "|#eta| (leading lepton)", 30, -2.5, 2.5),
        HistInfo("etaSubLeading", "|#eta| (subleading lepton)", 30, -2.5, 2.5),
        HistInfo("etaTrailing", "|#eta| (trailing lepton)", 30, -2.5, 2.5),
        HistInfo("nVertices", "number of vertices", 20, 0, 60),
        HistInfo("flavors", "flavors", 4, 0, 4, {"eee", "ee#mu", "e#mu#mu", "#mu#mu#mu"} )
    };

   	const unsigned nDist = histInfo.size(); //number of distributions to plot
    const unsigned nCr = 3;                 //Several categories enriched in different processes
    const std::string crNames[nCr] = {"WZ", "Xgamma", "ZZ"};

    //initialize vector holding all histograms
    std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > hists(nCr);
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            hists[cr].push_back(std::vector < std::shared_ptr< TH1D > >() );
            for(size_t sam = 0; sam < samples.size() + 2; ++sam){
                if(sam < samples.size()){
                    hists[cr][dist].push_back(histInfo[dist].makeHist(crNames[cr] + samples[sam].getUniqueName()) );
                } else {
                    if( sam == samples.size() ){
                        //extra histogram for nonprompt prediction
                        hists[cr][dist].push_back( histInfo[dist].makeHist(crNames[cr] + "nonprompt_2016") ); 
                    } else {
                        hists[cr][dist].push_back( histInfo[dist].makeHist(crNames[cr] + "nonprompt_2017") );
                    }
                }
            }
        }
    }

    const std::vector< std::string > uncNames = {"JEC_2016", "JEC_2017", "uncl", "scale", "pileup", "bTag_udsg_2016", "bTag_udsg_2017", "bTag_bc_2016", "bTag_bc_2017", "prefiring", "WZ_extrapolation",
        "lepton_reco", "muon_id_stat_2016", "muon_id_stat_2017", "electron_id_stat_2016", "electron_id_stat_2017", "lepton_id_syst", "pdf", "scaleXsec", "pdfXsec", "lumi_2016", "lumi_2017"};

    std::map < std::string, std::vector< std::vector< std::vector< std::shared_ptr< TH1D > > > > > uncHistMapDown;
    std::map < std::string, std::vector< std::vector< std::vector< std::shared_ptr< TH1D > > > > > uncHistMapUp;
    for( auto& key : uncNames ){
        uncHistMapDown[key] = std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > >(nCr);
        uncHistMapUp[key] = std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > >(nCr);
        for(unsigned cr = 0; cr < nCr; ++cr){
            uncHistMapUp[key][cr] = std::vector < std::vector< std::shared_ptr< TH1D > > >( nDist );
            uncHistMapDown[key][cr] = std::vector < std::vector< std::shared_ptr< TH1D > > >( nDist );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[key][cr][dist] = std::vector< std::shared_ptr< TH1D > >(samples.size() + 2);
                uncHistMapUp[key][cr][dist] = std::vector< std::shared_ptr< TH1D > >(samples.size() + 2);
                for(size_t sam = 0; sam < samples.size() + 2; ++sam){
                    std::string sampleName;
                    if(sam < samples.size() ){
                        sampleName = samples[sam].getUniqueName();
                    } else {
                        //the nonprompt prediction is also affected by all nuisances through the prompt MC subtraction
                        if( sam == samples.size() ){
                            sampleName = "nonprompt_2016";
                        } else {
                            sampleName = "nonprompt_2017";
                        }
                    }
                    uncHistMapDown[key][cr][dist][sam] = histInfo[dist].makeHist(crNames[cr] + sampleName + key + "Down");
                    uncHistMapUp[key][cr][dist][sam] = histInfo[dist].makeHist(crNames[cr] + sampleName + key + "Up");
                }
            }
        }
    }
    
    std::vector< std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > > > pdfUncHists(100);
    for(unsigned pdf = 0; pdf < 100; ++pdf){
        pdfUncHists[pdf] = std::vector < std::vector< std::vector< std::shared_ptr< TH1D > > > >(nCr);
        for(unsigned cr = 0; cr < nCr; ++cr){
            pdfUncHists[pdf][cr] = std::vector < std::vector< std::shared_ptr< TH1D > > >( nDist );
            for(unsigned dist = 0; dist < nDist; ++dist){
                pdfUncHists[pdf][cr][dist] = std::vector< std::shared_ptr< TH1D > >( samples.size() + 2);
                for(size_t sam = 0; sam < samples.size() + 2; ++sam){
                    std::string sampleName;
                    if(sam < samples.size() ){
                        sampleName = samples[sam].getUniqueName();
                    } else {
                        if( sam == samples.size() ){
                            sampleName = "nonprompt_2016";
                        } else {
                            sampleName = "nonprompt_2017";
                        }
                    } 
                    pdfUncHists[pdf][cr][dist][sam] = histInfo[dist].makeHist(crNames[cr] + sampleName + "pdf" + std::to_string(pdf) );
                }
            }
        }
    }

    //tweakable options
    const TString extra = ""; //for plot names

    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){

        initSample();          //2 = combined luminosity
        std::cout<<"Entries in "<< currentSample.getFileName() << " " << nEntries << std::endl;

        double progress = 0; 	//for printing progress bar
        for(long unsigned it = 0; it < nEntries; ++it){
            //print progress bar	
            if(it%100 == 0 && it != 0){
                progress += (double) (100./nEntries);
                analysisTools::printProgress(progress);
            } else if(it == nEntries -1){
                progress = 1.;
                analysisTools::printProgress(progress);
            }

            GetEntry(it);

			//apply triggers and MET filters
            if( !passTriggerCocktail() ) continue;
            if( !passMETFilters() ) continue;

            //vector containing good lepton indices
            std::vector<unsigned> ind;

            //select leptons
            const unsigned lCount = selectLepConeCorr(ind);
            if( !(lCount == 3 || lCount == 4) ) continue;

            //require pt cuts (25, 15, 10) to be passed
            if(!passPtCuts(ind)) continue;

            //require presence of OSSF pair
            if(trilep::flavorChargeComb(ind, _lFlavor, _lCharge, lCount) != 0) continue;

            //explicitly require selected leptons in MC to be prompt, if one of them is not prompt, reject the event
            if( isMC() && !isSMSignal() ){
                if( !promptLeptons() ) continue;
            }

            //index to use when filling histogram 
            unsigned fillIndex = sam;
            bool passTightCut = (tightLepCount(ind, lCount) == lCount);
            if(!passTightCut){
                //fill last histogram (nonprompt)
                fillIndex = samples.size();

                //apply fake-rate weights 
                weight *= fakeRateWeight();
            }

            //remove overlap between samples
            if(photonOverlap(currentSample, false)) continue;

            //make lorentzvectors for leptons
            TLorentzVector lepV[lCount];
            TLorentzVector lepSyst(0,0,0,0);    //vector sum of all leptons
            for(unsigned l = 0; l < lCount; ++l){
                lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
                lepSyst += lepV[l];
            }
            double m3l = lepSyst.M();

            //require best Z mass to be onZ
            std::pair<unsigned, unsigned> bestZ = trilep::bestZ(lepV, ind, _lFlavor, _lCharge, lCount);
            double mll = (lepV[bestZ.first] + lepV[bestZ.second]).M();

            //control region categorization
            unsigned controlRegion = tzq::controlRegion(lCount, mll, m3l);

            //reject events falling out of the control regions
            if( !tzq::isControlRegion(controlRegion) ) continue;

			//control region specific event selection
            if( tzq::isWZControlRegion(controlRegion) ){

            } else if ( tzq::isXgammaControlRegion(controlRegion) ){

                //WARNING: remove this cut here when moving to full systematics 

            } else if ( tzq::isZZControlRegion(controlRegion) ){

                //find leptons not making best Z pair
                std::pair<unsigned, unsigned> secondZ = {99, 99};
                for(unsigned l = 0; l < lCount; ++l){
                    if( (l != bestZ.first) && (l != bestZ.second) ){
                        if(secondZ.first == 99){
                            secondZ.first = l;
                        } else {
                            secondZ.second = l;
                        }
                    }
                }

                //check presence of second Z in the event
                if( _lCharge[ind[secondZ.first]] == _lCharge[ind[secondZ.second]]) continue;
                if( _lFlavor[ind[secondZ.first]] != _lFlavor[ind[secondZ.second]]) continue;

                double mllSecond = (lepV[secondZ.first] + lepV[secondZ.second]).M();
                if( fabs(mllSecond - 91.1876) > 15) continue;

            } else {
                std::cerr << "Error: controlRegion number does not match any known control region." << std::endl;
            }

            //compute nominal values for all search variables
            setSearchVariablestZq("nominal", ind, bestZ, false);

            //event reweighting
            if( isMC() ){
                weight *= sfWeight();
                if( weight == 0.){
                    continue;
                }
            }

            //keys to use for JEC variation (make sure to only change JEC for specific year)
            std::string JECKeyChanged = std::string("JEC_") + (is2016() ? "2016" : "2017");
            std::string JECKeyUnchanged= std::string("JEC_") + (is2016() ? "2017" : "2016");

            //compute nominal bdt value
			if( !tzq::isWZControlRegion(controlRegion) || (_met > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    0.5,
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _met,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };
                for(unsigned dist = 0; dist < nDist; ++dist){
                    hists[controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }

                //in case of data fakes fill all uncertainties for nonprompt with nominal values
                if( isData() && !passTightCut){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        for(auto& key: uncNames){
                            uncHistMapDown[key][controlRegion][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                            uncHistMapUp[key][controlRegion][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                        }
                        for(unsigned pdf = 0; pdf < 100; ++pdf){
                             pdfUncHists[pdf][controlRegion][dist][fillIndex]->Fill( std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                        }
                    }
                } else if( isMC() ){
                    for(unsigned dist = 0; dist < nDist; ++dist){
                        uncHistMapDown[JECKeyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                        uncHistMapUp[JECKeyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                    }
                }
            }

            //no nuisances for data
            if( isData() ) continue;

            //vary JEC Down
			setSearchVariablestZq("JECDown", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metJECDown > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    0.5,
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _metJECDown,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };
 
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown[JECKeyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary JEC up
			setSearchVariablestZq("JECUp", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metJECUp > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    0.5,
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _metJECUp,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp[JECKeyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

            //vary unclustered down
			setSearchVariablestZq("unclDown", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metUnclDown > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    0.5,
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _metUnclDown,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown["uncl"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

			//vary unclustered up
            setSearchVariablestZq("unclUp", ind, bestZ, false);
            if( !tzq::isWZControlRegion(controlRegion) || (_metUnclUp > 50 &&  bdtVariableMap["numberOfbJets"] == 0 ) ){
                double fill[nDist] = {
                    0.5,
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _metUnclUp,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapUp["uncl"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
                }
            }

			//reset nominal values 
            setSearchVariablestZq("nominal", ind, bestZ);

            //now nominal cuts can be safely used
            if( tzq::isWZControlRegion(controlRegion) && (_met < 50 ||  bdtVariableMap["numberOfbJets"] != 0 ) ) continue;
            
            double fill[nDist] = {
                    0.5,
                    bdtVariableMap["etaRecoilingJet"],
                    bdtVariableMap["maxMjj"],
                    bdtVariableMap["asymmetryWlep"],
                    bdtVariableMap["highestDeepCSV"],
                    bdtVariableMap["ltmet"],
                    bdtVariableMap["maxDeltaPhijj"],
                    bdtVariableMap["mTW"],
                    bdtVariableMap["topMass"],
                    bdtVariableMap["pTMaxjj"],
                    bdtVariableMap["minDeltaPhilb"],
                    bdtVariableMap["maxDeltaPhill"],
                    bdtVariableMap["ht"],
                    bdtVariableMap["deltaRTaggedbJetRecoilingJet"],
                    bdtVariableMap["deltaRWLeptonTaggedbJet"],
                    bdtVariableMap["m3l"],
                    bdtVariableMap["etaMostForward"],
                    bdtVariableMap["numberOfJets"],
                    bdtVariableMap["numberOfbJets"],
                    _met,
                    _lPt[ind[0]],
                    _lPt[ind[1]],
                    _lPt[ind[2]],
                    _lEta[ind[0]],
                    _lEta[ind[1]],
                    _lEta[ind[2]],
                    (double) _nVertex,
                    (double) ( tzq::isZZControlRegion(controlRegion) ? trilep::flavorCompositionFourlepton(ind, _lFlavor, lCount) :  trilep::flavorComposition(ind, _lFlavor, lCount)  ) + 0.5
                    };

            //set pdf and scale weight for GluGluToContinToZZ samples which do not include lhe weights
            if( currentSample.getFileName().find("GluGluToContinToZZ") != std::string::npos ){
                for(unsigned lhe = 0; lhe < 110; ++lhe){
                    _lheWeight[lhe] = 1.;
                }
            }
 
	        //vary scale down
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["scale"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[8]);
            }

            //vary scale up
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["scale"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[4]);
            }

            //vary pu down
            double puDownWeight = puWeight(1)/puWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["pileup"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puDownWeight);
            }

            //vary pu up            
            double puUpWeight = puWeight(2)/puWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["pileup"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*puUpWeight);
            }

            //vary b-tag down for udsg
            std::string bTag_udsg_keyChanged = std::string("bTag_udsg_") + ( is2016() ? "2016" : "2017" );
            std::string bTag_udsg_keyUnchanged = std::string("bTag_udsg_") + ( is2016() ? "2017" : "2016" );
            double bTag_udsg_downWeight = bTagWeight_udsg(1)/bTagWeight_udsg(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[bTag_udsg_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_udsg_downWeight);
                uncHistMapDown[bTag_udsg_keyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary b-tag up for udsg
            double bTag_udsg_upWeight = bTagWeight_udsg(2)/bTagWeight_udsg(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp[bTag_udsg_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_udsg_upWeight);
                uncHistMapUp[bTag_udsg_keyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary b-tag down for b and c (correlated)
            std::string bTag_bc_keyChanged = std::string("bTag_bc_") + ( is2016() ? "2016" : "2017" );
            std::string bTag_bc_keyUnchanged = std::string("bTag_bc_") + ( is2016() ? "2017" : "2016" );
            double bTag_bc_downWeight = bTagWeight_c(1)*bTagWeight_b(1)/ (bTagWeight_c(0)*bTagWeight_b(0) );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[bTag_bc_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_bc_downWeight);
                uncHistMapDown[bTag_bc_keyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary b-tag up for b and c (correlated)
            double bTag_bc_upWeight = bTagWeight_c(2)*bTagWeight_b(2)/ (bTagWeight_c(0)*bTagWeight_b(0) );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp[bTag_bc_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*bTag_bc_upWeight);
                uncHistMapUp[bTag_bc_keyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary jet prefiring probabilities down
            double prefiringDownWeight = jetPrefiringWeight(1)/jetPrefiringWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["prefiring"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*prefiringDownWeight );
            }

            //vary jet prefiring probabilities up
            double prefiringUpWeight = jetPrefiringWeight(2)/jetPrefiringWeight(0);
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["prefiring"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*prefiringUpWeight );
            }

            //extrapolation uncertainty for WZ from CR to SR
            double WZExtrapolationUnc;
            if( (currentSample.getProcessName() == "WZ") && ( bdtVariableMap["numberOfbJets"] > 0 ) ){
                WZExtrapolationUnc = 0.08;
            } else {
                WZExtrapolationUnc = 0.;
            }
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["WZ_extrapolation"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( 1. - WZExtrapolationUnc) );
                uncHistMapUp["WZ_extrapolation"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*( 1. + WZExtrapolationUnc) );
            }

            //vary lepton reco SF down
            double leptonRecoDownWeight = leptonWeight("recoDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["lepton_reco"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonRecoDownWeight );
            }

            //vary lepton reco SF up
            double leptonRecoUpWeight = leptonWeight("recoUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["lepton_reco"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonRecoUpWeight);
            }

            //vary muon stat down
            std::string muon_id_stat_keyChanged = std::string("muon_id_stat_") + ( is2016() ? "2016" : "2017" );
            std::string muon_id_stat_keyUnchanged = std::string("muon_id_stat_") + ( is2016() ? "2017" : "2016" );
            double muonStatDownWeight = leptonWeight("muon_idStatDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[muon_id_stat_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*muonStatDownWeight);
                uncHistMapDown[muon_id_stat_keyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary muon stat up
            double muonStatUpWeight = leptonWeight("muon_idStatUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp[muon_id_stat_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*muonStatUpWeight);
                uncHistMapUp[muon_id_stat_keyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary electron stat down
            std::string electron_id_stat_keyChanged = std::string("electron_id_stat_") + ( is2016() ? "2016" : "2017" );
            std::string electron_id_stat_keyUnchanged = std::string("electron_id_stat_") + ( is2016() ? "2017" : "2016" );
            double electronStatDownWeight = leptonWeight("electron_idStatDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[electron_id_stat_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*electronStatDownWeight);
                uncHistMapDown[electron_id_stat_keyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary electron stat up
            double electronStatUpWeight = leptonWeight("electron_idStatUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp[electron_id_stat_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*electronStatUpWeight);
                uncHistMapUp[electron_id_stat_keyUnchanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight);
            }

            //vary lepton syst down
            double leptonIdSystDownWeight = leptonWeight("idSystDown")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown["lepton_id_syst"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonIdSystDownWeight );
            }
            
            //vary lepton syst up
            double leptonIdSystUpWeight = leptonWeight("idSystUp")/leptonWeight("");
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapUp["lepton_id_syst"][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*leptonIdSystUpWeight);
            }

            //lumi uncertainty, fill it here to properly address the fact that it is uncorrelated between 2016 and 2017
            double lumiUnc = ( is2016() ? 0.025 : 0.023 );
            std::string lumi_keyChanged = std::string("lumi_") + (is2016() ? "2016" : "2017" );
            std::string lumi_keyUnChanged = std::string("lumi_") + (is2016() ? "2017" : "2016" );
            for(unsigned dist = 0; dist < nDist; ++dist){
                uncHistMapDown[lumi_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*(1. - lumiUnc) );
                uncHistMapUp[lumi_keyChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*(1. + lumiUnc) );
                uncHistMapDown[lumi_keyUnChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight );
                uncHistMapUp[lumi_keyUnChanged][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight );
            } 

            //100 pdf variations
            for(unsigned pdf = 0; pdf < 100; ++pdf){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    pdfUncHists[pdf][controlRegion][dist][fillIndex]->Fill(std::min(fill[dist], histInfo[dist].maxBinCenter() ), weight*_lheWeight[pdf + 9]);
                }
            }
        }
        //set histograms to 0 if negative
        for(unsigned cr = 0; cr < nCr; ++cr){
            for(unsigned dist = 0; dist < nDist; ++dist){
                analysisTools::setNegativeBinsToZero( hists[cr][dist][sam].get() );
                for(auto & key : uncNames){
                    analysisTools::setNegativeBinsToZero(uncHistMapDown[key][cr][dist][sam].get() );
                    analysisTools::setNegativeBinsToZero(uncHistMapUp[key][cr][dist][sam].get() );
                } 
                for(unsigned pdf = 0; pdf < 100; ++pdf){    
                    analysisTools::setNegativeBinsToZero(pdfUncHists[pdf][cr][dist][sam].get() );
                }
            }	
        }
    }

    std::cout << "crash 1" << std::endl;
    //set nonprompt bins to 0 if negative
    for( unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            
            //loop here to set both 2016 and 2017 nonprompt to zero
            for(unsigned i = 0; i < 2; ++i){
                unsigned nonpromptIndex = samples.size() + i;
                analysisTools::setNegativeBinsToZero( hists[cr][dist][nonpromptIndex].get() );
                for(auto & key : uncNames){
                    analysisTools::setNegativeBinsToZero(uncHistMapDown[key][cr][dist][nonpromptIndex].get() );
                    analysisTools::setNegativeBinsToZero(uncHistMapUp[key][cr][dist][nonpromptIndex].get() );
                } 
                for(unsigned pdf = 0; pdf < 100; ++pdf){    
                    analysisTools::setNegativeBinsToZero(pdfUncHists[pdf][cr][dist][nonpromptIndex].get() );
                }
            }
        }
    }

    std::cout << "crash 2" << std::endl;
    //compute lhe cross section ratio for every sample and variation
    std::vector< std::vector< double> > crossSectionRatio(samples.size(), std::vector<double>(110, 1.) );
    for(unsigned sam = 1; sam < samples.size(); ++sam){

        if(samples[sam].isData() ) continue;
        
        //lhe weights are not available for GluGlu->ZZ
        if( samples[sam].getFileName().find("GluGluToContinToZZ") != std::string::npos ) continue;

        std::shared_ptr<TFile> sample = samples[sam].getFile();
        
        //extract histogram containing nominal sum of weights
        std::shared_ptr<TH1D> hCounter = std::shared_ptr<TH1D>( (TH1D*) sample->Get("blackJackAndHookers/hCounter") );
        double sumOfWeights = hCounter->GetBinContent(1);

        //extract histogram containing sum of weights for all possible pdf variations
        std::shared_ptr<TH1D> lheCounter = std::shared_ptr<TH1D>( (TH1D*) sample->Get("blackJackAndHookers/lheCounter"));
        for(unsigned lhe = 0; lhe < 110; ++lhe){
            double variedSumOfWeights = lheCounter->GetBinContent(lhe + 1);
            crossSectionRatio[sam][lhe] = sumOfWeights/( variedSumOfWeights );
        }
    }

    std::cout << "crash 2" << std::endl;
    //compute final pdf uncertainties
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            for(unsigned sam = 0; sam < samples.size() + 2; ++sam){

                if(sam < samples.size() ){
                    if( samples[sam].isData() ) continue; 
                }

                for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["pdf"][cr][dist][sam]->GetNbinsX() + 1; ++bin){
                    double pdfVarRms = 0.;
                    for(unsigned pdf = 0; pdf < 100; ++pdf){

                        double variedBin = pdfUncHists[pdf][cr][dist][sam]->GetBinContent(bin);
                        if( !(sam >= samples.size() ) ){
                            variedBin *= crossSectionRatio[sam][pdf + 9];
                        }
                        double diff = (  variedBin - hists[cr][dist][sam]->GetBinContent(bin) );
                        pdfVarRms += diff * diff;
                    }
                    pdfVarRms = sqrt( 0.01 * pdfVarRms );
                    uncHistMapDown["pdf"][cr][dist][sam]->SetBinContent( bin, hists[cr][dist][sam]->GetBinContent(bin) - pdfVarRms);
                    uncHistMapUp["pdf"][cr][dist][sam]->SetBinContent( bin, hists[cr][dist][sam]->GetBinContent(bin) + pdfVarRms);
                }
            }
        }
    }

    std::cout << "crash 3" << std::endl;
    //add pdf and scale variation uncertainties of the cross-section
    std::vector< double > scaleXsecUncDown = {0.};  //first dummy entry for data
    std::vector< double > scaleXsecUncUp = {0.};
    std::vector< double > pdfXsecUncDown = {0.};
    std::vector< double > pdfXsecUncUp = {0.};

    for( unsigned sam = 1; sam < samples.size(); ++sam){ 

        if( samples[sam].isData() ) continue;

        //scale effect on xSec :
        double scaleDownUnc = (1./crossSectionRatio[sam][8] - 1.); //Warning: nominal sum of weights has to be in the denominator!
        scaleXsecUncDown.push_back( scaleDownUnc );

        double scaleUpUnc = (1./crossSectionRatio[sam][4]- 1.); //Warning: nominal sum of weights has to be in the denominator!
        scaleXsecUncUp.push_back( scaleUpUnc );

        //filter out cross section effects from scale shape uncertainty
        if( samples[sam].getFileName().find("GluGluToContinToZZ") == std::string::npos){
            for(unsigned cr = 0; cr < nCr; ++cr){
                for(unsigned dist = 0; dist < nDist; ++dist){
                    uncHistMapDown["scale"][cr][dist][sam]->Scale(crossSectionRatio[sam][8]);
                    uncHistMapUp["scale"][cr][dist][sam]->Scale(crossSectionRatio[sam][4]);
                }
            }
        }

        //pdf effect on xSec:
        double rms = 0.;
        for(unsigned pdf = 0; pdf < 100; ++pdf){
            double pdfUnc = (1./crossSectionRatio[sam][pdf + 9] - 1.);
            rms += pdfUnc*pdfUnc;
        }
        rms = sqrt( 0.01 * rms);

        //catch CRAZY pdf uncertainty of TTZ-M1to10 in 2016 and set to zero
        if( samples[sam].getFileName() == "TTZToLL_M-1to10_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_Summer16.root"){
            rms = 0.;
        }

        pdfXsecUncDown.push_back( -rms );
        pdfXsecUncUp.push_back( rms );

        //set pdf and scale uncertainties to zero for GluGluToContinToZZ samples which do not include the necessary weights
        if( samples[sam].getFileName().find("GluGluToContinToZZ") != std::string::npos){
            scaleXsecUncDown[sam] = 0.;
            scaleXsecUncUp[sam] = 0.;
            pdfXsecUncDown[sam] = 0.;
            pdfXsecUncUp[sam] = 0.;
        }
    }

    std::cout << "crash 4" << std::endl;
    //add pdf and scale effects to shape uncertainties to automatically take into account the fractional effects from every sample that is merged
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            for(unsigned sam = 0; sam < samples.size() + 2; ++sam){
                if(sam < samples.size() ){
                    for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["scaleXsec"][cr][dist][sam]->GetNbinsX() + 1; ++bin){
                        uncHistMapDown["scaleXsec"][cr][dist][sam]->SetBinContent(bin, (1 + scaleXsecUncDown[sam])*hists[cr][dist][sam]->GetBinContent(bin) );
                        uncHistMapUp["scaleXsec"][cr][dist][sam]->SetBinContent(bin, (1 + scaleXsecUncUp[sam])*hists[cr][dist][sam]->GetBinContent(bin) );
                        uncHistMapDown["pdfXsec"][cr][dist][sam]->SetBinContent(bin, (1 + pdfXsecUncDown[sam])*hists[cr][dist][sam]->GetBinContent(bin) );
                        uncHistMapUp["pdfXsec"][cr][dist][sam]->SetBinContent(bin, (1 + pdfXsecUncUp[sam])*hists[cr][dist][sam]->GetBinContent(bin) );
                    }
                //WARNING: CURRENTLY THE SCALE AND PDF XSEC UNCERTAINTIES ON DATA-DRIVEN FAKES ARE SET TO ZERO
                } else {
                    for(unsigned bin = 1; bin < (unsigned) uncHistMapDown["scaleXsec"][cr][dist][sam]->GetNbinsX() + 1; ++bin){
                        uncHistMapDown["scaleXsec"][cr][dist][sam]->SetBinContent(bin, hists[cr][dist][sam]->GetBinContent(bin) );
                        uncHistMapUp["scaleXsec"][cr][dist][sam]->SetBinContent(bin, hists[cr][dist][sam]->GetBinContent(bin) );
                        uncHistMapDown["pdfXsec"][cr][dist][sam]->SetBinContent(bin, hists[cr][dist][sam]->GetBinContent(bin) );
                        uncHistMapUp["pdfXsec"][cr][dist][sam]->SetBinContent(bin, hists[cr][dist][sam]->GetBinContent(bin) );
                    }
                }
            }
        }
    }

    std::cout << "crash 5" << std::endl;
    //merge histograms with the same physical background
    std::vector<std::string> proc = {"Data", "tZq", "WZ", "Multiboson", "t#bar{t} + Z", "t#bar{t}/t + X", "X + #gamma", "ZZ/H", "Nonprompt e/#mu"};
    std::vector< std::vector< std::vector< TH1D* > > > mergedHists(nCr);
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            mergedHists[cr].push_back(std::vector<TH1D*>(proc.size() ) );

            //cut off loop before nonprompt contribution
            for(size_t m = 0, sam = 0; m < proc.size() - 1; ++m){
                mergedHists[cr][dist][m] = (TH1D*) hists[cr][dist][sam]->Clone();
            
                //look for each sample with the same name 
                std::string currentName = samples[sam].getProcessName();

                //check how far to jump to next process 
                int nextProcessIndex = -1;
                for(size_t match = sam + 1; match < samples.size(); ++match){
                    std::string nameToCompare = samples[match].getProcessName();
                    if(currentName == nameToCompare){
                        mergedHists[cr][dist][m]->Add( hists[cr][dist][match].get() );
                    } else if( nextProcessIndex == -1 ){
                        nextProcessIndex = match;
                    }
                }
                sam = nextProcessIndex;
            }

            //add nonprompt histogram for 2016
            mergedHists[cr][dist][proc.size() - 1] = (TH1D*) hists[cr][dist][samples.size()]->Clone();

            //add nonprompt histogram for 2017
            mergedHists[cr][dist][proc.size() - 1]->Add( hists[cr][dist][samples.size() + 1].get() );
        }
    }

    std::cout << "crash 6" << std::endl;

    //merging for uncertainties
    std::map< std::string, std::vector< std::vector< std::vector< TH1D* > > > > mergedUncMapDown;
    std::map< std::string, std::vector< std::vector< std::vector< TH1D* > > > > mergedUncMapUp;
    for( auto& key : uncNames ){
        mergedUncMapDown[key] = std::vector< std::vector < std::vector < TH1D* > > >(nCr);
        mergedUncMapUp[key] = std::vector< std::vector < std::vector < TH1D* > > >(nCr);
        for(unsigned cr = 0; cr < nCr; ++cr){
            for(unsigned dist = 0; dist < nDist; ++dist){
                mergedUncMapDown[key][cr].push_back(std::vector<TH1D*>(proc.size() ) );
                mergedUncMapUp[key][cr].push_back(std::vector<TH1D*>(proc.size() ) );

                //cut off loop before nonprompt contribution
                for(size_t m = 0, sam = 0; m < proc.size() - 1; ++m){
                    mergedUncMapDown[key][cr][dist][m] = (TH1D*) uncHistMapDown[key][cr][dist][sam]->Clone();
                    mergedUncMapUp[key][cr][dist][m] = (TH1D*) uncHistMapUp[key][cr][dist][sam]->Clone();

                    //look for each sample with the same name 
                    std::string currentName = samples[sam].getProcessName();

                    //check how far to jump to next process 
                    int nextProcessIndex = -1;
                    for(size_t match = sam + 1; match < samples.size(); ++match){
                        std::string nameToCompare = samples[match].getProcessName();
                        if(currentName == nameToCompare){
                            mergedUncMapDown[key][cr][dist][m]->Add( uncHistMapDown[key][cr][dist][match].get() );
                            mergedUncMapUp[key][cr][dist][m]->Add( uncHistMapUp[key][cr][dist][match].get() );
                        } else if( nextProcessIndex == -1 ){
                            nextProcessIndex = match;
                        }
                    }
                    sam = nextProcessIndex;
                }

                //add nonprompt histograms for 2016
                mergedUncMapDown[key][cr][dist][proc.size() - 1] = (TH1D*) uncHistMapDown[key][cr][dist][samples.size()]->Clone();
                mergedUncMapUp[key][cr][dist][proc.size() - 1] = (TH1D*) uncHistMapUp[key][cr][dist][samples.size()]->Clone();

                //add nonprompt histograms for 2017
                mergedUncMapDown[key][cr][dist][proc.size() - 1]->Add( uncHistMapDown[key][cr][dist][samples.size() + 1].get() );
                mergedUncMapUp[key][cr][dist][proc.size() - 1]->Add( uncHistMapUp[key][cr][dist][samples.size() + 1].get() );
            }
        }
    } 

    std::cout << "crash 7" << std::endl;

    const std::vector< std::string > uncorrelatedBetweenProcesses = {"scale", "pdf", "scaleXsec", "pdfXsec"};

    //make final uncertainty histogram for plots 
    std::vector<double> flatUnc = {1.02}; //trigger
    std::map< std::string, double > backgroundSpecificUnc =        //map of background specific nuisances that can be indexed with the name of the process 
        {
            {"Nonprompt e/#mu", 1.3},
            {"WZ", 1.1},
            {"X + #gamma", 1.1},
            {"ZZ/H", 1.1},
            {"t#bar{t} + Z", 1.15} 
        };

    std::vector< std::string > ignoreTheoryUncInPlot = {"tZq", "WZ", "X + #gamma", "ZZ/H", "t#bar{t} + Z"};
    
    std::vector< std::vector< TH1D* > > totalSystUnc(nCr); //copy pointers to fix dimensionality of vector
    for(unsigned cr = 0; cr < nCr; ++cr){
        totalSystUnc[cr] = std::vector< TH1D* >(nDist);
        for(unsigned dist = 0; dist < nDist; ++dist){
            totalSystUnc[cr][dist] = (TH1D*) mergedHists[cr][dist][0]->Clone();

            for(unsigned bin = 1; bin < (unsigned) totalSystUnc[cr][dist]->GetNbinsX() + 1; ++bin){
                double binUnc = 0;

                //add all shape uncertainties 
                for(auto& key: uncNames ){
                    
                    bool nuisanceIsUnCorrelated = ( std::find( uncorrelatedBetweenProcesses.cbegin(), uncorrelatedBetweenProcesses.cend(), key ) != uncorrelatedBetweenProcesses.cend() );
                    
					double var = 0.;

					//for the correlated case
                    double varDown = 0.;
                    double varUp = 0.;

                    //linearly add the variations for each process 
                    for(unsigned p = 1; p < proc.size(); ++p){

						//ignore theoretical uncertainties on the normalization of certain processes 
                		if( key.find("Xsec") != std::string::npos ){
                    		bool processWithoutTheoryUnc =  ( std::find( ignoreTheoryUncInPlot.cbegin(), ignoreTheoryUncInPlot.cend(), proc[p] ) != ignoreTheoryUncInPlot.cend() );
                    	    if( processWithoutTheoryUnc){
                    	    	continue;
                    	    }
						}

                        double nominalContent = mergedHists[cr][dist][p]->GetBinContent(bin);
                        double downVariedContent = mergedUncMapDown[key][cr][dist][p]->GetBinContent(bin);
                        double upVariedContent = mergedUncMapUp[key][cr][dist][p]->GetBinContent(bin);
                        double down = fabs(downVariedContent - nominalContent);
                        double up = fabs(upVariedContent - nominalContent);
                    
						//uncorrelated case : 
						if( nuisanceIsUnCorrelated ){
							double variation = std::max(down, up);
							var += variation*variation;

						//correlated case : 	
						} else {
                        	varDown += down;
                        	varUp += up;
						}
                    }                    

                    //correlated case : 
					if( !nuisanceIsUnCorrelated ){
                    	var = std::max( varDown, varUp );
						var = var*var;
					}

                    //add (already quadratic) uncertainties 
                    binUnc += var;
                }

                //add general flat uncertainties (considered correlated among all processes)
                for( double unc : flatUnc ){
					double var = 0;
					for(unsigned p = 1; p < proc.size(); ++p){
						if( proc[p] == "Nonprompt e/#mu" ){
							continue;
						}
						double binContent = mergedHists[cr][dist][p]->GetBinContent(bin);
						double variation = binContent*(unc - 1.);
						var += variation;
					}
					binUnc += var*var;	
				}
                
                //add background specific uncertainties (uncorrelated between processes)
				for(auto& uncPair : backgroundSpecificUnc){
                    for(unsigned p = 1; p < proc.size(); ++p){
                        if(proc[p] == uncPair.first){
                            double var = mergedHists[cr][dist][p]->GetBinContent(bin)*(uncPair.second - 1.);
                            binUnc += var*var;
                        }
                    }
				}

				//square root of quadratic sum is total uncertainty
                totalSystUnc[cr][dist]->SetBinContent(bin, sqrt(binUnc) );
            }
        }
    }
    std::cout << "crash 9" << std::endl;

    //plot all distributions
    const bool isSMSignal[(const size_t) proc.size() - 1] = {true, false, false, false, false, false, false};
   	for(unsigned cr = 0; cr < nCr; ++cr){
	    for(unsigned dist = 0; dist < nDist; ++dist){
            plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/combined/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_" + crNames[cr] + "_combined", "tzq", false, false, "77.4 fb^{-1} (13 TeV)", totalSystUnc[cr][dist], isSMSignal);             //linear plots

            plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/combined/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_"  + crNames[cr] + "_combined" + "_log", "tzq", true, false, "77.4 fb^{-1} (13 TeV)", totalSystUnc[cr][dist], isSMSignal);    //log plots
        }
    }
 



    //remerge histograms (separately for 2016 and 2017) for postfit scaling
    std::vector< std::vector< std::vector< TH1D* > > > mergedHists2016(nCr);
    std::vector< std::vector< std::vector< TH1D* > > > mergedHists2017(nCr);

    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            mergedHists2016[cr].push_back(std::vector<TH1D*>(proc.size() ) );

            //cut off loop before nonprompt contribution
            for(size_t m = 0, sam = 0; m < proc.size() - 1; ++m){
                mergedHists2016[cr][dist][m] = (TH1D*) hists[cr][dist][sam]->Clone();

                //look for each sample with the same name 
                std::string currentName = samples[sam].getProcessName();

                //check how far to jump to next process 
                int nextProcessIndex = -1;
                for(size_t match = sam + 1; match < samples2016.size(); ++match){
                    std::string nameToCompare = samples[match].getProcessName();
                    if(currentName == nameToCompare){
                        mergedHists2016[cr][dist][m]->Add( hists[cr][dist][match].get() );
                    } else if( nextProcessIndex == -1 ){
                        nextProcessIndex = match;
                    }
                }
                sam = nextProcessIndex;
            }

            //add 2016 nonprompt histogram
            mergedHists2016[cr][dist][proc.size() - 1] = (TH1D*) hists[cr][dist][samples.size()]->Clone();
        }
    }

    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            mergedHists2017[cr].push_back(std::vector<TH1D*>(proc.size() ) );

            //cut off loop before nonprompt contribution
            for(size_t m = 0, sam = samples2016.size(); m < proc.size() - 1; ++m){
                mergedHists2017[cr][dist][m] = (TH1D*) hists[cr][dist][sam]->Clone();

                //look for each sample with the same name 
                std::string currentName = samples[sam].getProcessName();

                //check how far to jump to next process 
                int nextProcessIndex = -1;
                for(size_t match = sam + 1; match < samples2016.size() + samples2017.size(); ++match){
                    std::string nameToCompare = samples[match].getProcessName();
                    if(currentName == nameToCompare){
                        mergedHists2017[cr][dist][m]->Add( hists[cr][dist][match].get() );
                    } else if( nextProcessIndex == -1 ){
                        nextProcessIndex = match;
                    }
                }
                sam = nextProcessIndex;
            }

            //add 2017 nonprompt histogram
            mergedHists2017[cr][dist][proc.size() - 1] = (TH1D*) hists[cr][dist][samples.size() + 1]->Clone();
        }
    }

    //initialize postFitScaler
    PostFitScaler postFitScaler("total_postFit_yields.txt");
    for(unsigned cr = 0; cr < nCr; ++cr){
        if( cr == 1 ) continue;
        for(unsigned dist = 0; dist < nDist; ++dist){
            for(size_t m = 0; m < proc.size(); ++m){
                if(proc[m] != "Data"){
                    if( mergedHists2016[cr][dist][m]->GetSumOfWeights() > 0){
                        mergedHists2016[cr][dist][m]->Scale( postFitScaler.postFitScaling( mergedHists2016[cr][dist][m]->GetSumOfWeights() ) );
                    }
                    if( mergedHists2017[cr][dist][m]->GetSumOfWeights() > 0){
                        mergedHists2017[cr][dist][m]->Scale( postFitScaler.postFitScaling( mergedHists2017[cr][dist][m]->GetSumOfWeights() ) );
                    }
                }
                delete mergedHists[cr][dist][m];
                mergedHists[cr][dist][m] = (TH1D*) mergedHists2016[cr][dist][m]->Clone();
                mergedHists[cr][dist][m]->Add( mergedHists2017[cr][dist][m] );
            }
        }
    }

    //plot all distributions
    for(unsigned cr = 0; cr < nCr; ++cr){
        for(unsigned dist = 0; dist < nDist; ++dist){
            plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/combined/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_" + crNames[cr] + "_combined_postFit", "tzq", false, false, "77.4 fb^{-1} (13 TeV)", totalSystUnc[cr][dist], isSMSignal);             //linear plots
            
            plotDataVSMC(mergedHists[cr][dist][0], &mergedHists[cr][dist][1], &proc[0], mergedHists[cr][dist].size() - 1, "plots/tZq/combined/controlR/" + crNames[cr] + "/" + histInfo[dist].name() + "_"  + crNames[cr] + "_combined_postFit" + "_log", "tzq", true, false, "77.4 fb^{-1} (13 TeV)", totalSystUnc[cr][dist], isSMSignal);    //log plots
        }
    }
}

int main(){
    treeReader reader;
    reader.Analyze();
    return 0;
}
