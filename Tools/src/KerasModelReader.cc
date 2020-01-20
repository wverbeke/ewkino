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
    loadPythonModule( modelName );
}


KerasModelReader::~KerasModelReader(){
    Py_Finalize();
}


void KerasModelReader::initializePythonAPI() const{
    setenv("PYTHONPATH", "../python", 1);
    Py_Initialize();
}


void KerasModelReader::loadPythonModule( const std::string& modelName ){
    try{
        pythonModule = python::import("kerasPredict");
        kerasModel =  pythonModule.attr("kerasModel")(modelName);
        predictRoutine = kerasModel.attr("predict");
    } catch(...){
        PyErr_Print();
    }
}


double KerasModelReader::predict( const std::vector<double>& inputs ) const{
    if( inputs.size() != numberOfInputs ){
        throw std::invalid_argument( "Number of inputs should be " + std::to_string( numberOfInputs ) + " while " + std::to_string( inputs.size() ) + " inputs are given." );
    }
    python::object inputList = vectorToPyList( inputs );
    python::object pythonOutput = predictRoutine( inputList );
    return python::extract<double>( pythonOutput );
}
