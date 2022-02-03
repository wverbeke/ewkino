// import header
#include "../interface/prescaleMeasurementTools.h"

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

// function for filling prescale histograms for a single sample
void fillPrescaleMeasurementHistograms( const std::string& year,
	const std::string& sampleDirectoryPath, 
	const std::string& sampleListPath,
	const unsigned sampleIndex, 
	const bool isTestRun,
	const std::vector< std::string >& triggerVector,
	const bool useMT, const double metCut, double mtCut ){

    progressTracker progress = progressTracker("fillPrescaleMeasurement_progress_"+year
                                +"_sample_"+std::to_string(sampleIndex)+".txt");

    std::cout<<"start function fillPrescaleMeasurementHistograms"<<std::endl;
    analysisTools::checkYearString( year );

    // do some histogram initialization and map triggers to lepton pT and jet pT thresholds
    static constexpr unsigned numberOfBins = 16;
    static constexpr double maxBin = 160;
    HistInfo histInfo;
    if( useMT ){ histInfo = makeVarHistInfo( numberOfBins, mtCut, maxBin, true );}
    else { histInfo = makeVarHistInfo( numberOfBins, metCut, maxBin, false );}

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
    const bool isData = treeReader.isData();

    // make histograms for this sample
    std::map< std::string, std::shared_ptr< TH1D > > prompt_map;
    std::map< std::string, std::shared_ptr< TH1D > > nonprompt_map;
    std::map< std::string, std::shared_ptr< TH1D > > data_map;
    if( isData ){
        for( const auto& trigger : triggerVector ){
            data_map[trigger] = histInfo.makeHist( "data_mT_" + year + "_" + trigger );
        }
    }
    else{
        for( const auto& trigger : triggerVector ){
            prompt_map[trigger] = histInfo.makeHist( treeReader.currentSamplePtr()->processName()
                                                    + "_prompt_mT_" + year + "_" + trigger );
            nonprompt_map[trigger] = histInfo.makeHist( treeReader.currentSamplePtr()->processName()
                                                    + "_nonprompt_mT_" + year + "_" + trigger );
        }
    }

    // make reweighter
    // note: use 2016 reweighter for both 2016PreVFP and 2016PostVFP!
    //       maybe extend later.
    std::string reweighterYear = year;
    if( year=="2016PreVFP" || year=="2016PostVFP" ){ reweighterYear = "2016"; }
    std::shared_ptr< ReweighterFactory >reweighterFactory( new FourTopsFakeRateReweighterFactory() );
    // for testing:
    //std::shared_ptr< ReweighterFactory >reweighterFactory( new EmptyReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( "../weights/",
                                                        reweighterYear, treeReader.sampleVector() );
    
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( isTestRun ) numberOfEntries = 10000;
    std::cout<<"start event loop for "<<numberOfEntries<<" events"<<std::endl;
    for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
        if( entry%10000 == 0 ) progress.writeProgress( static_cast<double>(entry)/numberOfEntries );
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
        double weight = event.weight();
        if( !isData ) weight *= reweighter.totalWeight( event );
        else weight = 1;

	// temp: remove events with unphysical large weights
        // issue with unknown cause observed in a small number of generator weights 
        // in UL WJets samples
        if( weight>1e4 ){
            std::string msg = "WARNING: vetooing event with large weight.";
            msg += " (weight is " + std::to_string(weight) + ")";
            std::cerr << msg << std::endl;
            continue;
        }

	// loop over triggers
        for( const auto& trigger : triggerVector ){
	    // check if event passes trigger and if lepton is correct flavor
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
	    // check if lepton has correct pT for this trigger
            if( lepton.uncorrectedPt() <= leptonPtCutMap[trigger] ) continue;
	    // check if jet passes trigger requirements
            if( !fakeRate::passTriggerJetSelection( event, trigger, jetPtCutMap ) ) continue;

	    // fill correct histogram
            double valueToFill = ( useMT ? mT : event.metPt() );
            if( isData ){
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
    // write output histograms to file
    progress.close();
    std::cout<<"finished event loop"<<std::endl;
    std::string outfilename("prescaleMeasurement_");
    outfilename.append(useMT?"mT":"met");
    outfilename.append("_histograms_"+year+"_sample_"+std::to_string(sampleIndex)+".root");
    std::cout<<"writing to file "<<outfilename<<std::endl;
    TFile* histogram_file = TFile::Open( outfilename.c_str(), "RECREATE" );
    if( isData ){
        for( const auto& trigger : triggerVector ){
            data_map[ trigger ]->Write();
        }
    }
    else{
        for( const auto& trigger : triggerVector ){
            prompt_map[ trigger ]->Write();
            nonprompt_map[ trigger ]->Write();
        }
    }
    histogram_file->Close();
    std::cout<<"finished function fillPrescaleMeasurementHistograms"<<std::endl;
}
