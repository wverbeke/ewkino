#ifndef HToWDGenInfo_H
#define HToWDGenInfo_H

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"

class HToWDGenInfo{
    
    public:
        HToWDGenInfo( const TreeReader& );

        int genhtowdEventType() const{ return _genhtowdEventType; }
	double genhtowdHPt() const{ return _genhtowdHPt; }
	double genhtowdHEta() const{ return _genhtowdHEta; }
	double genhtowdHPhi() const{ return _genhtowdHPhi; }

    private:

        int _genhtowdEventType;
	double _genhtowdHPt;
	double _genhtowdHEta;
	double _genhtowdHPhi;

};

#endif 
