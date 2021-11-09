#include "../interface/HToWDGenInfo.h"

//include c++ library classes 
#include <stdexcept>



HToWDGenInfo::HToWDGenInfo( const TreeReader& treeReader ) :
    _genhtowdEventType( treeReader._genhtowdEventType ),
    _genhtowdHPt( treeReader._genhtowdHPt ),
    _genhtowdHEta( treeReader._genhtowdHEta ),
    _genhtowdHPhi( treeReader._genhtowdHPhi )
{}
