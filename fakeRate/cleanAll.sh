#!/bin/bash

bash cleanMCFakeRateMeasurement.sh
bash cleanFitTemplates.sh
rm FakeRateMeasurementSubFiles/*
rm MagicFactorSubFiles/*
rm PrescaleMeasurementSubFiles/*
rm -r fakeRateMaps
rm -r fakeRateMeasurementPlots
rm fakeRateMeasurement_*
rm fillFakeRateMeasurement
rm fillFakeRateMeasurement.sh*
rm fillMagicFactor
rm fillMagicFactor.sh*
rm fillPrescaleMeasurement
rm fillPrescaleMeasurement.sh*
rm fitFakeRateMeasurement
rm fitFakeRateMeasureemnt.sh*
rm -r magicFactorPlots
rm magicFactor_*
rm plotHistogramsInFile
rm plotMagicFactor
rm plotPrescaleMeasurement
rm -r prescaleMeasurementPlots*
rm prescaleMeasurement_*
