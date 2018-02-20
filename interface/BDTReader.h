/*
class to facilitate extraction of bdt output from xml files
*/

#ifndef BDTReader_H
#define BDTReader_H

//include other parts of code 
#include "../interface/Category.h"

//include c++ library functions
#include <memory>
#include <vector>
#include <map>
#include <string>

//include ROOT classes
#include "TMVA/Reader.h"

class BDTReader{
    public:
        BDTReader(const std::string&, const std::shared_ptr<Category>&, const std::map< std::string, float>&);
        BDTReader(const std::string&, const Category&, const std::map < std::string, float >&);
        BDTReader(const std::string&, const std::vector < std::vector < std::string > >&, const std::map < std::string, float >&);
            
        //get the BDT output for this category's BDT 
        float computeBDT(const std::vector< size_t > &, const std::map < std::string, float>& );
        
    private:
        std::vector< std::shared_ptr< TMVA::Reader > > readers;
        std::shared_ptr<Category> category;
        std::map<std::string, float> variableMap;
        std::string methodName;

        //add variables to bdt 
        void addVariables(const std::shared_ptr< TMVA::Reader>&);
        
        //get name of method (xml file) for given category
        std::string mvaMethodName(const size_t) const;

        //get the BDT output for this category's BDT 
        float computeBDT(const size_t categoryIndex, const std::map < std::string, float>& );
};
#endif
