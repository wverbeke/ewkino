/*
Test script for b-tag shape reweighter
*/

#include "../../weights/interface/ReweighterBTagShape.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/Sample.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/stringTools.h"


int main( int argc, char* argv[] ){

    int nargs = 3;
    if( argc != nargs+1 ){
        std::cerr << "### ERROR ###: ReweighterBTagShape_test.cc requires " << nargs;
	std::cerr << " arguments to run:" << std::endl;
	std::cerr << "- directory of input file(s)" << std::endl;
        std::cerr << "- name of input file (.root) OR samplelist (.txt)" << std::endl;
	std::cerr << "- number of events (use 0 for all events)" << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& inputDirectory = argvStr[1];
    std::string& sampleList = argvStr[2];
    long unsigned nEvents = std::stoul(argvStr[3]);

    // read the input file
    TreeReader treeReader;
    std::vector<Sample> samples;
    bool modeSampleList = false;
    // case 1: single input file
    if( stringTools::stringEndsWith(sampleList, ".root") ){
	std::string inputFilePath = stringTools::formatDirectoryName(inputDirectory)+sampleList;
	treeReader.initSampleFromFile( inputFilePath );
	samples.push_back( treeReader.currentSample() );
    }
    // case 2: samplelist
    else if( stringTools::stringEndsWith(sampleList, ".txt") ){
	treeReader.readSamples( sampleList, inputDirectory );
	samples = treeReader.sampleVector();
	modeSampleList = true;
    }
    std::cout << "will use the following samples:" << std::endl;
    for( Sample sample: samples ) std::cout << "- " << sample.fileName() << std::endl;

    // initialize year from first sample
    // note: in this simple test no check is done to assure all samples in the list are of same year!
    if( modeSampleList ) treeReader.initSample();
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";

    // initialize some histograms
    HistInfo histInfo = HistInfo( "", "b-tag reweighting factor", 50, -0.1, 2.1 );
    // histograms of nominal weight before and after normalization
    std::shared_ptr<TH1D> bWeightDistPreNorm = histInfo.makeHist( "bWeightDist_nominalPreNorm" );
    std::shared_ptr<TH1D> bWeightDistPostNorm = histInfo.makeHist( "bWeightDist_nominalPostNorm" );
    // histograms of varied weights
    std::vector<std::string> variations = {"jes", "hf","lf","hfstats1","hfstats2",
					    "lfstats1","lfstats2","cferr1","cferr2",
					    "jesAbsoluteMPFBias", "jesAbsoluteScale", 
					    "jesAbsoluteStat"};
    std::map<std::string, std::shared_ptr<TH1D>> bWeightDistVariations;
    for( std::string variation: variations ){
	bWeightDistVariations[variation+"Up"] = histInfo.makeHist( "bWeightDist_"+variation+"Up" );
	bWeightDistVariations[variation+"Down"] = histInfo.makeHist( "bWeightDist_"+variation+"Down" );
    }
    // histograms with JEC variation propagated
    std::shared_ptr<TH1D> bWeightDistJECUp = histInfo.makeHist( "bWeightDist_JECUp" );
    std::shared_ptr<TH1D> bWeightDistJECDown = histInfo.makeHist( "bWeightDist_JECDown" );
    
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
    // step 3: make the reweighter
    std::shared_ptr<ReweighterBTagShape> reweighterBTagShape = std::make_shared<ReweighterBTagShape>(
	    weightDirectory, sfFilePath, flavor, bTagAlgo, variations, samples );

    // === loop over events and calculate: ===
    // - distribution of reweighting factors (to see if they are within reasonable bounds)
    // - average of reweighting factors per jet multiplicity bin (to check normalization later on)
    std::map< int, double > averageOfWeights;
    std::map< int, int > nEntries;
    unsigned numberOfSamples = samples.size();
    for( unsigned i = 0; i < numberOfSamples; ++i ){
	std::cout<<"start processing sample n. "<<i+1<<" of "<<numberOfSamples<<std::endl;
        if( modeSampleList ) treeReader.initSample( samples[i] );
	long unsigned numberOfEntries = treeReader.numberOfEntries();
	if( nEvents==0 ) nEvents = numberOfEntries;
	else nEvents = std::min(nEvents, numberOfEntries);
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
	    for( std::string variation: variations ){
		bWeightDistVariations[variation+"Up"]->Fill( 
		    reweighterBTagShape->weightUp( event, variation ) );
		bWeightDistVariations[variation+"Down"]->Fill( 
                    reweighterBTagShape->weightDown( event, variation ) );
	    }
	    bWeightDistJECUp->Fill( reweighterBTagShape->weightJecVar( event, "JECUp") );
	    bWeightDistJECDown->Fill( reweighterBTagShape->weightJecVar( event, "JECDown") );
	}
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
    std::cout << "norm factors of b-tag reweighter before initialization: " << std::endl;
    reweighterBTagShape->printNormFactors();

    // === do normalization ===
    reweighterBTagShape->initialize( samples, nEvents );
    std::cout << "norm factors of b-tag reweighter after initialization: " << std::endl;
    reweighterBTagShape->printNormFactors();

    // === re-loop ===
    std::map< int, double > averageOfWeightsPostNorm;
    for( unsigned i = 0; i < numberOfSamples; ++i ){
        std::cout<<"start processing sample n. "<<i+1<<" of "<<numberOfSamples<<std::endl;
        if(modeSampleList ) treeReader.initSample( samples[i] );
        long unsigned numberOfEntries = treeReader.numberOfEntries();
        if( nEvents==0 ) nEvents = numberOfEntries;
        else nEvents = std::min(nEvents, numberOfEntries);
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
    for( std::string variation: variations ){
	bWeightDistVariations[variation+"Up"]->Write();
	bWeightDistVariations[variation+"Down"]->Write();
    }
    bWeightDistJECUp->Write();
    bWeightDistJECDown->Write();
    filePtr->Close();
}
