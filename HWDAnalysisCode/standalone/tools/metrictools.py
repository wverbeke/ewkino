##########################################################################
# tools for calculating various metrics from given signal and background #
##########################################################################

import math

class MetricCalculator():

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
