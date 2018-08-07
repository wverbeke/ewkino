#include "../interface/BDTReader.h"

BDTReader::BDTReader(const std::string& bdtName, const std::string& xmlFileName, const std::vector<std::string>& varsToUse) : 
    methodName(bdtName)
{

    //set up mapping of variables and their values
    std::map < std::string, float> varMap;
    for(auto& varName : varsToUse){
        varMap[varName] = 0.;
    }
    variableMap = varMap;

    reader.reset( new TMVA::Reader("!Color:!Silent") );

    //set all variables of Reader 
    addVariables(varsToUse);

    //book method
    reader->BookMVA(methodName, xmlFileName);
}

void BDTReader::addVariables(const std::vector<std::string>& varsToUse){
    for(auto& variable : varsToUse){

        //refer TMVA reader to values in map
        reader->AddVariable( (const TString&) variable, &( variableMap[variable] ) );
    }
}

float BDTReader::computeBDT(const std::map < std::string, float>& varMap){

    //check if all variables are present in map passed as argument
    static bool firstEvent = true;
    if(firstEvent){
        for(auto it = variableMap.cbegin(); it != variableMap.cend(); ++it){
            if(varMap.find(it->first) == varMap.end()){
                std::cerr << "Error in BDTReader computation, map passed as argument does not contain variable : " << it->first << std::endl;
            }
        }
        firstEvent = false;

    }
    for(auto it = variableMap.begin(); it != variableMap.end(); ++it){
        it->second = (varMap.find(it->first) )->second;
    }

    //retrieve bdt output and return
    return ( reader->EvaluateMVA(methodName) );
}
