/*
=== b-tag shape reweighter ===
This is an alternative b-tagging reweighter appropriate when reweighting the shape.
It should be used instead of ReweighterBTag.cc (fixed working point reweighting)
whenever the shape of the b-tagging discriminant matters,
e.g. if the b-tagging scores of jets are used as MVA inputs.

For more information, see following links:
-  general overview: https://twiki.cern.ch/twiki/bin/view/CMS/BTagSFMethods
-  this specific method: https://twiki.cern.ch/twiki/bin/view/CMS/BTagShapeCalibration
This method corresponds to "1d", whereas ReweighterBTag corresponds to "1a".
*/


#include "../interface/ReweighterBTagShape.h"


/// constructor ///
ReweighterBTagShape::ReweighterBTagShape(   const std::string& weightDirectory,
					    const std::string& sfFilePath, 
					    const std::string& flavor,
					    const std::string& bTagAlgo,
					    const std::vector<std::string>& variations,
					    const std::vector<Sample>& samples )
    // input arguments:
    // - weightDirectory: path to the ewkino/weights folder
    // - sfFilePath: path to the scale factor csv file relative from ewkino/weights
    // - flavor: either "heavy", "light" or "all"
    // - bTagAlgo: either "deepFlavor" or "deepCSV"
    // - variations: vector of systematic variations to consider
    //		     see list of allowed values below.
    //		     note: they must be provided at initialization so their validity can be checked,
    //			   and to make sure only the needed variations are read from the csv file.
    // - samples: vector of Sample objects that will be reweighted
    //		  note: they must be provided at initialization so their normalization 
    //			can be initialized.
{

    std::cout << "creating a ReweighterBTagShape with following parameters:" << std::endl;
    std::cout << "... (useful printing to implement)" << std::endl;

    // set the flavor to the provided value if it is valid
    if( !( flavor=="heavy" || flavor=="light" || flavor=="all" ) ){
	throw std::invalid_argument( std::string("ERROR in ReweighterBTagShape: ")
		+ "argument 'flavor' is '" + flavor + "' while it should be "
		+ "'heavy', 'light', or 'all'.");
    }
    _flavor = flavor;

    // set the b-tagging algorithm to the provided value if it is valid
    if( !( bTagAlgo=="deepCSV" || bTagAlgo=="deepFlavor" ) ){
	throw std::invalid_argument( std::string("ERROR in ReweighterBTagShape: ")
                + "argument 'bTagAlgo' is '" + bTagAlgo + "' while it should be "
                + "'deepCSV' or 'deepFlavor'.");
    }
    _bTagAlgo = bTagAlgo;

    // define lists of valid "variations" and "systematics"
    // note: "variations" are all varied scale factors present in the csv file;
    //       they consist of "systematics" on the scale factors
    //       (needed to determine an uncertainty on the b-tag reweighting)
    //       and jet energy variations
    //	     (needed only to propagate jet energy variations to central b-tag weights!)
    // note: maybe find a way to read these from the csv file 
    //       instead of copying them from the twiki mentioned above?
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
    _variations = std::vector<std::string>();
    _systematics = std::vector<std::string>();
    for( std::string variation: variations ){
	// check if provided variation is valid
	if( std::find(allowedvar.begin(),allowedvar.end(),variation)==allowedvar.end() ){
	    throw std::invalid_argument( std::string("ERROR in ReweighterBTagShape: ")
                + "argument 'variations' contains '" + variation + "' "
                + "which is not recognized." );
	}
	_variations.push_back( variation );
	// check if it is also a systematic
	if( std::find(allowedsys.begin(),allowedsys.end(),variation)!=allowedsys.end() ){
	    _systematics.push_back( variation );
	}
    }

    // initialize normalization factors
    for( Sample sample: samples){
        std::string sampleName = sample.fileName();
        _normFactors[sampleName][0] = 1.;
	// (initialize one element at 0 jets for each sample;
	// events with higher jet multiplicities will fall back to this default value)
    }
 
    // set the the working point to "reshaping"
    BTagEntry::OperatingPoint wp = BTagEntry::OP_RESHAPING;

    // make the scale factor reader
    std::vector<std::string> var_ext;
    for( std::string var: _variations ){
	var_ext.push_back("up_"+var);
	var_ext.push_back("down_"+var);
    }
    std::cout << "creating BTagCalibrationReader instance..." << std::endl;
    bTagSFReader.reset( new BTagCalibrationReader( wp, "central", var_ext ) );

    // set the type of scale factors to be extracted
    std::string fitMethod = "iterativefit";

    // calibrate the reader
    std::cout << "reading requested scale factors from csv file..." << std::endl;
    bTagSFCalibration = std::shared_ptr< BTagCalibration >( 
	new BTagCalibration( "", stringTools::formatDirectoryName(weightDirectory)+sfFilePath ) );
    if( _flavor=="heavy" || _flavor=="all" ){
	bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_B, fitMethod );
	bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_C, fitMethod );
    }
    if( _flavor=="light" || _flavor=="all" ){
	bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_UDSG, fitMethod );
    }

    std::cout << "done creating the ReweighterBTagShape instance." << std::endl;
}


/// initializer ///
void ReweighterBTagShape::initialize( const std::vector<Sample>& samples, 
					long unsigned numberOfEntries ){
    // initialize the reweighter for a collection of samples, i.e. set the normalization factors
    // input arguments:
    // - samples: vector of Sample objects
    // - numberOfEntries: maximum number of entries to take into account
    //                    (default value of 0 = all events) should be used
    // note: it is not very clear at what point the normalization factors should be determined...
    //       in principle, after applying all selections except for b-tag selections,
    //       but that is very hard to implement at this level since it depends on the 
    //       event selection for the specific use case. 
    //       it is also not very clear how to weight the events correctly for this normalization
    //       (all reweighting factors except for b-tag factors?);
    //       one can either use this function (which does basically no event selection 
    //	     and uses weight 1 for each entry) and assume this is "good enough", 
    //       OR, one has to manually calculate the sum of weights after appropriate selections,
    //       and then set the norm factors with setNormFactors (see below).
    std::cout << "initializing ReweighterBTagShape" << std::endl;
    // loop over samples
    for( Sample sample: samples){
        std::string pathToFile = sample.filePath();
        std::string sampleName = sample.fileName();
        // calculate the sum of weights for this sample (per jet multiplicity)
        // and update the normalization factor
        std::map<int, double> averageOfWeights = this->calcAverageOfWeights( sample, 
									     numberOfEntries );
        this->setNormFactors( sample, averageOfWeights );
    }
    std::cout << "done initializing ReweighterBTagShape" << std::endl;
}


/// help functions for checking a variation or systematic ///

bool ReweighterBTagShape::hasVariation( const std::string& variation ) const{
    // determine whether this instance has a given variation
    // note: the variation could be either a systematic uncertainty or a JEC variation
    if( std::find(_variations.begin(),_variations.end(),variation)
	==_variations.end() ) return false;
    return true;
}

bool ReweighterBTagShape::hasSystematic( const std::string systematic ) const{
    // determine whether this instance has a given systematic uncertainty
    if( std::find(_systematics.begin(),_systematics.end(),systematic)
        ==_systematics.end() ) return false;
    return true;
}

bool ReweighterBTagShape::considerSystematic( const Jet& jet, const std::string& systematic ) const{
    // check if a given systematic needs to be considered for a given jet
    std::vector<std::string> forbidden_systematics;
    if( jet.hadronFlavor()==5 || jet.hadronFlavor()==0 ){
	forbidden_systematics = {"cferr1", "cferr2"};
    } else if( jet.hadronFlavor()==4 ){
	forbidden_systematics = {"hf","lf","hfstats1","hfstats2","lfstats1","lfstats2"};
    }
    for( std::string sys: forbidden_systematics ){
        if( systematic==sys || systematic=="up_"+sys || systematic=="down_"+sys ) return false;
    }
    return true;
}


/// help functions for getting and setting normalization factors ///

void ReweighterBTagShape::setNormFactors( const Sample& sample, 
			    const std::map<int,double> normFactors ){
    // set the normalization factors
    // input arguments:
    // - sample: a Sample object for which to set the normalization
    // - normFactors: a map of jet multiplicity to averages-of-weights
    //                note: it is initialized to {0: 1.} in the constructor,
    //		      which implies the normalization factor will be 1 for each event.
    std::string sampleName = sample.fileName();
    if( _normFactors.find(sampleName)==_normFactors.end() ){
	throw std::invalid_argument(std::string("ERROR: ")
	    + "ReweighterBTagShape was not initialized for this sample!");
    }
    _normFactors[sampleName] = normFactors;
}

double ReweighterBTagShape::getNormFactor( const Event& event, 
					    const std::string& jecVariation ) const{
    // get the normalization factor for an event
    // note: the normalization factor depends on the sample to which the event belongs
    //       and on the jet multiplicity of the event.
    // note: jecVariation has a default value: 'nominal', i.e. no variation of JEC
    std::string sampleName = event.sample().fileName();
    // check validity of sample to which event belongs
    if( _normFactors.find(sampleName)==_normFactors.end() ){
        throw std::invalid_argument(std::string("ERROR: ")
            + "ReweighterBTagShape was not initialized for this sample!");
    }
    // determine number of jets
    int njets = event.getJetCollection( jecVariation ).size();
    // retrieve the normalization factor
    // note: if no normalization factor was initialized for this jet multiplicity,
    //	     the value for lower jet multiplicities is retrieved instead.
    for( int n=njets; n>=0; n-- ){
	if(_normFactors.at(sampleName).find(n)!=_normFactors.at(sampleName).end()){
	    return _normFactors.at(sampleName).at(n);
	}
    }
    throw std::invalid_argument(std::string("ERROR: ")
	    + "ReweighterBTagShape got event for which no norm factor could be retrieved.");
}


/// member functions for weights ///

double ReweighterBTagShape::weight( const Jet& jet, const std::string& variation ) const{
    // get the weight for a single jet
    // the weight is determined as follows:
    // - if this instance if for heavy flavor and the jet is light, weight = 1
    // - if this instance is for light flavor and the jet is heavy, weight = 1
    // - if the jet is outside b-tag acceptance, weight = 1
    // - else correct weight is read depending on flavor, eta, pt, b-tag score and systematic.
    
    std::string sys = variation;
    // check if variation is valid for this jet
    if( !this->considerSystematic( jet, variation ) ) sys = "central";
    // check if jet is of correct flavor for this reweighter
    if( jet.hadronFlavor()==5 || jet.hadronFlavor()==4 ){
	if( !(_flavor=="heavy" || _flavor=="all") ) return 1;
    } else{
        if( !(_flavor=="light" || _flavor=="all") ) return 1;
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


double ReweighterBTagShape::weight( const Event& event, const std::string& variation ) const{
    // get the weight for an event by multiplying individual jet weights
    // note: the nominal jet collection in the event is used;
    //       for the JEC variations: see below
    double weight = 1.;
    for( const auto& jetPtr: event.jetCollection().goodJetCollection() ){ 
	weight *= this->weight( *jetPtr, variation );
    }
    // take into account normalization
    double normweight = weight/getNormFactor(event);
    // prints for testing
    //std::cout << "raw weight: " << weight << std::endl;
    //std::cout << "normalized weight: " << normweight << std::endl;
    return normweight;
}

double ReweighterBTagShape::weight( const Event& event ) const{
    // get nominal weight for event
    return this->weight( event, "central" );
}

double ReweighterBTagShape::weightUp( const Event& event,
                                        const std::string& systematic ) const{
    // get up weight for event and given systematic 
    return this->weight( event, "up_"+systematic );
}

double ReweighterBTagShape::weightDown( const Event& event,
                                        const std::string& systematic ) const{
    // get down weight for event and given systematic
    return this->weight( event, "down_"+systematic );
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
	std::string msg = "### ERROR ### in ReweighterBTagShape::weightJecVar:";
	msg += " jec variation '"+jecVariation+"' (corresponding to '"+varName+"') not valid";
	throw std::invalid_argument(msg);
    }
    double weight = 1.;
    for( const auto& jetPtr: event.getJetCollection(jecVariation) ){
        if(isup) weight *= this->weightUp( *jetPtr, varName );
	else weight *= this->weightDown( *jetPtr, varName );
    }
    return weight;
}


/// help function for calculating normalization factors ///

std::map< int, double > ReweighterBTagShape::calcAverageOfWeights( const Sample& sample,
					      long unsigned numberOfEntries ) const{
    // calculate the average of b-tag weights in a given sample
    // the return type is a map of jet multiplicity to average of weights
    // input arguments:
    // - sample: a Sample object
    // - numberOfEntries: number of entries to consider for the average of weights
    //   note: defaults to 0, in which case all entries in the file are used
    // note: for the averaging, each entry in the input sample is counted as 1, 
    //       regardless of lumi, cross-section, generator weight or other reweighting factors!

    // make a TreeReader
    std::string inputFilePath = sample.filePath();
    std::cout << "making TreeReader..." << std::endl;
    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );

    // initialize the output map
    std::map< int, double > averageOfWeights;
    std::map< int, int > nEntries;

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

        // determine (nominal) b-tag reweighting and number of jets
        double btagreweight = this->weight( event );
	int njets = event.jetCollection().goodJetCollection().size();	

        // add it to the map
	if(averageOfWeights.find(njets)==averageOfWeights.end()){ 
	    averageOfWeights[njets] = btagreweight;
	    nEntries[njets] = 1;
	}
	else{
	    averageOfWeights[njets] += btagreweight;
	    nEntries[njets] += 1;
	}
    } 

    // divide sum by number to get average
    for( std::map<int,double>::iterator it = averageOfWeights.begin(); 
	    it != averageOfWeights.end(); ++it){
	averageOfWeights[it->first] = it->second / nEntries[it->first];
    }

    return averageOfWeights;
}
