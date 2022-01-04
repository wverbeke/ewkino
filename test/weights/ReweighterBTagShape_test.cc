/*
Test script for b-tag shape reweighter
*/

#include "../../weights/interface/ReweighterBTagShape.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/Sample.h"
#include "../../Tools/interface/HistInfo.h"


int main( int argc, char* argv[] ){

    int nargs = 2;
    if( argc != nargs+1 ){
        std::cerr << "### ERROR ###: ReweighterBTagShape_test.cc requires " << nargs;
	std::cerr << " arguments to run:" << std::endl;
        std::cerr << "- path to input file" << std::endl;
	std::cerr << "- number of events (use 0 for all events)" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& inputFilePath = argvStr[1];
    long unsigned nEvents = std::stoul(argvStr[2]);

    // read the input file
    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEvents==0 ) nEvents = numberOfEntries;
    else nEvents = std::min(nEvents, numberOfEntries);
    std::cout << "will use sample " << inputFilePath;
    std::cout << " (contains " << numberOfEntries << " events";
    std::cout << " of which " << nEvents << " will be used)" << std::endl;
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";
    std::vector<Sample> thisSample;
    thisSample.push_back( treeReader.currentSample() );

    // initialize some histograms
    HistInfo histInfo = HistInfo( "", "b-tag reweighting factor", 50, 0.5, 1.5 );
    std::shared_ptr<TH1D> bWeightDistPreNorm = histInfo.makeHist( "bWeightDistPreNorm" );
    std::shared_ptr<TH1D> bWeightDistPostNorm = histInfo.makeHist( "bWeightDistPostNorm" );
    
    // make the b-tag shape reweighter
    // step 1: set correct csv file
    std::string bTagSFFileName;
    if( year == "2016" ) bTagSFFileName = "DeepJet_2016LegacySF_V1.csv";
    else if( year == "2017" ) bTagSFFileName = "DeepFlavour_94XSF_V4_B_F.csv";
    else bTagSFFileName = "DeepJet_102XSF_V2.csv";
    std::string weightDirectory = "../../weights";
    std::string sfFilePath = "weightFiles/bTagSF/"+bTagSFFileName;
    // step 2: set other parameters
    std::string flavor = "all";
    std::string bTagAlgo = "deepFlavor";
    std::vector<std::string> variations = {"jes","hf","lf","hfstats1","hfstats2",
                                        "lfstats1","lfstats2","cferr1","cferr2" };
    // step 3: make the reweighter
    std::shared_ptr<ReweighterBTagShape> reweighterBTagShape = std::make_shared<ReweighterBTagShape>(
	    weightDirectory, sfFilePath, flavor, bTagAlgo, variations, thisSample );

    // === loop over events and calculate: ===
    // - distribution of reweighting factors (to see if they are within reasonable bounds)
    // - average of reweighting factors per jet multiplicity bin (to check normalization later on)
    std::map< int, double > averageOfWeights;
    std::map< int, int > nEntries;
    std::cout << "starting event loop for " << nEvents << " events..." << std::endl;
    for( long unsigned entry = 0; entry < nEvents; ++entry ){
	if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
        Event event = treeReader.buildEvent( entry );
        // do basic jet cleaning
        event.cleanJetsFromFOLeptons();
        event.jetCollection().selectGoodJets();
        // determine (nominal) b-tag reweighting and number of jets
        double btagreweight = reweighterBTagShape->weight( event );
        int njets = event.jetCollection().goodJetCollection().size();
        // add it to the map
        if(averageOfWeights.find(njets)==averageOfWeights.end()){
            averageOfWeights[njets] = btagreweight;
            nEntries[njets] = 1; }
        else{
            averageOfWeights[njets] += btagreweight;
            nEntries[njets] += 1; }
	// fill the histogram
	bWeightDistPreNorm->Fill( btagreweight );
    }
    // divide sum by number to get average
    for( std::map<int,double>::iterator it = averageOfWeights.begin();
            it != averageOfWeights.end(); ++it){
        averageOfWeights[it->first] = it->second / nEntries[it->first];
    }
    // printouts
    std::cout << "average of b-tag weights:" << std::endl;
    for( std::map<int,double>::iterator it = averageOfWeights.begin();
            it != averageOfWeights.end(); ++it){
        std::cout << "njets: " << it->first << "-> average: " << it->second << std::endl;
    }

    // === do normalization and re-loop ===
    reweighterBTagShape->initialize( thisSample, nEvents );
    std::map< int, double > averageOfWeightsPostNorm;
    std::cout << "starting event loop for " << nEvents << " events..." << std::endl;
    for( long unsigned entry = 0; entry < nEvents; ++entry ){
        Event event = treeReader.buildEvent( entry );
        // do basic jet cleaning
        event.cleanJetsFromFOLeptons();
        event.jetCollection().selectGoodJets();
        // determine (nominal) b-tag reweighting and number of jets
        double btagreweight = reweighterBTagShape->weight( event );
        int njets = event.jetCollection().goodJetCollection().size();
        // add it to the map
        if(averageOfWeightsPostNorm.find(njets)==averageOfWeightsPostNorm.end()){
            averageOfWeightsPostNorm[njets] = btagreweight; }
        else{
            averageOfWeightsPostNorm[njets] += btagreweight; }
	// fill the histogram
        bWeightDistPostNorm->Fill( btagreweight );
    }
    // divide sum by number to get average
    for( std::map<int,double>::iterator it = averageOfWeightsPostNorm.begin();
            it != averageOfWeightsPostNorm.end(); ++it){
        averageOfWeightsPostNorm[it->first] = it->second / nEntries[it->first];
    }
    // printouts
    std::cout << "average of b-tag weights:" << std::endl;
    for( std::map<int,double>::iterator it = averageOfWeightsPostNorm.begin();
            it != averageOfWeightsPostNorm.end(); ++it){
        std::cout << "njets: " << it->first << "-> average: " << it->second << std::endl;
    }

    // write histograms to output file
    std::string outputFileName = "output_ReweighterBTagShape_test.root";
    TFile* filePtr = TFile::Open( outputFileName.c_str(), "recreate" );
    bWeightDistPreNorm->Write();
    bWeightDistPostNorm->Write();
    filePtr->Close();
}
