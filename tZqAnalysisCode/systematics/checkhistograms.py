#########################################
# small script to print some histograms #
#########################################

import sys
import os
sys.path.append('../tools')
import histtools as ht

mustcontainall = ['tZq_','_eventBDT','JECAll_AbsoluteMPFBias']
mustcontainone = []
maynotcontainall = []
maynotcontainone = []

histfile = sys.argv[1]

ht.printhistograms(histfile, naninfo=True, mustcontainone=mustcontainone,
                            mustcontainall=mustcontainall,
                            maynotcontainone=maynotcontainone,
                            maynotcontainall=maynotcontainall)
