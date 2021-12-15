#ifndef fakeRateTools_H
#define fakeRateTools_H

//include c++ library classes 
#include <string>
#include <vector>
#include <map>

//include ROOT classes 
#include "TFile.h"
#include "TH2D.h"

//include other parts of framework
#include "Prescale.h"


namespace fakeRate{
    
    void checkFlavorString( const std::string& flavorString );
    std::vector< std::string > listHistogramNamesInFile( TFile* filePtr );
    std::vector< std::string > listTriggersWithHistogramInFile( TFile* filePtr );

    std::string extractTriggerName( const std::string& );
    std::string extractYear( const std::string& );

    std::map< std::string, Prescale > fitTriggerPrescales_cut( TFile* filePtr, 
					const double min, const double max, 
					const bool doPlot=true, const bool doSave=true );

    std::map< std::string, Prescale > fitTriggerPrescales_shape( TFile* filePtr );
    //( this function seems to be not present in the source file? is it obsolete? )

    std::shared_ptr< TH2D > produceFakeRateMap_cut( TFile* filePtr, 
				const double maxValue, 
				const bool doPlot=true, const bool doSave=true );
}

#endif
