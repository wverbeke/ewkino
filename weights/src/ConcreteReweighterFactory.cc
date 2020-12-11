#include "../interface/ConcreteReweighterFactory.h"

//include ROOT classes
#include "TFile.h"
#include "TH2.h"

//include other parts of framework
#include "../../Tools/interface/analysisTools.h"
#include "../../Tools/interface/stringTools.h"
#include "../../objects/interface/LeptonSelector.h"
#include "../interface/ConcreteLeptonReweighter.h"
#include "../interface/ConcreteReweighterLeptons.h"
#include "../interface/ConcreteSelection.h"
#include "../interface/ReweighterPileup.h"
#include "../interface/ConcreteReweighterBTag.h"
#include "../interface/ReweighterPrefire.h"


CombinedReweighter EwkinoReweighterFactory::buildReweighter( const std::string& weightDirectory, 
		    const std::string& year, const std::vector< Sample >& samples ) const{

    analysisTools::checkYearString( year );

    //reweighter to return
    CombinedReweighter combinedReweighter;

    //make muon ID Reweighter
    // new version (files not present here)
    //TFile* muonSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/leptonSF_m_" + year + "_3lTight.root" ).c_str() );
    //std::shared_ptr< TH2 > muonSFHist( dynamic_cast< TH2* >( muonSFFile->Get( "SFglobal" ) ) );
    // old version
    TFile* muonSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/looseToTight_" + year + "_m_3l.root" ).c_str() );
    std::shared_ptr< TH2 > muonSFHist( dynamic_cast< TH2* >( muonSFFile->Get( "EGamma_SF2D" ) ) );
    muonSFHist->SetDirectory( gROOT );
    muonSFFile->Close();

    MuonReweighter muonReweighter( muonSFHist, new TightSelector );
    combinedReweighter.addReweighter( "muonID", std::make_shared< ReweighterMuons >( muonReweighter ) );

    //make electron ID Reweighter
    // new version (files not present here)
    //TFile* eleSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/leptonSF_e_" + year + "_3lTight.root" ).c_str() );
    //std::shared_ptr< TH2 > electronSFHist( dynamic_cast< TH2* >( eleSFFile->Get( "SFglobal" ) ) );
    // old version
    TFile* eleSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/looseToTight_" + year + "_e_3l.root" ).c_str() );
    std::shared_ptr< TH2 > electronSFHist( dynamic_cast< TH2* >( eleSFFile->Get( "EGamma_SF2D" ) ) );
    electronSFHist->SetDirectory( gROOT );
    eleSFFile->Close();

    ElectronIDReweighter electronIDReweighter( electronSFHist, new TightSelector );
    combinedReweighter.addReweighter( "electronID", std::make_shared< ReweighterElectronsID >( electronIDReweighter ) );

    //make electron Reconstruction Reweighter
    if( year == "2016" || year == "2017" ){

        //pT below 20 GeV
        TFile* eleRecoSFFile_pTBelow20 = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/egamma_recoEff_" + year + "_pTBelow20.root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist_pTBelow20( dynamic_cast< TH2* >( eleRecoSFFile_pTBelow20->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist_pTBelow20->SetDirectory( gROOT );
        eleRecoSFFile_pTBelow20->Close();

        ElectronIDReweighter electronRecoReweighter_pTBelow20( electronRecoSFHist_pTBelow20, new LooseMaxPtSelector< 20 > );
        combinedReweighter.addReweighter( "electronReco_pTBelow20", std::make_shared< ReweighterElectronsID >( electronRecoReweighter_pTBelow20 ) );

        //pT above 20 GeV
        TFile* eleRecoSFFile_pTAbove20 = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/egamma_recoEff_" + year + "_pTAbove20.root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist_pTAbove20( dynamic_cast< TH2* >( eleRecoSFFile_pTAbove20->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist_pTAbove20->SetDirectory( gROOT );
        eleRecoSFFile_pTAbove20->Close();

        ElectronIDReweighter electronRecoReweighter_pTAbove20( electronRecoSFHist_pTAbove20, new LooseMinPtSelector< 20 > );
        combinedReweighter.addReweighter( "electronReco_pTAbove20", std::make_shared< ReweighterElectronsID >( electronRecoReweighter_pTAbove20 ) );

    } else if( year == "2018" ){

        //inclusive pT 
        TFile* eleRecoSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/egamma_recoEff_" + year + ".root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist ( dynamic_cast< TH2* >( eleRecoSFFile->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist->SetDirectory( gROOT );
        eleRecoSFFile->Close();

        ElectronIDReweighter electronRecoReweighter( electronRecoSFHist, new LooseSelector );
        combinedReweighter.addReweighter( "electronReco", std::make_shared< ReweighterElectronsID >( electronRecoReweighter ) );

    }
    
    //make pileup Reweighter
    combinedReweighter.addReweighter( "pileup", std::make_shared< ReweighterPileup >( samples, weightDirectory ) );
    
    // make b-tagging Reweighter 
    const std::string& bTagWP = "medium";

    // read MC b-tagging efficiency histograms and scalefactor files
    const std::string& leptonCleaning = "looseLeptonCleaned";
    TFile* bTagEffMCFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/bTagEff/bTagEff_" + leptonCleaning + "_" + year + ".root" ).c_str() );
    std::shared_ptr< TH2 > bTagEffMCHist_udsg( dynamic_cast< TH2* >( bTagEffMCFile->Get( ( "bTagEff_" + bTagWP + "_udsg" ).c_str() ) ) );
    bTagEffMCHist_udsg->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEffMCHist_c( dynamic_cast< TH2* >( bTagEffMCFile->Get( ( "bTagEff_" + bTagWP + "_charm" ).c_str() ) ) );
    bTagEffMCHist_c->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEffMCHist_b( dynamic_cast< TH2* >( bTagEffMCFile->Get( ( "bTagEff_" + bTagWP + "_beauty" ).c_str() ) ) );
    bTagEffMCHist_b->SetDirectory( gROOT );
    bTagEffMCFile->Close();

    std::string bTagSFFileName;
    if( year == "2016" ){
        bTagSFFileName= "DeepCSV_2016LegacySF_WP_V1.csv";
    } else if( year == "2017" ){
        bTagSFFileName = "DeepCSV_94XSF_WP_V4_B_F.csv";
    } else {
        bTagSFFileName = "DeepCSV_102XSF_WP_V1.csv";
    }
    std::string bTagSFPath = "weightFiles/bTagSF/" + bTagSFFileName;

    combinedReweighter.addReweighter( "bTag_heavy", std::make_shared< ReweighterBTagHeavyFlavorDeepCSV >( weightDirectory, bTagSFPath, bTagWP, bTagEffMCHist_c, bTagEffMCHist_b ) );
    combinedReweighter.addReweighter( "bTag_light", std::make_shared< ReweighterBTagLightFlavorDeepCSV >( weightDirectory, bTagSFPath, bTagWP, bTagEffMCHist_udsg ) );

    //make prefire Reweighter
    combinedReweighter.addReweighter( "prefire", std::make_shared< ReweighterPrefire >() );

    return combinedReweighter;
}

// ------------------------------------------------------------------------------- //
// empty reweighter for testing purposes

CombinedReweighter EmptyReweighterFactory::buildReweighter( const std::string& weightDirectory,
		    const std::string& year, const std::vector< Sample >& samples ) const{

    // reweighter to return
    CombinedReweighter combinedReweighter;
    // dummy condition on args to avoid compilation warnings
    if(weightDirectory=="" && year=="" && samples.size()==0) return combinedReweighter;

    // add all reweighters you need formally, but each reweighter returns unity for each event
    combinedReweighter.addReweighter( "muonID", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "muonIDSyst", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "muonIDStat", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "electronID", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "electronIDSyst", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "electronIDStat", std::make_shared< ReweighterEmpty >() );
    if( year == "2016" || year == "2017" ){
	combinedReweighter.addReweighter( "electronReco_pTBelow20",
            std::make_shared< ReweighterEmpty >() );
        combinedReweighter.addReweighter( "electronReco_pTAbove20",
            std::make_shared< ReweighterEmpty >() );
    } else if( year == "2018" ){
	combinedReweighter.addReweighter( "electronReco",
            std::make_shared< ReweighterEmpty >() );
    }
    combinedReweighter.addReweighter( "pileup", std::make_shared<ReweighterEmpty>() );
    combinedReweighter.addReweighter( "bTag_heavy", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "bTag_light", std::make_shared< ReweighterEmpty >() );
    combinedReweighter.addReweighter( "prefire", std::make_shared< ReweighterEmpty >() );

    return combinedReweighter;
}

// ------------------------------------------------------------------------------- //
// user specified function for the tZq Reweighter

CombinedReweighter tZqReweighterFactory::buildReweighter( const std::string& weightDirectory, 
		    const std::string& year, const std::vector< Sample >& samples ) const{

    analysisTools::checkYearString( year );

    // reweighter to return
    CombinedReweighter combinedReweighter;

    // determine what working point is being used 
    // function definition in ewkino/objectSelection/LeptonSelector!
    std::string leptonID = LeptonSelector::leptonID();
    std::string interpendix = "";
    if(leptonID=="tzqtight") interpendix = "Tight";
    else if(leptonID=="tzqmedium0p4") interpendix = "Medium040";
    else{ 
	std::cerr << "### ERROR ###: lepton ID '" << leptonID;
	std::cerr << "' not recognized (in tZqReweighterFactory);";
	std::cerr << " loading of scale factors will crash..." << std::endl;
    }

    // make muon ID Reweighter
    std::string muonSFFileName = stringTools::formatDirectoryName( weightDirectory )
        + "weightFiles/leptonSF/SFTOPLeptonID"+interpendix+"_" + year + "_muon.root";
    TFile* muonSFFile = TFile::Open( (muonSFFileName).c_str() );
    // load the scalefactor histogram and set the errors to zero,
    // load the systematic errors and set the bin contents to one and errors relative,
    // (note: the histogram _syst contains the SF as bin contents and the uncertainties as bin errors!)
    // load the statistical errors and set the bin contents to one and the errors relative
    // (note: the histogram _stat contains the SF as bin contents and the uncertainties as bin errors!)
    std::shared_ptr< TH2 > muonSFHist_nom( dynamic_cast< TH2* >( 
	muonSFFile->Get( ("NUM_LeptonMva"+interpendix+"_DEN_genTracks_abseta_pt").c_str() ) ) );
    muonSFHist_nom->SetDirectory( gROOT );
    std::shared_ptr< TH2 > muonSFHist_syst( dynamic_cast< TH2* >( 
	muonSFFile->Get( ("NUM_LeptonMva"+interpendix+"_DEN_genTracks_abseta_pt_syst").c_str() ) ) );
    muonSFHist_syst->SetDirectory( gROOT );
    std::shared_ptr< TH2 > muonSFHist_stat( dynamic_cast< TH2* >( 
	muonSFFile->Get( ("NUM_LeptonMva"+interpendix+"_DEN_genTracks_abseta_pt_stat").c_str() ) ) );
    muonSFHist_stat->SetDirectory( gROOT );
    muonSFFile->Close();
    for(int i = 0; i <= muonSFHist_nom->GetNbinsX()+1; ++i){
	for(int j = 0; j <= muonSFHist_nom->GetNbinsY()+1; ++j){

	    // prints for testing
	    /*std::cout << muonSFFileName << std::endl;
	    std::cout << "--- bin " << i << "," << j << "(raw values) ---" << std::endl;
            std::cout << muonSFHist_nom->GetBinContent(i,j) << std::endl;
            std::cout << muonSFHist_nom->GetBinError(i,j) << std::endl;
            std::cout << muonSFHist_syst->GetBinContent(i,j) << std::endl;
            std::cout << muonSFHist_syst->GetBinError(i,j) << std::endl;
            std::cout << muonSFHist_stat->GetBinContent(i,j) << std::endl;
	    std::cout << muonSFHist_stat->GetBinError(i,j) << std::endl;*/

	    // process values
	    muonSFHist_nom->SetBinError(i,j,0.);
	    double sf = muonSFHist_nom->GetBinContent(i,j);
	    muonSFHist_syst->SetBinError(i,j,muonSFHist_syst->GetBinError(i,j)/sf);
	    muonSFHist_syst->SetBinContent(i,j,1.);
	    muonSFHist_stat->SetBinError(i,j,muonSFHist_stat->GetBinError(i,j)/sf);
            muonSFHist_stat->SetBinContent(i,j,1.);
	
	    // print for testing
	    /*std::cout << "--- bin (processed values) ---" << std::endl;
	    std::cout << muonSFHist_nom->GetBinContent(i,j) << std::endl;
	    std::cout << muonSFHist_nom->GetBinError(i,j) << std::endl;
	    std::cout << muonSFHist_syst->GetBinContent(i,j) << std::endl;
            std::cout << muonSFHist_syst->GetBinError(i,j) << std::endl;
	    std::cout << muonSFHist_stat->GetBinContent(i,j) << std::endl;
            std::cout << muonSFHist_stat->GetBinError(i,j) << std::endl;*/
	}
    }

    MuonReweighter muonReweighter_nom( muonSFHist_nom, new TightSelector );
    combinedReweighter.addReweighter("muonID",std::make_shared<ReweighterMuons>(muonReweighter_nom));
    MuonReweighter muonReweighter_syst( muonSFHist_syst, new TightSelector );
    combinedReweighter.addReweighter("muonIDSyst",std::make_shared<ReweighterMuons>(muonReweighter_syst));
    MuonReweighter muonReweighter_stat( muonSFHist_stat, new TightSelector );
    combinedReweighter.addReweighter("muonIDStat",std::make_shared<ReweighterMuons>(muonReweighter_stat));

    // make electron ID Reweighter
    TFile* eleSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) 
	+ "weightFiles/leptonSF/SFTOPLeptonID"+interpendix+"_" + year + "_electron.root" ).c_str() );
    // load the scalefactor histogram and set the errors to zero,
    // load the systematic errors and set the bin contents to one,
    // (note: the histogram syst contains the relative uncertainties as bin contents (?))
    // load the statistical errors and set the bin contents to one
    // (note: the histogram stat contains the relative uncertainties as bin contents (?))
    std::shared_ptr< TH2 > electronSFHist_nom( dynamic_cast< TH2* >
	( eleSFFile->Get( "EGamma_SF2D" ) ) );
    electronSFHist_nom->SetDirectory( gROOT );
    std::shared_ptr< TH2 > electronSFHist_syst( dynamic_cast< TH2* >
        ( eleSFFile->Get( "sys" ) ) );
    electronSFHist_syst->SetDirectory( gROOT );
    std::shared_ptr< TH2 > electronSFHist_stat( dynamic_cast< TH2* >
        ( eleSFFile->Get( "stat" ) ) );
    electronSFHist_stat->SetDirectory( gROOT );
    eleSFFile->Close();
    for(int i = 0; i <= electronSFHist_nom->GetNbinsX()+1; ++i){
        for(int j = 0; j <= electronSFHist_nom->GetNbinsY()+1; ++j){
            electronSFHist_nom->SetBinError(i,j,0.);
            //double sf = electronSFHist_nom->GetBinContent(i,j);
            electronSFHist_syst->SetBinError(i,j,electronSFHist_syst->GetBinContent(i,j));
            electronSFHist_syst->SetBinContent(i,j,1.);
            electronSFHist_stat->SetBinError(i,j,electronSFHist_stat->GetBinContent(i,j));
            electronSFHist_stat->SetBinContent(i,j,1.);
        }
    }

    ElectronIDReweighter electronIDReweighter_nom( electronSFHist_nom, new TightSelector );
    combinedReweighter.addReweighter( "electronID",
	std::make_shared<ReweighterElectronsID>(electronIDReweighter_nom) );
    ElectronIDReweighter electronIDReweighter_syst( electronSFHist_syst, new TightSelector );
    combinedReweighter.addReweighter( "electronIDSyst",
        std::make_shared<ReweighterElectronsID>(electronIDReweighter_syst) );
    ElectronIDReweighter electronIDReweighter_stat( electronSFHist_stat, new TightSelector );
    combinedReweighter.addReweighter( "electronIDStat",
        std::make_shared<ReweighterElectronsID>(electronIDReweighter_stat) );

    // make electron Reconstruction Reweighter
    if( year == "2016" || year == "2017" ){

        // pT below 20 GeV
        TFile* eleRecoSFFile_pTBelow20 = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) 
	    + "weightFiles/leptonSF/egamma_recoEff_" + year + "_pTBelow20.root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist_pTBelow20( dynamic_cast< TH2* >( 
	    eleRecoSFFile_pTBelow20->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist_pTBelow20->SetDirectory( gROOT );
        eleRecoSFFile_pTBelow20->Close();

        ElectronIDReweighter electronRecoReweighter_pTBelow20( electronRecoSFHist_pTBelow20, 
	    new LooseMaxPtSelector< 20 > );
        combinedReweighter.addReweighter( "electronReco_pTBelow20", 
	    std::make_shared<ReweighterElectronsID>( electronRecoReweighter_pTBelow20 ) );

        // pT above 20 GeV
        TFile* eleRecoSFFile_pTAbove20 = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) 
	    + "weightFiles/leptonSF/egamma_recoEff_" + year + "_pTAbove20.root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist_pTAbove20( dynamic_cast< TH2* >( 
	    eleRecoSFFile_pTAbove20->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist_pTAbove20->SetDirectory( gROOT );
        eleRecoSFFile_pTAbove20->Close();

        ElectronIDReweighter electronRecoReweighter_pTAbove20( electronRecoSFHist_pTAbove20, 
	    new LooseMinPtSelector< 20 > );
        combinedReweighter.addReweighter( "electronReco_pTAbove20", 
	    std::make_shared<ReweighterElectronsID>( electronRecoReweighter_pTAbove20 ) );

    } else if( year == "2018" ){

        // inclusive pT 
        TFile* eleRecoSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) 
	    + "weightFiles/leptonSF/egamma_recoEff_" + year + ".root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist ( dynamic_cast< TH2* >( 
	    eleRecoSFFile->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist->SetDirectory( gROOT );
        eleRecoSFFile->Close();

        ElectronIDReweighter electronRecoReweighter( electronRecoSFHist, new LooseSelector );
        combinedReweighter.addReweighter( "electronReco", 
	    std::make_shared<ReweighterElectronsID>( electronRecoReweighter ) );
    }

    // make pileup Reweighter
    combinedReweighter.addReweighter( "pileup", 
	std::make_shared<ReweighterPileup>( samples, weightDirectory ) );

    // make b-tagging Reweighter 
    const std::string& bTagWP = "medium";
    const std::string& bTagReweighterWP = "medium"; // temp since "reshaping" is not in the csv files
    // (need different 'WP' for reweighter than for selection as we need to use scale factors
    // that correct the whole shape of the b-tagger output, as it is used in the BDT!)

    // read MC b-tagging efficiency histograms
    const std::string& leptonCleaning = "looseLeptonCleaned";
    const std::string& btagger = "deepFlavor"; // choose from "deepFlavor" or "deepCSV"
    TFile* bTagEffMCFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) 
	+ "weightFiles/bTagEff/bTagEff_" + btagger + "_" + leptonCleaning + "_" + year + ".root" ).c_str() );
    std::shared_ptr< TH2 > bTagEffMCHist_udsg( dynamic_cast< TH2* >( 
	bTagEffMCFile->Get( ( "bTagEff_" + bTagWP + "_udsg" ).c_str() ) ) );
    bTagEffMCHist_udsg->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEffMCHist_c( dynamic_cast< TH2* >( 
	bTagEffMCFile->Get( ( "bTagEff_" + bTagWP + "_charm" ).c_str() ) ) );
    bTagEffMCHist_c->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEffMCHist_b( dynamic_cast< TH2* >( 
	bTagEffMCFile->Get( ( "bTagEff_" + bTagWP + "_beauty" ).c_str() ) ) );
    bTagEffMCHist_b->SetDirectory( gROOT );
    bTagEffMCFile->Close();

    // read b-tagging scale factor histograms
    std::string bTagSFFileName;
    if( year == "2016" ){
        bTagSFFileName= "DeepJet_2016LegacySF_WP_V1.csv";
    } else if( year == "2017" ){
        bTagSFFileName = "DeepFlavour_94XSF_WP_V3_B_F.csv";
    } else {
        bTagSFFileName = "DeepJet_102XSF_WP_V1.csv";
    }
    std::string bTagSFPath = "weightFiles/bTagSF/" + bTagSFFileName;

    combinedReweighter.addReweighter( "bTag_heavy", 
	std::make_shared< ReweighterBTagHeavyFlavorDeepFlavor >( weightDirectory, bTagSFPath, 
	    bTagReweighterWP, bTagEffMCHist_c, bTagEffMCHist_b ) );
    combinedReweighter.addReweighter( "bTag_light", 
	std::make_shared< ReweighterBTagLightFlavorDeepFlavor >( weightDirectory, bTagSFPath, 
	    bTagReweighterWP, bTagEffMCHist_udsg ) );   

    // make prefire Reweighter
    combinedReweighter.addReweighter( "prefire", std::make_shared< ReweighterPrefire >() );

    return combinedReweighter;
}
