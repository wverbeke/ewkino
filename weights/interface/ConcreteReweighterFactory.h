/*
New user specified ReweighterFactory derivates should be listed here
*/

#ifndef ConcreteReweighterFactory_H
#define ConcreteReweighterFactory_H

#include "ReweighterFactory.h"

class EwkinoReweighterFactory : public ReweighterFactory {

    public:
        virtual CombinedReweighter buildReweighter( const std::string&, const std::string&, const std::vector< Sample >& ) const override;
};

class EmptyReweighterFactory : public ReweighterFactory {
    // empty reweighter for testing purposes

    public:
	virtual CombinedReweighter buildReweighter( const std::string&, const std::string&, const std::vector< Sample>& ) const override;
};

class tZqReweighterFactory : public ReweighterFactory {
    // user specified Reweighter class for tZq analysis

    public:
        virtual CombinedReweighter buildReweighter( const std::string&, const std::string&, const std::vector< Sample >& ) const override;
};

class BTagShapeReweighterFactory : public ReweighterFactory {
    // btag shape reweighter for testing

    public:
	virtual CombinedReweighter buildReweighter( const std::string&, const std::string&, const std::vector< Sample >& ) const override;
};

#endif
