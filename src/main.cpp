#include "main.h"
#include "neural_net.h"

const bool DEBUG = false;
const bool RECORD_NN_DATA = false;
const bool RECORD_COPYCAT_DATA = true;
const bool LOGGING_RATE = 100; // ms * 10, ie 100 results in data every second

// Wheel Definitions
okapi::Motor mWheelBackLeft(2);
okapi::Motor mWheelFrontLeft(14);
okapi::Motor mWheelFrontRight(9);
okapi::Motor mWheelBackRight(8);

// Controlle and camera definitions + green is defined
pros::Vision sCamera(4, pros::E_VISION_ZERO_CENTER);
pros::vision_signature colorCode = sCamera.signature_from_utility(1, -4915, -4609, -4762, -5121, -4807, -4964, 11.000, 0);

Net* neuralNetwork;

/**
 * Runs when program is started. Blocks everything else.
 */
void initialize() {
	std::cout << 0;
	sCamera.set_wifi_mode(true);
	if(pros::usd::is_installed()){
		// Initialize the neural network
		std::vector<unsigned int> topology; // Load the necessary file handlers
		std::cout << 1;
		TrainingData trainData("/usd/trainingData.txt");
		std::cout << 2;
		trainData.getTopology(topology);
		neuralNetwork = new Net(topology, "/usd/NNsave.txt");

		// Create the network and load data if necessary
		neuralNetwork->load();
	}
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
			if(sCamera.get_object_count() == 0){
				leftSpeed = 0;
				rightSpeed = 0;
			}
			// Otherwise, drive forward, decreasing our speed as we get closer
			else{
				leftSpeed = 127 - tempobj.width + tempobj.x_middle_coord * .5;
				rightSpeed = -127 + tempobj.width + tempobj.x_middle_coord * .5;
			}
		} else {
			std::vector<double> inputs;
			inputs.push_back(greenX);
			inputs.push_back(width);
			neuralNetwork->feedForward(inputs);
			std::vector<double> results;
			neuralNetwork->getResults(results);
			cout << results[0] << " " << results[1];
			leftSpeed = results[0] * 100;
			rightSpeed = results[1] * 100;
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
