/*
Some useful functions used at multiple places in this directory 
*/

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <map>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TROOT.h"

// include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/HistInfo.h"

std::vector< std::shared_ptr< TH1 > > getHistogramsFromFile( const std::string& );

std::vector< std::shared_ptr< TH1 > > selectHistograms( std::vector<std::shared_ptr<TH1>>,
                std::vector<std::string>, std::vector<std::string>);

std::shared_ptr< TH1 > addHistograms( std::vector<std::shared_ptr<TH1>> );

std::map< std::string, std::shared_ptr<TH1> > getEnvelope( std::vector< std::shared_ptr<TH1> >,
						    std::shared_ptr<TH1>);

std::map< std::string, std::shared_ptr<TH1> > getRMS( std::vector< std::shared_ptr<TH1> >, 
						    std::shared_ptr<TH1>);
