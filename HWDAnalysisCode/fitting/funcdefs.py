###############################
# definition of fit functions #
###############################

import numpy as np

### polynomial functions

def polynomial(x, parameters):
    ### function of the form:
    # y = sum_k parameters[k] * x**k
    res = parameters[0]
    for k in range(1,len(parameters)):
	res += parameters[k]*np.power(x[0],k)
    return res
