#include "main.h"
#include <fstream>

const bool DEBUG = false;
const bool RECORD_NN_DATA = false;
const bool RECORD_COPYCAT_DATA = true;
const bool LOGGING_RATE = 100; // ms * 10, ie 100 results in data every second

// Wheel Definitions
okapi::Motor mWheelBackLeft(20);
okapi::Motor mWheelFrontLeft(11);
okapi::Motor mWheelFrontRight(1);
okapi::Motor mWheelBackRight(9);

// Controlle and camera definitions + green is defined
okapi::Controller master(okapi::ControllerId::master);
pros::Vision sCamera(2, pros::E_VISION_ZERO_CENTER);
pros::vision_signature colorCode = sCamera.signature_from_utility(1, -4275, -3275, -3774, -7043, -5763, -6402, 2.400, 0);

/**
 * Runs when program is started. Blocks everything else.
 */
void initialize() {
	sCamera.set_wifi_mode(true);
	sCamera.set_exposure(79);
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

		int leftSpeed;
		int rightSpeed;

		// If there are no green object >= 30 pix, don't move
		if(sCamera.get_object_count() == 0 || tempobj.width * tempobj.height < 30){
			leftSpeed = 0;
			rightSpeed = 0;
		}
		// Otherwise, drive forward, decreasing our speed as we get closer
		else{
			leftSpeed = -127 + tempobj.width + tempobj.x_middle_coord * .5;
			rightSpeed = 127 - tempobj.width + tempobj.x_middle_coord * .5;
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

				// Capture training data
				int greenX = tempobj.x_middle_coord;
				int width = tempobj.width;

				// Record training sensor data (obj_x, obj_width)
				std::ofstream dataFile;
				dataFile.open("/usd/nn_data.csv", std::ofstream::out | std::ofstream::app);
				dataFile << greenX << ", " << width << std::endl;
				dataFile.close();

				// Record output training data (left_wheel_speed, right_wheel_speed)
				std::ofstream resultsFile;
				resultsFile.open("/usd/nn_results.csv", std::ofstream::out | std::ofstream::app);
				resultsFile << leftSpeed << ", " << rightSpeed << std::endl;
				resultsFile.close();
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
