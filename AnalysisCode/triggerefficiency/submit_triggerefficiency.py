#############################################################################################
# very simple sumbitter to do triggerefficiency measurement with conventional folder naming #
#############################################################################################

import sys
import os

years = []
years.append('2016')
years.append('2017')
years.append('2018')
years.append('allyears')
event_selections = []
#event_selections.append('3tight')
event_selections.append('3tight_ptcuts')
#event_selections.append('3fo')
#event_selections.append('3fo_recoptcuts')
#event_selections.append('3fo_coneptcuts')
event_selections.append('3fo_3tightveto_recoptcuts')
#event_selections.append('3fo_3tightveto_coneptcuts')
for year in years:
    for event_selection in event_selections:
	inputfolder = '~/Files/trileptonskim/'+year+'trigger'
	outputfolder = 'output/'+event_selection+'_'+year
	if os.path.exists(outputfolder):
	    os.system('rm -r '+outputfolder)
	command = 'python triggerefficiency.py '+inputfolder+' '+outputfolder+' '+event_selection
	os.system(command)
