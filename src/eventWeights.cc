//include c++ library classes
#include <iostream>

//include other parts of code
#include "../interface/treeReader.h"
#include "../bTagSFCode/BTagCalibrationStandalone.h"


//read all weights from files
void treeReader::setWeights(){
    //pu weights
    const std::string vars[3] = {"central", "down", "up"};
    for(unsigned v = 0; v < 3; ++v){
        TFile* puFile = TFile::Open((const TString&) "../weights/puw_nTrueInt_Moriond2017_36p5fb_Summer16_" + vars[v] + ".root");
        puWeights[v] = (TH1D*) puFile->Get("puw");
        puFile->Close();
    }
}

//return PU weight
double treeReader::puWeight(const unsigned period, const unsigned unc){ //unc = 0: central, = 1: down, = 2: up
    if(unc < 3){
        if(period == 0) return puWeights[unc]->GetBinContent(puWeights[unc]->FindBin(std::min(_nTrueInt, (float) 49.) ) );
        else return 1.;                                                 //weights for 2017 data not yet available
    }
    else{
        std::cerr << "wrong pu uncertainty requested: returning weight 1" << std::endl;
        return 1.;
    }
}
