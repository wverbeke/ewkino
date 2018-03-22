#!/usr/bin/env python

from ROOT import TMVA, TFile, TTree, TCut
from subprocess import call
from os.path import isfile

from keras.models import Sequential
from keras.layers import Dense, Activation
from keras.regularizers import l2
from keras.optimizers import SGD

##EDIT##
from keras import optimizers

# Setup TMVA
TMVA.Tools.Instance()
TMVA.PyMethodBase.PyInitialize()

output = TFile.Open('TMVA.root', 'RECREATE')
factory = TMVA.Factory('TMVAClassification', output,
                       '!V:!Silent:Color:DrawProgressBar:Transformations=D,G:AnalysisType=Classification')

data = TFile.Open('leptonMvaTrainingTree_muon.root')
signal = data.Get('signalTree')
background = data.Get('backgroundTree')

dataloader = TMVA.DataLoader('dataset')
dataloader.AddVariable("pt")
dataloader.AddVariable("eta")
dataloader.AddVariable("trackMultClosestJet", 'F');
dataloader.AddVariable("miniIsoCharged", 'F');
dataloader.AddVariable("miniIsoNeutral", 'F');
dataloader.AddVariable("pTRel", 'F');
dataloader.AddVariable("ptRatio", 'F');
dataloader.AddVariable("csvV2ClosestJet", 'F');
dataloader.AddVariable("sip3d", 'F');
dataloader.AddVariable("dxy", 'F');
dataloader.AddVariable("dz", 'F');
dataloader.AddVariable("segmentCompatibility", 'F');


dataloader.AddSignalTree(signal, 1.0)
dataloader.AddBackgroundTree(background, 1.0)

dataloader.SetSignalWeightExpression("eventWeight");
dataloader.SetBackgroundWeightExpression("eventWeight");

dataloader.PrepareTrainingAndTestTree(TCut(''),
                                      'nTrain_Signal=36000:nTrain_Background=36000:nTest_Signal=36000:nTest_Background=36000:SplitMode=Random:NormMode=NumEvents:!V')

# Generate model

# Define model
model = Sequential()
model.add(Dense(256, activation='relu', W_regularizer=l2(1e-5), input_dim=12))
model.add(Dense(256, activation='relu'))
model.add(Dense(256, activation='relu'))
#model.add(Dense(2, activation='softmax'))
model.add(Dense(2, activation='sigmoid'))

# Set loss and optimizer
#model.compile(loss='categorical_crossentropy',

#test optimizer
sgd = optimizers.SGD(lr=0.01, decay=1e-6, momentum=0.9, nesterov=True)
###

model.compile(loss='binary_crossentropy',
              #optimizer=SGD(lr=0.01), metrics=['accuracy', ])
              optimizer=sgd, metrics=['accuracy', ])

# Store model to file
model.save('model_classification.h5')
model.summary()

# Book methods
factory.BookMethod(dataloader, TMVA.Types.kFisher, 'Fisher',
                   '!H:!V:Fisher:VarTransform=D,G')
factory.BookMethod(dataloader, TMVA.Types.kPyKeras, 'PyKeras',
        'H:!V:VarTransform=D,G:FilenameModel=model_classification.h5:NumEpochs=12:BatchSize=32:') #default NumEpochs = 20

factory.BookMethod(dataloader, TMVA.Types.kBDT, 'BDTG_m1Cuts_Depth4_baggedGrad_1000trees_shrinkage0p1',
            '!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.1:nCuts=200:MaxDepth=4:IgnoreNegWeightsInTraining:UseBaggedGrad=True:DoBoostMonitor=True');

# Run training, test and evaluation
factory.TrainAllMethods()
factory.TestAllMethods()
factory.EvaluateAllMethods()
