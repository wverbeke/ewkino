// include header
#include "../interface/fakeRateMeasurementTools.h"

// help function for histogram info
HistInfo makeVarHistInfo( const unsigned numberOfBins, const double cut, const double max, 
			    const bool useMT){
    if( cut >= max ){
	std::string errorm("Cut should be smaller than ");
	errorm.append(std::to_string(cut));
	errorm.append("; while maximum allowed value is ");
	errorm.append(std::to_string(max));
	errorm.append("!");
        throw std::invalid_argument(errorm);
    }
    unsigned numberOfBinsToUse = static_cast< unsigned >( ( max - cut )/max * numberOfBins );
    if( useMT ){
        return HistInfo( "mT", "m_{T} (GeV)", numberOfBinsToUse, cut, max );
    } else {
        return HistInfo( "met", "E_{T}^{miss} (GeV)", numberOfBinsToUse, cut, max );
    }
}

// help function for creating a 2D histogram map
RangedMap< RangedMap< std::shared_ptr< TH1D > > > build2DHistogramMap( 
    const std::vector< double >& ptBinBorders, const std::vector< double >& etaBinBorders, 
    const HistInfo& mtHistInfo, const std::string& name ){

    std::map< double, RangedMap< std::shared_ptr< TH1D > > >  histMap2DTemp;
    for( auto ptBinBorder : ptBinBorders ){
        std::map< double, std::shared_ptr< TH1D > > histMapTemp;

        for( auto etaBinBorder : etaBinBorders ){ 
            histMapTemp[etaBinBorder] = mtHistInfo.makeHist( name + "_pT_" 
		+ std::to_string( int( ptBinBorder ) ) + "_eta_" 
		+ stringTools::replace( stringTools::doubleToString( etaBinBorder, 2 ), ".", "p" ) );
        }
        histMap2DTemp.insert( { ptBinBorder, RangedMap< std::shared_ptr< TH1D > >( histMapTemp ) } );
    }
    return RangedMap< RangedMap< std::shared_ptr< TH1D > > >( histMap2DTemp );
}

// help function for writing a 2D histogram map
void write2DHistogramMap( const RangedMap< RangedMap< std::shared_ptr< TH1D > > >& histMap ){
    for( auto& map_pair : histMap ){
        for( auto& hist_pair : map_pair.second ){
            hist_pair.second->Write();
        }
    }
}

// help function for creating a lepton reweigther
// NOTE: REQUESTED WEIGHT FILES ARE NOT PRESENT,
// THIS FUNCTION IS EWKINO SPECIFIC, DO NOT USE!
/*std::shared_ptr< Reweighter > makeLeptonReweighter( const std::string& year, const bool isMuon, 
    const bool isFO){

    std::string flavorString = ( isMuon ? "m" : "e" );
    std::string wpString = ( isFO ? "FO" : "3lTight" );
    const std::string weightDirectory = "../../weights/";
    std::string file_name = stringTools::formatDirectoryName(weightDirectory);
    file_name.append("weightFiles/leptonSF/leptonSF_" + flavorString);
    file_name.append("_" + year + "_" + wpString + ".root");
    TFile* leptonSFFile = TFile::Open( file_name.c_str() );
    std::shared_ptr< TH2 > leptonSFHist( dynamic_cast< TH2* >( leptonSFFile->Get( "EGamma_SF2D" ) ) );
    leptonSFHist->SetDirectory( gROOT );
    leptonSFFile->Close();

    if( isMuon ){
        if( isFO ){
            MuonReweighter muonReweighter( leptonSFHist, new FOSelector );
            return std::make_shared< ReweighterMuons >( muonReweighter );
        } else {
            MuonReweighter muonReweighter( leptonSFHist, new TightSelector );
            return std::make_shared< ReweighterMuons >( muonReweighter );
        }
    } else {
        if( isFO ){
            ElectronIDReweighter electronIDReweighter( leptonSFHist, new FOSelector );
            return std::make_shared< ReweighterElectronsID >( electronIDReweighter );
        } else {
            ElectronIDReweighter electronIDReweighter( leptonSFHist, new TightSelector );
            return std::make_shared< ReweighterElectronsID >( electronIDReweighter );
        }
        return nullptr;
    }
}*/

// function for filling prescale histograms for a single sample
void fillPrescaleMeasurementHistograms( const std::string& year, 
    const std::string& sampleDirectoryPath, const std::string& sampleListPath,
    const unsigned sampleIndex, const std::vector< std::string >& triggerVector, 
    const bool useMT, const double metCut, double mtCut){

    std::cout<<"start function fillPrescaleMeasurementHistograms"<<std::endl;
    analysisTools::checkYearString( year );

    static constexpr unsigned numberOfBins = 16;
    static constexpr double maxBin = 160;
    HistInfo histInfo;
    if( useMT ){ histInfo = makeVarHistInfo( numberOfBins, mtCut, maxBin, true );} 
    else { histInfo = makeVarHistInfo( numberOfBins, metCut, maxBin, false );}

    std::map< std::string, std::shared_ptr< TH1D > > prompt_map;
    std::map< std::string, std::shared_ptr< TH1D > > nonprompt_map;
    std::map< std::string, std::shared_ptr< TH1D > > data_map;

    for( const auto& trigger : triggerVector ){
        prompt_map[trigger] = histInfo.makeHist( "prompt_mT_" + year + "_" + trigger );
        nonprompt_map[trigger] = histInfo.makeHist( "nonprompt_mT_" + year + "_" + trigger );
        data_map[trigger] = histInfo.makeHist( "data_mT_" + year + "_" + trigger );
    }

    std::map<std::string,double> leptonPtCutMap = fakeRate::mapTriggerToLeptonPtThreshold( 
						    triggerVector );
    std::map<std::string,double> jetPtCutMap = fakeRate::mapTriggerToJetPtThreshold( 
						triggerVector );

    // initialize TreeReader and select correct sample
    std::cout<<"creating TreeReader and set to sample n. "<<sampleIndex<<std::endl;
    TreeReader treeReader( sampleListPath, sampleDirectoryPath);
    treeReader.initSample();
    for(unsigned idx=1; idx<=sampleIndex; ++idx){
	treeReader.initSample();
    }
    std::shared_ptr< ReweighterFactory >reweighterFactory( new EwkinoReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( "../../weights/", 
							year, treeReader.sampleVector() );
    
    //long unsigned numberOfEntries = 1000;
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    std::cout<<"start event loop for "<<numberOfEntries<<" events"<<std::endl;
    for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
        Event event = treeReader.buildEvent( entry, true, false );
	
	// check if event passes necessary selections
	// consider both electrons and muons, only tight leptons and events with a jet.
	if(!fakeRate::passFakeRateEventSelection(event,false,false,true,true,0.7,40)) continue;
        LightLepton& lepton = event.lightLepton(0);
        double mT = mt( lepton, event.met() );
        if( mT <= mtCut ) continue;
        if( event.metPt() <= metCut ) continue;
        if( mT > maxBin ) continue;

	// determine event weight
	// use scaledWeight to include xsection, lumi and sum of weights!
	double weight = event.scaledWeight();
	//std::cout<<"bare weight: "<<event.weight()<<std::endl;
	//std::cout<<"scaled weight: "<<event.scaledWeight()<<std::endl;
        if( event.isMC() ) weight *= reweighter.totalWeight( event );
	else weight = 1;

	for( const auto& trigger : triggerVector ){
	    if( !event.passTrigger( trigger ) ) continue;
	    if( stringTools::stringContains( trigger, "Mu" ) ){
		    if( !lepton.isMuon() ) continue;
	    } else if( stringTools::stringContains( trigger, "Ele" ) ){
		if( !lepton.isElectron() ) continue;
	    } else {
		std::string errorm("Can not measure prescale for trigger ");
		errorm.append(trigger);
		errorm.append(" since it is neither a muon nor electron trigger.");
                throw std::invalid_argument(errorm);
            }
	    if(lepton.uncorrectedPt() <= leptonPtCutMap[trigger]) continue;
	    if( !fakeRate::passTriggerJetSelection( event, trigger, jetPtCutMap ) ) continue;
	    double valueToFill = ( useMT ? mT : event.metPt() );
	    if( event.isData() ){
		data_map[ trigger ]->Fill( std::min( valueToFill, histInfo.maxBinCenter() ), weight );
	    } else {
		if( lepton.isPrompt() ){
		    prompt_map[ trigger ]->Fill( std::min( valueToFill, histInfo.maxBinCenter() ), 
		    				weight ); 
		} else {
		    nonprompt_map[ trigger ]->Fill( std::min( valueToFill, histInfo.maxBinCenter() ), 
							weight );
		}
	    }
	}
    }
    std::cout<<"finished event loop"<<std::endl;
    std::string outfilename("prescaleMeasurement_");
    outfilename.append(useMT?"mT":"met");
    outfilename.append("_histograms_"+year+"_sample_"+std::to_string(sampleIndex)+".root");
    std::cout<<"writing to file "<<outfilename<<std::endl;
    TFile* histogram_file = TFile::Open( outfilename.c_str(), "RECREATE" );
    for( const auto& trigger : triggerVector ){
	data_map[ trigger ]->Write();
	prompt_map[ trigger ]->Write();
	nonprompt_map[ trigger ]->Write();
    }
    histogram_file->Close();
    std::cout<<"finished function fillPrescaleMeasurementHistograms"<<std::endl;
}

// function for filling fake rate histograms for a single sample
void fillFakeRateMeasurementHistograms(const std::string& leptonFlavor, const std::string& year, 
    const std::string& sampleDirectory, const std::string& sampleList, const unsigned sampleIndex,
    const std::vector< std::string >& triggerVector, 
    const std::map< std::string, Prescale >& prescaleMap, double maxMT, double maxMet){ 
 
    std::cout<<"start function fillFakeRateMeasurementHistograms"<<std::endl;
    fakeRate::checkFlavorString( leptonFlavor );
    analysisTools::checkYearString( year );
    for( const auto& trigger : triggerVector ){
        if( prescaleMap.find( trigger ) == prescaleMap.end() ){
	    std::string errorm("Given vector of triggers contains triggers");
	    errorm.append("that are not present in the given prescale map.");
            throw std::invalid_argument(errorm);
        }
    }
    
    bool isMuonMeasurement = ( leptonFlavor == "muon" );

    std::vector< double > ptBinBorders;
    std::vector< double > etaBinBorders;

    if( isMuonMeasurement ){
        etaBinBorders = {0., 1.2, 2.1};
        ptBinBorders = { 10, 20, 30, 45, 65 };
    } else{
        etaBinBorders = {0., 0.8, 1.442};
        ptBinBorders = { 10, 20, 30, 45, 65 };
    }

    unsigned numberOfMTBins = 16; 
    HistInfo mtHistInfo( "mT", "m_{T}( GeV )", numberOfMTBins, 0., 160. );

    std::cout<<"start building histogram maps"<<std::endl;
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_numerator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, "prompt_numerator_mT_" + year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_denominator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, "prompt_denominator_mT_" + year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_numerator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, "nonprompt_numerator_mT_" + year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_denominator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, "nonprompt_denominator_mT_" + year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_numerator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, "data_numerator_mT_" + year + "_" + leptonFlavor);
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_denominator_map = build2DHistogramMap( 
	ptBinBorders, etaBinBorders, mtHistInfo, "data_denominator_mT_" + year + "_" + leptonFlavor );

    
    RangedMap< std::string > leptonPtToTriggerMap = fakeRate::mapLeptonPtToTriggerName( 
						    triggerVector, isMuonMeasurement );

    std::map< double, std::string > conePtLowerBoundMap;
    for( auto it = leptonPtToTriggerMap.cbegin(); it != leptonPtToTriggerMap.cend(); ++it ){
        double conePtBound;
        if( it == leptonPtToTriggerMap.cbegin() ){conePtBound = it->first;} 
	else {
            if( isMuonMeasurement ){conePtBound = 2*it->first;} 
	    else {conePtBound = 1.5*it->first;}
        }
	std::cout<<"cone pt bound: "<<conePtBound<<std::endl;
        conePtLowerBoundMap[ conePtBound ] = it->second;
    }
    RangedMap< std::string > conePtToTriggerMap( conePtLowerBoundMap );
    // temporary replacement corresponding to older code version:
    //RangedMap< std::string > conePtToTriggerMap = leptonPtToTriggerMap;

    std::map<std::string,double> triggerToJetPtMap = fakeRate::mapTriggerToJetPtThreshold(triggerVector);
    
    std::cout<<"initializing TreeReader and setting to sample n. "<<std::to_string(sampleIndex)<<std::endl;
    TreeReader treeReader( sampleList , sampleDirectory );
    treeReader.initSample();
    for(unsigned idx=1; idx<=sampleIndex; ++idx){
        treeReader.initSample();
    }
    std::cout<<"building reweighter"<<std::endl;
    std::shared_ptr< ReweighterFactory >reweighterFactory( new EwkinoReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( "../../weights/", year, 
					treeReader.sampleVector() );

    //reweighter.eraseReweighter( "muonID" );
    //reweighter.eraseReweighter( "electronID" );
    //std::shared_ptr< Reweighter > FOReweighter = makeLeptonReweighter( year, isMuonMeasurement, true );
    //std::shared_ptr< Reweighter > tightReweighter = makeLeptonReweighter( year, isMuonMeasurement, false );
    
    //long unsigned numberOfEntries = 200000;
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    std::cout<<"starting event loop for "<<numberOfEntries<<" events"<<std::endl;
    for(long unsigned entry=0; entry<numberOfEntries; ++entry){
	Event event = treeReader.buildEvent( entry, true, false );
	// original
	if( !fakeRate::passFakeRateEventSelection( event, isMuonMeasurement, 
		!isMuonMeasurement, false, true, 0.7, 30 ) ) continue;
	// temporary replacement corresponding to older code version:
	//if( !fakeRate::passFakeRateEventSelection( event, isMuonMeasurement,
	//	!isMuonMeasurement, false, true, 1.)) continue;

        LightLepton& lepton = event.lightLepton( 0 );

	if( lepton.pt() < 10 ) continue;

	const double pTFix = 35.;
	// temporary replacement corresponding to older code version:
	//const double pTFix = lepton.pt();
        PhysicsObject leptonFix( pTFix, lepton.eta(), lepton.phi(), lepton.energy() );
        double mT = mt( leptonFix, event.met() );

	if( mT >= maxMT ) continue;
        if( event.metPt() >= maxMet ) continue;

	std::string triggerToUse = conePtToTriggerMap[ lepton.pt() ];
	// temporary replacement corresponding to older code version
	//std::string triggerToUse = conePtToTriggerMap[ lepton.uncorrectedPt() ];
        if( !event.passTrigger( triggerToUse ) ) continue;

	if( !fakeRate::passTriggerJetSelection( event, triggerToUse, triggerToJetPtMap ) ) continue;

	double weight = event.scaledWeight();
	//std::cout<<"unscaled: "<<event.weight()<<",  scaled:"<<event.scaledWeight()<<std::endl;
        if( event.isMC() ){
            const Prescale& prescale = prescaleMap.find( triggerToUse )->second;
            weight *= prescale.value();
            weight *= reweighter.totalWeight( event );
	    //std::cout<<triggerToUse<<std::endl;
	    //std::cout<<lepton.pt()<<std::endl;
	    //std::cout<<"prescale: "<<prescale.value()<<", reweighter: "<<reweighter.totalWeight(event)<<std::endl;
        }

	if( lepton.isTight() ){
            double tightWeight = 1.;
            //if( event.isMC() ){tightWeight = tightReweighter->weight( event );}
	    if( treeReader.isData() ){
                data_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		    std::min( mT, mtHistInfo.maxBinCenter() ), weight );
            } else if( lepton.isPrompt() ){
                prompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		    std::min( mT, mtHistInfo.maxBinCenter() ), weight*tightWeight );
            } else {
                nonprompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		    std::min( mT, mtHistInfo.maxBinCenter() ), weight*tightWeight );
            }
        }

	//if( event.isMC() ){weight *= FOReweighter->weight( event );}

	if( treeReader.isData() ){
            data_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		std::min( mT, mtHistInfo.maxBinCenter() ), weight );
        } else if( lepton.isPrompt() ){
            prompt_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( 
		mT, mtHistInfo.maxBinCenter() ), weight );
        } else {
            nonprompt_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( 
		std::min( mT, mtHistInfo.maxBinCenter() ), weight );
        }
    }
    std::cout<<"finished event loop"<<std::endl;

    std::string file_name = "fakeRateMeasurement_" + leptonFlavor + "_" + year;
    file_name.append("_mT_histograms_sample_"+std::to_string(sampleIndex)+".root");
    TFile* histogram_file = TFile::Open( file_name.c_str(), "RECREATE" );
    
    write2DHistogramMap( prompt_numerator_map );
    write2DHistogramMap( prompt_denominator_map );
    write2DHistogramMap( nonprompt_numerator_map );
    write2DHistogramMap( nonprompt_denominator_map );
    write2DHistogramMap( data_numerator_map );
    write2DHistogramMap( data_denominator_map );

    histogram_file->Close();
    std::cout<<"finished function fillFakeRateMeasurementHistograms"<<std::endl;
}  

void fillMCFakeRateMeasurementHistograms(const std::string& flavor, const std::string& year, 
					    const std::string& sampleDirectory, 
					    const std::string& sampleList,
					    const unsigned sampleIndex){

    std::cout<<"starting function fillMCFakeRateMeasurementHistograms"<<std::endl;
    fakeRate::checkFlavorString( flavor );
    analysisTools::checkYearString( year );
    const bool isMuon =  ( flavor == "muon" );
    
    const std::vector< double > ptBins = {10., 20., 30., 45., 65., 100.};
    std::vector< double > etaBins;
    if( isMuon ){
        etaBins = { 0., 1.2, 2.1, 2.4 }; 
    } else {
	etaBins = { 0., 0.8, 1.442, 2.5 };
    }

    //initialize 2D histograms for numerator and denominator
    std::string numerator_name = "fakeRate_numerator_" + flavor + "_" + year;
    std::shared_ptr< TH2D > numeratorMap( 
	new TH2D(   numerator_name.c_str(), ( numerator_name+ "; p_{T} (GeV); |#eta|").c_str(), 
		    ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) 
    );
    numeratorMap->Sumw2();
    std::string denominator_name = "fakeRate_denominator_" + flavor + "_" + year;
    std::shared_ptr< TH2D > denominatorMap( 
	new TH2D(   denominator_name.c_str(), denominator_name.c_str(), 
		    ptBins.size() - 1, &ptBins[0], etaBins.size() - 1, &etaBins[0] ) 
    );
    denominatorMap->Sumw2();

    unsigned nbins = 10;
    double ptlow = 10.;
    double pthigh = 70.;
    numerator_name = "fakeRate_numerator_heavyflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > heavynumeratorMap(
        new TH1D(   numerator_name.c_str(), ( numerator_name+ "; p_{T} (GeV)").c_str(),
                    nbins, ptlow, pthigh)
    );
    heavynumeratorMap->Sumw2();
    denominator_name = "fakeRate_denominator_heavyflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > heavydenominatorMap(
        new TH1D(   denominator_name.c_str(), denominator_name.c_str(),
                    nbins, ptlow, pthigh)
    );
    heavydenominatorMap->Sumw2();

    numerator_name = "fakeRate_numerator_lightflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > lightnumeratorMap(
        new TH1D(   numerator_name.c_str(), ( numerator_name+ "; p_{T} (GeV)").c_str(),
                    nbins, ptlow, pthigh)
    );
    lightnumeratorMap->Sumw2();
    denominator_name = "fakeRate_denominator_lightflavor_" + flavor + "_" + year;
    std::shared_ptr< TH1D > lightdenominatorMap(
        new TH1D(   denominator_name.c_str(), denominator_name.c_str(),
                    nbins, ptlow, pthigh)
    );
    lightdenominatorMap->Sumw2();

    std::cout<<"making TreeReader and setting to sample no. "<<sampleIndex<<"."<<std::endl;
    // make TreeReader and set to correct sample
    TreeReader treeReader( sampleList, sampleDirectory );
    treeReader.initSample();
    for( unsigned i = 1; i <= sampleIndex; ++i ){
        treeReader.initSample();
    }

    // loop over events to fill histograms
    unsigned numberOfEntries = treeReader.numberOfEntries();
    //unsigned numberOfEntries = 1000; // for testing
    std::cout<<"start event loop for "<<numberOfEntries<<" events."<<std::endl;
    for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
        Event event = treeReader.buildEvent( entry );

        //apply fake-rate selection
	// arguments: event, onlyMuons, onlyElectrons, onlyTight, requireJet, deltaR, jetPt (def 25) 
        if( !fakeRate::passFakeRateEventSelection( event, isMuon, !isMuon, false, true, 0.7) ){
	    continue;
	}

        LightLepton& lepton = event.lightLeptonCollection()[ 0 ];

        //lepton should be nonprompt and should not originate from a photon
        if( lepton.isPrompt() ) continue;
        if( lepton.matchPdgId() == 22 ) continue;

	double weight = event.scaledWeight();
	//std::cout<<"unscaled: "<<event.weight()<<", scaled: "<<event.scaledWeight()<<std::endl;
        //fill denominator histogram 
        histogram::fillValues( denominatorMap.get(), lepton.pt(), lepton.absEta(), weight );
    
        //fill numerator histogram
        if( lepton.isTight() ){
            histogram::fillValues(numeratorMap.get(), lepton.pt(), lepton.absEta(), weight );
        }
    
	if(lepton.provenanceCompressed()==1 || lepton.provenanceCompressed()==2){
	    heavydenominatorMap.get()->Fill(lepton.pt(), weight);
	    if(lepton.isTight()) heavynumeratorMap.get()->Fill(lepton.pt(), weight);
	}
	else{
	    lightdenominatorMap.get()->Fill(lepton.pt(), weight);
            if(lepton.isTight()) lightnumeratorMap.get()->Fill(lepton.pt(), weight);
	}
    }
    std::cout<<"finished event loop"<<std::endl;

    std::string file_name = "fakeRateMeasurement_MC_" + flavor + "_" + year;
    file_name.append("_histograms_sample_"+std::to_string(sampleIndex)+".root");
    TFile* histogram_file = TFile::Open( file_name.c_str(), "RECREATE" );

    numeratorMap->Write();
    denominatorMap->Write();

    heavynumeratorMap->Write();
    heavydenominatorMap->Write();
    lightnumeratorMap->Write();
    lightdenominatorMap->Write();

    histogram_file->Close();
    std::cout<<"finished function fillMCFakeRateMeasurementHistograms"<<std::endl;
}   