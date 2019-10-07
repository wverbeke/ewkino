#include "../interface/EventTags.h"

//include c++ library classes
#include <stdexcept>

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"


EventTags::EventTags( const TreeReader& treeReader ) :
    _runNumber( treeReader._runNb ),
    _luminosityBlock( treeReader._lumiBlock ),
    _eventNumber( treeReader._eventNb )
    {}


EventTags::EventTags( const long unsigned runNumber, const long unsigned luminosityBlock, const long unsigned eventNumber ) :
    _runNumber( runNumber ),
    _luminosityBlock( luminosityBlock ),
    _eventNumber( eventNumber )
    {}

EventTags::EventTags( const std::initializer_list< long unsigned >& init_list )
{
    if( init_list.size() != 3 ){
        throw std::length_error( "EventTags initializer must have length 3!" );
    }
    _runNumber = *( init_list.begin() );
    _luminosityBlock = *( init_list.begin() + 1 );
    _eventNumber = *( init_list.begin() + 2 );
}


bool operator==( const EventTags& lhs, const EventTags& rhs ){
    return (
        lhs._runNumber == rhs._runNumber &&
        lhs._luminosityBlock == rhs._luminosityBlock &&
        lhs._eventNumber == rhs._eventNumber 
    );
}


bool operator <( const EventTags& lhs, const EventTags& rhs ){
    if( lhs._runNumber != rhs._runNumber ){
        return ( lhs._runNumber < rhs._runNumber );
    } else if( lhs._luminosityBlock != rhs._luminosityBlock ){
        return ( lhs._luminosityBlock < rhs._luminosityBlock );
    } 
    return ( lhs._eventNumber < rhs._eventNumber );
}


std::ostream& operator<<( std::ostream& os, const EventTags& eventTags ){
    os << "run number/luminosity block/event number : " << eventTags._runNumber << "/" << eventTags._luminosityBlock << "/" << eventTags._eventNumber;
    return os;
}
