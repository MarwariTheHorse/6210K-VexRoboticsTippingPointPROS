#include "main.h"

// Motor port definitions
#define WHEEL_LEFT 1
#define WHEEL_RIGHT 2
#define CONVEYOR_LOWER 3
#define CONVEYOR_UPPER 4
#define ROTATOR_LEFT 5
#define ROTATOR_RIGHT 6
#define TILT_LEFT 7
#define TILT_RIGHT 8

const bool DEBUG = false;
const bool RECORD_NN_DATA = true;

okapi::Motor mWheelBackLeft(20);
okapi::Motor mWheelFrontLeft(11);
okapi::Motor mWheelFrontRight(1);
okapi::Motor mWheelBackRight(9);

okapi::Controller master(okapi::ControllerId::master);
pros::Vision sCamera(2, pros::E_VISION_ZERO_CENTER);
pros::vision_signature colorCode = sCamera.signature_from_utility(1, -4561, -4203, -4382, -5005, -4321, -4664, 2.400, 0);


/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs when program is started. Blocks everything else.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");
	pros::lcd::register_btn1_cb(on_center_button);
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
	while (true) {
		// a ridiculously complicated print statement. originally returns three bits. each value is masked 
		// via the bitwise AND operator and then bit shifted so that it is the only bit left.
		pros::lcd::print(0, "%d %d %d", (pros::lcd::read_buttons() & LCD_BTN_LEFT) >> 2,
		                 (pros::lcd::read_buttons() & LCD_BTN_CENTER) >> 1,
		                 (pros::lcd::read_buttons() & LCD_BTN_RIGHT) >> 0);


		// Movement assigning code //

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
		if(pros::usd::is_installed() && count == 0 && RECORD_NN_DATA){
			// Capture training data
			int greenX = tempobj.x_middle_coord;

			// Open the predefined data file
			FILE* dataFile;
			dataFile = fopen("usd/data.ntd", "a");

			// Write the data
			fputs("\n" + greenX + "\n" + leftSpeed + "\n" + rightSpeed + "\n", dataFile);

			// Close the file
			fclose(dataFile);
		}

		count++;
		if(count == 10) count = 0;
		pros::delay(10);
	}
}