##########################################
# basic functionality for muon selection #
##########################################

import sys
import os
from muon import Muon, MuonCollection

def select_default( muon ):
    if muon.pt < 26: return False
    if abs(muon.eta) > 2.4: return False
    if not muon.istight: return False
    if muon.isolation > 0.15: return False
    return True

def select( muoncollection, selection ):
    selection_to_function_map = {'default': select_default}
    if not selection in selection_to_function_map:
        raise Exception('ERROR: unrecognized muon selection: {}'.format(selection))
    res = [True]*len(muoncollection)
    for i,muon in enumerate(muoncollection):
        if not selection_to_function_map[selection]( muon ): res[i] = False
    return res

