/*
Code to merge root files and remove overlap.
This is necessary to merge several primary datasets and make sure no events are double counted.
*/

#ifndef mergeAndRemoveOverlap_H
#define mergeAndRemoveOverlap_H

//include c++ library classes
#include <vector>
#include <string>

//merge several ROOT files and remove overlap
//input is a vector of file paths (strings) and the output path
void mergeAndRemoveOverlap( const std::vector< std::string >&, const std::string&, const bool allowMergingYears = false );

#endif
