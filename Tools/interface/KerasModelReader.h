/*
Class to make predictions from a trained Keras neural network 
*/

#ifndef KerasModelReader_H
#define KerasModelReader_H

//include libraries for interfacing c++ with python
#include <Python.h>
#include <boost/python.hpp>
namespace python = boost::python;

//include c++ library classes 
#include <vector>


class KerasModelReader{

    public:
        KerasModelReader(const std::string& modelName, size_t);
        ~KerasModelReader();

        double predict( const std::vector<double>& ) const;
        
    private:
        void initializePythonAPI() const;

        void loadPythonModule( const std::string& );

        python::object pythonModule;
        python::object predictRoutine;
        python::object kerasModel;
        size_t numberOfInputs;
};
#endif
