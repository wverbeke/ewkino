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
    TFile* bTagFile = TFile::Open("../weights/bTagEff_deepCSV_medium_cleaned.root");
    const std::string quarkFlavors[3] = {"udsg", "charm", "beauty"};
    for(unsigned flav = 0; flav < nFlav; ++flav){
        bTagEff[flav] = (TH1D*) bTagFile->Get("bTagEff_" + quarkFlavors[flav];
    }
    bTagFile.Close();
}

double Reweighter::puWeight(const double nTrueInt, const unsigned period, const unsigned unc){
    if(unc < 3){
        if(period == 0) return puWeights[unc]->GetBinContent(puWeights[unc]->FindBin(std::min(nTrueInt, (float) 49.) ) );
        else return 1.;                                                 //weights for 2017 data not yet available
    }
    else{
        std::cerr << "wrong pu uncertainty requested: returning weight 1" << std::endl;
        return 1.;
    }
}

double Reweighter::bTagWeight(const unsigned jetFlavor, const double jetPt, const double jetEta, const double jetCSV, const unsigned unc = 0){
    static const BTagEntry::JetFlavor flavorEntries[3] = {BTagEntry::FLAV_UDSG, BTagEntry::FLAV_C, BTagEntry::FLAV_B};
    static const std::string uncName[3] = {"central", "down", "up"};
    const unsigned flav = 0 + (_jetHadronFlavor[j] == 4) + 2*(_jetHadronFlavor[j] == 5);
    return bTagCalibReader.eval_auto_bounds(uncName[unc], flavorEntries[flav], jetEta, jetPt, jetCSV);
}
