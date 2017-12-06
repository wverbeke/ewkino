
void extractPuWeight(const std::string& mcFile){
    //read MC pu distribution from given MC sample
    TFile* inputFile = TFile::Open((const TString&) mcFile);
    TH1D* mcPuDist = (TH1D*) mcFile->Get("blackJackAndHookers/nTrue");
    //rebin MC distribution to have same binning as data 
    mcPuDist->Rebin(2);
    //pu weights
    TH1D* puWeights[5]; 
    for(unsigned e = 0; e < 5; ++e) puWeights[e] = (TH1D*) mcPuDist->Clone(); 
    //read data pu distributions 
    const std::string eras[5] = {"B", "C", "D", "E", "F"};
    for(unsigned e = 0; e < 5; ++e){
        TFile* dataFile = TFile::Open( (const TString&) "../weights/pileUpData/dataPuHist_era" + eras[e] + " .root");
        TH1D* dataHist = (TH1D*) dataFile->Get("pileup");
        puWeights[e]->Divide(dataHist);
        dataFile->Close();
    }
    //write pu Weights to file
    TFile outputFile = TFile::Open("../weights/puWeights2017.root");
    for(unsigned e = 0; e < 5; ++e){
        puWeights[e]->Write((const TString&) "puw_Run" + eras[e]);
    }
    outputFile.Close();
}



int main(int argc, char* argv[]){
    extractPuWeight("../../ntuples_ewkino/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8.root");
    return 0;
}
