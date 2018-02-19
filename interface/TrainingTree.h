/*
class to easily make and write a file of root trees for BDT training
*/

#ifndef TrainingTree_H
#define TrainingTree_H

//include other parts of code
#include "Category.h"
#include "Sample.h"

//include c++ library classes
#include <vector>
#include <map>

//include ROOT classes
#include "TFile.h"
#include "TTree.h"

class TrainingTree{
    public:
        //TrainingTree(const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const std::vector<std::string>&);
        TrainingTree(const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const std::map< std::string, double >&);
        TrainingTree(const Sample&, const Category&, const std::map<std::string, double>&);
        TrainingTree(const Sample&, const std::vector < std::vector < std::string > >&, const std::map< std::string, double >&);
        ~TrainingTree();

        void fill(const size_t, const bool, const std::map<std::string, double>&);

        void write(const std::string& );
    private:
        std::shared_ptr< Category > category;
        std::shared_ptr< Sample > sample;
        std::map< std::string, double > variableMap;
        std::vector< TTree* > signalTrees;
        std::vector< TTree* > backgroundTrees;
        TFile* treeFile;
        
        //set up tree branch adresses
        void setBranches();
};

//merge all training trees in given directory
void mergeTrees(const std::string&);

#endif
