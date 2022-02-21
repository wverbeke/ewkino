**Tools for measuring b-tagging efficiencies and reading scale factors**  
  
B-tagging efficiencies can be measured with computeBTagEfficienciesMC.cc.
This is a custom script that simply runs over MC events and measures
the tagging efficiencies and mistagging rates for b-jets.  
  
Centrally provided b-tagging scale factors are available in csv format.
They are read by BTagCalibrationUL.cc.
Note that this is the UL version (with different naming conventions w.r.t. pre-UL). 
The pre-UL version is currently moved to the deprecated folder and not used.
Also note that in order to switch, one should modify ewkino/codeLibrary.cc,
as the pre-UL and UL version cannot be simultaneously compiled because of conflicting name spaces.
