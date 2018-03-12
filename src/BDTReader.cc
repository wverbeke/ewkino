#include "../interface/BDTReader.h"

BDTReader::BDTReader(const std::string& bdtName, const std::string& xmlFileName, const std::shared_ptr< std::map < std::string, float > >& varMap):
    methodName(bdtName), variableMap(varMap)
{
    reader.reset( new TMVA::Reader("!Color:!Silent") );
    //set all variables
    addVariables();
    //book method 
    reader->BookMVA(methodName, xmlFileName); 
}


void BDTReader::addVariables() const{
    for(auto& variable : *variableMap){
        reader->AddVariable( (const TString&) variable.first, &( (*variableMap)[variable.first] ) );
    }
}

float BDTReader::computeBDT(const std::map < std::string, float>& varMap){
    for(auto it = varMap.cbegin(); it != varMap.cend(); ++it){
        (*variableMap)[it->first] = it->second;
    }
    //retrieve bdt output and return
    return ( reader->EvaluateMVA(methodName) );
}
