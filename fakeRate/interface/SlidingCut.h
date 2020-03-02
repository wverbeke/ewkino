#ifndef SlidingCut_H
#define SlidingCut_H

//include c++ library classes
#include <string>
#include <vector>


class SlidingCut{

    public:
        SlidingCut( const double minX, const double maxX, const double leftCut, const double rightCut );
        double cut( const double x ) const;
        std::string name( const std::string& xName, const std::string& cutName ) const;

    private:
        double _minX;
        double _maxX;
        double _leftCut;
        double _rightCut;
};


class SlidingCutCollection{

    public:
        using size_type = std::vector< SlidingCut >::size_type;
        SlidingCutCollection( const double minX, const double maxX, const double minCut, const double maxCut, const double granularity );
        size_type size() const{ return _collection.size(); }
        const SlidingCut& operator[]( const size_type index ) const{ return _collection[ index ]; }

    private:
        std::vector< SlidingCut > _collection;
};



#endif
