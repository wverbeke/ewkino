#include "../interface/KerasModelReader.h"

//include c++ library functions
#include <iostream>


//convert std::vector to python list 
template <class T> inline boost::python::list vectorToPyList(const std::vector<T>& vector) {
    python::list pyList;
    for(auto& entry : vector ){
        pyList.append( entry );
    }
    return pyList;
}


KerasModelReader::KerasModelReader( const std::string& modelName, size_t numInputs):
    numberOfInputs( numInputs )
{
    initializePythonAPI();
    loadPythonModule();
    loadKerasModel( modelName );
}


KerasModelReader::~KerasModelReader(){
    Py_Finalize();
}


void KerasModelReader::initializePythonAPI() const{
    setenv("PYTHONPATH", "./python", 1);
    Py_Initialize();
}

void KerasModelReader::loadPythonModule(){
    try{
        pythonModule = python::import("kerasPredict");
        predictRoutine = pythonModule.attr("predict");
    } catch(...){
        PyErr_Print();
    }
}


void KerasModelReader::loadKerasModel( const std::string& modelName ) const{
    try{
        python::object loadModelRoutine = pythonModule.attr("loadModel");
        loadModelRoutine( modelName );
    } catch(...){
        PyErr_Print();
    }
}


double KerasModelReader::predict( const std::vector<double>& inputs ) const{
    if( inputs.size() != numberOfInputs ){
        std::cerr << "Error in KerasModelReader::predict : number of inputs given does not correspond to number of inputs. Returning 9999." << std::endl;
        return 9999.;
    }
    python::object inputList = vectorToPyList( inputs );
    python::object pythonOutput = predictRoutine( inputList );
    return python::extract<double>( pythonOutput );
}
