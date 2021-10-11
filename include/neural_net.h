#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>

using namespace std;

// A data structure for the links between each neuron
struct Connection
{
	double weight;
	double deltaWeight;
};

class TrainingData
{
public:
	TrainingData(const string filename);

	bool isEof(void) {return m_trainingDataFile.eof();};

	// Interact with the input file
	void getTopology(vector<unsigned> &topology);
	unsigned getNextInputs(vector<double> &inputVals);
	unsigned getTargetOutputs(vector<double> &targetOutputVals);

private:
	ifstream m_trainingDataFile;
};

class Neuron;
typedef vector<Neuron> Layer; // Translation: Layers are simply vectors of Neurons

// ****************** class Neuron ******************

class Neuron
{
public:
	Neuron(unsigned numOutputs, unsigned myIndex, bool isOutput);
	void setOutputVal(double val) { m_outputVal = val; } // TODO: Would it be cleaner to merge this and the func below
	double getOutputVal(void) const { return m_outputVal; }
	void feedForward(const Layer &prevLayer);
	void calcOutputGradients(double targetVals);
	void calcHiddenGradients(const Layer &nextLayer);
	void updateInputWeights(Layer &prevLayer);
	vector<Connection> getWeights();
	double getGradient();
	void setGradient(double g) {m_gradient = g;};
	void addConnection(int index, Connection c) {m_outputWeights[index] = c;};
private:
	static double eta; // [0.0...1.0] overall net training rate
	static double alpha; // [0.0...n] multiplier of last weight change [momentum]
	double transferFunction(double x);
	double transferFunctionDerivative(double x);
	static double randomWeight(void) { return rand() / double(RAND_MAX); }
	double sumDOW(const Layer &nextLayer) const;
	double m_outputVal;
	vector<Connection> m_outputWeights;
	unsigned m_myIndex;
	double m_gradient;
	bool m_isOutput;
};

// ****************** class Net ******************
class Net
{
public:
	Net(vector<unsigned> &topology, const string filename);
	void feedForward(const vector<double> &inputVals);
	void backProp(const vector<double> &targetVals);
	void getResults(vector<double> &resultVals) const;
	double getRecentAverageError(void) const { return m_recentAverageError; }
	void save();
	void load();
private:
	vector<Layer> m_layers; //m_layers[layerNum][neuronNum]
	double m_error;
	double m_recentAverageError;
	static double m_recentAverageSmoothingFactor;
	fstream nnDataFile;
};