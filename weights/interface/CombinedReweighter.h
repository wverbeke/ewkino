#ifndef CombinedReweighter_H
#define CombinedReweighter_H

//include c++ library classes
#include <map>
#include <memory>
#include <string>

//include other parts of framework
#include "Reweighter.h"



class CombinedReweighter{

    public:
        CombinedReweighter() = default;

        void addReweighter( const std::string&, const std::shared_ptr< Reweighter >& );
        void eraseReweighter( const std::string& );

        const Reweighter* operator[]( const std::string& ) const;
        double totalWeight( const Event& ) const;

    private:
        std::map< std::string, std::shared_ptr< Reweighter > > reweighterMap;
        std::vector< std::shared_ptr< Reweighter > > reweighterVector;
};


#endif
