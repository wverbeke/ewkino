###################################################
# evaluate a neural network and plot some metrics #
###################################################

import sys
import os
import numpy as np
#from tensorflow.keras.models import load_model
from tensorflow import keras
sys.path.append('../tools')
import optiontools as opt
import metrictools as mt
sys.path.append('../plotting')
from plotmetrics import plot_stosqrtn, plot_roc


if __name__=='__main__':

    options = []
    options.append( opt.Option('model', vtype='path') )
    options.append( opt.Option('evaldata', vtype='path') )
    options.append( opt.Option('outputfile', vtype='path') )
    options = opt.OptionCollection( options )
    if len(sys.argv)==1:
        print('Use with following options:')
        print(options)
        sys.exit()
    else:
        options.parse_options( sys.argv[1:] )
        print('Found following configuration:')
        print(options)

    # parse arguments
    if not os.path.exists(options.model):
        raise Exception('ERROR: model file {} does not seem to exist.')
    if not os.path.exists(options.evaldata):
        raise Exception('ERROR: evaluation data file {} does not seem to exist.')
    outputfile = os.path.splitext(options.outputfile)[0]

    # load evaluation data
    with open(options.evaldata, 'r') as f:
        evaldata = np.load(f)
    #evaldata = evaldata[:1000,:]
    print('Loaded evaluation data with shape {}'.format(evaldata.shape))

    # load model
    model = keras.models.load_model(options.model)

    # get true and predicted labels
    y_true = evaldata[:,-1].astype(int)
    X_eval = evaldata[:,:-1]
    y_pred = model.predict(X_eval)

    # make a metric calculator
    metric = mt.ContinuousMetricCalculator( y_true, y_pred )
    wprange = np.linspace(0, 1, num=100)

    # plot the signal and background efficiencies
    effs = metric.signal_efficiency( wprange )
    effb = metric.background_efficiency( wprange )
    stosqrtn = metric.stosqrtn( wprange )
    fig,ax = plot_stosqrtn( wprange, stosqrtn, label='S/sqrt(S+B)',
				sig_eff=effs, sig_label='signal efficiency',
				bck_eff=effb, bck_label='background efficiency' )
    fig.savefig(outputfile+'_stosqrtn.png')

    # plot the roc curve
    (effs,effb) = metric.roc( wprange )
    fig,ax = plot_roc( effs, effb, label='ROC', plotstyle='plot' )
    fig.savefig(outputfile+'_roc.png')
