############################################################
# small testing script for custom tensorflow/keras metrics #
############################################################

import sys
import os
import numpy as np
import tensorflow as tf
sys.path.append('../tools')
import metrictools as mt

y_true = np.array([1,1,1,0,0,0])
y_pred = np.array([0.9,0.7,0.5,0.6,0.2,0.1])

y_true = tf.constant(y_true, dtype=tf.float32)
y_pred = tf.constant(y_pred, dtype=tf.float32)

res = mt.KerasMetricCalculator.stosqrtn( y_true, y_pred, threshold=0.45 )
print(res)
with tf.Session() as sess:  print(res.eval()) 
