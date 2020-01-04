/*
Virtual class to compute the reweighting of an event. This should be overridden for specific cases such as pilup, lepton efficiency and b-tagging weights.
*/

#ifndef Reweighter_H
#define Reweighter_H

//include other parts of framework
#include "../../Event/interface/Event.h"


class Reweighter{

    public:
        virtual double weight( const Event& ) const = 0;
        virtual double weightDown( const Event& ) const = 0;
        virtual double weightUp( const Event& ) const = 0;

};
#endif
