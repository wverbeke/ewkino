#include "../interface/Reweighter.h"

//include c++ library classes

//include ROOT classes
#include "TFile.h"
#include "TROOT.h"

Reweighter::Reweighter(){

    //initialize pu weights
    const std::string vars[3] = {"central", "down", "up"};
    for(unsigned v = 0; v < 3; ++v){
        TFile* puFile = TFile::Open( (const TString&) "weights/puw_nTrueInt_Moriond2017_36p5fb_Summer16_" + vars[v] + ".root");
        puWeights[v] = (TH1D*) puFile->Get("puw");
        //make sure histogram does not get deleted when closing file
        puWeights[v]->SetDirectory(gROOT);
        puFile->Close();
    }

    //initialize b-tag weights
    //currently assuming medium WP of deepCSV tagger 
    bTagCalib = new BTagCalibration("deepCsv", "weights/DeepCSV_Moriond17_B_H.csv");
    bTagCalibReader =  new BTagCalibrationReader(BTagEntry::OP_MEDIUM, "central", {"up", "down"});
    bTagCalibReader->load(*bTagCalib, BTagEntry::FLAV_B, "comb");
    bTagCalibReader->load(*bTagCalib, BTagEntry::FLAV_C, "comb");
    bTagCalibReader->load(*bTagCalib, BTagEntry::FLAV_UDSG, "incl");    

    //initialize b-tag efficiencies
    TFile* bTagFile = TFile::Open("weights/bTagEff_deepCSV_medium_cleaned_ewkino.root");
    const std::string quarkFlavors[3] = {"udsg", "charm", "beauty"};
    for(unsigned flav = 0; flav < 3; ++flav){
        bTagEffHist[flav] = (TH1D*) bTagFile->Get( (const TString&) "bTagEff_" + quarkFlavors[flav]);
        bTagEffHist[flav]->SetDirectory(gROOT);
    }
    bTagFile->Close();

    //Read Muon reco SF Weights
    TFile* muonRecoFile = TFile::Open("weights/muonTrackingSF_2016.root");
    muonRecoSF = (TGraph*) muonRecoFile->Get("ratio_eff_eta3_dr030e030_corr");
    //WARNING: not clear how ownership works for TGraph, can not set directory
    //make sure the TGraph is not DELETED when file is closed!
    //muonRecoSF->SetDirectory(gROOT);
    muonRecoFile->Close();

    //Read Electron reco SF Weights
    TFile* electronRecoFile = TFile::Open("weights/electronRecoSF_2016.root");
    electronRecoSF = (TH2D*) electronRecoFile->Get("EGamma_SF2D");
    electronRecoSF->SetDirectory(gROOT);
    electronRecoFile->Close();

    //Read muon id SF weights
    TFile* muonMediumFile = TFile::Open("weights/muonScaleFactors_MediumIDtoReco.root");
    muonMediumSF = (TH2D*) muonMediumFile->Get("SF");
    muonMediumSF->SetDirectory(gROOT);
    muonMediumFile->Close();

    TFile* muonMiniIsoFile = TFile::Open("weights/muonScaleFactors_miniIso0p4toMediumID.root");
    muonMiniIsoSF = (TH2D*) muonMiniIsoFile->Get("SF");
    muonMiniIsoSF->SetDirectory(gROOT);
    muonMiniIsoFile->Close();

    TFile* muonIPFile = TFile::Open("weights/muonScaleFactors_dxy0p05dz0p1toMediumID.root");
    muonIPSF = (TH2D*) muonIPFile->Get("SF");
    muonIPSF->SetDirectory(gROOT);
    muonIPFile->Close();

    TFile* muonSIP3DFile = TFile::Open("weights/muonScaleFactors_sip3d4toMediumID.root");
    muonSIP3DSF = (TH2D*) muonSIP3DFile->Get("SF");
    muonSIP3DSF->SetDirectory(gROOT);
    muonSIP3DFile->Close();

    TFile* muonLeptonMvaFile = TFile::Open("weights/muonScaleFactors_ttHMvaTight_old.root");
    muonLeptonMvaSF = (TH2D*) muonLeptonMvaFile->Get("sf");
    muonLeptonMvaSF->SetDirectory(gROOT);
    muonLeptonMvaFile->Close();

    //read electron id SF weights       
    /*
    TFile* electronIdFile = TFile::Open("weights/electronIDScaleFactors_ttHMvaTight.root");
    electronIdSF = (TH2D*) electronIdFile->Get("GsfElectronToTTZ2017");
    electronIdSF->SetDirectory(gROOT);
    electronIdFile->Close();
    */
    TFile* electronGeneralIdFile = TFile::Open("weights/electronIDScaleFactors.root");
    electronEmuIPMvaLooseSF = (TH2D*) electronGeneralIdFile->Get("GsfElectronToMVAVLooseFOIDEmuTightIP2D");
    electronEmuIPMvaLooseSF->SetDirectory(gROOT);
    electronMiniIsoSF = (TH2D*) electronGeneralIdFile->Get("MVAVLooseElectronToMini4");
    electronMiniIsoSF->SetDirectory(gROOT);
    //WARNING: should this be applied considering we have no conersion veto??
    electronConvVetoMissingHitsSF = (TH2D*) electronGeneralIdFile->Get("MVAVLooseElectronToConvVetoIHit1");
    electronConvVetoMissingHitsSF->SetDirectory(gROOT);

    TFile* frFile = TFile::Open("weights/FR_data_ttH_mva.root");
    const std::string frUnc[3] = {"", "_down", "_up"};
    for(unsigned unc = 0; unc < 3; ++unc){
        frMapEle[unc] = (TH2D*) frFile->Get((const TString&) "FR_mva090_el_data_comb_NC" + frUnc[unc]);
        frMapEle[unc]->SetDirectory(gROOT);
        frMapMu[unc] = (TH2D*) frFile->Get((const TString&) "FR_mva090_mu_data_comb" + frUnc[unc]);
        frMapMu[unc]->SetDirectory(gROOT);
    }
    frFile->Close();
}

Reweighter::~Reweighter(){
    delete bTagCalib;
    delete bTagCalibReader;

    for(unsigned v = 0; v < 3; ++v){
        delete puWeights[v];
    }

    for(unsigned flav = 0; flav < 3; ++flav){
        delete bTagEffHist[flav];
    }

    delete muonRecoSF;
    delete electronRecoSF;
    delete muonMediumSF;
    delete muonMiniIsoSF;
    delete muonIPSF;
    delete muonSIP3DSF;
    delete muonLeptonMvaSF;

    delete electronEmuIPMvaLooseSF;
    delete electronMiniIsoSF;
    delete electronConvVetoMissingHitsSF;

    for(unsigned unc = 0; unc < 3; ++unc){
        delete frMapEle[unc];
        delete frMapMu[unc];
    }
}


double Reweighter::puWeight(const double nTrueInt, const unsigned period, const unsigned unc) const{
    if(unc < 3){
        if(period == 0){
            return puWeights[unc]->GetBinContent(puWeights[unc]->FindBin(std::min(nTrueInt, 49.) ) );
        //!!!! 2017 pileup weights to be added !!!!
        } else {
            return 1.;
        }
    }
    else{
        std::cerr << "Error: invalid pu uncertainty requested: returning weight 1" << std::endl;
        return 1.;
    }
}

double Reweighter::bTagWeight(const unsigned jetFlavor, const double jetPt, const double jetEta, const double jetCSV, const unsigned unc) const{
    static const BTagEntry::JetFlavor flavorEntries[3] = {BTagEntry::FLAV_UDSG, BTagEntry::FLAV_C, BTagEntry::FLAV_B};
    static const std::string uncName[3] = {"central", "down", "up"};
    if(unc < 3){
        return bTagCalibReader->eval_auto_bounds(uncName[unc], flavorEntries[flavorInd(jetFlavor)], jetEta, jetPt, jetCSV);
    } else{
        std::cerr << "Error: invalid b-tag SF uncertainty requested: returning weight 1" << std::endl;
        return 1.;
    }
}

double Reweighter::bTagEff(const unsigned jetFlavor, const double jetPt, const double jetEta) const{
    //!!!! To be split for 2016 and 2017 data !!!!
    return bTagEffHist[flavorInd(jetFlavor)]->GetBinContent(bTagEffHist[flavorInd(jetFlavor)]->FindBin(std::min(jetPt, 599.), std::min(fabs(jetEta), 2.4)) );
}

double Reweighter::muonRecoWeight(const double eta) const{
    //!!!! To be split for 2016 and 2017 data !!!!
    return muonRecoSF->Eval(std::max(-2.4,std::min(eta, 2.4) ) );
}

double Reweighter::electronRecoWeight(const double superClusterEta, const double pt) const{
    //!!!! To be split for 2016 and 2017 data !!!!
    return electronRecoSF->GetBinContent( electronRecoSF->FindBin( std::max(-2.5, std::min(superClusterEta, 2.5)) , std::max(40., std::min(pt, 499.) )  ) );
}

double Reweighter::muonIdWeight(const double pt, const double eta) const{
    //!!!! To be split for 2016 and 2017 data !!!!
    double sf = muonMediumSF->GetBinContent(muonMediumSF->FindBin(std::min(pt, 110.), std::min(fabs(eta), 2.4) ) );
    sf *= muonMiniIsoSF->GetBinContent(muonMiniIsoSF->FindBin(std::min(pt, 110.), std::min(fabs(eta), 2.4) ) );
    sf *= muonIPSF->GetBinContent(muonIPSF->FindBin(std::min(pt, 110.), std::min(fabs(eta), 2.4) ) );
    sf *= muonSIP3DSF->GetBinContent(muonSIP3DSF->FindBin(std::min(pt, 110.), std::min(fabs(eta), 2.4) ) );
    sf *= muonLeptonMvaSF->GetBinContent(muonLeptonMvaSF->FindBin(std::min(pt, 99.), std::min(fabs(eta), 2.4) ) );
    return sf;
}

double Reweighter::electronIdWeight(const double pt, const double eta) const{
    //!!!! To be split for 2016 and 2017 data !!!!
    //return electronIdSF->GetBinContent(electronIdSF->FindBin(std::min(pt, 199.), std::min(fabs(eta), 2.5) ) );

    //Use TTH analysis weights for now:
    double sf = electronEmuIPMvaLooseSF->GetBinContent( electronEmuIPMvaLooseSF->FindBin(std::min(pt, 199.), std::min( fabs(eta), 2.5 ) ) );
    sf *= electronMiniIsoSF->GetBinContent( electronMiniIsoSF->FindBin(std::min(pt, 199.), std::min( fabs(eta), 2.5 ) ) );
    sf *= electronConvVetoMissingHitsSF->GetBinContent( electronConvVetoMissingHitsSF->FindBin(std::min(pt, 199.), std::min( fabs(eta), 2.5 ) ) );
    return sf;
}

double Reweighter::muonFakeRate(const double pt, const double eta, const unsigned unc) const{
    //!!!! To be split for 2016 and 2017 data !!!!
    return frMapMu[unc]->GetBinContent(frMapMu[unc]->FindBin(std::min(pt, 99.), std::min(fabs(eta), 2.4) ) );
}

double Reweighter::electronFakeRate(const double pt, const double eta, const unsigned unc) const{
    //!!!! To be split for 2016 and 2017 data !!!!
    return frMapEle[unc]->GetBinContent(frMapEle[unc]->FindBin(std::min(pt, 99.), std::min(fabs(eta), 2.5) ) );
}
