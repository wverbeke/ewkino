from keras import models 
import numpy as np
import os

model = []
#calling_dir = os.getcwd()
#full_file = os.path.join( calling_dir, 'kerasModels/model_2hiddenLayers_512unitsPerLayer_relu_learningRate0p0001.h5')
#model = models.load_model( full_file )


def loadModel( file_name ):
    #file_name = os.path.basename( file_name )
    #calling_dir = os.getcwd()
    #file_name = os.path.join( calling_dir, file_name )
    model.append(models.load_model( file_name ) )


def predict( x ):
    x = np.asarray(x, dtype=float)
    x = x.reshape( (1, len(x)) )
    output =  float(model[0].predict(x))
    #output =  float(model.predict(x))
    return output


if __name__ == '__main__':
    pass
    #loadModel( 'model_4hiddenLayers_16unitsPerLayer_relu_learningRate0p0001.h5' )
    #x = np.ones( 36 )
    #print( predict( x ) )

