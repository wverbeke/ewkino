#include "../interface/BDTReader.h"

BDTReader::BDTReader(const std::string& bdtName, const std::string& xmlFileName, const std::shared_ptr< std::map < std::string, float > >& varMap):
    variableMap(varMap), methodName(bdtName)
{
    reader.reset( new TMVA::Reader("!Color:!Silent") );

    //set all variables
    addVariables();

    //book method 
    reader->BookMVA(methodName, xmlFileName); 
}

BDTReader::BDTReader(const std::string& bdtName, const std::string& xmlFileName, const std::map < std::string, float >& varMap):
    BDTReader(bdtName, xmlFileName, std::make_shared< std::map < std::string, float> >(varMap) ) {} 

BDTReader::BDTReader(const std::string& bdtName, const std::string& xmlFileName, const std::map < std::string, float >& varMap, const std::vector<std::string>& varsToUse){

    //construct new map, containing only the variables specified in the 'varsToUse' vector
    std::map < std::string, float > selectedVarMap;
    for(auto& varName : varsToUse){

        //create entries in map by adding dummy 0 elements
        selectedVarMap[varName] = 0.;
    }
    
    //use new map to construct reader
    *this = BDTReader(bdtName, xmlFileName, selectedVarMap);
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
