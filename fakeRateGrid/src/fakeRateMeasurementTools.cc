// include header
#include "../interface/fakeRateMeasurementTools.h"

void fillMCFakeRateMeasurementHistograms( const std::string& flavor, const std::string& year, 
					    const std::string& sampleDirectory, 
					    const std::string& sampleList,
					    const unsigned sampleIndex,
					    double ptRatioCut, double deepFlavorCut, int extraCut,
					    const bool isTestRun, const long nEvents ){

    std::cout<<"starting function fillMCFakeRateMeasurementHistograms"<<std::endl;
    fakeRate::checkFlavorString( flavor );
    analysisTools::checkYearString( year );
    const bool isMuon =  ( flavor == "muon" );
    
    // define binning for 2D histograms
    const std::vector< double > ptBins = {10., 15., 20., 30., 45., 100.};
    std::vector< double > etaBins;
    if( isMuon ){
        etaBins = { 0., 1.2, 2.1, 2.4 }; 
    } else {
	etaBins = { 0., 0.8, 1.442, 2.5 };
    }

    // initialize 2D histograms for numerator and denominator
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

    // initialize 1D histograms for light and heavy flavour fake rate separately
    // (can be a useful tool to check if they are approximately equal, but not used further)
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

    // make TreeReader and set to correct sample
    std::cout<<"making TreeReader and setting to sample no. "<<sampleIndex<<"."<<std::endl;
    TreeReader treeReader( sampleList, sampleDirectory );
    treeReader.initSample();
    for( unsigned i = 1; i <= sampleIndex; ++i ){
        treeReader.initSample();
    }

    // loop over events to fill histograms
    unsigned numberOfEntries = treeReader.numberOfEntries();
    if( isTestRun){ numberOfEntries = 5000; }
    if( nEvents>0 && nEvents < numberOfEntries ){ numberOfEntries = (unsigned) nEvents; }
    std::cout<<"start event loop for "<<numberOfEntries<<" events."<<std::endl;
    for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
        Event event = treeReader.buildEvent( entry );
	if( isTestRun ){
	    /*std::cout << "-------------------------" << std::endl;
	    std::cout << "event ID: " << event.runNumber() << "/" << event.luminosityBlock();
	    std::cout << "/" << event.eventNumber() << std::endl;
	    for( auto lepton : event.leptonCollection() ){
		lepton->print();
		std::cout << std::endl;
	    }*/
	}

	// apply MET filters (not included in passFakeRateEventSelection!)
	if( !event.passMetFilters() ) continue;

        // apply fake-rate selection
	// arguments: event, onlyMuons, onlyElectrons, 
	//            onlyTight, ptRatioCut, deepFlavorCut, 
	//            requireJet, deltaR, jetPt (def 25) 
        if( !fakeRate::passFakeRateEventSelection( event, isMuon, !isMuon, 
						    false, 
						    ptRatioCut, deepFlavorCut, extraCut,
						    true, 0.7, 25.) ) continue;

        LightLepton& lepton = event.lightLeptonCollection()[ 0 ];

        // lepton should be nonprompt
        if( lepton.isPrompt() ) continue;
	// note: in the line below leptons from photons were excluded, 
	// but this doesn't seem to be correct
        //if( lepton.matchPdgId() == 22 ) continue;

	if( isTestRun ){
	    //std::cout << "event passed FO selection" << std::endl;
	    //std::cout << "event ID: " << event.runNumber() << "/" << event.luminosityBlock();
            //std::cout << "/" << event.eventNumber() << std::endl;
	    //std::cout << "(scaled) weight: " << event.weight() << std::endl;
	    //if( event.eventNumber()==3088713 ){
		//std::cout << lepton << std::endl;
	    //}
	}

	double weight = event.weight();

        // fill 2D denominator histogram 
        histogram::fillValues( denominatorMap.get(), lepton.pt(), lepton.absEta(), weight );
    
        // fill 2D numerator histogram
        if( lepton.isTight() ){
            histogram::fillValues(numeratorMap.get(), lepton.pt(), lepton.absEta(), weight );
        }
    
	// fill heavy flavour 1D histograms
	if(lepton.provenanceCompressed()==1 || lepton.provenanceCompressed()==2){
	    heavydenominatorMap.get()->Fill(lepton.pt(), weight);
	    if(lepton.isTight()) heavynumeratorMap.get()->Fill(lepton.pt(), weight);
	}

	// fill light flavour 1D histograms
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
