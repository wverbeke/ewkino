##########################################################################
# tools for calculating various metrics from given signal and background #
##########################################################################

import numpy
import math
import tensorflow as tf
import numpy as np


class MetricCalculator():
    ### calculation of metrics with given number of passing and failing instances

    def __init__( self, nsig_tot, nsig_pass, nbck_tot, nbck_pass ):
	self.nsig_tot = nsig_tot
	self.nsig_pass = nsig_pass
	self.nbck_tot = nbck_tot
	self.nbck_pass = nbck_pass

    def signal_efficiency( self ):
	return float(self.nsig_pass) / self.nsig_tot

    def background_efficiency( self ):
	return float(self.nbck_pass) / self.nbck_tot

    def auc( self ):
	### AUC-like metric where the ROC is approximated by a rectangle defined by the WP
	return self.signal_efficiency() * (1 - self.background_efficiency())

    def stob( self ):
	return float(self.nsig_pass) / self.nbck_pass

    def negstob( self ):
	return -self.stob()

    def stosqrtb( self ):
	return float(self.nsig_pass) / math.sqrt(self.nbck_pass)

    def negstosqrtb( self ):
	return -self.stosqrtb()

    def stosqrtn( self ):
	return float(self.nsig_pass) / math.sqrt(self.nsig_pass + self.nbck_pass)

    def negstosqrtn( self ):
	return -self.stosqrtn()

    # to extend whenever needed...


class ContinuousMetricCalculator():
    ### calculation of metrics with given true and predicted labels
    
    def __init__( self, y_true, y_pred ):
	self.y_pred = y_pred
	self.sig_mask = (y_true==1)
	self.bck_mask = (y_true==0)
	self.nsig = np.sum(self.sig_mask.astype(int))
	self.nbck = np.sum(self.bck_mask.astype(int))

    def npass( self, wprange, mask=None ):
	npass = np.zeros(len(wprange))
	for i, wp in enumerate(wprange):
	    if mask is None: npass[i] = len(np.nonzero(self.y_pred>wp)[0])
	    else: npass[i] = len(np.nonzero(self.y_pred[mask]>wp)[0])
	return npass

    def signal_efficiency( self, wprange ):
	return self.npass( wprange, mask=self.sig_mask )/float(self.nsig)

    def background_efficiency( self, wprange ):
	return self.npass( wprange, mask=self.bck_mask )/float(self.nbck)

    def roc( self, wprange ):
	return (self.signal_efficiency(wprange), self.background_efficiency(wprange))

    def stosqrtn( self, wprange ):
	ns = self.npass( wprange, mask=self.sig_mask )
	nb = self.npass( wprange, mask=self.bck_mask )
	return np.divide( ns, np.sqrt(ns+nb) )
    

class KerasMetricCalculator():
    ### collection of static functions that can be used in keras internally
    # note that they operate on tensorflow Tensor objects, not numpy arrays!

    @classmethod
    def stosqrtn( self, y_true, y_pred, threshold=0.5 ):
	sig_pred = tf.boolean_mask(y_pred, tf.equal(y_true,tf.constant(1,dtype=tf.float32)))
	bck_pred = tf.boolean_mask(y_pred, tf.equal(y_true,tf.constant(0,dtype=tf.float32)))
	sig_mask = tf.cast(tf.greater(sig_pred, tf.constant(threshold,dtype=tf.float32)), tf.float32)
	bck_mask = tf.cast(tf.greater(bck_pred, tf.constant(threshold,dtype=tf.float32)), tf.float32)
	try: # newer versions of tensorflow
	    nsig_pass = tf.math.reduce_sum( sig_mask )
	    nbck_pass = tf.math.reduce_sum( bck_mask )
	    sqrtn_pass = tf.math.sqrt( nsig_pass + nbck_pass )
	except: # older versions of tensorflow
	    nsig_pass = tf.reduce_sum( sig_mask )
	    nbck_pass = tf.reduce_sum( bck_mask )
	    sqrtn_pass = tf.sqrt( nsig_pass + nbck_pass )
	stosqrtn = nsig_pass / sqrtn_pass
	# printouts for testing
	#with tf.Session() as sess:
	#	print(nsig_pass.eval())
	#	print(nbck_pass.eval())
	#	print(sqrtn_pass.eval())
	#	print(stosqrtn.eval())
	return stosqrtn

    @classmethod
    def negstosqrtn( self, y_true, y_pred, threshold=0.5 ):
	return -stosqrtn( y_true, y_pred, threshold=threshold )
