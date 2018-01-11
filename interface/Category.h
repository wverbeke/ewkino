#ifndef Category_H
#define Category_H

//include c++ library classes
#include <string>
#include <vector>
#include <iostream>

class Category{
    using pos = std::vector<std::string>::size_type;
    public:
        Category(const std::vector < std::vector <std::string> >&);
        size_t getIndex(const std::vector<pos>&);
        const std::vector<std::string>& getCat() const { return cat; }
    private:
        std::vector< std::string > cat;
        std::vector< pos > ranges;
};
#endif 
