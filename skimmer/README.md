##Tools for skimming and merging ntuplizer output

The procedure usually consists of two steps: skimming and merging. 
The scripts that can be used for both operations are listed below,
as well as some utilities.

###Utilities
These include the following files:  
- fileListing: tools for listing, grouping and splitting file names  
- jobSubmission: (older) tools for sumbitting qsub jobs  
Note: use the scripts condorTools in ewkino/jobSubmission for condor jobs instead

###Skimming
For this step the following scripts are available:  
- skimTuples.py: (older) script for skimming all samples in a directory with fixed version name  
- skimTuplesFromList.py: (newer) script for skimming using a sample list  
Note: skimTuples.py has not been used since a while and might require some updates (e.g. to condor).  
Note: both scrips call the ./skimmer executable, built from skimmer.cc by makeSkimmer.

###Merging
For this step the following scripts are available:  
- mergeTuples.py: merges all files in a directory (output from skimming step) into one file, using hadd  
- mergeHadd.py: merges files given on the command line into one file, using hadd  
- mergeDataSets.py: merges fils given on the command line into one file, with removal of duplicate events  
Note: mergeDataSets.py calls the ./mergeDataSets executable, built from mergeDataSets.cc by makeMergeDataSets.
