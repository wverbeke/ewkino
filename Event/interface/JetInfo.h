#ifndef JetInfo_H
#define JetInfo_H

// include c++ library classes
#include <map>
#include <string>
#include <algorithm>


class JetInfo{

    public: 

	JetInfo( const TreeReader&, 
		 const bool readAllJECVariations = false, 
		 const bool readGroupedJECVariations = false );
	std::vector< std::string > allJECVariations(){ return _JECSources; }
	std::vector< std::string > allValidJECVariations(){ return _JECSourcesValid; }
	std::vector< std::string > groupedJECVariations(){ return _JECGrouped; }
	std::vector< std::string > groupedValidJECVariations(){ return _JECGroupedValid; }

	bool hasJECVariation( const std::string& ) const;
	void printAllJECVariations() const;
	void printAllValidJECVariations() const;
	void printGroupedJECVariations() const;
	void printGroupedValidJECVariations() const;

    private:

	std::vector< std::string > _JECSources;
	std::vector< std::string > _JECSourcesValid;
	std::vector< std::string > _JECGrouped;
	std::vector< std::string > _JECGroupedValid;
};

#endif
