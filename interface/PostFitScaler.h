/*
Class that extracts post-fit normalizations from Combine's output and retrieves the scaling 
*/

//include c++ library classes 
#include <map>
#include <vector>
#include <string>
#include <utility>

#ifndef PostFitScaler_H
#define PostFitScaler_H
class PostFitScaler{
    public: 
        PostFitScaler( const std::string& inputFileName );

        double postFitYield( const double preFitYield ) const;
        double postFitScaling( const double preFitYield ) const;

    private:
        std::map< std::string, std::vector< std::pair < double, double  > > > postAndPreFitYields;

        //retrieve which process and bin corresponds to a certain pre fit yield
        std::pair< std::string, size_t> findProcessAndBin( const double preFitYield ) const;
};
#endif
