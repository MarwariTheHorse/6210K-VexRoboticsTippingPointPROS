#include "main.h"
#include <fstream>

// Motor port definitions
#define WHEEL_LEFT_LOWER 1
#define WHEEL_LEFT_UPPER 2
#define WHEEL_RIGHT_LOWER 3
#define WHEEL_RIGHT_UPPER 3
#define WHEEL_BACK_LOWER 4
#define WHEEL_BACK_UPPER 5
#define LOCK_LEFT 6
#define LOCK_RIGHT 7
#define LIFT_LEFT 8
#define LIFT_RIGHT 9

const bool DEBUG = false;
const bool RECORD_NN_DATA = false;
const bool RECORD_COPYCAT_DATA = false;
const bool LOGGING_RATE = 100; // ms * 10, plus execution per loop time. ie 100 results in data appox. every second

okapi::Motor mWheelBackLeft(20);
okapi::Motor mWheelFrontLeft(11);
okapi::Motor mWheelFrontRight(1);
okapi::Motor mWheelBackRight(9);

okapi::Controller master(okapi::ControllerId::master);
pros::Vision sCamera(2, pros::E_VISION_ZERO_CENTER);
pros::vision_signature colorCode = sCamera.signature_from_utility(1, -4275, -3275, -3774, -7043, -5763, -6402, 2.400, 0);

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

		// Store joysticks
		int joyLY = master.getAnalog(ControllerAnalog::leftY);
		int joyRX = master.getAnalog(ControllerAnalog::rightX);

		// Filter joysticks
		if(math.abs(joyLY) < 10){
			joyLY = 0;
		}

		if(math.abs(joyRX) < 10){
			joyRX = 0;
		}

		// Convert joysticks to wheel speeds
		int wheelLeftSpeed = joyLY + joyRX;
		int wheelRightSpeed = joyLY - joyRX;
		int wheelBackSpeed = (wheelLeftSpeed + wheelRightSpeed) / 2;

		// Filter wheel speeds (We got none right now)

		// Wheel speed assignments
		mWheelFrontLeft.moveVelocity(leftSpeed);
		mWheelFrontRight.moveVelocity(rightSpeed);
		mWheelBackRight.moveVelocity(rightSpeed);
		mWheelBackLeft.moveVelocity(leftSpeed);

		count++;
		if(count == LOGGING_RATE) count = 0;
		pros::delay(10);
	}
}
