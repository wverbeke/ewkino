#ifndef IndexFlattener_H
#define IndexFlattener_H


//include C++ library classes
#include <vector>
#include <map>



class IndexFlattener {

    public:
        using size_type = size_t;

        //construct using a vector of index-ranges for the multidimensional indices
        IndexFlattener( const std::vector< size_type >& );

        //convert multidimensional indices to one-dimensional index
        size_type index( const std::vector< size_type >& ) const;

        //convert one-dimensional index to multidimensional indices 
        std::vector< size_type > indices( const size_type ) const;

        //number of one-dimensional indices
        size_type size() const;

        //number of indices along each dimension
        size_type range( size_type index ) const;

    private:
        std::vector< size_type > ranges;
};

#endif
