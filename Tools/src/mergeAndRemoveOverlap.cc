#include "../interface/mergeAndRemoveOverlap.h"

//include ROOT classes 
#include "TFile.h"

//include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/analysisTools.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/EventTags.h"
#include "../../Event/interface/Event.h"



bool yearsAreConsistent( const std::vector< std::string >& inputPathVector ){

    //index specifying the year of the previous file 
    int previousYearIndex = -1;
    for( const auto& inputFilePath : inputPathVector ){
        
        std::pair< bool, bool > is2017Or2018 = analysisTools::fileIs2017Or2018( inputFilePath );

        //from the second file onwards, check that all files have the same year 
        if( previousYearIndex != -1 ){
            int yearIndex = 1*( is2017Or2018.first ) + 2*( is2017Or2018.second );
            if( yearIndex != previousYearIndex ){
                return false;
            }
            previousYearIndex = yearIndex;
        }

    }
    return true;
}



bool eventIsNew( const Event& event, std::set< EventTags >& usedEventTags ){

    //search set for the current event 
    auto tagIt = usedEventTags.find( event.eventTags() );

    //continue if event with the same tags was seen before
    if( tagIt != usedEventTags.end() ){
        return false;
    }

    //add unseen events to the set 
    usedEventTags.emplace( event.eventTags() ); 

    return true;
}


void mergeAndRemoveOverlap( const std::vector< std::string >& inputPathVector, 
			    const std::string& outputPath, 
			    const bool allowMergingYears ){

    // size of input vector must be at least 2, otherwise there can be no merging 
    if( inputPathVector.size() < 2 ){
	std::string msg = "ERROR in mergeAndRemoveOverlap: Input path vector has size ";
	msg += std::to_string( inputPathVector.size() ) + ", while it should be at least 2.";
        throw std::length_error( msg );
    }

    // unless explicitly specified, don't allow the merging of files for different years
    if( !( allowMergingYears || yearsAreConsistent( inputPathVector ) ) ){
	std::string msg = "ERROR in mergeAndRemoveOverlap: Can't merge datasets";
	msg += " corresponding to different files unless explicitly specified.";
        throw std::logic_error( msg );
    }

    // initialize TreeReader
    TreeReader treeReader;

    // make output file and output Tree
    TFile* outputFilePtr = TFile::Open( outputPath.c_str(), "RECREATE" );
    outputFilePtr->mkdir( "blackJackAndHookers" );
    outputFilePtr->cd( "blackJackAndHookers" );
    std::shared_ptr< TTree > outputTreePtr( std::make_shared< TTree >( 
				"blackJackAndHookersTree","blackJackAndHookersTree" ) );

    // histograms stored in file
    std::map< std::string, std::shared_ptr< TH1 > > outputHistogramMap;

    // set of events that has been seen
    // use std::set so search scales as log(N) 
    std::set< EventTags > usedEventTags;

    // loop over files
    for( auto inputPathIt = inputPathVector.cbegin(); 
	inputPathIt != inputPathVector.cend(); 
	++inputPathIt ){

	std::cout << "processing file " << inputPathIt-inputPathVector.begin()+1;
	std::cout << " of " << inputPathVector.size() << std::endl;
        const auto& inputFilePath = *inputPathIt;

        // open next sample
        // DO NOT reset triggers because this will invalidate the addresses set by setOutputTree 
	// and trigger decisions in output file will be wrong!
        treeReader.initSampleFromFile( inputFilePath, false );
        outputTreePtr->SetDirectory( outputFilePtr );

        // set output histograms and output tree for first file
        if( inputPathIt == inputPathVector.cbegin() ){

            for( const auto& histPtr : treeReader.getHistogramsFromCurrentFile() ){
                outputHistogramMap[ histPtr->GetName() ] = histPtr;
            }

            // set uo output tree
            treeReader.setOutputTree( outputTreePtr.get() );

        // for next files, add the histograms to the current histograms 
	// and check that no unknown histograms are present
        } else {
            for( const auto& histPtr : treeReader.getHistogramsFromCurrentFile() ){
                auto histIt = outputHistogramMap.find( histPtr->GetName() );
                if( histIt == outputHistogramMap.cend() ){
		    std::string msg = "ERROR in mergeAndRemoveOverlap: ";
		    msg += "Histogram " + std::string( histPtr->GetName() ); 
		    msg += " not found in file " + inputPathVector[0] + ".";
                    throw std::invalid_argument( msg );
                }
                histIt->second->Add( histPtr.get() );
            }
        }

        // loop over events in tree and write them to the output tree if there is no overlap
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
	    if(entry%1000 == 0){
		std::cout << "processed: " << entry;
		std::cout << " of "<< treeReader.numberOfEntries() << std::endl;
	    }

            Event event = treeReader.buildEvent( entry );

            // check if event is new and insert it into the list of used events 
            if( eventIsNew( event, usedEventTags ) ){

                // write event to output tree
                outputTreePtr->Fill();
            }
        }

    }

    // need to change directory for writing
    outputFilePtr->cd( "blackJackAndHookers" );

    // write output histograms 
    for( const auto& histPair : outputHistogramMap ){
        histPair.second->Write();
    }
    
    // write output tree 
    outputTreePtr->Write( "", BIT(2) );

    // close output file
    outputFilePtr->Close();
}
