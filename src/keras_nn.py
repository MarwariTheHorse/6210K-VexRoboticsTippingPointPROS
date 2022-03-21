from locale import normalize
from pandas import read_csv
from keras.models import Sequential
from keras.layers import Dense
from tensorflow.keras.optimizers import Adam
from keras.wrappers.scikit_learn import KerasRegressor
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import KFold
from sklearn.preprocessing import normalize, StandardScaler
from sklearn.pipeline import Pipeline
# load dataset
dataframe = read_csv("src\COPilot.csv", header=None)
dataset = dataframe.values
# split into input (X) and output (Y) variables
X = dataset[:,1:16]
Y = dataset[:,0]
# define base model
def baseline_model():
	# create model
	model = Sequential()
	model.add(Dense(15, input_dim=15, kernel_initializer='normal', activation='relu'))
	model.add(Dense(8, input_dim=8, kernel_initializer='normal', activation='relu'))
	model.add(Dense(1, kernel_initializer='normal', activation='sigmoid'))
	adam = Adam(learning_rate=0.002)
	model.compile(loss='mean_squared_error', optimizer=adam, metrics=['accuracy'])
	model_json = model.to_json()
	with open("model.json", "w") as json_file:
		json_file.write(model_json)
	# serialize weights to HDF5
	model.save_weights("model.h5")
	print("Saved model to disk")
	return model
# evaluate model with standardized dataset
estimators = []
X = normalize(X)
estimators.append(('standardize', StandardScaler()))
estimators.append(('mlp', KerasRegressor(build_fn=baseline_model, epochs=100, batch_size=10, verbose=1)))
pipeline = Pipeline(estimators)
pipeline.fit(X, Y)
kfold = KFold(n_splits=10)
results = cross_val_score(pipeline, X, Y, cv=kfold)
print("Standardized: %.2f (%.2f) MSE" % (results.mean(), results.std()))
# serialize model to YAML