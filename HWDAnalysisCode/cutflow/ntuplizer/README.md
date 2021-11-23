**Apply cutflow plotting on ntuplizer level**

Input: the scripts in this folder assume an ntuple with a variable that takes discrete values depending on which cut exactly was failed. See the DEfficiencyAnalyzer for an example.

Here, a histogram will be filled by simply copying that value from the ntuple.
Next, the same plotting procedures are applied to the resulting histogram as are applied to histograms resulting from the event selection level. 
There is one exception: the labels for each discrete value need to be set manually using a json file, as they are not stored in the ntuple. Care should be taken to make sure the labels correspond to what was actually used in the ntuplizer!
