#include "../Tools/interface/mergeAndRemoveOverlap.h"

//include c++ library classes
#include <string> 
#include <vector>

int main( int argc, char* argv[] ){

	std::vector< std::string > argvStr;
	for( int i = 1; i < argc; ++i ){
		argvStr.push_back( argv[i] );
	}

    mergeAndRemoveOverlap( argvStr, "test.root" );


	return 0;
}
