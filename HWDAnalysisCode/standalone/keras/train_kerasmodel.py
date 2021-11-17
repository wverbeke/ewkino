######################################
# train a keras neural network model #
######################################

import sys
import os
import json
import pickle
import numpy as np
sys.path.append('../tools')
import optiontools as opt
import logtools as lt
import metrictools as mt


def get_dense_model(input_size, architecture, 
		    activation=None, last_activation=None,
		    optimizer='adam', loss='binary_crossentropy',
		    do_normalization=False,
		    metrics=[]):
    ### get a trainable dense neural network model
    # input args:
    # - input_size: size of vector that model will operate on
    # - architecture: list of number of nodes per layer
    # - activation: list of activations per layer (default: linear in each layer)
    # - last_activation: override activation in last layer (default: determined by activation)
    # - optimizer: optimizer to use (default: adam)
    # - loss: loss function to use (defualt: binary crossentropy)
    # - do_normalization: add a normalization layer as first layer
    #   note: if true, should call model.adapt() before model.fit()!
    #   note: does not seem to be supported in keras version on T2B,
    #         use batch normalization instead...
    # - metrics: list of metrics to track during training
    
    import math
    from keras.models import Sequential, load_model
    from keras.layers import Dense, Dropout, Activation, Lambda
    #from keras.layers import Normalization
    from keras.layers import BatchNormalization
    from keras.utils import np_utils
    import keras.optimizers as kopt
    import keras.activations as kact
    import tensorflow as tf
    from keras import backend as K

    # parse arguments
    if activation is None: activation = ['linear']*len(architecture)
    elif not isinstance(activation,list): activation = [activation]*len(architecture)
    if last_activation is not None: activation[-1] = last_activation

    layers = []
    # normalization layer
    if do_normalization:
	#layers.append(Normalization(input_dim=input_size))
	layers.append(BatchNormalization(input_shape=(input_size,)))
    # first layer manually to set input_dim
    layers.append(Dense(architecture[0],activation=activation[0],input_dim=input_size))
    # rest of layers in a loop
    for nnodes,act in zip(architecture[1:],activation[1:]):
        layers.append(Dense(nnodes,activation=act))
    model = Sequential()
    for i,l in enumerate(layers):
        model.add(l)
    model.compile(optimizer=optimizer, loss=loss, metrics=metrics)
    model.summary()
    return model


def make_sets_equal( X_train, labels ):
    ### make equally large signal and background training sets
    # by removing instances of the category that is overrepresented
    nsig = int(np.sum(labels))
    nbck = len(labels)-nsig
    X_sig = X_train[np.nonzero(labels>0.5)[0],:]
    X_bck = X_train[np.nonzero(labels<0.5)[0],:]
    nlimit = min(nsig,nbck)
    X_sig = X_sig[:nlimit,:]
    X_bck = X_bck[:nlimit,:]
    X_train = np.vstack((X_sig,X_bck))
    labels = np.concatenate((np.ones(nlimit),np.zeros(nlimit)))
    labels = np.expand_dims(labels,1)
    data = np.concatenate((X_train,labels),axis=1)
    np.random.shuffle(data)
    X_train = data[:,:-1]
    labels = np.squeeze(data[:,-1])
    return (X_train, labels)


def get_class_weight( X_train, labels ):
    ### get weights that make classes balanced
    # note: alternative to make_sets_equal
    nsig = int(np.sum(labels))
    nbck = len(labels)-nsig
    n = len(labels)
    weights = {0:float(nsig)/n, 1:float(nbck)/n}
    return weights


if __name__=='__main__':

    # read options
    options = []
    options.append( opt.Option('inputdata', vtype='path') )
    options.append( opt.Option('configuration', vtype='path') )
    options.append( opt.Option('outputdir', vtype='path') )
    options.append( opt.Option('savemodel', vtype='bool', default=True) )
    options.append( opt.Option('savehistory', vtype='bool', default=True) )
    options.append( opt.Option('istest', vtype='bool', default=False) )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )

    # reset some options for testing mode
    if options.istest: 
	print('WARNING: found "--istest",'
		+' will overwrite "--savemodel" and "--savehistory"')
	options.savemodel = False
	options.savehistory = False

    # parse options
    if not options.inputdata[-4:]=='.npy':
        raise Exception('ERROR: input file is supposed to be an ndarray in .npy')
    if not os.path.exists(options.inputdata):
        raise Exception('ERROR: input file {} does not seem to exist...'.format(options.inputdata))
    if not options.configuration[-5:]=='.json':
        raise Exception('ERROR: configuration file is supposed to be in .json format')
    if not os.path.exists(options.configuration):
        raise Exception('ERROR: configuration file {} does not seem to exist...'.format(options.configuration))
    if( options.savemodel or options.savehistory ):
	if not os.path.exists(options.outputdir):
	    os.makedirs(options.outputdir)
 
    # read input data
    with open(options.inputdata, 'r') as f:
	data = np.load(f)
    #np.random.shuffle(data)
    X_train = data[:,:-1]
    labels = np.squeeze(data[:,-1])
    print('loaded input data with following shape:')
    print('training set: {}'.format(X_train.shape))
    print('labels: {}'.format(labels.shape))
    print('example training data:')
    print(X_train[:5,:])
    print('example labels:')
    print(labels[:5])
    nsig = int(np.sum(labels))
    nbck = len(labels)-nsig
    print('number of signal instances: {}'.format(nsig))
    print('number of background instances: {}'.format(nbck))
    X_sig = X_train[np.nonzero(labels>0.5)[0],:]
    X_bck = X_train[np.nonzero(labels<0.5)[0],:]
    print('example signal instances:')
    print(X_sig[:5,:])
    print('example background instances:')
    print(X_bck[:5,:])
    # temp for testing: remove part of the background for more balanced sets
    #(X_train,labels) = make_sets_equal(X_train,labels)
    #print('example data:')
    #print(data[:10,:])
    # alternatively: get weights to use in training
    class_weight = get_class_weight(X_train, labels)
    print('will use following weights: {}'.format(class_weight))

    # read configuration
    with open(options.configuration, 'r') as f:
	configuration = json.load(f)
    print('loaded following configuration:')
    print(configuration)
    
    # get configuration for building the network
    if 'architecture' in configuration.keys():
	architecture = configuration.pop('architecture')
    else:
	raise Exception('ERROR: no "architecture" found in configuration.')
    if 'activation' in configuration.keys():
	activation = configuration.pop('activation')
    else:
	print('WARNING: no "activation" found in configuration, using linear default.')
	activation = 'linear'
    if 'last_activation' in configuration.keys():
	last_activation = configuration.pop('last_activation')
    else:
	print('WARNING: no "last_activation" found in configuration, using default')
	last_activation = None
    if 'optimizer' in configuration.keys():
	optimizer = configuration.pop('optimizer')
    else:
	print('WARNING: no "optimizer" found in setings, using adam default')
	optimizer = 'adam'
    if 'loss' in configuration.keys():
	loss = configuration.pop('loss')
    else:
	print('WARNING: no "loss" found in configuration, using binary_crossentropy default')
	loss = 'binary_crossentropy'
    
    # get configuration for training the network
    if 'training_options' in configuration.keys():
	training_options = configuration['training_options']
    else:
	print('WARNING: no "training_options" found in configuration, using empty dict default')
	training_options = {}

    # overwrite some parameters for testing
    if options.istest:
	print('WARNING: found "--istest",'
		+' resetting some training parameters!')
	training_options['epochs'] = 5
	nmax = 1000
	if len(X_train)>nmax:
	    X_train = X_train[:nmax]
	    labels = labels[:nmax]
	options.savemodel = False
	options.savehistory = False

    # options that are hard-coded for now
    metrics = []
    do_normalization = True

    # build and train the network
    model = get_dense_model(X_train.shape[1], architecture, 
                activation=activation, last_activation=last_activation,
		optimizer=optimizer, loss=loss, 
		do_normalization=do_normalization,
		metrics=metrics)
    history = model.fit(X_train, labels, class_weight=class_weight, **training_options)

    # write output files
    if( options.savemodel or options.savehistory ):
	print('training done; saving output...')
	mfile = os.path.join(options.outputdir, 'model')
	if options.savemodel: model.save(mfile)
	hfile = os.path.join(options.outputdir, 'history.pkl')
	if options.savehistory:
	    with open(hfile, 'w') as f:
		pickle.dump(history.history, f)

    print('###done###')
