/*
Class that expands multidimensional categorization tree along a single axis and keeps track of all indices 
*/

#ifndef Category_H
#define Category_H

//include c++ library classes
#include <string>
#include <vector>
#include <iostream>

class Category{
    using pos = size_t;
    using constIterator = std::vector< std::string >::const_iterator;
    public:

        /*
        Constructor takes a 2D vector of strings, every vectory is an independent categorization layer, 
        leading to a multidimensional cateogirization structure.
        */
        Category( const std::vector < std::vector <std::string> >& );


        size_t getIndex( const std::vector<pos>& ) const;

        size_t getRange(pos ind) const { return ranges[ind]; }

        size_t size() const { return cat.size(); }

        std::string name( const std::vector<pos>& ) const;

        std::string name( pos ind ) const { return cat[ind]; }

        constIterator cbegin() const { return cat.cbegin(); }
        constIterator begin() const { return cbegin(); }

        constIterator cend() const { return cat.cend(); }
        constIterator end() const { return cend(); }

        const std::string& operator[]( pos ind ) const { return cat[ind]; }

        //find all category indices whose names contain a given string
        std::vector< size_t > findCategoriesByName( const std::string& ) const;

    private:
        std::vector< std::string > cat;
        std::vector< pos > ranges;
};
#endif 
