### Run hyperopt for cut optimization ###

## Input data format ##

The input data is supposed to be an ntuple .root file.
The cuts can be peformed both at event level (i.e. on variables with one value per ntuple entry)
and at any object level (i.e. variables with an array of values per ntuple entry),
by specifying the --nvar argument in run_hyperopt.py 
(e.g. set --nvar nL to consdider lepton objects).
Note: currently, having different nvar for different variables is not supported.
      In other words, you can only run hyperopt for one type of object simultaneously.
      Need to extend by adding this argument to the grid json file when the need arises.

## Search grid ##

# Defining the grid in json format #
The search grid can be specified via a json file.
This json file is supposed to contain a list of dicts,
where each dict has the following keys:
- "variable": name of the variable in the ntuple to cut on.
- "cuttype": either "min" (for selecting values > threshold) 
             or "max" (for selecting values < threshold).
- "hptype": name of a hyperopt range function, e.g. "quniform".
- "minvalue"
- "maxvalue"
- "stepsize"
Note: the last three options are specific to "quniform".
      Currently this is the only supported range function.
      Need to extend to other functions when the need arises.

# Making the grid #
Make the actual grid by running make_grid.py with options --inputjson and --outputfile.
The output file is a .pkl object containing the grid.

## Run hyperopt ##
Run run_hyperopt.py without options too see a list of command line arguments.
