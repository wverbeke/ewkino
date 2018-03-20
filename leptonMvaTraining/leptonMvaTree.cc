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
#include <stdlib.h>

//include other parts of the code
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"
#include "../interface/TrainingTree.h"


bool treeReader::lepPassBaseline(const unsigned ind) const{
    //don't consider taus!
    if(_lFlavor[ind] == 2) return false;

    if(fabs(_lEta[ind]) >= (2.5 - 0.1*_lFlavor[ind]) ) return false;
    if(_lPt[ind] < 10) return false;
    if(fabs(_dxy[ind]) >= 0.05) return false;
    if(fabs(_dz[ind]) >= 0.1) return false;
    if(fabs(_3dIPSig[ind]) >= 8) return false;
    if(_miniIso[ind] >= 0.4) return false;
    if(_lFlavor[ind] == 1){
        if(!_lPOGMedium[ind]) return false;
    } else if(_lFlavor[ind] == 0){
        if( !eleIsClean(ind) ) return false;
        if( !_lElectronPassEmu[ind] ) return false;
        if(_lElectronMissingHits[ind] > 1) return false;
    }
    return true;
}

void treeReader::setup(){
    gROOT->SetBatch(kTRUE);
    //read samples and cross sections from txt file
    readSamples("sampleLists/samples_leptonMvaTraining.txt");

    for(auto& sam : samples){
        Analyze(sam);
    }
}

void treeReader::Analyze(const Sample& samp){
    std::map < std::string, float > trainingVariableMap =
        {
            {"pt", 0.},
            {"eta", 0.},
            {"trackMultClosestJet", 0.},
            {"miniIsoCharged", 0.},
            {"miniIsoNeutral", 0.},
            {"pTRel", 0.},
            {"ptRatio", 0.},
            {"csvV2ClosestJet", 0.},
            {"sip3d", 0.},
            {"dxy", 0.},
            {"dz", 0.},
            {"segmentCompatibility", 0.},
            {"electronMva", 0.}
            {"eventWeight", 0.}
        };

    TrainingTree muonTree("leptonMvaTraining/", samp, {{""}}, trainingVariableMap, samp.isSMSignal() );
    TrainingTree electronTree("leptonMvaTraining/", samp, {{""}}, trainingVariableMap, samp.isSMSignal() );


    initSample(samp, 1);  //use 2017 lumi
    for(long unsigned it = 0; it < nEntries; ++it){
        GetEntry(samp, it);
    
        for(unsigned l = 0; l < _nLight; ++l){
            if( lepPassBaseline(l) ){
                bool isPrompt = _lIsPrompt[l] && ( _lMatchPdgId[l] != 22) && (_lProvenance[l] != 1);
                bool nonPrompt = !_lIsPrompt[l];
                
                trainingVariableMap["pt"] = _lPt[l];
                trainingVariableMap["eta"] = fabs(_lEta[l]);
                trainingVariableMap["trackMultClosestJet"] = _selectedTrackMult[l];
                trainingVariableMap["miniIsoCharged"] = _miniIsoCharged[l];
                trainingVariableMap["miniIsoNeutral"] = _miniIso[l] - _miniIsoCharged[l];
                trainingVariableMap["pTRel"] = _ptRel[l];
                trainingVariableMap["ptRatio"] = std::min(_ptRatio[l], 1.5);
                trainingVariableMap["csvV2ClosestJet"] = std::max(_closestJetCsvV2[l], 0.);
                trainingVariableMap["sip3d"] = _3dIPSig[l];
                trainingVariableMap["dxy"] = log( fabs( _dxy[l] ) );
                trainingVariableMap["dz"] = log( fabs( _dz[l] ) );
                trainingVariableMap["segmentCompatibility"] = ( (_lFlavor[l] == 1) ? _lMuonSegComp[l] : 0.);
                trainingVariableMap["electronMva"] = ( (_lFlavor[l] == 0) ? _lElectronMva[l] : 0.); 
                trainingVariableMap["eventWeight"] = ( (_weight > 0) ? 1 : -1);
                
                if(isPrompt && samp.isSMSignal() ){ 
                    if(_lFlavor[l] == 0){
                        electronTree.fill( std::vector<size_t>({0}), trainingVariableMap);    
                    } else if(_lFlavor[l] == 1){
                        muonTree.fill( std::vector<size_t>({0}), trainingVariableMap);
                    }
                } else if(nonPrompt && !samp.isSMSignal() ){
                    if(_lFlavor[l] == 0){
                        electronTree.fill(std::vector<size_t>({0}), trainingVariableMap);    
                    } else if(_lFlavor[l] == 1){
                        muonTree.fill(std::vector<size_t>({0}), trainingVariableMap);
                    }
                }
            }
        }       
    }
}

int main(int argc, char* argv[]){
    treeReader reader;
    reader.setup();
}
