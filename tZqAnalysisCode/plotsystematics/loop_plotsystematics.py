######################################################################
# temporary script to run a few plotsystematics commands in sequence #
######################################################################

import sys
import os
sys.path.append(os.path.abspath('../../jobSubmission/'))
import condorTools as ct

if __name__=='__main__':

  # define which jobs to run
  years = ['2016','2017','2018']
  processes = ['WZ','ZZH','tX','tZq','ttZ','multiboson','Xgamma','all']

  # settings common to all jobs
  inputfile = os.path.join('../systematics/output_tzqidmedium0p4_rescaledlumi/{}combined/',
                           'signalregion_cat123/npfromdata/combined.root')
  variables = 'variables/variables_full.json'
  outputdir = 'output_temp'
  includetags = 'fsr'
  excludetags = ''
  tags = '{}_simulation,Inclusive_signal_region'

  # loop over jobs
  cmds = []
  for year in years:
    for process in processes:
      thisinputfile = inputfile.format(year)
      thisoutputdir = os.path.join(outputdir,year,process)
      thistags = tags.format(year)
      cmd = 'python plotsystematics.py'
      cmd += ' --inputfile {}'.format(thisinputfile)
      cmd += ' --processes {}'.format(process)
      cmd += ' --variables {}'.format(variables)
      cmd += ' --outputdir {}'.format(thisoutputdir)
      if len(includetags)>0:
        cmd += ' --includetags {}'.format(includetags)
      if len(excludetags)>0:
        cmd += ' --excludetags {}'.format(excludetags)
      if len(tags)>0:
        cmd += ' --tags {}'.format(thistags)
      cmds.append(cmd)

  # submit job
  ct.submitCommandsAsCondorJob( 'cjob_plotsystematics', cmds )
