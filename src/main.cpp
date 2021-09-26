#include "main.h"
#include <fstream>
#include "neural_network.h"
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <sstream>

const bool DEBUG = false;
const bool RECORD_NN_DATA = true;
const bool RECORD_COPYCAT_DATA = true;
const bool LOGGING_RATE = 100; // ms * 10, ie 100 results in data every second

// Wheel Definitions
okapi::Motor mWheelBackLeft(19);
okapi::Motor mWheelFrontLeft(12);
okapi::Motor mWheelFrontRight(9);
okapi::Motor mWheelBackRight(8);

// Controlle and camera definitions + green is defined
pros::Vision sCamera(2, pros::E_VISION_ZERO_CENTER);
pros::vision_signature colorCode = sCamera.signature_from_utility(1, 10347, 10767, 10556, -1, 313, 156, 3.000, 0);

Net* neuralNetwork;

/**
 * Runs when program is started. Blocks everything else.
 */
void initialize() {
	sCamera.set_wifi_mode(true);
	sCamera.set_exposure(79);

	// // Initialize the neural network
	// std::vector<unsigned> topology; // Load the necessary file handlers

	// std::string line;
	// std::string label;
	// std::string filename = "usd/NNsave.txt";
	// std::ifstream nnDataFile;

	// nnDataFile.open(filename.c_str());
	// getline(nnDataFile, line); // Toplogy data > line
	// std::stringstream ss(line); // Do the string converty thing or whatever
	// ss >> label; // Store the clean stuff into the label variable

	// while(!ss.eof()) // Continue while not at the end of the ss
	// {
	// 		unsigned n;
	// 		ss >> n; // Store next value into the 32 bit int
	// 		topology.push_back(n); // Slap that bad boy right into that vector
	// }

	// // Create the network and load data if necessary
	// neuralNetwork = new Net(topology);
	// neuralNetwork->load("usd/NNsave.txt");
}

/**
 * Runs when something is disabling the robot following either autonomous or opcontrol. Exits when re-enabled.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to a controller. 
 *
 * For an auton selector
 *
 * Exits when the robot is enabled
 */
void competition_initialize() {}

/**
 * Runs for auton. Can be called for testing purposes.
 */
void autonomous() {}

/**
 * For operator control. Automatically runs after initialize if not connected 
 * to a field controller or etc.
 */
void opcontrol() {
	int count = 0;
	int ccCount = 0;
	while (true) {
		// Get the largest green object
		pros::vision_object tempobj = sCamera.get_by_sig(0, 1);
		int greenX = tempobj.x_middle_coord;
		int width = tempobj.width;
		int leftSpeed;
		int rightSpeed;

		if(RECORD_NN_DATA){
			// If there are no green object >= 30 pix, don't move
			if(sCamera.get_object_count() == 0 || tempobj.width * tempobj.height < 30){
				leftSpeed = 0;
				rightSpeed = 0;
			}
			// Otherwise, drive forward, decreasing our speed as we get closer
			else{
				leftSpeed = 127 - tempobj.width + tempobj.x_middle_coord * .5;
				rightSpeed = -127 + tempobj.width + tempobj.x_middle_coord * .5;
			}
			std::cout << leftSpeed << ", " << rightSpeed << std::endl;
		} else {
			std::vector<double> inputs;
			inputs.push_back(greenX);
			inputs.push_back(width);
			neuralNetwork->feedForward(inputs);
			std::vector<double> results;
			neuralNetwork->getResults(results);

		}

		mWheelFrontLeft.moveVelocity(leftSpeed);
		mWheelFrontRight.moveVelocity(rightSpeed);
		mWheelBackRight.moveVelocity(rightSpeed);
		mWheelBackLeft.moveVelocity(leftSpeed);

		// Capture the data (Runs every ten loops, ie >100ms)
		// Steps: Store data into variables, write the group of data into the file
		if(pros::usd::is_installed()){
			if(RECORD_NN_DATA && count == 0){ // NN data runs on a delay because I'm not sure how long file interactions take
				// TODO: measure how long file interactions take


				// Record training sensor data (obj_x, obj_width)
				std::ofstream dataFile;
				dataFile.open("/usd/trainingData.txt", std::ofstream::out | std::ofstream::app);
				dataFile << "in: " << greenX << " " << width << std::endl;
				dataFile << "out: " << leftSpeed << " " << rightSpeed << std::endl;
				dataFile.close();
			}
			// if(RECORD_COPYCAT_DATA){
			// 	if(master[okapi::ControllerDigital::B].changedToPressed()){
			// 		// Log stuff
			// 		ccCount++;
			// 		std::ofstream dataFile;
			// 		dataFile.open("/usd/cc_data.csv", std::ofstream::out | std::ofstream::app);
			// 		dataFile << ccCount << ", " << mWheelBackLeft.getPosition() << ", " 
			// 			<< mWheelBackRight.getPosition() << ", " << std::endl;
			// 		dataFile.close();
			// 	}
			// }
		}

		count++;
		if(count == LOGGING_RATE) count = 0;
		pros::delay(10);
	}
}
