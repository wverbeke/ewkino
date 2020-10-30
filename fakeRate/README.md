# a readme for the fake rate code

Almost every step in the fake rate measurement (e.g. magic factor determination, fake rate measurement from MC, fake rate measurement from data, etc.) is split between a 'filling' stage and a 'plotting' or 'fitting' stage. In the first step the histograms are filled. Often many histograms (one per file) are filled in parallel using job submission. Once all the jobs are done, they have to be merged and then the plotting step can be invoked on the resulting file(s).

For example: to do a fake rate measurement from MC, you would do:
- python fillMCFakeRateMeasurement.py
(after having verified that the settings in this file are correct, i.e. the correct years and lepton flavours are chosen and the sample directory and sample list are set correctly)
- wait till all jobs finish
- move the resulting output .root files to a folder MCFakeRateMeasurementSubFiles (you may have to create it first if it does not already exist)
- python plotMCFakeRateMeasurement.py
(after having verified that the settings in this file are correct)
(this script will also perform a correct hadd of the files in MCFakeRateMeasurementSubFiles, taking care to merge only the files that need to be merged, i.e. not merging different years and lepton flavours)

The other tasks (e.g. fake rate measurement from data) are very similar in general structure.

In most cases, the 'plotting' step will not involve any jobs, everything is run locally.
But for some cases (e.g. the template fit method), the 'plotting'/'fitting' step involves job submission as well.

Note that for some tasks you will have many jobs.
You can quickly check if all of them finished without errors by doing python checkoutput.py. This little script checks if all jobs that started by printing ###starting### finished by printing ###done###.

Furthermore, the scripts with 'clean' in their name should allow you to quickly remove common temporary files, but be aware that some of them also may remove results you want to keep.
This can be adapted of course.
