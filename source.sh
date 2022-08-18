#!/usr/bin/bash

# Set the environment for the ewkino project.
# Currently in experimental stage;
# running this script is not needed for now,
# but is supposed to make some things a little easier.
# Run with "source source.sh" (NOT "bash source.sh")

CWD=$( pwd; )

# Set a cmsenv
# Modify the CMSSW directory to your liking
CMSSW=~/CMSSW_10_6_29
echo setting cmsenv to $CMSSW
cd $CMSSW/src
eval `scramv1 runtime -sh`
cd $CWD

# Export the path to include the jobSubmission subdirectory.
# This allows to run "jobCheck.py [+args]" from anywhere with less typing.
echo expanding PATH with $( pwd; )/jobSubmission
export PATH=$PATH:$( pwd; )/jobSubmission
