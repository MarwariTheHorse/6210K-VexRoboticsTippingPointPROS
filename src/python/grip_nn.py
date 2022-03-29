from pandas import read_csv
from keras.models import Sequential
from keras.layers import Dense, BatchNormalization, LSTM
from tensorflow.keras.optimizers import Adam
import numpy as np
from keras2cpp import export_model

def create_model():
	adam = Adam(learning_rate=0.1)
	dataframe = read_csv("nn_data/COPilot.csv", header=None)
	dataset = dataframe.values
	# split into input (X) and output (Y) variables
	X = dataset[:,1:16]
	Y = dataset[:,0]
	X = np.expand_dims(X, 1)
	Y = np.expand_dims(Y, 1)
	# create model
	model = Sequential()
	model.add(BatchNormalization(input_shape=(1, 15)))
	model.add(LSTM(15, return_sequences=True))
	model.add(LSTM(9, return_sequences=True))
	model.add(LSTM(3))
	model.add(Dense(1, input_dim=1, kernel_initializer='normal', activation='sigmoid'))
	model.compile(loss='mean_squared_error', optimizer=adam, metrics=['accuracy'])
	model.fit(X, Y, batch_size=5, epochs=10, verbose=1)
	model.summary()
	return model

# save model
model = create_model()
export_model(model, "keras_nn.model")
no_fire_test = np.expand_dims(np.array([2930,-32,-102,72,-67,-52,3570,-114,-55,160,-14.2388,0.0126953,0.894775,1.33944,0]), 0)
fire_test = np.expand_dims(np.array([2790,73,3,800,-145,-28,364,65,-10,42,-27.845,-0.0571289,1.01196,0.575,0]), 0)
#Following lines should return 1 from the NN
fire = np.array([fire_test])
no_fire = np.array([no_fire_test])
prediction_fire = model.predict(fire)
prediction_no_fire = model.predict(no_fire)
print(prediction_fire)
print(prediction_no_fire)