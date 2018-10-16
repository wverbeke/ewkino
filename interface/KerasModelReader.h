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
        //typedef size_t numInputs;

        void initializePythonAPI() const;

        void loadKerasModel(const std::string&) const;

        void loadPythonModule();

        python::object pythonModule;
        python::object predictRoutine;
        size_t numberOfInputs;

};
#endif
