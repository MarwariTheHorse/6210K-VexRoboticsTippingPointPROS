#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <vector>

// Typedefs for easily changing data types
typedef float Scalar;
typedef Eigen::MatrixXf Matrix;
typedef Eigen::RowVectorXf RowVector;
typedef Eigen::VectorXf ColVector;

// The actual class
class NeuralNetwork {
	std::vector<RowVector*> neuronLayers;
	std::vector<RowVector*> cacheLayers;
	std::vector<RowVector*> deltas;
	std::vector<Matrix*> weights;
	Scalar learningRate;
	
	NeuralNetwork(std::vector(uint) topology, Scalar learningRate = Scalar(0.005));
	void propogateForward(RowVector& input);
	void propogateBackward(RowVector& output);
	void calcErrors(RowVector& output);
	void updateWeights();
	void train(std::vector<RowVector*> data);
}

// Bonus code
void readCSV();
void genData();