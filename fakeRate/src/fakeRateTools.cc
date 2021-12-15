#include "../interface/fakeRateTools.h"


//include c++ library classes 
#include <stdexcept>
#include <iostream>
#include <set>
#include <algorithm>

//include ROOT classes 
#include "TH1.h"
#include "TROOT.h"

//include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/analysisTools.h"
#include "../../Tools/interface/ConstantFit.h"
#include "../../plotting/plotCode.h"

/****************************************************************************
general help functions for fitTriggerPresacles_cut and produceFakeRateMap_cut
****************************************************************************/
 
void fakeRate::checkFlavorString( const std::string& flavorString ){
    // check if flavor is either "muon" or "electron"
    if( !( flavorString == "muon" || flavorString == "electron" ) ){
        throw std::invalid_argument( "Flavor string is '" + flavorString 
		+ "' while it must be either 'muon' or 'electron'" );
    }
}

std::vector< std::string > fakeRate::listHistogramNamesInFile( TFile* filePtr ){
    // return a list of names of histograms (in std::string format) in a file
    std::vector< std::string > histogramNameList;
    for( const auto& key : *filePtr->GetListOfKeys() ){
        std::string keyName = key->GetName();
        //check if key corresponds to a histogram
        if( dynamic_cast< TH1* >( filePtr->Get( keyName.c_str() ) ) ){
            histogramNameList.push_back( keyName );
        }
    }
    return histogramNameList;
}

bool histIsPrompt( const std::string& histName ){
    return ( stringTools::stringContains( histName, "prompt" ) 
	     && !stringTools::stringContains( histName, "nonprompt" ) );
}

bool histIsNonPrompt( const std::string& histName ){
    return ( stringTools::stringContains( histName, "nonprompt" ) );
}

bool histIsData( const std::string& histName ){
    return stringTools::stringContains( histName, "data" );
}


/****************************************
help functions for producePrescaleMap_cut
****************************************/
 
std::string fakeRate::extractTriggerName( const std::string& histogramName ){
    // get the trigger name from a histogram name;
    // assumes: 
    // - histogram names ends with the trigger name
    // - trigger name starts with "HLT"
    auto beginPos = histogramName.find( "HLT" );
    return histogramName.substr( beginPos );
}

std::vector< std::string > fakeRate::listTriggersWithHistogramInFile( TFile* filePtr ){
    // get a list of triggers (in std::string format) present in histograms in a file
    std::set< std::string > triggerSet;
    std::vector< std::string > histogramNameVector = fakeRate::listHistogramNamesInFile( filePtr );
    for( const auto& name : histogramNameVector ){
        // if the histogram contains a triggername, add the trigger name to the list
        if( stringTools::stringContains( name, "HLT" ) ){
            triggerSet.insert( extractTriggerName( name ) );
        }
    }
    return std::vector< std::string >( triggerSet.cbegin(), triggerSet.cend() );
}

std::string fakeRate::extractYear( const std::string& histogramName ){
    // get the year from a histogram name
    if( stringTools::stringContains( histogramName, "2016" ) ) return "2016";
    else if( stringTools::stringContains( histogramName, "2017" ) ) return "2017";
    else if( stringTools::stringContains( histogramName, "2018" ) ) return "2018";
    else{
        throw std::invalid_argument( "histogram name '" + histogramName 
		    + "' does not contain a valid year tag (2016, 2017 or 2018)" );
    }
}

/********************************************************************************
function to fit prescales from a file containing prescale measurement  histograms
********************************************************************************/
std::map< std::string, Prescale > fakeRate::fitTriggerPrescales_cut( TFile* filePtr, 
					const double min, const double max, 
					const bool doPlot, const bool doSave ){
    
    // make vectors of all available trigger names and histogram names in a file
    std::vector< std::string > triggerNames = listTriggersWithHistogramInFile( filePtr );
    std::vector< std::string > histogramNames = listHistogramNamesInFile( filePtr );

    // check which year the histograms belong to, and check the consistency
    std::string year = extractYear( histogramNames.front() );
    for( const auto& histogram : histogramNames ){
        if( !stringTools::stringContains( histogram, year ) ){
            throw std::invalid_argument( "histogram name '" + histogram 
		    + "' does not contain year '" + year + "'" );
        }
    }

    // initialize the prescale map    
    std::map< std::string, Prescale > prescaleMap;

    // initialize the output file
    std::string outputFileName = "prescaleMeasurementPlots_" + year + ".root";
    if( doSave ){
        TFile* outputFilePtr = TFile::Open( outputFileName.c_str(), "RECREATE" );
	outputFilePtr->Close();
    }

    // loop over all triggers
    for( const auto& trigger : triggerNames ){
	
	// initialize map of process tags to prompt histograms and data histogram
	std::map< std::string, std::shared_ptr< TH1D > > prompt_histograms;
	std::map< std::string, std::shared_ptr< TH1D > > nonprompt_histograms;
	std::vector< std::string > prompt_processtags;
	std::vector< std::string > nonprompt_processtags;
	std::shared_ptr< TH1D > prompt_total;
	std::shared_ptr< TH1D > nonprompt_total;
	std::shared_ptr< TH1D > data_histogram;

	// set individual histograms
        for( const auto& histogram : histogramNames ){
            if( !stringTools::stringContains( histogram, trigger ) ) continue;
            if( histIsPrompt( histogram ) || histIsNonPrompt(histogram) ){
		// find process tag
                std::string tag = stringTools::split( histogram, "_" )[0];
		// add the histogram to the correct map
		if( histIsPrompt( histogram) ){
		    prompt_processtags.push_back(tag);
		    prompt_histograms[tag] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( 
						filePtr->Get( histogram.c_str() ) ) );
		}
		else if( histIsNonPrompt(histogram) ){
		    nonprompt_processtags.push_back(tag);
		    nonprompt_histograms[tag] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >(
                                                filePtr->Get( histogram.c_str() ) ) );
		}
            } else if( histIsData( histogram ) ){
                data_histogram = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( 
					    filePtr->Get( histogram.c_str() ) ) );
            } else {
                throw std::invalid_argument( "histogram name '" + histogram 
			+ "' cannot be interpreted." );
            }
        }
    
	// sum individual prompt contributions to total
	prompt_total = std::shared_ptr< TH1D >( dynamic_cast<TH1D*> (
                                prompt_histograms[prompt_processtags[0]]->Clone() ) );
        for( auto it = std::next(prompt_processtags.begin()); 
		it != prompt_processtags.end(); ++it){
            std::string thistag = *it;
            prompt_total->Add( prompt_histograms[thistag].get() );
        }

	// sum individual nonprompt contributions to total
	nonprompt_total = std::shared_ptr< TH1D >( dynamic_cast<TH1D*> (
                                nonprompt_histograms[nonprompt_processtags[0]]->Clone() ) );
        for( auto it = std::next(nonprompt_processtags.begin()); 
		it != nonprompt_processtags.end(); ++it){
            std::string thistag = *it;
            nonprompt_total->Add( nonprompt_histograms[thistag].get() );
        }

        // divide data by the prompt contribution and then fit it
        // clone data histogram so the original can still be plotted later on
	std::shared_ptr< TH1D > ratio_histogram( dynamic_cast< TH1D* >( 
				    data_histogram->Clone() ) );
        ratio_histogram->Divide( prompt_total.get() );
	
        // make fit to the data
        ConstantFit fitInfo( ratio_histogram, min, max );
	
        
        prescaleMap[ trigger ] = Prescale( fitInfo );
	std::cout << "prescale for " << trigger << ": " << prescaleMap[trigger].value() << std::endl;

	if( doPlot || doSave ){
	    // further histogram processing before plotting or saving

	    // scale all prompt and nonprompt histograms by the prescale
	    for( auto it = prompt_processtags.begin(); it != prompt_processtags.end(); ++it){
		prompt_histograms[*it]->Scale( prescaleMap[trigger].value() );
	    }
	    for( auto it = nonprompt_processtags.begin(); it != nonprompt_processtags.end(); ++it){
                nonprompt_histograms[*it]->Scale( prescaleMap[trigger].value() );
            }
	    prompt_total->Scale( prescaleMap[trigger].value() );
	    nonprompt_total->Scale( prescaleMap[trigger].value() );

	    // vary prompt histogram within uncertainty on prescale measurement 
            std::shared_ptr< TH1D > systUnc( dynamic_cast< TH1D* >( prompt_total->Clone() ) );
            double prescaleFractionalUnc = fabs( prescaleMap[ trigger ].uncertaintySymmetric() 
                                                 / prescaleMap[ trigger ].value() );
            for( int b = 1; b < systUnc->GetNbinsX() + 1; ++b ){
                systUnc->SetBinContent( b , systUnc->GetBinContent(b) * prescaleFractionalUnc );
            }

	    // set correct names
	    std::string promptName = prompt_total->GetName();
	    promptName = "total" + promptName.substr( promptName.find("_prompt_") );
	    prompt_total->SetName( promptName.c_str() );
	    std::string nonpromptName = nonprompt_total->GetName();
            nonpromptName = "total" + nonpromptName.substr( nonpromptName.find("_nonprompt_") );
            nonprompt_total->SetName( nonpromptName.c_str() );
	    std::string uncName = systUnc->GetName();
	    uncName = "unc" + uncName.substr( uncName.find("_prompt_") );
	    systUnc->SetName( uncName.c_str() );

	    // set correct titles
	    prompt_total->SetTitle("Prompt");
	    nonprompt_total->SetTitle("Nonprompt");
	    systUnc->SetTitle("Uncertainty");
	    for( auto it = prompt_processtags.begin(); it != prompt_processtags.end(); ++it){
                prompt_histograms[*it]->SetTitle( (*it).c_str() );
            }
            for( auto it = nonprompt_processtags.begin(); it != nonprompt_processtags.end(); ++it){
                nonprompt_histograms[*it]->SetTitle( (*it).c_str() );
            }

	    if( doPlot ){
	    
		// make array for prompt histograms
		// first case where all histograms are added together
		TH1D* predictedHist[1] = { prompt_total.get() };
		std::string predictedName[2] = {"data", "prompt"};
		// now case where different processes are kept apart
		TH1D* predictedHists[prompt_processtags.size()];
		std::string predictedNames[prompt_processtags.size()+1];
		predictedNames[0] = "data";
		for(unsigned int i=0; i<prompt_processtags.size(); ++i){
		    predictedHists[i] = prompt_histograms[prompt_processtags[i]].get();
		    predictedNames[i+1] = prompt_processtags[i];
		}

		// make plot directory if it does not already exist 
		std::string outputDirectory_name = "prescaleMeasurementPlots_" + year;
		systemTools::makeDirectory( outputDirectory_name );

		// plot with all prompt histograms added together
		plotDataVSMC( data_histogram.get(), predictedHist, predictedName, 1, 
			    stringTools::formatDirectoryName( outputDirectory_name ) 
			    + trigger + "_prescaleMeasurement_" + year + ".pdf", 
			    "fakerate", false, false, "(13 TeV)", systUnc.get() ); 
		// plot with processes kept apart
		plotDataVSMC( data_histogram.get(), predictedHists, 
			    predictedNames, prompt_processtags.size(), 
			    stringTools::formatDirectoryName( outputDirectory_name ) 
			    + trigger + "_prescaleMeasurement_" + year + "_split.pdf", 
			    "fakerate", false, false, "(13 TeV)", systUnc.get() );		
	    }
	
	    if( doSave ){
		// store the resulting histograms 
		// (e.g. in order to plot them later on with a different plotting function)
		
		// open the file for adding content (it was already created above)
		TFile* outputFilePtr = TFile::Open( outputFileName.c_str(), "UPDATE" );
		// write all histograms
		for( auto it = prompt_processtags.begin(); it != prompt_processtags.end(); ++it){
		    prompt_histograms[*it]->Write();
		}
		for( auto it = nonprompt_processtags.begin(); it != nonprompt_processtags.end(); ++it){
		    nonprompt_histograms[*it]->Write();
		}
		prompt_total->Write();
		nonprompt_total->Write();
		systUnc->Write();
		data_histogram->Write();
		// close the file
		outputFilePtr->Close();
	    }
	}
    }
    return prescaleMap;
}


/****************************************************
help functions for produceFakeRateMap_cut (see below)
****************************************************/ 
std::string extractPtEtaString( const std::string& histName ){

    //string is expected to end in _pT_x_eta_y
    //find last occurence of pT and cut off string there
    //auto pos = histName.find_last_of( "pT" );
    auto pos = histName.rfind( "pT" );
    return histName.substr( pos, histName.size() );
}


std::vector< std::string > histogramListToPtEtaBinList( const std::vector< std::string >& histogramNameList ){
    std::set< std::string > binList;
    for( const auto& histName : histogramNameList ){
       binList.insert( extractPtEtaString( histName ) );
    }
    return std::vector< std::string >( binList.cbegin(), binList.cend() );
}

std::vector< std::string > listPtEtaBinsInFile( TFile* filePtr ){
    std::vector< std::string > histogramNameList = fakeRate::listHistogramNamesInFile( filePtr );
    return histogramListToPtEtaBinList( histogramNameList );
}


double ptBorder( const std::string& ptEtaBinName ){
    return std::stod( stringTools::split( ptEtaBinName, "_" )[1] );
}


double etaBorder( const std::string& ptEtaBinName ){
    return std::stod( stringTools::replace( stringTools::split( stringTools::split( ptEtaBinName, "eta" )[1], "_")[1], "p", "." ) );
}


std::pair< std::vector< double >, std::vector< double > > ptEtaBinNamesToBinVectors(  const std::vector< std::string > binNames, const bool isMuon ){
    std::set< double > ptBinSet;
    std::set< double > etaBinSet;
    
    for( const auto& binName : binNames ){
        double ptBin = ptBorder( binName );
        double etaBin = etaBorder( binName );
        ptBinSet.insert( ptBin );
        etaBinSet.insert( etaBin );
    }
    
    double currentMaxPt = *std::max_element( ptBinSet.begin(), ptBinSet.end() );
    double maxPt = ( ( 100. > currentMaxPt ) ? 100. : currentMaxPt + 20. );
    ptBinSet.insert( maxPt );

    double maxEta = ( isMuon ? 2.4 : 2.5 );
    etaBinSet.insert( maxEta );

    return { std::vector< double >( ptBinSet.begin(), ptBinSet.end() ), std::vector< double >( etaBinSet.begin(), etaBinSet.end() ) };
}


bool histIsNumerator( const std::string& histName ){
    return stringTools::stringContains( histName, "numerator" );
}


bool histIsDenominator( const std::string& histName ){
    return stringTools::stringContains( histName, "denominator" );
}

/******************************************************************************
function to produce a fake rate map from a file containing the right histograms
******************************************************************************/
std::shared_ptr< TH2D > fakeRate::produceFakeRateMap_cut( TFile* filePtr, 
							const double maxValue,
							const bool doPlot,
							const bool doSave ){

    // get histograms and pt/eta bins present in the file
    std::vector< std::string > ptEtaBinNames = listPtEtaBinsInFile( filePtr );
    std::vector< std::string > histogramNames = fakeRate::listHistogramNamesInFile( filePtr );

    // check which year the histograms belong to
    std::string year = extractYear( histogramNames.front() );

    // check whether the fake-rate is for muons or electrons
    bool isMuon = stringTools::stringContains( histogramNames.front(), "muon" );
    if( !isMuon && !stringTools::stringContains( histogramNames.front(), "electron" ) ){
        throw std::invalid_argument( histogramNames.front() 
		+ " corresponds to neither electrons nor muons" );
    }
    std::string flavorString = ( isMuon ? "muon" : "electron" );

    // initialize 2D histogram
    std::pair< std::vector< double >, std::vector< double > > ptEtaBins;
    ptEtaBins = ptEtaBinNamesToBinVectors( ptEtaBinNames, isMuon );
    std::shared_ptr<TH2D> fakeRateMap = std::make_shared< TH2D >( 
	"fake-rate", "fake-rate; p_{T} (GeV); |#eta|", 
	ptEtaBins.first.size() - 1, &ptEtaBins.first[0], 
	ptEtaBins.second.size() - 1, &ptEtaBins.second[0] );
    fakeRateMap->SetDirectory( gROOT );
    fakeRateMap->Sumw2();

    // initialize output file for intermediate histograms
    std::string outputFileName = "fakeRateMeasurementPlots_" + year +
                                    + (isMuon ? "_muon" : "_electron") + ".root";
    if( doSave ){
	TFile* outputFilePtr = TFile::Open( outputFileName.c_str(), "RECREATE" );
	outputFilePtr->Close();
    }

    // loop over pT and eta bins
    for( const auto& bin : ptEtaBinNames ){

	// intialize histograms for this bin
	std::shared_ptr< TH1D > data_hist_numerator;
	std::map< std::string, std::shared_ptr< TH1D > > prompt_hists_numerator;
	std::vector< std::string > prompt_tags_numerator;
	std::shared_ptr< TH1D > prompt_numerator;
	std::map< std::string, std::shared_ptr< TH1D > > nonprompt_hists_numerator;
        std::vector< std::string > nonprompt_tags_numerator;
        std::shared_ptr< TH1D > nonprompt_numerator;

	std::shared_ptr< TH1D > data_hist_denominator;
	std::map< std::string, std::shared_ptr< TH1D > > prompt_hists_denominator;
	std::vector< std::string > prompt_tags_denominator;
	std::shared_ptr< TH1D > prompt_denominator;
	std::map< std::string, std::shared_ptr< TH1D > > nonprompt_hists_denominator;
        std::vector< std::string > nonprompt_tags_denominator;
        std::shared_ptr< TH1D > nonprompt_denominator;

	// get the correct histograms for this bin
	for( const auto& name : histogramNames ){
	    std::string binName = stringTools::split( name, "_" + flavorString + "_" ).back();
            if( binName != bin ) continue;
	    if( histIsNumerator( name ) ){
		if( histIsData( name ) ){
		    data_hist_numerator = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( 
						filePtr->Get( name.c_str() ) ) );
		} else if( histIsPrompt( name ) ){
		    if(stringTools::stringContains(name,"QCD")) continue; 
		    // (leave out QCD from prompt, 
		    // see also fillFakeRateMeasurement where all leptons from QCD 
		    // are set to nonprompt manually.)
		    std::string tag = stringTools::split(name,"_")[0];
		    prompt_tags_numerator.push_back(tag);
		    prompt_hists_numerator[tag] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( 
						filePtr->Get( name.c_str() ) ) );
		} else if( histIsNonPrompt( name ) ){
		    std::string tag = stringTools::split(name,"_")[0];
                    nonprompt_tags_numerator.push_back(tag);
                    nonprompt_hists_numerator[tag] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >(
							filePtr->Get( name.c_str() ) ) );
		}
	    } else if( histIsDenominator( name ) ){
		if( histIsData( name ) ){
		    data_hist_denominator = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( 
						filePtr->Get( name.c_str() ) ) );
		} else if( histIsPrompt( name ) ){
		    std::string tag = stringTools::split(name,"_")[0];
                    prompt_tags_denominator.push_back(tag);
		    prompt_hists_denominator[tag] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( 
						filePtr->Get( name.c_str() ) ) );
		} else if( histIsNonPrompt( name ) ){
		    std::string tag = stringTools::split(name,"_")[0];
                    nonprompt_tags_denominator.push_back(tag);
                    nonprompt_hists_denominator[tag] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >(
							filePtr->Get( name.c_str() ) ) );
		}
	    } else {
		throw std::invalid_argument( "histogram " + name + 
			" is neither numerator nor denominator" );
	    }
	}
	
	// sum prompt histograms to get total prompt contribution
	prompt_numerator = std::shared_ptr< TH1D >( dynamic_cast<TH1D*> (
                            prompt_hists_numerator[prompt_tags_numerator[0]]->Clone() ) );
        for( auto it = std::next(prompt_tags_numerator.begin()); 
	    it != prompt_tags_numerator.end(); ++it){
            std::string thistag = *it;
            prompt_numerator->Add( prompt_hists_numerator[thistag].get() );
        }
	prompt_denominator = std::shared_ptr< TH1D >( dynamic_cast<TH1D*> (
                            prompt_hists_denominator[prompt_tags_denominator[0]]->Clone() ) );
        for( auto it = std::next(prompt_tags_denominator.begin()); 
            it != prompt_tags_denominator.end(); ++it){
            std::string thistag = *it;
            prompt_denominator->Add( prompt_hists_denominator[thistag].get() );
        }
	

	// same for nonprompt histograms
	nonprompt_numerator = std::shared_ptr< TH1D >( dynamic_cast<TH1D*> (
				nonprompt_hists_numerator[nonprompt_tags_numerator[0]]->Clone() ) );
        for( auto it = std::next(nonprompt_tags_numerator.begin());
            it != nonprompt_tags_numerator.end(); ++it){
            std::string thistag = *it;
            nonprompt_numerator->Add( nonprompt_hists_numerator[thistag].get() );
        }
        nonprompt_denominator = std::shared_ptr< TH1D >( dynamic_cast<TH1D*> (
				nonprompt_hists_denominator[nonprompt_tags_denominator[0]]->Clone() ) );
        for( auto it = std::next(nonprompt_tags_denominator.begin());
            it != nonprompt_tags_denominator.end(); ++it){
            std::string thistag = *it;
            nonprompt_denominator->Add( nonprompt_hists_denominator[thistag].get() );
        }


	// plot data and prompt histogram for both numerator and denominator
	if( doPlot ){
	
	    // make plot directory if it does not already exist 
	    std::string outputDirectory_name = "fakeRateMeasurementPlots";
	    systemTools::makeDirectory( outputDirectory_name );

	    // plot numerator with all prompt histograms added together
	    TH1D* predictedHist_numerator[1] = { prompt_numerator.get() };
	    std::string predictedName[2] = {"data", "prompt"};
	    plotDataVSMC( data_hist_numerator.get(), predictedHist_numerator, predictedName, 1, 
		stringTools::formatDirectoryName( outputDirectory_name ) + 
		( isMuon ? "muon_" : "electron_" ) + bin + "_numerator_fakeRateMeasurement" + 
		year + ".pdf", "fakerate", false, false, "(13 TeV)" ); 
	    // plot numerator with prompt histograms split per process
	    TH1D* predictedHists_numerator[prompt_tags_numerator.size()];
	    std::string predictedNames_numerator[prompt_tags_numerator.size()+1];
	    predictedNames_numerator[0] = "data";
	    for(unsigned int i=0; i<prompt_tags_numerator.size(); ++i){
		predictedHists_numerator[i] = prompt_hists_numerator[prompt_tags_numerator[i]].get();
		predictedNames_numerator[i+1] = prompt_tags_numerator[i];
	    }
	    plotDataVSMC( data_hist_numerator.get(), predictedHists_numerator, 
		predictedNames_numerator, prompt_tags_numerator.size(),
                stringTools::formatDirectoryName( outputDirectory_name ) +
                ( isMuon ? "muon_" : "electron_" ) + bin + "_numerator_fakeRateMeasurement" +
                year + "_split.pdf", "fakerate", false, false, "(13 TeV)" );
        
	    // plot denominator with all prompt histograms added together
	    TH1D* predictedHist_denominator[1] = { prompt_denominator.get() };
	    plotDataVSMC( data_hist_denominator.get(), predictedHist_denominator, predictedName, 1, 
		stringTools::formatDirectoryName( outputDirectory_name ) + 
		( isMuon ? "muon_" : "electron_" ) + bin + "_denominator_fakeRateMeasurement" + 
		year + ".pdf", "fakerate", false, false, "(13 TeV)" ); 
	    // plot denominator with prompt histograms split per process
	    TH1D* predictedHists_denominator[prompt_tags_denominator.size()];
            std::string predictedNames_denominator[prompt_tags_denominator.size()+1];
            predictedNames_denominator[0] = "data";
            for(unsigned int i=0; i<prompt_tags_denominator.size(); ++i){
                predictedHists_denominator[i] = prompt_hists_denominator[prompt_tags_denominator[i]].get();
                predictedNames_denominator[i+1] = prompt_tags_denominator[i];
            }
            plotDataVSMC( data_hist_denominator.get(), predictedHists_denominator, 
		predictedNames_denominator, prompt_tags_denominator.size(),
                stringTools::formatDirectoryName( outputDirectory_name ) +
                ( isMuon ? "muon_" : "electron_" ) + bin + "_denominator_fakeRateMeasurement" +
                year + "_split.pdf", "fakerate", false, false, "(13 TeV)" );
	}

	// store the histograms (e.g. for plotting later with a different plotting function)
	if( doSave ){
	    TFile* outputFilePtr = TFile::Open( outputFileName.c_str(), "UPDATE" );
	    for( auto it = prompt_tags_numerator.begin(); 
		it != prompt_tags_numerator.end(); ++it){
		prompt_hists_numerator[*it]->Write();
	    }
	    for( auto it = prompt_tags_denominator.begin();
		it != prompt_tags_denominator.end(); ++it){
		prompt_hists_denominator[*it]->Write();
	    }
      	    for( auto it = nonprompt_tags_numerator.begin(); 
                it != nonprompt_tags_numerator.end(); ++it){
                nonprompt_hists_numerator[*it]->Write();
            }
            for( auto it = nonprompt_tags_denominator.begin();
                it != nonprompt_tags_denominator.end(); ++it){
                nonprompt_hists_denominator[*it]->Write();
            }
	    prompt_numerator->Write();
	    prompt_denominator->Write();
	    nonprompt_numerator->Write();
	    nonprompt_denominator->Write();
	    outputFilePtr->Close();
	}

        // subtract prompt contamination from data
        data_hist_numerator->Add( prompt_numerator.get(), -1. );
        data_hist_denominator->Add( prompt_denominator.get(), -1. );

        // set negative bins to 0
        analysisTools::setNegativeBinsToZero( data_hist_numerator );
        analysisTools::setNegativeBinsToZero( data_hist_denominator );

        // set fake-rate to 0 if there are no entries 
        double fakeRate, fakeRateUncertainty;
        if( data_hist_numerator->GetSumOfWeights() < 1e-6 
	    || data_hist_denominator->GetSumOfWeights() < 1e-6){
            fakeRate = 0.;
            fakeRateUncertainty = 0.;
        } else{ 
            // divide remaining numerator and denominator and fit their ratio
            data_hist_numerator->Divide( data_hist_denominator.get() );
            ConstantFit fit( data_hist_numerator, 0, maxValue );
            fakeRate = fit.value();
            fakeRateUncertainty = fit.uncertainty();
        }
    
        // write to corresponding TH2 bin
        auto binIndex = fakeRateMap->FindBin( ptBorder( bin), etaBorder( bin ) );
	std::cout<<"bin: "<<ptBorder(bin)<<", "<<etaBorder(bin)<<std::endl;
	std::cout<<"content, error: "<<fakeRate<<", "<<fakeRateUncertainty<<std::endl;
        fakeRateMap->SetBinContent( binIndex, fakeRate );
        fakeRateMap->SetBinError( binIndex, fakeRateUncertainty );
    }
    return fakeRateMap;
}
