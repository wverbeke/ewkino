
//include c++ library classes
#include <string>

//include ROOT classes
#include "TFile.h"
#include "TH1D.h"

void extractPuWeight(const std::string& mcFile){
    //read MC pu distribution from given MC sample
    TFile* inputFile = TFile::Open((const TString&) mcFile);
    TH1D* mcPuDist = (TH1D*) inputFile->Get("blackJackAndHookers/nTrue");
    //rebin MC distribution to have same binning as data 
    //mcPuDist->Rebin(2);
    //normalize histogram to unity
    mcPuDist->Scale(1/mcPuDist->GetSumOfWeights());
    //pu weights
    TH1D* puWeights[6]; 
    TFile* dataFile[6];
    //read data pu distributions 
    const std::string eras[6] = {"Inclusive", "B", "C", "D", "E", "F"};
    for(unsigned e = 0; e < 6; ++e){
        dataFile[e] = TFile::Open( (const TString&) "weights/pileUpData/dataPuHist_era" + eras[e] + ".root");
        TH1D* dataHist = (TH1D*) dataFile[e]->Get("pileup");
        //normalize histogram to unity
        puWeights[e] = (TH1D*) dataHist->Clone();
        puWeights[e]->Scale(1/dataHist->GetSumOfWeights());
        puWeights[e]->Divide(mcPuDist);
    }
    //write pu Weights to file
    TFile* outputFile = TFile::Open("weights/puWeights2017.root", "RECREATE");
    for(unsigned e = 0; e < 6; ++e){
        puWeights[e]->Write((const TString&) "puw_Run" + eras[e]);
    }
    outputFile->Close();
    inputFile->Close();
    for(unsigned e = 0; e < 6; ++e) dataFile[e]->Close();
}

int main(int argc, char* argv[]){
    extractPuWeight("../../ntuples_ewkino/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8.root");
    return 0;
}
