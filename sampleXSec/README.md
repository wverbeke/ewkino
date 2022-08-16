# Tools and scripts for determining the cross-section of samples using GenXSecAnalyzer

### The GenXSecAnalyzer tool
The GenXSecAnalyzer is a built-in CMSSW analyzer module,  
that retrieves the generator cross-section from a MC sample.  
The tool can be run on existing MC samples retrieved from DAS.  
Note that the generator cross-section is not necessarily the best  
estimate of a process cross-section, but it can be used as a cross-check.  
More info [here](https://twiki.cern.ch/twiki/bin/viewauth/CMS/HowToGenXSecAnalyzer)  

### How to install the GenXSecAnalyzer
Before using the scripts in this folder,  
you need to install the GenXSecAnalyzer.  
In a CMSSW/src folder of your choice, download the analyzer
as explained [here](https://twiki.cern.ch/twiki/bin/viewauth/CMS/HowToGenXSecAnalyzer#Running_the_GenXSecAnalyzer_on_a).  
Note: you can choose a different name for the cfg file than `ana.py`  
(as used in the tutorial above), you will anyway need to provide this  
as an argument to the scripts in this folder.  
The scripts in this folder are wrappers around the `cmsRun ana.py` command.  

### How to run the scripts in this folder
You can run each script with the -h option,
to see a list of all available options and their usage.
