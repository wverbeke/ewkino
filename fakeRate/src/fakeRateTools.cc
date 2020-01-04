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
#include "../interface/ConstantFit.h"
#include "../../plotting/plotCode.h"


void fakeRate::checkFlavorString( const std::string& flavorString ){
    if( !( flavorString == "muon" || flavorString == "electron" ) ){
        throw std::invalid_argument( "Flavor string is '" + flavorString + "' while it must be either 'muon' or 'electron'" );
    }
}


std::vector< std::string > fakeRate::listHistogramNamesInFile( TFile* filePtr ){
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


std::string extractTriggerName( const std::string& histogramName ){

    //assumes histogram names that end with the trigger name
    auto beginPos = histogramName.find( "HLT" );
    return histogramName.substr( beginPos );
}


std::vector< std::string > fakeRate::listTriggersWithHistogramInFile( TFile* filePtr ){
    std::set< std::string > triggerSet;

	//loop over all the histograms
    std::vector< std::string > histogramNameVector = fakeRate::listHistogramNamesInFile( filePtr );
    for( const auto& name : histogramNameVector ){

        //if the histogram contains a triggername, add the trigger name to the list
        if( stringTools::stringContains( name, "HLT" ) ){

            triggerSet.insert( extractTriggerName( name ) );
        }
    }
    return std::vector< std::string >( triggerSet.cbegin(), triggerSet.cend() );
}


std::string extractYear( const std::string& histogramName ){
    if( stringTools::stringContains( histogramName, "2016" ) ){
        return "2016";
    } else if( stringTools::stringContains( histogramName, "2017" ) ){
        return "2017";
    } else if( stringTools::stringContains( histogramName, "2018" ) ){
        return "2018";
    } else{
        throw std::invalid_argument( "histogram name '" + histogramName + "' does not contain a year (2016, 2017 or 2018)" );
    }
}


std::map< std::string, Prescale > fakeRate::fitTriggerPrescales_cut( TFile* filePtr, const double min, const double max ){
    std::vector< std::string > triggerNames = listTriggersWithHistogramInFile( filePtr );
    std::vector< std::string > histogramNames = listHistogramNamesInFile( filePtr );

    //check which year the histograms belong to, and check the consistency
    std::string year = extractYear( histogramNames.front() );
    for( const auto& histogram : histogramNames ){
        if( !stringTools::stringContains( histogram, year ) ){
            throw std::invalid_argument( "histogram name '" + histogram + "' does not contain year '" + year + "'" );
        }
    }
    
    std::map< std::string, std::shared_ptr< TH1D > > prompt_histograms;
    std::map< std::string, std::shared_ptr< TH1D > > data_histograms;

    for( const auto& trigger : triggerNames ){

        for( const auto& histogram : histogramNames ){
            if( !stringTools::stringContains( histogram, trigger ) ) continue;
            if( stringTools::stringContains( histogram, "nonprompt" ) ) continue;
            if( stringTools::stringContains( histogram, "prompt" ) ){
                prompt_histograms[trigger] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( filePtr->Get( histogram.c_str() ) ) );
            } else if( stringTools::stringContains( histogram, "data" ) ){
                data_histograms[trigger] = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( filePtr->Get( histogram.c_str() ) ) );
            } else {
                throw std::invalid_argument( "histogram name '" + histogram + "' does not correspond to data or a prompt simulation." );
            }
        }
    }
    
    //apply the fit to each histogram and store them in the returned map
    std::map< std::string, Prescale > prescaleMap;
    for( const auto& trigger: triggerNames ){

        //divide data by the prompt contribution and then fit it
        //clone data histogram so the original can still be plotted later on
		std::shared_ptr< TH1D > ratio_histogram( dynamic_cast< TH1D* >( data_histograms[trigger]->Clone() ) );
        ratio_histogram->Divide( prompt_histograms[ trigger ].get() );

        //make fit to the data
        ConstantFit fitInfo( ratio_histogram, min, max );
        
        prescaleMap[trigger] = Prescale( fitInfo );
    }

	//plot the prescale measurements 
	//make plot directory if it does not already exist 
	std::string outputDirectory_name = "prescaleMeasurementPlots";
	systemTools::makeDirectory( outputDirectory_name );

	for( const auto& trigger : triggerNames ){

        //make array for prompt histogram and scale it by the prescale
		TH1D* predictedHists[1] = { prompt_histograms[trigger].get() };
        predictedHists[0]->Scale( prescaleMap[trigger].value() );

        //vary prompt histogram within uncertainty on prescale measurement 
		TH1D* systUnc = dynamic_cast< TH1D* >( predictedHists[0]->Clone() );
        double prescaleFractionalUnc = fabs( prescaleMap[trigger].uncertaintySymmetric() / prescaleMap[trigger].value() );
        for( int b = 1; b < systUnc->GetNbinsX() + 1; ++b ){
            systUnc->SetBinContent( b , systUnc->GetBinContent(b) * prescaleFractionalUnc );
        }

		std::string predictedNames[2] = {"data", "prompt"};
        plotDataVSMC( data_histograms[trigger].get(), predictedHists, predictedNames, 1, stringTools::formatDirectoryName( outputDirectory_name ) + trigger + "_prescaleMeasurement_" + year + ".pdf", "", false, false, "(13 TeV)", systUnc ); 
		
	}
    return prescaleMap;
}


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


//list all pt and eta bins that are present in a file with all the fake rate histograms
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


bool histIsPrompt( const std::string& histName ){
    return ( stringTools::stringContains( histName, "prompt" ) && !stringTools::stringContains( histName, "nonprompt" ) );
}


bool histIsData( const std::string& histName ){
    return stringTools::stringContains( histName, "data" );
}


bool histIsNumerator( const std::string& histName ){
    return stringTools::stringContains( histName, "numerator" );
}


bool histIsDenominator( const std::string& histName ){
    return stringTools::stringContains( histName, "denominator" );
}


std::shared_ptr< TH2D > fakeRate::produceFakeRateMap_cut( TFile* filePtr, const double maxValue ){

    //histogram and pt/eta bins present in the file
	std::vector< std::string > ptEtaBinNames = listPtEtaBinsInFile( filePtr );
    std::vector< std::string > histogramNames = fakeRate::listHistogramNamesInFile( filePtr );

    //check which year the histograms belong to
    std::string year = extractYear( histogramNames.front() );

    //check whether the fake-rate is for muons or electrons
    bool isMuon = stringTools::stringContains( histogramNames.front(), "muon" );
    if( !isMuon && !stringTools::stringContains( histogramNames.back(), "electron" ) ){
        throw std::invalid_argument( histogramNames.front() + " corresponds to neither electrons nor muons" );
    }

    //initialize 2D histogram
    std::pair< std::vector< double >, std::vector< double > > ptEtaBins = ptEtaBinNamesToBinVectors( ptEtaBinNames, isMuon );
    std::shared_ptr<TH2D> fakeRateMap = std::make_shared< TH2D >( "fake-rate", "fake-rate; p_{T} (GeV); |#eta|", ptEtaBins.first.size() - 1, &ptEtaBins.first[0], ptEtaBins.second.size() - 1, &ptEtaBins.second[0] );
    fakeRateMap->SetDirectory( gROOT );

	//measure fake-rate for each pT and eta bin 
	for( const auto& bin : ptEtaBinNames ){

		//extract data and prompt histogram for this bin
		std::shared_ptr< TH1D > data_hist_numerator;
		std::shared_ptr< TH1D > prompt_hist_numerator;

		std::shared_ptr< TH1D > data_hist_denominator;
		std::shared_ptr< TH1D > prompt_hist_denominator;

        for( const auto& name : histogramNames ){
            if( !stringTools::stringContains( name, bin ) ) continue;
            if( histIsNumerator( name ) ){
                if( histIsData( name ) ){
                    data_hist_numerator = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( filePtr->Get( name.c_str() ) ) );
                } else if( histIsPrompt( name ) ){
                    prompt_hist_numerator = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( filePtr->Get( name.c_str() ) ) );
                }
            } else if( histIsDenominator( name ) ){
                if( histIsData( name ) ){
                    data_hist_denominator = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( filePtr->Get( name.c_str() ) ) );
                } else if( histIsPrompt( name ) ){
                    prompt_hist_denominator = std::shared_ptr< TH1D >( dynamic_cast< TH1D* >( filePtr->Get( name.c_str() ) ) );
                }
            } else {
                throw std::invalid_argument( "histogram " + name + " is neither numerator nor denominator" );
            }
        }

        //plot data and prompt histogram for both numerator and denominator before computing fakerate 
		//make plot directory if it does not already exist 
		std::string outputDirectory_name = "fakeRateMeasurementPlots";
		systemTools::makeDirectory( outputDirectory_name );

       	//plot numerator
        TH1D* predictedHists_numerator[1] = { prompt_hist_numerator.get() };
        std::string predictedNames[2] = {"data", "prompt"};
        plotDataVSMC( data_hist_numerator.get(), predictedHists_numerator, predictedNames, 1, stringTools::formatDirectoryName( outputDirectory_name ) + ( isMuon ? "muon_" : "electron_" ) + bin + "_numerator_fakeRateMeasurement" + year + ".pdf", "", false, false, "(13 TeV)" ); 
        
		//plot denominator
        TH1D* predictedHists_denominator[1] = { prompt_hist_denominator.get() };
        plotDataVSMC( data_hist_numerator.get(), predictedHists_denominator, predictedNames, 1, stringTools::formatDirectoryName( outputDirectory_name ) + ( isMuon ? "muon_" : "electron_" ) + bin + "_denominator_fakeRateMeasurement" + year + ".pdf", "", false, false, "(13 TeV)" ); 
		

        //subtract prompt contamination from data
        data_hist_numerator->Add( prompt_hist_numerator.get(), -1. );
        data_hist_denominator->Add( prompt_hist_denominator.get(), -1. );

        //set fake-rate to 0 if there are no entries 
        double fakeRate, fakeRateUncertainty;
        if( data_hist_numerator->GetSumOfWeights() < 1e-6 || data_hist_denominator->GetSumOfWeights() < 1e-6){
            fakeRate = 0.;
            fakeRateUncertainty = 0.;
        } else{ 

            //divide remaining numerator and denominator and fit their ratio
            data_hist_numerator->Divide( data_hist_denominator.get() );
            ConstantFit fit( data_hist_numerator, 0, maxValue );
            fakeRate = fit.value();
            fakeRateUncertainty = fit.uncertainty();
        }
    
        //write to corresponding TH2 bin
        auto binIndex = fakeRateMap->FindBin( ptBorder( bin), etaBorder( bin ) );
        fakeRateMap->SetBinContent( binIndex, fakeRate );
        fakeRateMap->SetBinError( binIndex, fakeRateUncertainty );
	}

    return fakeRateMap;
}
