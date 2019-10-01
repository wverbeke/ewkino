/*
Class that expands multidimensional categorization tree along a single axis and keeps track of all indices 
*/

#ifndef Categorization_H
#define Categorization_H

//include c++ library classes
#include <string>
#include <vector>
#include <memory>

//include other parts of code 
#include "IndexFlattener.h"



class Categorization{
    
    public:
        using size_type = std::vector< std::string >::size_type;
        using const_iterator = std::vector< std::string >::const_iterator;
        
        //Constructor takes a 2D vector of strings, every vectory is an independent categorization layer, leading to a multidimensional cateogirization structure.
        Categorization( const std::vector < std::vector <std::string> >& );
        
        size_type index( const std::vector< size_type >& ) const;
        
        size_type range( size_type ind ) const { return flattenerPtr->range( ind ); }
        
        size_type size() const { return categories.size(); }
        
        std::string name( const std::vector< size_type >& ) const;
        
        std::string name( size_type ind ) const { return categories[ind]; }
        
        const_iterator cbegin() const { return categories.cbegin(); }
        const_iterator begin() const { return cbegin(); }
        
        const_iterator cend() const { return categories.cend(); }
        const_iterator end() const { return cend(); }
        
        const std::string& operator[]( size_type ind ) const { return categories[ind]; }
        
        //find all category indices whose names contain a given string
        std::vector< size_type > findCategoriesByName( const std::string& ) const;

    private:
        std::vector< std::string > categories;
        std::shared_ptr< IndexFlattener > flattenerPtr;
};
#endif 
