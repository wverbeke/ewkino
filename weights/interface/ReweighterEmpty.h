#ifndef ReweighterEmpty_H
#define ReweighterEmpty_H

#include "Reweighter.h"

class ReweighterEmpty : public Reweighter {

    public:
        virtual double weight( const Event& ) const override;
        virtual double weightDown( const Event& ) const override;
        virtual double weightUp( const Event& ) const override;
};

#endif
