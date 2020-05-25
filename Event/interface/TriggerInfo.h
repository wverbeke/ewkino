#ifndef TriggerInfo_H
#define TriggerInfo_H

//include c++ library classes 
#include <map>
#include <string>

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"


class TriggerInfo{

    public:

        //WARNING : turning on 'readIndividualTriggers' and/or 'readIndividualMETFilters' is relatively slow ( takes slightly more time than building the entire event! )
        TriggerInfo( const TreeReader&, const bool readIndividualTriggers = false, const bool readIndividualMetFilters = false );

        bool passTriggers_e() const{ return _passTriggers_e; }
        bool passTriggers_m() const{ return _passTriggers_m; }
        bool passTriggers_ee() const{ return _passTriggers_ee; }
        bool passTriggers_em() const{ return _passTriggers_em; }
        bool passTriggers_et() const{ return _passTriggers_et; }
        bool passTriggers_mm() const{ return _passTriggers_mm; }
        bool passTriggers_mt() const{ return _passTriggers_mt; }
        bool passTriggers_eee() const{ return _passTriggers_eee; }
        bool passTriggers_eem() const{ return _passTriggers_eem; }
        bool passTriggers_emm() const{ return _passTriggers_emm; }
        bool passTriggers_mmm() const{ return _passTriggers_mmm; }
        bool passTriggers_FR() const{ return _passTriggers_FR; }
        bool passTriggers_FR_iso() const{ return _passTriggers_FR_iso; }
	bool passTriggers_ref() const{ return _passTriggers_ref; }
        bool passMetFilters() const{ return _passMetFilters; }
        bool passTrigger( const std::string& ) const;
        bool passMetFilter( const std::string& ) const;

        void printAvailableIndividualTriggers() const;
        void printAvailableMetFilters() const;

    private:
        bool _passTriggers_e;
        bool _passTriggers_m;
        bool _passTriggers_ee;
        bool _passTriggers_em;
        bool _passTriggers_et;
        bool _passTriggers_mm;
        bool _passTriggers_mt;
        bool _passTriggers_eee;
        bool _passTriggers_eem;
        bool _passTriggers_emm;
        bool _passTriggers_mmm;
        bool _passTriggers_FR;
        bool _passTriggers_FR_iso;
	bool _passTriggers_ref;
        bool _passMetFilters;
        std::map< std::string, bool > individualTriggerMap; 
        std::map< std::string, bool > individualMetFilterMap; 
};

#endif 
