#!/bin/bash

bash cleanMCFakeRateMeasurement.sh
bash cleanFitTemplates.sh
rm FakeRateMeasurementSubFiles/*
rm MagicFactorSubFiles/*
rm TuneFOSelectionSubFiles/*
rm PrescaleMeasurementSubFiles/*
rm -r fakeRateMaps
rm -r fakeRateMeasurementPlots
rm fakeRateMeasurement_*
rm fillFakeRateMeasurement
rm fillFakeRateMeasurement.sh*
rm fillMagicFactor
rm fillMagicFactor.sh*
rm fillTuneFOSelection
rm fillTuneFOSelection.sh*
rm tuneFOSelection_*
rm fillPrescaleMeasurement
rm fillPrescaleMeasurement.sh*
rm fitFakeRateMeasurement
rm fitFakeRateMeasureemnt.sh*
rm -r magicFactorPlots
rm magicFactor_*
rm plotHistogramsInFile
rm plotMagicFactor
rm plotTuneFOSelection
rm plotTuneFOSelection.sh*
rm -r tuningPlots_*
rm plotPrescaleMeasurement
rm -r prescaleMeasurementPlots*
rm prescaleMeasurement_*
