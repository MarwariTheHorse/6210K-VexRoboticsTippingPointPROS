# cd to src before running with "python3 ./python/grip_ann.py"
from pandas import read_csv
from keras.models import Sequential
from keras.layers import Dense, BatchNormalization
from tensorflow.keras.optimizers import Adam
from keras2cpp import export_model

def create_model():
	# Customize adam learning rate. Changing will require retrain.
	adam = Adam(learning_rate=.001)
	# Read csv into pandas dataframe
	dataframe = read_csv("nn_data/COPilot.csv", header=None)
	# split into input (X) and output (Y) variables (values will be floats as they are floats in the c++ program)
	dataset = dataframe.values
	print(dataset)
	X = dataset[:,1:5].astype(float)
	Y = dataset[:,0]
	# create model
	model = Sequential()
	# Add normalization layer
	# This is not an algorithm because I didn't want to write two identical normalization algorithms for python and c++
	model.add(BatchNormalization(input_shape=(5,)))
	# "input" layer
	model.add(Dense(5, activation='sigmoid'))
	# Can customize but will require a retrain
	model.add(Dense(14, activation='sigmoid'))
	model.add(Dense(28, activation='sigmoid'))
	model.add(Dense(56, activation='sigmoid'))
	model.add(Dense(32, activation='sigmoid'))
	model.add(Dense(18, activation='sigmoid'))
	model.add(Dense(6, activation='sigmoid'))
	# single neuron output for pneumatic value
	# sigmoid activation to keep values between 0 and 1
	# sigmoid performed better than relu for some reason
	model.add(Dense(1, kernel_initializer='normal', activation='sigmoid'))
	# compile showing accuracy and using mse and adam as they performed the best
	model.compile(loss='mean_squared_error', optimizer=adam, metrics=['accuracy'])
	# can modify batch size and epochs but will require a retrain
	model.fit(X, Y, batch_size=20, epochs=10, verbose=1)
	# print summary to terminal
	model.summary()
	return model

# save model with keras2cpp so we can use it in c++
model = create_model()
export_model(model, "keras_ann.model")
#Following lines should return 1 and 0 from the NN, the 0 should be tougher to get
prediction_no_fire = model.predict([[]]) # Add sample data for predictions here
prediction_fire = model.predict([[]]) # Add sample data for predictions here
print(prediction_fire)
print(prediction_no_fire)