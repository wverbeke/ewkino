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
    const std::string& sampleDirectoryPath, const std::string& sampleListPath,
    const unsigned sampleIndex, const std::vector< std::string >& triggerVector,
    const bool useMT, const double metCut, double mtCut){

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
    std::shared_ptr< ReweighterFactory >reweighterFactory( new tZqReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( "../../weights/",
                                                        year, treeReader.sampleVector() );
    
    //long unsigned numberOfEntries = 500000;
    long unsigned numberOfEntries = treeReader.numberOfEntries();
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
        // use scaledWeight to include xsection, lumi and sum of weights!
        double weight = event.scaledWeight();
        //std::cout<<"bare weight: "<<event.weight()<<std::endl;
        //std::cout<<"scaled weight: "<<event.scaledWeight()<<std::endl;
        if( !isData ) weight *= reweighter.totalWeight( event );
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
            if( lepton.uncorrectedPt() <= leptonPtCutMap[trigger] ) continue;
            if( !fakeRate::passTriggerJetSelection( event, trigger, jetPtCutMap ) ) continue;
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

// function for getting a set of prescales from histograms 
// created with fillPrescaleMeasurementHistograms
// and plotting the results
std::map< std::string, Prescale > fitTriggerPrescales( TFile* filePtr,
                                                const double min, const double max,
                                                const bool doPlot ){

    // make vectors of triggers and histograms in file
    std::vector< std::string > triggerNames = fakeRate::listTriggersWithHistogramInFile( filePtr );
    std::vector< std::string > histogramNames = fakeRate::listHistogramNamesInFile( filePtr );
    // initialize set of process tags in file (filled dynamically below)
    std::map< std::string, std::vector< std::string > > processtags;

    // check which year the histograms belong to, and check the consistency
    std::string year = fakeRate::extractYear( histogramNames.front() );
    for( const auto& histogram : histogramNames ){
        if( !stringTools::stringContains( histogram, year ) ){
            throw std::invalid_argument( "histogram '"+histogram+"' does not contain year '"+year+"'" );
        }
    }

    // map trigger to map of process tags to corresponding histograms
    std::map< std::string, std::map< std::string, std::shared_ptr< TH1D > > > prompt_histograms;
    // same map but adding all MC histograms together
    std::map< std::string, std::shared_ptr< TH1D > > totalprompt_histograms;
    // map trigger to data histograms
    std::map< std::string, std::shared_ptr< TH1D > > data_histograms;

    for( const auto& trigger : triggerNames ){

        for( const auto& histogram : histogramNames ){
            if( !stringTools::stringContains( histogram, trigger ) ) continue;
            if( stringTools::stringContains( histogram, "nonprompt" ) ) continue;
            if( stringTools::stringContains( histogram, "prompt" ) ){
                // find process tag
                std::string tag = stringTools::split( histogram, "_" )[0];
                processtags[trigger].push_back(tag);
                // add prompt MC histogram to list for this trigger
                prompt_histograms[trigger][tag] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >(
                                                    filePtr->Get( histogram.c_str() ) ) );
            } else if( stringTools::stringContains( histogram, "data" ) ){
                // add data histogram for this trigger
                data_histograms[trigger] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >(
                                                    filePtr->Get( histogram.c_str() ) ) );
            } else {
                throw std::invalid_argument( "histogram name '" + histogram
                        + "' does not correspond to data or a prompt simulation." );
            }
        }
        // add all prompt MC histograms for this trigger
        totalprompt_histograms[trigger] = std::shared_ptr< TH1D >( dynamic_cast<TH1D*> (
                                            prompt_histograms[trigger][processtags[trigger][0]]->Clone() ) );
        for( auto it = std::next(processtags[trigger].begin()); it != processtags[trigger].end(); ++it){
            std::string thistag = *it;
            totalprompt_histograms[trigger]->Add( prompt_histograms[trigger][thistag].get() );
        }
    }
    
    // apply the fit to each histogram and store them in the returned map
    std::map< std::string, Prescale > prescaleMap;
    for( const auto& trigger: triggerNames ){

        // divide data by the prompt contribution and then fit it
        // clone data histogram so the original can still be plotted later on
        std::shared_ptr< TH1D > ratio_histogram( dynamic_cast< TH1D* >(
                                    data_histograms[ trigger ]->Clone() ) );
        ratio_histogram->Divide( totalprompt_histograms[ trigger ].get() );

        // make fit to the data
        ConstantFit fitInfo( ratio_histogram, min, max );

        prescaleMap[ trigger ] = Prescale( fitInfo );
    }

    if( !doPlot ) return prescaleMap;

    // plot the prescale measurements 
    // make plot directory if it does not already exist 
    std::string outputDirectory_name = "prescaleMeasurementPlots_" + year;
    systemTools::makeDirectory( outputDirectory_name );

    for( const auto& trigger : triggerNames ){

        // make arrays for prompt histograms and scale it by the prescale
        // first for the case where all MC is added together
        TH1D* totalPredictedHists[1] = { totalprompt_histograms[ trigger ].get() };
        totalPredictedHists[0]->Scale( prescaleMap[ trigger ].value() );
        std::string totalPredictedNames[2] = {"data", "prompt"};
        // now for the case where processes are kept apart
        TH1D* predictedHists[processtags[trigger].size()];
        std::string predictedNames[processtags[trigger].size()+1];
        predictedNames[0] = "data";
        for(unsigned int i=0; i<processtags[trigger].size(); ++i){
            predictedHists[i] = prompt_histograms[trigger][processtags[trigger][i]].get();
            predictedHists[i]->Scale( prescaleMap[trigger].value() );
            predictedNames[i+1] = processtags[trigger][i];
        }

        // vary prompt histogram within uncertainty on prescale measurement 
        TH1D* systUnc = dynamic_cast< TH1D* >( totalPredictedHists[0]->Clone() );
        double prescaleFractionalUnc = fabs( prescaleMap[ trigger ].uncertaintySymmetric()
                                                / prescaleMap[ trigger ].value() );
        for( int b = 1; b < systUnc->GetNbinsX() + 1; ++b ){
            systUnc->SetBinContent( b , systUnc->GetBinContent(b) * prescaleFractionalUnc );
        }

        // make plot for the case where all MC is added together
        plotDataVSMC( data_histograms[ trigger ].get(), totalPredictedHists, totalPredictedNames,
                        1, stringTools::formatDirectoryName( outputDirectory_name ) + trigger
                        + "_prescaleMeasurement_" + year + ".pdf", "", false, false, "(13 TeV)", systUnc );
        // make plot for the case where processes are kept apart
        plotDataVSMC( data_histograms[ trigger ].get(), predictedHists, predictedNames,
                        processtags[trigger].size(), stringTools::formatDirectoryName( outputDirectory_name )
                        + trigger + "_prescaleMeasurement_" + year + "_split.pdf", "prescale", false, false,
                        "(13 TeV)", systUnc );

    }
    return prescaleMap;
}

