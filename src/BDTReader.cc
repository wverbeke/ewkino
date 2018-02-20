#include "../interface/BDTReader.h"

BDTReader::BDTReader(const std::string& bdtName, const std::shared_ptr< Category >& cat, const std::map < std::string, float>& varMap):
    methodName(bdtName), category(cat), variableMap(varMap){
    for(size_t c = 0; c < category->size(); ++c){
        readers.push_back( std::shared_ptr<TMVA::Reader> ( new TMVA::Reader( "!Color:!Silent" ) ) );
        //set all variables         
        addVariables(readers[c]);
        //book method
        readers[c]->BookMVA(methodName, mvaMethodName(c) + ".xml");
    }            

}

void BDTReader::addVariables(const std::shared_ptr< TMVA::Reader >& reader){
    for(auto mapIt = variableMap.begin(); mapIt != variableMap.end(); ++mapIt){
        reader->AddVariable( (const TString&) mapIt->first, &variableMap[mapIt->first]);
    }
}

std::string BDTReader::mvaMethodName(const size_t categoryIndex) const{
    return ( category->name(categoryIndex) + "_" + methodName );
}

//compute mva value for given category's mva
float BDTReader::computeBDT(const size_t categoryIndex, const std::map < std::string, float>& varMap){
    for(auto mapIt = varMap.cbegin(); mapIt != varMap.cend(); ++mapIt){
        variableMap[mapIt->first] = mapIt->second;
    }
    return ( readers[categoryIndex]->EvaluateMVA(methodName) );
}

float BDTReader::computeBDT(const std::vector<size_t>& categoryIndices, const std::map < std::string, float>& varMap){
    size_t categoryIndex = category->getIndex(categoryIndices);
    return computeBDT(categoryIndex, varMap);
}
