from pandas import read_csv
from keras.models import Sequential
from keras.layers import Dense, BatchNormalization
from tensorflow.keras.optimizers import Adam
from keras2cpp import export_model
from matplotlib import pyplot

def create_model():
	adam = Adam(learning_rate=.001)
	dataframe = read_csv("nn_data/COPilot.csv", header=None)
	# split into input (X) and output (Y) variables
	dataset = dataframe.values
	print(dataset)
	X = dataset[:,1:17].astype(float)
	Y = dataset[:,0]
	# create model
	model = Sequential()
	model.add(BatchNormalization(input_shape=(16,)))
	model.add(Dense(16, activation='relu'))
	'''model.add(Dense(18))
	model.add(Dense(10))
	model.add(Dense(6))
	model.add(Dense(3))'''
	model.add(Dense(1, kernel_initializer='normal', activation='sigmoid'))
	model.compile(loss='mean_squared_error', optimizer=adam, metrics=['accuracy'])
	model.fit(X, Y, batch_size=10, epochs=10, verbose=1)
	model.summary()
	return model

def plot_data():
	dataframe = read_csv("nn_data/COPilot.csv", header=None)
	# split into input (X) and output (Y) variables
	dataset = dataframe.values
	fig = pyplot.figure()
	axis = fig.add_subplot(1, 1, 1, projection="3d") # 3D plot with scalar values in each axis
	reflect, echo, lift = dataset[:, 1], dataset[:, 2], dataset[:, 15]
	axis.scatter(reflect, echo, lift, marker="o")
	axis.set_xlabel("Reflection")
	axis.set_ylabel("Echo Distance")
	axis.set_zlabel("LiftPos")
	pyplot.show()

# save model
model = create_model()
export_model(model, "keras_ann.model")
#Following lines should return 1 and 0 from the NN, the 0 should be tougher to get
prediction_no_fire = model.predict([[2758, 186, 47, -80, 270, 0, 1792, 557709950, -50, -11, 448, -488.388, -0.452393, 1.08862, 1.72111, 0]])
prediction_fire = model.predict([[2755, 35, -40, -94, 420, -34, -14, 120, 0, 1792, 557709950, -268.13, 0.0551758, 0.968262, 1.52167, 1]])
print(prediction_fire)
print(prediction_no_fire)