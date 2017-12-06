#include "../interface/ewkinoTools.h"

//return run period for 2017 data
unsigned ewk::runPeriod(const unsigned long runNb){
    if(runNb < 297020){
        return 0; //Run A
    } else if(runNb < 299337){
        return 1; //Run B
    } else if(runNb < 302030){
        return 2; //Run C
    } else if(runNb < 303435){
        return 3; //Run D
    } else if(runNb < 304911){
        return 4; //Run E
    } else if(runNb < 306827){
        return 5; //Run F
    } else{
        return 6;
    }
}
