#include "../interface/Reweighter.h"
 
//include c++ library classes

//include ROOT classes
#include "TFile.h"
#include "TROOT.h"

//include other parts of code 
#include "../interface/analysisTools.h"

Reweighter::Reweighter(const std::vector<Sample>& samples, const bool is2016){
	initializeAllWeights(samples, is2016);
}

void Reweighter::initializeAllWeights(const std::vector<Sample>& samples, const bool is2016){

    //initialize pu weights
    initializePuWeights(samples);

    //initialize b-tag weights
    initializeBTagWeights(is2016);

	//initialize electron weights 
	initializeElectronWeights(is2016);

	//initialize muon weights 
	initializeMuonWeights(is2016);

	//initialize fake-rate 

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

void Reweighter::initializePuWeights(const std::vector< Sample >& sampleList){
    
    static const std::string minBiasVariation[3] = {"central", "down", "up"};
    for(auto& sample : sampleList){

        //open root file corresponding to sample
        TFile* puFile = TFile::Open( (const TString&) "weights/pileUpWeights/puWeights_" + sample.getFileName() + ".root");
        
        //extract pu weights 
        for(unsigned var = 0; var < 3; ++var){
            std::string histName = "puw_Run";
            histName += (sample.is2016() ? "2016" : "2017");
            histName += "Inclusive_" + minBiasVariation[var];
            puWeights[sample.getUniqueName()].push_back( std::shared_ptr<TH1D> ( (TH1D*)puFile->Get( (const TString&) histName ) ) );

            //make sure histogram does not get deleted when closing file
            puWeights[sample.getUniqueName()].back()->SetDirectory(gROOT);
        }
        puFile->Close();
    }
}

void Reweighter::initializeBTagWeights(const bool is2016){

    //assuming medium WP of deepCSV tagger 
    std::string sfFileName;
    if(is2016){
        sfFileName = "DeepCSV_Moriond17_B_H.csv";
    } else {
        sfFileName = "DeepCSV_94XSF_V2_B_F.csv";
    }
    bTagCalib = new BTagCalibration("deepCsv", "weights/" + sfFileName);
    bTagCalibReader =  new BTagCalibrationReader(BTagEntry::OP_MEDIUM, "central", {"up", "down"});
    bTagCalibReader->load(*bTagCalib, BTagEntry::FLAV_B, "comb");
    bTagCalibReader->load(*bTagCalib, BTagEntry::FLAV_C, "comb");
    bTagCalibReader->load(*bTagCalib, BTagEntry::FLAV_UDSG, "incl");    

    //initialize b-tag efficiencies
    std::string effFileName;
    if(is2016){
        sfFileName = "bTagEff_deepCSV_medium_cleaned_tZq_2016.root";
    } else {
        sfFileName = "bTagEff_deepCSV_medium_cleaned_tZq_2017.root";
    }

    TFile* bTagFile = TFile::Open( (const TString&) "weights/" + effFileName);
    const std::string quarkFlavors[3] = {"udsg", "charm", "beauty"};
    for(unsigned flav = 0; flav < 3; ++flav){
        bTagEffHist[flav] = (TH1D*) bTagFile->Get( (const TString&) "bTagEff_" + quarkFlavors[flav]);
        bTagEffHist[flav]->SetDirectory(gROOT);
    }
    bTagFile->Close();
}

void Reweighter::initializeElectronWeights(const bool is2016){	

	//read electron reco SF weights


    //read electron ID SF weights
    TFile* electronIdFile = TFile::Open("weights/electronIDScaleFactors_2016.root");
	std::string sfFileName;
	if(is2016){
		sfFileName = "electronIDScaleFactors_2016.root";
	} else {
		sfFileName = "electronIDScaleFactors_2017.root";
	} 
	electronLooseToRecoSF = std::shared_ptr<TH2D>( (TH2D*) electronIdFile->Get("EleToTTVLoose") );
	electronLooseToRecoSF->SetDirectory(gROOT);
	electronTightToLooseSF = std::shared_ptr<TH2D>( (TH2D*) electronIdFile->Get("TTVLooseToTTVLeptonMvatZq") );
	electronTightToLooseSF->SetDirectory(gROOT);
	electronIdFile->Close();
}


void Reweighter::initializeMuonWeights(const bool is2016){

	//read muon reco SF weights
	if(is2016){

    	//WARNING: not clear how ownership works for TGraph, can not set directory
    	//make sure the TGraph is not DELETED when file is closed!
		TFile* muonRecoFile = TFile::Open("weights/muonTrackingSF_2016.root");
    	muonRecoSF = std::shared_ptr<TGraph>( (TGraph*) muonRecoFile->Get("ratio_eff_eta3_dr030e030_corr") );
		muonRecoFile->Close();
	} else {
		
	}	

    //read muon ID SF weights
    TFile* muonIdFile = TFile::Open("weights/muonIDScaleFactors_2016.root");
    std::string sfFileName;
    if(is2016){
        sfFileName = "muonIDScaleFactors_2016.root";
    } else {
        sfFileName = "muonIDScaleFactors_2017.root";
    }
    muonLooseToRecoSF = std::shared_ptr<TH2D>( (TH2D*) muonIdFile->Get("MuonToTTVLoose") );
    muonLooseToRecoSF->SetDirectory(gROOT);
    muonTightToLooseSF = std::shared_ptr<TH2D>( (TH2D*) muonIdFile->Get("TTVLooseToTTVLeptonMvatZq") );
    muonTightToLooseSF->SetDirectory(gROOT);
    muonIdFile->Close();	
}

void Reweighter::initializeFakeRate(const bool is2016){

	//WARNING : To be updated with new fake-rate in 2016/2017 splitting
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

    for(unsigned flav = 0; flav < 3; ++flav){
        delete bTagEffHist[flav];
    }

    for(unsigned unc = 0; unc < 3; ++unc){
        delete frMapEle[unc];
        delete frMapMu[unc];
    }
}


double Reweighter::puWeight(const double nTrueInt, const Sample& sample, const unsigned unc) const{
    if(unc < 3){

        //find weights for given sample
        const auto& weightVectorIter = puWeights.find(sample.getUniqueName() );

        //check if pu weights are available for this sample
        if( weightVectorIter == puWeights.cend() ){
            std::cerr << "Error: no pu weights found for sample : " << sample.getUniqueName() << " returning weight 0  " << std::endl;
            return 0.;
        }

        double maxBin = (sample.is2016() ?  49.5 : 99.5 );
        TH1D* weights = ( (*weightVectorIter).second)[unc].get();
        return weights->GetBinContent(weights->FindBin(std::min(nTrueInt, maxBin) ) );
    }
    else {
        std::cerr << "Error: invalid pu uncertainty requested: returning weight 0" << std::endl;
        return 0.;
    }
}

double Reweighter::bTagWeight(const unsigned jetFlavor, const double jetPt, const double jetEta, const double jetCSV, const unsigned unc) const{
    static const BTagEntry::JetFlavor flavorEntries[3] = {BTagEntry::FLAV_UDSG, BTagEntry::FLAV_C, BTagEntry::FLAV_B};
    static const std::string uncName[3] = {"central", "down", "up"};
    if(unc < 3){
        return bTagCalibReader->eval_auto_bounds(uncName[unc], flavorEntries[flavorInd(jetFlavor)], jetEta, jetPt, jetCSV);
    } else {
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
    return electronRecoSF->GetBinContent( electronRecoSF->FindBin( std::max(-2.49, std::min(superClusterEta, 2.49)) , std::max(40., std::min(pt, 499.) )  ) );
}

double Reweighter::muonIdWeight(const double pt, const double eta) const{
	double croppedPt = std::min(pt, 199.);
	double croppedEta = std::min( fabs(eta), 2.39 ); 
	double sf = muonLooseToRecoSF->GetBinContent( muonLooseToRecoSF->FindBin( croppedPt, croppedEta) );
	sf*= muonTightToLooseSF->GetBinContent( muonTightToLooseSF->FindBin( croppedPt, croppedEta) );
    return sf;
}

double Reweighter::electronIdWeight(const double pt, const double eta) const{
	double croppedPt = std::min(pt, 199.);
	double croppedEta = std::min( fabs(eta), 2.49 ); 
	double sf = electronLooseToRecoSF->GetBinContent( electronLooseToRecoSF->FindBin( croppedPt, croppedEta) );
	sf*= electronTightToLooseSF->GetBinContent( electronTightToLooseSF->FindBin( croppedPt, croppedEta) );
    return sf;
}

double Reweighter::muonFakeRate(const double pt, const double eta, const unsigned unc) const{
    //!!!! To be split for 2016 and 2017 data !!!! 
    if(unc < 3){
        return frMapMu[unc]->GetBinContent(frMapMu[unc]->FindBin(std::min(pt, 99.), std::min(fabs(eta), 2.4) ) );
    } else {
        std::cerr << "Error: invalid muon fake-rate uncertainty requested: returning fake-rate 99" << std::endl;
        return 99;
    }
}

double Reweighter::electronFakeRate(const double pt, const double eta, const unsigned unc) const{
    //!!!! To be split for 2016 and 2017 data !!!!
    if(unc < 3){ 
        return frMapEle[unc]->GetBinContent(frMapEle[unc]->FindBin(std::min(pt, 99.), std::min(fabs(eta), 2.5) ) );
    } else {
        std::cerr << "Error: invalid electron fake-rate uncertainty requested: returning fake-rate 99" << std::endl;
        return 99;
    }
}
