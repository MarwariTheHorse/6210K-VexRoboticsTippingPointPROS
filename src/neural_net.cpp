#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
#include "neural_net.h"

using namespace std;

bool LOAD_FROM_NN_SAVE = true;

// Add topology data to the vector passed into the function
void TrainingData::getTopology(vector<unsigned> &topology)
{
	string line;
	string label;

	getline(m_trainingDataFile, line); // Toplogy data > line
	stringstream ss(line); // Do the string converty thing or whatever
	ss >> label; // Store the clean stuff into the label variable

	if(this->isEof() || label.compare("topology:") != 0) // If there is no topology abort
	{
		abort();
	}

	while(!ss.eof()) // Continue while not at the end of the ss
	{
		unsigned n;
		ss >> n; // Store next value into the 32 bit int
		topology.push_back(n); // Slap that bad boy right into that vector
	}
	return;
}

// Constructs the TrainingData file
TrainingData::TrainingData(const string filename)
{
	m_trainingDataFile.open(filename.c_str());
}

// Load up the next round of inputs
// TODO: make it such that once the network has taken the data into account, the data is deleted
unsigned TrainingData::getNextInputs(vector<double> &inputVals)
{
    // Screw this godforsaken inconsistent indentation
    inputVals.clear();

    // Get the next line and store it into the ss thing
    string line;
    getline(m_trainingDataFile, line);
    stringstream ss(line);

    // Check for in data and slap it into inputVals
    string label;
    ss >> label;
    if (label.compare("in:") == 0) {
        double oneValue;
        while (ss >> oneValue) {
            inputVals.push_back(oneValue);
        }
    }

    return inputVals.size();
}

// Load output data into the targetOutputVals vector
unsigned TrainingData::getTargetOutputs(vector<double> &targetOutputVals)
{
    // Review the function above this one for commentary on the code
    targetOutputVals.clear();

    string line;
    getline(m_trainingDataFile, line);
    stringstream ss(line);

    string label;
    ss>> label;
    if (label.compare("out:") == 0) {
        double oneValue;
        while (ss >> oneValue) {
            targetOutputVals.push_back(oneValue);
        }
    }

    return targetOutputVals.size();
}

double Neuron::eta = 0.15; // overall net learning rate
double Neuron::alpha = 0.5; // momentum, multiplier of last deltaWeight, [0.0..n]

vector<Connection> Neuron::getWeights(){
	return m_outputWeights;
}

double Neuron::getGradient(){
	return m_gradient;
}

void Neuron::updateInputWeights(Layer &prevLayer)
{
	// The weights to be updated are in the Connection container
	// in the neurons in the preceding layer

	for(unsigned n = 0; n < prevLayer.size(); ++n)
	{
		Neuron &neuron = prevLayer[n];
		double oldDeltaWeight = neuron.m_outputWeights[m_myIndex].deltaWeight;

		double newDeltaWeight = 
				// Individual input, magnified by the gradient and train rate:
				eta
				* neuron.getOutputVal()
				* m_gradient
				// Also add momentum = a fraction of the previous delta weight
				+ alpha
				* oldDeltaWeight;
		neuron.m_outputWeights[m_myIndex].deltaWeight = newDeltaWeight;
		neuron.m_outputWeights[m_myIndex].weight += newDeltaWeight;
	}
}
double Neuron::sumDOW(const Layer &nextLayer) const
{
	double sum = 0.0;

	// Sum our contributions of the errors at the nodes we feed

	for (unsigned n = 0; n < nextLayer.size() - 1; ++n)
	{
		sum += m_outputWeights[n].weight * nextLayer[n].m_gradient;
	}

	return sum;
}

void Neuron::calcHiddenGradients(const Layer &nextLayer)
{
	double dow = sumDOW(nextLayer);
	m_gradient = dow * Neuron::transferFunctionDerivative(m_outputVal);
}
void Neuron::calcOutputGradients(double targetVals)
{
	double delta = targetVals - m_outputVal;
	m_gradient = delta * Neuron::transferFunctionDerivative(m_outputVal);
}

double Neuron::transferFunction(double x)
{
	// // tanh - output range [-1.0..1.0]
	// return tanh(x);
	
	// NEW ACTIVATION FUNCTION
	return x/(1+exp(-x))
}

double Neuron::transferFunctionDerivative(double x)
{
	// // tanh derivative
	// return 1.0 - x * x;
	double s = Neuron::transferFunction(x);
	return s + (1-s)/(1+exp(-x))
}

void Neuron::feedForward(const Layer &prevLayer)
{
	double sum = 0.0;

	// Sum the previous layer's outputs (which are our inputs)
    // Include the bias node from the previous layer.

	for(unsigned n = 0 ; n < prevLayer.size(); ++n)
	{
		sum += prevLayer[n].getOutputVal() * 
				 prevLayer[n].m_outputWeights[m_myIndex].weight;
	}

	m_outputVal = Neuron::transferFunction(sum);
}

Neuron::Neuron(unsigned numOutputs, unsigned myIndex)
{
	for(unsigned c = 0; c < numOutputs; ++c){
		m_outputWeights.push_back(Connection());
		m_outputWeights.back().weight = randomWeight();
	}

	m_myIndex = myIndex;
}

double Net::m_recentAverageSmoothingFactor = 100.0; // Number of training samples to average over

void Net::getResults(vector<double> &resultVals) const
{
	resultVals.clear();

	for(unsigned n = 0; n < m_layers.back().size() - 1; ++n)
	{
		resultVals.push_back(m_layers.back()[n].getOutputVal());
	}
}

void Net::backProp(const std::vector<double> &targetVals)
{
	// Calculate overal net error (RMS of output neuron errors)

	Layer &outputLayer = m_layers.back();
	m_error = 0.0;

	for(unsigned n = 0; n < outputLayer.size() - 1; ++n)
	{
		double delta = targetVals[n] - outputLayer[n].getOutputVal();
		m_error += delta *delta;
	}
	m_error /= outputLayer.size() - 1; // get average error squared
	m_error = sqrt(m_error); // RMS

	// Implement a recent average measurement:

	m_recentAverageError = 
			(m_recentAverageError * m_recentAverageSmoothingFactor + m_error)
			/ (m_recentAverageSmoothingFactor + 1.0);
	// Calculate output layer gradients

	for(unsigned n = 0; n < outputLayer.size() - 1; ++n)
	{
		outputLayer[n].calcOutputGradients(targetVals[n]);
	}
	// Calculate gradients on hidden layers

	for(unsigned layerNum = m_layers.size() - 2; layerNum > 0; --layerNum)
	{
		Layer &hiddenLayer = m_layers[layerNum];
		Layer &nextLayer = m_layers[layerNum + 1];

		for(unsigned n = 0; n < hiddenLayer.size(); ++n)
		{
			hiddenLayer[n].calcHiddenGradients(nextLayer);
		}
	}

	// For all layers from outputs to first hidden layer,
	// update connection weights

	for(unsigned layerNum = m_layers.size() - 1; layerNum > 0; --layerNum)
	{
		Layer &layer = m_layers[layerNum];
		Layer &prevLayer = m_layers[layerNum - 1];

		for(unsigned n = 0; n < layer.size() - 1; ++n)
		{
			layer[n].updateInputWeights(prevLayer);
		}
	}
}

void Net::feedForward(const vector<double> &inputVals)
{
	// Check the num of inputVals euqal to neuronnum expect bias
	assert(inputVals.size() == m_layers[0].size() - 1);

	// Assign {latch} the input values into the input neurons
	for(unsigned i = 0; i < inputVals.size(); ++i){
		m_layers[0][i].setOutputVal(inputVals[i]); 
	}

	// Forward propagate
	for(unsigned layerNum = 1; layerNum < m_layers.size(); ++layerNum){
		Layer &prevLayer = m_layers[layerNum - 1];
		for(unsigned n = 0; n < m_layers[layerNum].size() - 1; ++n){
			m_layers[layerNum][n].feedForward(prevLayer);
		}
	}
}

void Net::save()
{
	// Add vector (For each vector add the neurons, for each neuron add the connections, for each connection add weight and delta weight
	for (Layer l : m_layers){
		nnDataFile << "LAYER" << endl;
		for (Neuron n : l){
			nnDataFile << "NODE" << endl;

			// Gradient
			nnDataFile << "GRADIENT " << n.getGradient() << endl;

			// Connections
			vector<Connection> connections = n.getWeights();
			for(Connection c : connections){
				nnDataFile << "CONNECTION " << c.weight << " " << c.deltaWeight << endl;
			}
		}
	}
}

void Net::load()
{
	int layerCount = -1;
	int nCount = -1;
	int connectionIndex;
	string line;
	string label;
	
	while(getline(nnDataFile, line)){
		stringstream ss(line);
		ss >> label;
		
		if(label.compare("LAYER") == 0){
			layerCount++;
			nCount = -1;
		}
		
		if(label.compare("NODE") == 0){ 
			// Save node into array thing
			nCount++;
			connectionIndex = 0;
		}
		if(label.compare("GRADIENT") == 0){
			double value;
			ss >> value;
			m_layers[layerCount][nCount].setGradient(value);
		}
		if(label.compare("CONNECTION") == 0){
			double value;
			double value2;
			ss >> value;
			ss >> value2; 
			Connection c = {value, value2};
			m_layers[layerCount][nCount].addConnection(connectionIndex, c);
			connectionIndex++;
		}
	}
}

Net::Net(vector<unsigned> &topology, const string filename)
{
	nnDataFile.open(filename.c_str());

	// Made the neural network
	unsigned numLayers = topology.size();
	for(unsigned layerNum = 0; layerNum < numLayers; ++layerNum){
		m_layers.push_back(Layer());
		// numOutputs of layer[i] is the numInputs of layer[i+1]
		// numOutputs of last layer is 0
		unsigned numOutputs = layerNum == topology.size() - 1 ? 0 :topology[layerNum + 1];

		// We have made a new Layer, now fill it ith neurons, and
		// add a bias neuron to the layer:
		for(unsigned neuronNum = 0; neuronNum <= topology[layerNum]; ++neuronNum){
			m_layers.back().push_back(Neuron(numOutputs, neuronNum));
		}

		// Force the bias node's output value to 1.0. It's the last neuron created above
		m_layers.back().back().setOutputVal(1.0);
	}
}