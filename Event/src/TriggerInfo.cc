#include "../interface/TriggerInfo.h"

//include c++ library classes
#include <iostream>
#include <stdexcept>


TriggerInfo::TriggerInfo( const TreeReader& treeReader, const bool readIndividualTriggers, const bool readIndividualMetFilters ) :
    _passTriggers_e( treeReader._passTrigger_e ),
    _passTriggers_m( treeReader._passTrigger_m ),
    _passTriggers_em( treeReader._passTrigger_em ),
    _passTriggers_et( treeReader._passTrigger_et ),
    _passTriggers_mm( treeReader._passTrigger_mm ),
    _passTriggers_mt( treeReader._passTrigger_mt ),
    _passTriggers_eee( treeReader._passTrigger_eee ),
    _passTriggers_eem( treeReader._passTrigger_eem ),
    _passTriggers_emm( treeReader._passTrigger_emm ),
    _passTriggers_mmm( treeReader._passTrigger_mmm ),
    _passMetFilters( treeReader._passMETFilters )
{
    if( readIndividualTriggers ){
        for( const auto& trigger : treeReader._triggerMap ){
            individualTriggerMap.insert( trigger );
        }
    }
    if( readIndividualMetFilters ){
        for( const auto& filter : treeReader._MetFilterMap ){
            individualMetFilterMap.insert( filter );
        }   
    }
}


bool passTriggerOrFilter( const std::map< std::string, bool >& decisionMap,  const std::string& name ){
    auto decisionIt = decisionMap.find( name );

    //throw error if non-existing trigger or MET filter is requested
    if( decisionIt == decisionMap.cend() ){
        throw std::invalid_argument( "Requested trigger or MET filter does not exist." );
    } else {
        return (*decisionIt).second;
    }
}


bool TriggerInfo::passTrigger( const std::string& triggerName ) const{
    return passTriggerOrFilter( individualTriggerMap, triggerName );
}


bool TriggerInfo::passMetFilter( const std::string& filterName ) const{
    return passTriggerOrFilter( individualMetFilterMap, filterName );
}


void printAvailableInfo( const std::map< std::string, bool >& decisionMap, const std::string& decisionType ){
    std::cout << "Available " << decisionType << " :" << std::endl;
    for( const auto& decision : decisionMap ){
        std::cout << decision.first << std::endl;
    }
}


void TriggerInfo::printAvailableIndividualTriggers() const{
    printAvailableInfo( individualTriggerMap, "triggers" );
}


void TriggerInfo::printAvailableMetFilters() const{
    printAvailableInfo( individualMetFilterMap, "MET filters");
}
