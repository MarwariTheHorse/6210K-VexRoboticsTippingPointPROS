from locale import normalize
from pandas import read_csv
from keras.models import Sequential
from keras.layers import Dense, BatchNormalization
from tensorflow.keras.optimizers import Adam
from sklearn.preprocessing import normalize
from keras2cpp import export_model

def create_model():
	adam = Adam(learning_rate=0.002)
	dataframe = read_csv("nn_data/COPilot.csv", header=None)
	dataset = dataframe.values
	# split into input (X) and output (Y) variables
	X = dataset[:,1:16]
	Y = dataset[:,0]
	'''	X = normalize(X)'''
	# create model
	model = Sequential()
	model.add(BatchNormalization())
	model.add(Dense(15, input_dim=15, kernel_initializer='normal', activation='relu'))
	model.add(BatchNormalization())
	model.add(Dense(8, input_dim=8, kernel_initializer='normal', activation='relu'))
	model.add(BatchNormalization())
	model.add(Dense(1, kernel_initializer='normal', activation='sigmoid'))
	model.compile(loss='mean_squared_error', optimizer=adam, metrics=['accuracy'])
	model.fit(X, Y, batch_size=10, epochs=10, verbose=1)
	return model

# save model
model = create_model()
export_model(model, "keras_nn.model")