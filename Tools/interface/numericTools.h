#ifndef numericTools_H
#define numericTools_H

//include c++ library tools
#include <cmath>


namespace numeric{
    
    template< typename T > bool floatEquals( const T lhs, const T rhs, const T precision = 1e-6 ){
            return ( std::abs( ( lhs - rhs ) / lhs ) < precision );
    }


    template< typename T > int floatToInt( const T f ){
        return static_cast< int >( std::floor( f + 0.5 ) );
    }


    template< typename T > unsigned floatToUnsigned( const T f ){

        //set negative floats to zero to avoid unintentional overflow
        return static_cast< unsigned >( std::floor( std::max( f + 0.5, 0.1 ) ) );
    }
}

#endif
