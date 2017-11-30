#include "../interface/Reweighter.h"

Reweighter::Reweighter(){
    //initialize pu weights
    const std::string vars[3] = {"central", "down", "up"};
    for(unsigned v = 0; v < 3; ++v){
        TFile* puFile = TFile::Open((const TString&) "../weights/puw_nTrueInt_Moriond2017_36p5fb_Summer16_" + vars[v] + ".root");
        puWeights[v] = (TH1D*) puFile->Get("puw");
        puFile->Close();
    }
    //initialize b-tag weights
    //currently assuming medium WP of deepCSV tagger 
    bTagCalib = new BTagCalibration("deepCsv", "../weights/DeepCSV_Moriond17_B_H.csv");
    bTagCalibReader =  new BTagCalibrationReader(BTagEntry::OP_MEDIUM, "central", {"up", "down"});
    bTagCalibReader->load(calib, BTagEntry::FLAV_B, "comb");
    bTagCalibReader->load(calib, BTagEntry::FLAV_C, "comb");
    bTagCalibReader->load(calib, BTagEntry::FLAV_UDSG, "incl");    
    //initialize b-tag efficiencies
    
    

}
