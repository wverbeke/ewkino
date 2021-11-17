####################
# check input data #
####################

import sys
import os
import numpy as np

if __name__=='__main__':
    
    inputfile = sys.argv[1]

    with open(inputfile, 'r') as f:
        data = np.load(f)
    X_train = data[:,:-1]
    labels = np.squeeze(data[:,-1]).astype(int)

    # check if labels contain only 0 and 1
    print('checking labels...')
    one_inds = np.nonzero(labels==1)[0]
    zero_inds = np.nonzero(labels==0)[0]
    print('number of signal instances: {}'.format(len(one_inds)))
    print('number of background instances: {}'.format(len(zero_inds)))
    print('sum of both: {}'.format(len(one_inds)+len(zero_inds)))
    print('total number of instances: {}'.format(len(labels)))

    # check if data contains nans
    print('checking training data...')
    nans = np.isnan(X_train)
    print('number of nans: {}'.format(np.sum(nans)))
    # check if data contains infs
    infs = np.isinf(X_train)
    print('number of infs: {}'.format(np.sum(infs)))
    # check if data contains other extremely large values
    nlarges = len(np.nonzero(X_train>1e300)[0])
    print('number of insanely large values: {}'.format(nlarges))    

    # check segments of the data
    begin_idx = 56030
    end_idx = 56050
    print('segment from X_train')
    print(X_train[begin_idx:end_idx,:])
    print('segment from labels:')
    print(labels[begin_idx:end_idx])
