#include "main.h"
#include "neural_net.h"

const bool DEBUG = false; // true = debugging mode. Not actually supported rn, but it's there just in case
const bool RECORD_NN_DATA = false; // when false runs NN, when true gathers data
const bool RECORD_COPYCAT_DATA = true; // When true it runs copycat code in the op loop
const bool LOGGING_RATE = 100; // ms * 10, ie 100 results in data every second

// Wheel Definitions
okapi::Motor mWheelBackLeft(2);
okapi::Motor mWheelFrontLeft(14);
okapi::Motor mWheelFrontRight(9);
okapi::Motor mWheelBackRight(8);

// Controlle and camera definitions + green is defined
pros::Vision sCamera(4, pros::E_VISION_ZERO_CENTER);
pros::vision_signature colorCode = sCamera.signature_from_utility(1, -4915, -4609, -4762, -5121, -4807, -4964, 11.000, 0);

// A variable to store the future NN in
Net* neuralNetwork;

/**
 * Runs when program is started. Blocks everything else.
 */
void initialize() {
	sCamera.set_wifi_mode(true);
	if(pros::usd::is_installed()){
		// Initialize the neural network
		std::vector<unsigned int> topology; // Load the necessary file handlers
		TrainingData trainData("/usd/trainingData.txt"); // Open the file
		trainData.getTopology(topology);
		neuralNetwork = new Net(topology, "/usd/NNsave.txt"); // Store a Net inside of the NN variable

		// Create the network and load data if necessary
		neuralNetwork->load(); // Loads the NN data from the NNsave.txt file
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
	int count = 0; // Count for NN logging
	int ccCount = 0; // Count for CC logging
	while (true) {
		// Get the largest green object
		pros::vision_object tempobj = sCamera.get_by_sig(0, 1);

		// Drivetrain variables
		double leftSpeed;
		double rightSpeed;
		double greenX;
		double width;

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
			// Assign values based off of NN output
			greenX = tempobj.x_middle_coord;
			width = tempobj.width;
			std::vector<double> inputs;
			inputs.push_back(greenX / 320);
			inputs.push_back(width / 320);
			neuralNetwork->feedForward(inputs);
			std::vector<double> results;
			neuralNetwork->getResults(results);
			cout << results[0] << " " << results[1];
			leftSpeed = results[0] * 127;
			rightSpeed = results[1] * 127;
		}

		mWheelFrontLeft.moveVelocity(leftSpeed);
		mWheelFrontRight.moveVelocity(rightSpeed);
		mWheelBackRight.moveVelocity(rightSpeed);
		mWheelBackLeft.moveVelocity(leftSpeed);

		// Capture the data (Runs every ten loops, ie >100ms)
		// Steps: Store data into variables, write the group of data into the file
		if(pros::usd::is_installed()){
			if(RECORD_NN_DATA && count == 0){ // NN data runs on a delay because I'm not sure how long file interactions take
				std::ofstream dataFile;
				greenX /= 320;
				width /= 320;
				leftSpeed /= 127;
				rightSpeed /= 127;
				dataFile.open("/usd/trainingData.txt", std::ofstream::out | std::ofstream::app);
				dataFile << "in: " << greenX << " " << width << std::endl;
				dataFile << "out: " << leftSpeed << " " << rightSpeed << std::endl;
				dataFile.close();
			}
			if(RECORD_COPYCAT_DATA){
				if(master[okapi::ControllerDigital::B].changedToPressed()){
					// Log stuff
					ccCount++;
					std::ofstream dataFile;
					dataFile.open("/usd/cc_data.csv", std::ofstream::out | std::ofstream::app);
					dataFile << ccCount << ", " << mWheelBackLeft.getPosition() << ", " 
						<< mWheelBackRight.getPosition() << ", " << std::endl;
					dataFile.close();
				}
			}
		}

		count++;
		if(count == LOGGING_RATE) count = 0;
		pros::delay(10);
	}
}
