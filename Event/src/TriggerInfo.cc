#include "../interface/TriggerInfo.h"

//include c++ library classes
#include <iostream>


TriggerInfo::TriggerInfo( const TreeReader& treeReader, const bool readIndividualTriggers ) :
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
    _passMETFilters( treeReader._passMETFilters )
{
    if( readIndividualTriggers ){
        for( const auto& trigger : treeReader._triggerMap ){
            individualTriggerMap.insert( trigger );
        }
    }
}


bool TriggerInfo::passTrigger( const std::string& triggerName ) const{
    auto decisionIt = individualTriggerMap.find( triggerName );
    if( decisionIt != individualTriggerMap.cend() ){
        return (*decisionIt).second;
    }
    return false;
}


void TriggerInfo::printAvailableIndividualTriggers() const{
    std::cout << "Available triggers :" << std::endl;
    for( const auto& trigger : individualTriggerMap ){
        std::cout << trigger.first << std::endl;
    }
}
