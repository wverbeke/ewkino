#include "../interface/PostFitScaler.h"

#include <iostream>

int main(){
    
    PostFitScaler scaler( "testData/postFitTable.txt" );
	std::cout << " scaler.postFitYield( 30.082 ) = " << scaler.postFitYield( 30.082 ) << std::endl;
	std::cout << " scaler.postFitYield( TTZ, 30.082 ) = " << scaler.postFitYield( "TTZ", 30.082 ) << std::endl;
	std::cout << " scaler.postFitYield( 67.050 ) = " << scaler.postFitYield( 67.050 ) << std::endl;
	std::cout << " scaler.postFitYield( WZ, 67.050 ) = " << scaler.postFitYield( "WZ", 67.050 ) << std::endl;
	std::cout << " scaler.postFitYield( 2263.580 ) = " << scaler.postFitYield( 2263.580 ) << std::endl;
	std::cout << " scaler.postFitYield( WZ, 2263.580 ) = " << scaler.postFitYield( "WZ", 2263.580 ) << std::endl;
	std::cout << " scaler.postFitYield( 356.634 ) = " << scaler.postFitYield( 356.634 ) << std::endl;
	std::cout << " scaler.postFitYield( ZZH, 356.634 ) = " << scaler.postFitYield( "ZZH", 356.634 ) << std::endl;
	std::cout << " scaler.postFitYield( 3177.480 ) = " << scaler.postFitYield( 3177.480 ) << std::endl;
	std::cout << " scaler.postFitYield( WZ, 3177.480 ) = " << scaler.postFitYield( "WZ", 3177.480 ) << std::endl;
    return 0;
}
