from pandas import read_csv
from keras.models import Sequential
from keras.layers import Dense, BatchNormalization
from tensorflow.keras.optimizers import Adam
from keras2cpp import export_model

def create_model():
	adam = Adam(learning_rate=.01)
	dataframe = read_csv("nn_data/COPilot.csv", header=None)
	dataframe.replace(557709950, 0, inplace=True)
	dataframe.replace(1792, 0, inplace=True)
	# split into input (X) and output (Y) variables
	dataset = dataframe.values
	print(dataset)
	X = dataset[:,1:17]
	print(X)
	Y = dataset[:,0]
	# create model
	model = Sequential()
	model.add(BatchNormalization(input_shape=(16,)))
	model.add(Dense(16))
	model.add(Dense(9))
	model.add(Dense(6))
	model.add(Dense(3))
	model.add(Dense(1, kernel_initializer='normal', activation='sigmoid'))
	model.compile(loss='mean_squared_error', optimizer=adam, metrics=['accuracy'])
	model.fit(X, Y, batch_size=10, epochs=100, verbose=1)
	model.summary()
	return model

# save model
model = create_model()
export_model(model, "keras_ann.model")
#Following lines should return 1 and 0 from the NN, the 0 should be tougher to get
prediction_fire = model.predict([[2759, 186, 55, -75, 130, -126, -95, 48, -31, -12, 294, -488.821, 0.393311, 1.06177, 1.72111, 0]])
prediction_no_fire = model.predict([[2764, 447, 49, 90, 40, 0, 1792, 557709950, -103, -1, 306, -476.139, -0.0344238, 1.12524, 1.72, 0]])
print(prediction_fire)
print(prediction_no_fire)