#ifndef SusyMassInfo_H
#define SusyMassInfo_H

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"

class SusyMassInfo{

    public:
        SusyMassInfo( const TreeReader& );

        double massNLSP() const{ return _massNLSP; }
        double massLSP() const{ return _massLSP; }

    private:
        double _massNLSP;
        double _massLSP;
};




#endif
