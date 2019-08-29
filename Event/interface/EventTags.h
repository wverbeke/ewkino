#ifndef EventTags_H
#define EventTags_H

//include c++ library classes
#include <ostream>
#include <initializer_list>

class TreeReader;

class EventTags{

    friend bool operator==( const EventTags&, const EventTags& );
    friend std::ostream& operator<<( std::ostream&, const EventTags& );

    public:
        EventTags( const TreeReader& );
        EventTags( const long unsigned, const long unsigned, const long unsigned );
        EventTags( const std::initializer_list< long unsigned>& );

        long unsigned runNumber() const{ return _runNumber; }
        long unsigned luminosityBlock() const{ return _luminosityBlock; }
        long unsigned eventNumber() const{ return _eventNumber; }

    private:
        long unsigned _runNumber = 0;
        long unsigned _luminosityBlock = 0;
        long unsigned _eventNumber = 0;
};

bool operator==( const EventTags&, const EventTags& );
std::ostream& operator<<( std::ostream&, const EventTags& );

#endif
