from pandas import read_csv
from keras.models import Sequential
from keras.layers import Dense, BatchNormalization, LSTM
from tensorflow.keras.optimizers import Adam
import numpy as np
from keras2cpp import export_model

def create_model():
	adam = Adam(learning_rate=0.01)
	dataframe = read_csv("nn_data/COPilot.csv", header=None)
	dataset = dataframe.values
	# split into input (X) and output (Y) variables
	X = dataset[:,(1,2,3,4,5,6,7,8,9,10,14,15,16)]
	Y = dataset[:,(0)]
	X = np.expand_dims(X, 1)
	Y = np.expand_dims(Y, 1)
	# create model
	model = Sequential()
	model.add(BatchNormalization(input_shape=(1, 13)))
	model.add(LSTM(32, return_sequences=True))
	model.add(LSTM(18, return_sequences=True))
	model.add(LSTM(9, return_sequences=True))
	model.add(LSTM(5))
	model.add(Dense(1, kernel_initializer='normal', activation='sigmoid'))
	model.compile(loss='mean_squared_error', optimizer=adam, metrics=['accuracy'])
	model.fit(X, Y, batch_size=10, epochs=10, verbose=1)
	model.summary()
	return model

# save model
model = create_model()
export_model(model, "keras_rnn.model")
no_fire_test = np.expand_dims(np.array([2479, 216, 133, -17, 920, -141, 37, 2312, 62, -47, 320, 0]), 0)
fire_test = np.expand_dims(np.array([2760, 165, 0, 1792, 557709950, 0, 1792, 557709950, 0, 1792, 557709950, 0]), 0)
#Following lines should return 1 from the NN
fire = np.array([fire_test])
no_fire = np.array([no_fire_test])
prediction_fire = model.predict(fire)
prediction_no_fire = model.predict(no_fire)
print(prediction_fire)
print(prediction_no_fire)