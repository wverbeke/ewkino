/*
Class to collect information about SUSY samples. These samples contain several signal simulation points within a single sample.
*/
#ifndef SusyScan_H
#define SusyScan_H

//include c++ library classes 
#include <map>
#include <utility>

//include other parts of code
#include "Sample.h"



class SusyScan {

    public:
        SusyScan() = default;
        SusyScan( const double massSplitting );
        SusyScan( const Sample& );
        SusyScan( const Sample&, const double massSplitting ); 

        size_t numberOfPoints() const;
        size_t index( const double mChi2, const double mChi1 ) const;

        std::pair< double, double > masses( const size_t ) const;
        std::string massesString( const size_t ) const;

        double sumOfWeights( const double mChi2, const double mChi1) const;
        double sumOfWeights( const size_t ) const;

        void addScan( const Sample& sample ){ addMassPoints_Fast( sample ); }
        std::vector< unsigned > massSplittings() const;

    private:
        std::map< std::pair< unsigned, unsigned>, size_t > massesToIndices;
        std::map< size_t, std::pair< unsigned, unsigned > > indicesToMasses;
        std::map< size_t, double > indicesToSumOfWeights;
        unsigned _massSplitting = 0;
        
        void addMassPoints_Fast( const Sample& );

        std::pair< unsigned, unsigned > massesAtIndex( const size_t ) const;
};

#endif 
