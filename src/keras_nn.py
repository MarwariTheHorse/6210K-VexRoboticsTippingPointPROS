from locale import normalize
from pandas import read_csv
from keras.models import Sequential
from keras.layers import Dense
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
	model.add(Dense(10, input_dim=10, kernel_initializer='normal', activation='relu'))
	model.add(Dense(3, input_dim=3, kernel_initializer='normal', activation='relu'))
	model.add(Dense(1, kernel_initializer='normal', activation='sigmoid'))
	# Compile model
	model.compile(loss='mean_squared_error', optimizer='adam')
	return model
# evaluate model with standardized dataset
estimators = []
X = normalize(X)
estimators.append(('standardize', StandardScaler()))
estimators.append(('mlp', KerasRegressor(build_fn=baseline_model, epochs=100, batch_size=10, verbose=1)))
pipeline = Pipeline(estimators)
kfold = KFold(n_splits=10)
results = cross_val_score(pipeline, X, Y, cv=kfold)
print("Standardized: %.2f (%.2f) MSE" % (results.mean(), results.std()))