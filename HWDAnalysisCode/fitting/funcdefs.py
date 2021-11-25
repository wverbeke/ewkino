###############################
# definition of fit functions #
###############################
# to do: see if duplication of function definitions in standard/RooFit format can be avoided,
#        but probably not...

import numpy as np
import ROOT

### polynomial functions

def polynomial(x, parameters):
    ### function of the form:
    # y = sum_k parameters[k] * x**k
    res = parameters[0]
    for k in range(1,len(parameters)):
	res += parameters[k]*np.power(x[0],k)
    return res

def RooFitPolynomial(x, parameters, bounds=None):
    ### same as polynomial but in RooFit format
    # note: x is supposed to be a RooRealVar!
    # note: internally, the constant parameter is set fixed to 1 
    #       and a normalization that affects all parameters is used instead.
    # note: a bunch of objects that are usually not needed are returned as well,
    #       since this is needed to keep them in memory and avoid nullptr exceptions...
    if bounds is None: bounds = [(-1e5,1e5)]*len(parameters)
    roorealvars = []
    for i,param in enumerate(parameters):
	if(i==0): continue
	name = 'a{}'.format(i)
	boundsi = bounds[i]
	roorealvars.append( ROOT.RooRealVar(name,name,param,boundsi[0],boundsi[1]) )
    rooarglist = ROOT.RooArgList()
    for var in roorealvars: rooarglist.add(var)
    pdf = ROOT.RooPolynomial("polypdf","polypdf",x,rooarglist)
    norm = ROOT.RooRealVar("norm","norm",1.,0.,2.)
    roorealvars.append(norm)
    func = ROOT.RooExtendPdf("poly","poly",pdf,norm)
    otherstuff = {'pdf':pdf, 'otherparams':roorealvars}
    return (func,norm,otherstuff)
