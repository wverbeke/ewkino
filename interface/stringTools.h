//methods for std::string formatting 

//include c++ library classes
#include <string>
#include <utility>

namespace stringTools{

    //remove all trailing spaces and tabs 
    std::string removeBackSpaces( const std::string& s );

	//remove all leading spaces and tabs
    std::string removeFrontSpaces( const std::string&s );

	//remove all leading and trailing spaces and tabs
    std::string cleanSpaces( const std::string& s );

	//extract first word from string ( separated by spaces )
    std::string extractFirst( std::string& s );

    //add trailing / to directoryName if needed
    std::string directoryName( const std::string& );

    //check whether string contains substring
    bool stringContains( const std::string& s, const std::string& substring );

    //check whether string ends with substring 
    bool stringEndsWith( const std::string& s, const std::string& ending );

    //split file name and extentions
    std::pair< std::string, std::string > splitFileExtension( const std::string& );
}
