#include "../interface/ConcreteReweighterFactory.h"

//include ROOT classes
#include "TFile.h"
#include "TH2.h"

//include other parts of framework
#include "../../Tools/interface/analysisTools.h"
#include "../../Tools/interface/stringTools.h"
#include "../interface/ConcreteLeptonReweighter.h"
#include "../interface/ConcreteReweighterLeptons.h"
#include "../interface/ConcreteSelection.h"
#include "../interface/ReweighterPileup.h"
#include "../interface/ConcreteReweighterBTag.h"
#include "../interface/ReweighterPrefire.h"



CombinedReweighter EwkinoReweighterFactory::buildReweighter( const std::string& weightDirectory, const std::string& year, const std::vector< Sample >& samples ) const{

    analysisTools::checkYearString( year );

    //reweighter to return
    CombinedReweighter combinedReweighter;

    //make muon ID Reweighter
    TFile* muonSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/leptonSF_m_" + year + "_3lTight.root" ).c_str() );
    std::shared_ptr< TH2 > muonSFHist( dynamic_cast< TH2* >( muonSFFile->Get( "SFglobal" ) ) );
    muonSFHist->SetDirectory( gROOT );
    muonSFFile->Close();

    MuonReweighter muonReweighter( muonSFHist, new TightSelector );
    combinedReweighter.addReweighter( "muonID", std::make_shared< ReweighterMuons >( muonReweighter ) );

    //make electron ID Reweighter
    TFile* eleSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/leptonSF_e_" + year + "_3lTight.root" ).c_str() );
    std::shared_ptr< TH2 > electronSFHist( dynamic_cast< TH2* >( eleSFFile->Get( "SFglobal" ) ) );
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
    
    //make b-tagging Reweighter 
    const std::string& bTagWP = "tight";

    //read MC b-tagging efficiency histograms
    // and scalefactor files
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
// user specified function for the tZq Reweighter

CombinedReweighter tZqReweighterFactory::buildReweighter( const std::string& weightDirectory, const std::string& year, const std::vector< Sample >& samples ) const{

    analysisTools::checkYearString( year );

    //reweighter to return
    CombinedReweighter combinedReweighter;

    //make muon ID Reweighter
    TFile* muonSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/looseToTight_" + year + "_m_3l.root" ).c_str() );
    std::shared_ptr< TH2 > muonSFHist( dynamic_cast< TH2* >( muonSFFile->Get( "EGamma_SF2D" ) ) );
    muonSFHist->SetDirectory( gROOT );
    muonSFFile->Close();

    MuonReweighter muonReweighter( muonSFHist, new TightSelector );
    combinedReweighter.addReweighter( "muonID", std::make_shared<ReweighterMuons>( muonReweighter ) );

    //make electron ID Reweighter
    TFile* eleSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/looseToTight_" + year + "_e_3l.root" ).c_str() );
    std::shared_ptr< TH2 > electronSFHist( dynamic_cast< TH2* >( eleSFFile->Get( "EGamma_SF2D" ) ) );
    electronSFHist->SetDirectory( gROOT );
    eleSFFile->Close();

    ElectronIDReweighter electronIDReweighter( electronSFHist, new TightSelector );
    combinedReweighter.addReweighter( "electronID", std::make_shared<ReweighterElectronsID>( electronIDReweighter ) );

    //make electron Reconstruction Reweighter
    if( year == "2016" || year == "2017" ){

        //pT below 20 GeV
        TFile* eleRecoSFFile_pTBelow20 = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/egamma_recoEff_" + year + "_pTBelow20.root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist_pTBelow20( dynamic_cast< TH2* >( eleRecoSFFile_pTBelow20->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist_pTBelow20->SetDirectory( gROOT );
        eleRecoSFFile_pTBelow20->Close();

        ElectronIDReweighter electronRecoReweighter_pTBelow20( electronRecoSFHist_pTBelow20, new LooseMaxPtSelector< 20 > );
        combinedReweighter.addReweighter( "electronReco_pTBelow20", std::make_shared<ReweighterElectronsID>( electronRecoReweighter_pTBelow20 ) );

        //pT above 20 GeV
        TFile* eleRecoSFFile_pTAbove20 = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/egamma_recoEff_" + year + "_pTAbove20.root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist_pTAbove20( dynamic_cast< TH2* >( eleRecoSFFile_pTAbove20->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist_pTAbove20->SetDirectory( gROOT );
        eleRecoSFFile_pTAbove20->Close();

        ElectronIDReweighter electronRecoReweighter_pTAbove20( electronRecoSFHist_pTAbove20, new LooseMinPtSelector< 20 > );
        combinedReweighter.addReweighter( "electronReco_pTAbove20", std::make_shared<ReweighterElectronsID>( electronRecoReweighter_pTAbove20 ) );

    } else if( year == "2018" ){

        //inclusive pT 
        TFile* eleRecoSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/egamma_recoEff_" + year + ".root" ).c_str() );
        std::shared_ptr< TH2 > electronRecoSFHist ( dynamic_cast< TH2* >( eleRecoSFFile->Get( "EGamma_SF2D" ) ) );
        electronRecoSFHist->SetDirectory( gROOT );
        eleRecoSFFile->Close();

        ElectronIDReweighter electronRecoReweighter( electronRecoSFHist, new LooseSelector );
        combinedReweighter.addReweighter( "electronReco", std::make_shared<ReweighterElectronsID>( electronRecoReweighter ) );

    }

    //make pileup Reweighter
    combinedReweighter.addReweighter( "pileup", std::make_shared<ReweighterPileup>( samples, weightDirectory ) );

    //make b-tagging Reweighter 
    const std::string& bTagWP = "tight";

    //read MC b-tagging efficiency histograms (use DeepFlavor!!!)
    const std::string& leptonCleaning = "looseLeptonCleaned";
    TFile* bTagEffMCFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/bTagEff/bTagEff_deepFlavor_" + leptonCleaning + "_" + year + ".root" ).c_str() );
    std::shared_ptr< TH2 > bTagEffMCHist_udsg( dynamic_cast< TH2* >( bTagEffMCFile->Get( ( "bTagEff_" + bTagWP + "_udsg" ).c_str() ) ) );
    bTagEffMCHist_udsg->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEffMCHist_c( dynamic_cast< TH2* >( bTagEffMCFile->Get( ( "bTagEff_" + bTagWP + "_charm" ).c_str() ) ) );
    bTagEffMCHist_c->SetDirectory( gROOT );
    std::shared_ptr< TH2 > bTagEffMCHist_b( dynamic_cast< TH2* >( bTagEffMCFile->Get( ( "bTagEff_" + bTagWP + "_beauty" ).c_str() ) ) );
    bTagEffMCHist_b->SetDirectory( gROOT );
    bTagEffMCFile->Close();

    std::string bTagSFFileName;
    if( year == "2016" ){
        bTagSFFileName= "DeepJet_2016LegacySF_WP_V1.csv";
    } else if( year == "2017" ){
        bTagSFFileName = "DeepFlavour_94XSF_WP_V3_B_F.csv";
    } else {
        bTagSFFileName = "DeepJet_102XSF_WP_V1.csv";
    }
    std::string bTagSFPath = "weightFiles/bTagSF/" + bTagSFFileName;

    combinedReweighter.addReweighter( "bTag_heavy", std::make_shared< ReweighterBTagHeavyFlavorDeepFlavor >( weightDirectory, bTagSFPath, "tight", bTagEffMCHist_c, bTagEffMCHist_b ) );
    combinedReweighter.addReweighter( "bTag_light", std::make_shared< ReweighterBTagLightFlavorDeepFlavor >( weightDirectory, bTagSFPath, "tight", bTagEffMCHist_udsg ) );   

    //make prefire Reweighter
    combinedReweighter.addReweighter( "prefire", std::make_shared< ReweighterPrefire >() );

    return combinedReweighter;
}
