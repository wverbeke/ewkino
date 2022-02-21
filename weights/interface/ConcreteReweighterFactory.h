/*
New user specified ReweighterFactory derivates should be listed here
*/

#ifndef ConcreteReweighterFactory_H
#define ConcreteReweighterFactory_H

#include "ReweighterFactory.h"

class EmptyReweighterFactory : public ReweighterFactory {
    public: 
	virtual CombinedReweighter buildReweighter( const std::string&, const std::string&, const std::vector< Sample >& ) const override;
};


class EwkinoReweighterFactory : public ReweighterFactory {

    public:
        virtual CombinedReweighter buildReweighter( const std::string&, const std::string&, const std::vector< Sample >& ) const override;
};


class Run2ULReweighterFactory : public ReweighterFactory {

    public:
        virtual CombinedReweighter buildReweighter( const std::string&, const std::string&, const std::vector< Sample >& ) const override;
};

#endif
