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

BDTReader::BDTReader(const std::string& bdtName, const std::string& xmlFileName, const std::map < std::string, float >& varMap, const std::vector<std::string>& varsToUse):
    variableMap(std::make_shared< std::map < std::string, float > >(varMap) ), methodName(bdtName)
{
    //set up BDT reader 
    reader.reset( new TMVA::Reader("!Color:!Silent") );    

    //safety check that every element in the variables to use is in variableMap
    for(auto& varName : varsToUse){
        if(variableMap->find(varName) == variableMap->end()){
            std::cerr << "Error: Requesting to use variable that is not in the map of variables in BDTReader constructor. Reader will remain uninitialized." << std::endl;
            return;
        }
    }

    //for every variable to use, add it to the list for the MVA method
    for(auto& varName : varsToUse){
        reader->AddVariable( (const TString&) varName, &( (*variableMap)[varName] ) );
    }

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
