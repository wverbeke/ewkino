#ifndef CombinedReweighterFactory_H
#define CombinedReweighterFactory_H

#include "CombinedReweighter.h"

class Sample;

class ReweighterFactory {
    
    public:
        virtual CombinedReweighter buildReweighter( const std::string& weightDirectory, const std::string& year, const std::vector< Sample >& samples ) const = 0;
};

#endif
