#ifndef Category_H
#define Category_H

//include c++ library classes
#include <string>
#include <vector>
#include <iostream>

class Category{
    using pos = size_t;
    public:
        Category(const std::vector < std::vector <std::string> >&);
        size_t getIndex(const std::vector<pos>&) const;
        size_t getRange(size_t ind) const { return ranges[ind].size(); }
        std::string name(const std::vector<pos>&) const;
        const std::vector<std::string>& getCat() const { return cat; }
    private:
        std::vector< std::string > cat;
        std::vector< pos > ranges;
};
#endif 
