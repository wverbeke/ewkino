///// Alternative b-tagging reweighter appropriate when reweighting the shape /////
// see https://twiki.cern.ch/twiki/bin/view/CMS/BTagShapeCalibration
// and https://twiki.cern.ch/twiki/bin/view/CMS/BTagSFMethods
// This method corresponds to '1d', whereas ReweighterBTag corresponds to '1a'

#include "../interface/ReweighterBTagShape.h"

// include c++ library classes
#include "stdexcept"

// include other parts of the framework

/// constructor ///
ReweighterBTagShape::ReweighterBTagShape( const std::string& sfFilePath, 
					    const std::string& flavor,
					    const std::string& bTagAlgo,
					    const std::vector<Sample>& samples,
					    const std::vector<std::string>& variations,
					    const std::string& weightDirectory )
{

    // check the arguments
    if( !( flavor=="heavy" || flavor=="light" || flavor=="all" ) ){
	throw std::invalid_argument( std::string("ERROR in ReweighterBTagShape: ")
		+ "argument 'flavor' is '" + flavor + "' while it should be "
		+ "'heavy', 'light', or 'all'.");
    }
    _flavor = flavor;
    if( !( bTagAlgo=="deepCSV" || bTagAlgo=="deepFlavor" ) ){
	throw std::invalid_argument( std::string("ERROR in ReweighterBTagShape: ")
                + "argument 'bTagAlgo' is '" + bTagAlgo + "' while it should be "
                + "'deepCSV' or 'deepFlavor'.");
    }
    _bTagAlgo = bTagAlgo;
    std::vector<std::string> allowedvar = {"jes","hf","lf","hfstats1","hfstats2",
					"lfstats1","lfstats2","cferr1","cferr2",
					"jesAbsoluteMPFBias", "jesAbsoluteScale", "jesAbsoluteStat",
					"jesRelativeBal", "jesRelativeFSR", "jesRelativeJEREC1", 
					"jesRelativeJEREC2", "jesRelativeJERHF", 
					"jesRelativePtBB", "jesRelativePtEC1", "jesRelativePtEC2", 
					"jesRelativePtHF", 
					"jesRelativeStatEC","jesRelativeStatFSR","jesRelativeStatHF",
					"jesPileUpDataMC", "jesPileUpPtBB", "jesPileUpPtEC1", 
					"jesPileUpPtEC2", "jesPileUpPtHF", "jesPileUpPtRef",
					"jesFlavorQCD", "jesFragmentation", "jesSinglePionECAL", 
					"jesSinglePionHCAL", "jesTimePtEta" };
    std::vector<std::string> allowedsys = {"hf","lf","hfstats1","hfstats2",
                                        "lfstats1","lfstats2","cferr1","cferr2"};
    // (note: allowedsys must be a subcollection of allowedvar, excluding jec variations)
    // (note: maybe a way to extract variations in csv file automatically?)
    _variations = std::vector<std::string>();
    _systematics = std::vector<std::string>();
    for( std::string variation: variations ){
	if( std::find(allowedvar.begin(),allowedvar.end(),variation)==allowedvar.end() ){
	    throw std::invalid_argument( std::string("ERROR in ReweighterBTagShape: ")
                + "argument 'variations' contains '" + variation + "' "
                + "which is not recognized.");
	}
	_variations.push_back( variation );
	if( std::find(allowedsys.begin(),allowedsys.end(),variation)!=allowedsys.end() ){
	    _systematics.push_back( variation );
	}
    }
    _weightDirectory = weightDirectory;
    for( Sample sample: samples){
        std::string sampleName = sample.fileName();
        _normFactors[sampleName][0] = 1.; // initialize one element in each map
    }
 
    // set the the working point to "reshaping"
    BTagEntry::OperatingPoint wp = BTagEntry::OP_RESHAPING;

    // make the scale factor reader
    std::vector<std::string> sys_ext;
    for( std::string sys: _variations ){
	sys_ext.push_back("up_"+sys);
	sys_ext.push_back("down_"+sys);
    }
    bTagSFReader.reset( new BTagCalibrationReader( wp, "central", sys_ext ) );

    // set the type of scale factors to be extracted
    std::string fitMethod = "iterativefit";

    // calibrate the reader
    bTagSFCalibration = std::shared_ptr< BTagCalibration >( new BTagCalibration( "", sfFilePath ));
    if( _flavor=="heavy" || _flavor=="all" ){
	bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_B, fitMethod );
	bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_C, fitMethod );
    }
    if( _flavor=="light" || _flavor=="all" ){
	bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_UDSG, fitMethod );
    }
}

bool ReweighterBTagShape::hasVariation( const std::string& variation ) const{
    if( std::find(_variations.begin(),_variations.end(),variation)
	==_variations.end() ) return false;
    return true;
}

// following function is deprecated, do not use anymore!
// instead, determine normalization factor manually for every case separately
// and set it using the setter function below.
/*void ReweighterBTagShape::initialize( const std::vector<Sample>& samples ){

    std::cout << "initializing ReweighterBTagShape" << std::endl;
    // loop over samples
    for( Sample sample: samples){
	std::string pathToFile = sample.filePath();
	std::string sampleName = sample.fileName();
	// initialize the normalization factor for this sample to one
	_normFactors[sampleName] = 1.;
	// calculate the sum of weights for this sample
	// and update the normalization factor
	_normFactors[sampleName] = writeSumOfWeights( sample, _weightDirectory, 0, false );
	// print some info
	std::cout << "normalization factor for sample " << sampleName << ": ";
	std::cout << _normFactors[sampleName] << std::endl;
    }
    std::cout << "done initializing ReweighterBTagShape" << std::endl;
}*/

void ReweighterBTagShape::setNormFactors( const Sample& sample, 
			    const std::map<int,double> normFactors ){
    std::string sampleName = sample.fileName();
    if( _normFactors.find(sampleName)==_normFactors.end() ){
	throw std::invalid_argument(std::string("ERROR: ")
	    + "ReweighterBTagShape was not initialized for this sample!");
    }
    _normFactors[sampleName] = normFactors;
}

double ReweighterBTagShape::getNormFactor( const Event& event, 
					    const std::string& jecVariation ) const{
    // note: jecVariation has a default value: 'nominal', i.e. no variation of jec
    std::string sampleName = event.sample().fileName();
    int njets = event.getJetCollection( jecVariation ).size();
    for( int n=njets; n>=0; n-- ){
	if(_normFactors.at(sampleName).find(n)!=_normFactors.at(sampleName).end()){
	    return _normFactors.at(sampleName).at(n);
	}
    }
    throw std::invalid_argument(std::string("ERROR: ")
	    + "ReweighterBTagShape got event for which no norm factor could be retrieved.");
}

// note: the function below is already contained in ReweighterBTag, but copy here for reference
/*BTagEntry::JetFlavor jetFlavorEntry( const Jet& jet ){
    if( jet.hadronFlavor() == 0 ){
        return BTagEntry::FLAV_UDSG;
    } else if( jet.hadronFlavor() == 4 ){
        return BTagEntry::FLAV_C;
    } else if( jet.hadronFlavor() == 5 ){
        return BTagEntry::FLAV_B;
    } else {
        throw std::invalid_argument( "Jet has hadron flavor " + std::to_string( jet.hadronFlavor() ) + ", while expected numbers are 0, 4 or 5." );
    }
}*/

/// member functions for weights ///

double ReweighterBTagShape::weight( const Jet& jet, const std::string& uncertainty ) const{

    std::string sys = uncertainty;
    // check if jet is of correct flavor for this reweighter and systematic
    if( jet.hadronFlavor()==5 ){
	// case of b-jet
	if( !(_flavor=="heavy" || _flavor=="all") ) return 1;
	if( stringTools::stringContains(sys,"cferr") ) sys = "central";
    }
    if( jet.hadronFlavor()==4 ){
	// case of c-jet
	if( !(_flavor=="heavy" || _flavor=="all") ) return 1;
        if( !stringTools::stringContains(sys,"cferr") ) sys = "central";
    }
    else{
        // case of udsg-jet
        if( !(_flavor=="light" || _flavor=="all") ) return 1;
        if( stringTools::stringContains(sys,"cferr") ) sys = "central";
    }
    // make sure jet is in b-tag acceptance
    if( ! jet.inBTagAcceptance() ){
        return 1.;
    }
    // determine bTagScore (note: maybe modify later to the model of bTagReweighter,
    // which is probably faster as it does not involve evaluating this string for every jet)
    double bTagScore = (_bTagAlgo=="deepFlavor")?jet.deepFlavor():
			(_bTagAlgo=="deepCSV")?jet.deepCSV():-99; 
    // (in principle no checking for other values is needed, as already done in constructor)

    // note: https://twiki.cern.ch/twiki/bin/view/CMS/BTagCalibration#Using_b_tag_scale_factors_in_you
    // this page recommends to use absolute value of eta, but BTagCalibrationStandalone.cc
    // seems to handle negative values of eta more correctly (only taking abs when needed)
    double scaleFactor = bTagSFReader->eval_auto_bounds( sys, jetFlavorEntry( jet ),
					jet.eta(), jet.pt(), bTagScore );
    return scaleFactor;
}

double ReweighterBTagShape::weight( const Jet& jet ) const{
    return weight( jet, "central" );
}

double ReweighterBTagShape::weightUp( const Jet& jet, const std::string& systematic ) const{
    return weight( jet, "up_"+systematic );
}

double ReweighterBTagShape::weightDown( const Jet& jet, const std::string& systematic ) const{
    return weight( jet, "down_"+systematic );
}

double ReweighterBTagShape::weight( const Event& event ) const{
    // loop over jets and multiply jet weights
    double weight = 1.;
    for( const auto& jetPtr: event.jetCollection().goodJetCollection() ){ 
	weight *= this->weight( *jetPtr );
    }
    // take into account normalization
    double normweight = weight/getNormFactor(event);
    // prints for debugging:
    //std::cout << "raw weight: " << weight << std::endl;
    //std::cout << "normalized weight: " << normweight << std::endl;
    return normweight;
}

double ReweighterBTagShape::weightNoNorm( const Event& event ) const{
    // same as weight but no normalization factor (mainly for testing)
    double weight = 1.;
    for( const auto& jetPtr: event.jetCollection().goodJetCollection() ){
        weight *= this->weight( *jetPtr );
    }
    return weight;
}

double ReweighterBTagShape::weightJecVar( const Event& event, 
					    const std::string& jecVariation ) const{
    // same as weight but with propagation of jec variations
    // jecvar is expected to be of the form e.g. AbsoluteScaleUp or AbsoluteScaleDown
    // special case JECUp and JECDown (for single variations) are also allowed
    std::string jecVar = stringTools::removeOccurencesOf(jecVariation,"JEC");
    std::string varName;
    bool isup = true;
    if( stringTools::stringEndsWith(jecVar,"Up") ){
        varName = "jes"+jecVar.substr(0, jecVar.size()-2);
    } else if( stringTools::stringEndsWith(jecVar,"Down") ){
        varName = "jes"+jecVar.substr(0, jecVar.size()-4);
	isup = false;
    }
    if( !hasVariation(varName) ){
	throw std::invalid_argument(std::string("### ERROR ### in ReweighterBTagShape::weightJecVar:")
		    + " jec variation '"+jecVariation+"' (corresponding to '"+varName+"') not valid");
    }
    double weight = 1.;
    for( const auto& jetPtr: event.getJetCollection(jecVariation) ){
        if(isup) weight *= this->weightUp( *jetPtr, varName );
	else weight *= this->weightDown( *jetPtr, varName );
    }
    return weight;
}

double ReweighterBTagShape::weightUp( const Event& event, 
					const std::string& systematic ) const{
    // loop over jets and multiply jet weights
    double weight = 1.;
    for( const auto& jetPtr: event.jetCollection().goodJetCollection() ){
	weight *= this->weightUp( *jetPtr, systematic );
    }
    // take into account normalization
    weight /= getNormFactor(event);
    return weight;
}

double ReweighterBTagShape::weightDown( const Event& event, 
					const std::string& systematic ) const{
    // loop over jets and multiply jet weights
    double weight = 1.;
    for( const auto& jetPtr: event.jetCollection().goodJetCollection() ){
	 weight *= this->weightDown( *jetPtr, systematic );
    }
    // take into account normalization
    weight /= getNormFactor(event);
    return weight;
}

// functions below are deprecated, do not use anymore!
// instead, determine normalization factor for each case separately
// and set it manually using setNormFactor function above!
/*double ReweighterBTagShape::writeSumOfWeights( const Sample& sample,
					    const std::string& outputDirectory,
					    long unsigned numberOfEntries,
					    bool doWrite ) const{
    // note: numberOfEntries defaults to 0, in which case all entries in the file are used

    // make a TreeReader
    std::string inputFilePath = sample.filePath();
    std::cout << "making TreeReader..." << std::endl;
    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );

    // initialize the histograms
    std::cout << "initializing histograms..." << std::endl;
    std::shared_ptr<TH1D> bWeightHist = std::make_shared<TH1D>(
        "bWeightHist",
        "bWeightHist;reweighting factor;number of events",
        100,0,5);
    std::shared_ptr<TH1D> bWeightYieldHist = std::make_shared<TH1D>(
        "bWeightYieldHist",
        "bWeightYieldHist;reweighting factor;number of events",
        1,0,1);

    // avg weight (used for testing)
    double avgweight = 0;

    // loop over events
    long unsigned availableEntries = treeReader.numberOfEntries();
    if( numberOfEntries==0 ) numberOfEntries = availableEntries;
    else numberOfEntries = std::min(numberOfEntries, availableEntries);
    std::cout << "starting event loop for " << numberOfEntries << " events..." << std::endl;
    for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
        Event event = treeReader.buildEvent( entry );

        // do basic jet cleaning
        event.cleanJetsFromFOLeptons();
        event.jetCollection().selectGoodJets();

        // determine b-tag reweighting
        double btagreweight = this->weight( event );
	avgweight += btagreweight/numberOfEntries;

        // fill the histograms
        bWeightHist->Fill( btagreweight );
        bWeightYieldHist->Fill( 0.5, btagreweight/numberOfEntries );
    }
    
    std::cout << "finished event loop, found average weight of " << avgweight << std::endl;
    if( !doWrite ) return avgweight;

    // write output file
    std::cout << "writing output file..." << std::endl;
    systemTools::makeDirectory( outputDirectory );
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory)
                                + treeReader.currentSample().fileName();
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str(), "RECREATE" );
    bWeightHist->Write();
    bWeightYieldHist->Write();
    outputFilePtr->Close();
    return avgweight;
}

double ReweighterBTagShape::readSumOfWeights( const Sample& sample,
					    const std::string& weightFileDirectory,
					    long unsigned numberOfEntries ) const{
    // note: numberOfEntries defaults to 0, in which case all entries in the file are used
    // (only relevant if the weight file does not exist yet)

    // WARNING: default behaviour at this point is to only make new if not already existing.
    // BUT: this behaviour is overwritten in ReweighterBTagShap::initialize

    // check if a sum-of-weights file for this input file already exists
    bool fileExists = false;
    std::string weightFilePath = stringTools::formatDirectoryName(weightFileDirectory);
    weightFilePath += sample.fileName();
    fileExists = systemTools::fileExists( weightFilePath );

    // if the weight file for this input file does not exist, make it
    if( !fileExists ) writeSumOfWeights( sample, weightFileDirectory, numberOfEntries );

    // check again if file exists now (expect this to be the case)
    fileExists = systemTools::fileExists( weightFilePath );
    if( !fileExists ){
        throw std::runtime_error(std::string("ERROR in BTagShapeNormalizer:")
                + "file '"+weightFilePath+"' should have been created but it is not found.");
    }

    // read the file and get the value
    TFile* weightFilePtr = TFile::Open( weightFilePath.c_str() );
    std::shared_ptr<TH1> bWeightYieldHist( dynamic_cast<TH1*>(
        weightFilePtr->Get("bWeightYieldHist") ) );
    double sumOfWeights = bWeightYieldHist->GetBinContent(1);

    // return this value
    return sumOfWeights;
}*/
