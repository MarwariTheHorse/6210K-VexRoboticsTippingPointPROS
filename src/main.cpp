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

pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::Motor left_mtr(1);
pros::Motor right_mtr(2);

bool disableControllerScreenLoop = false;

const bool DEBUG = false;

/**
 * A callback for LLEMU's left button.
 */
void on_left_button() {
    
}

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
 * A callback for LLEMU's right button
 */
void on_right_button() {
    
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
		pros::lcd::print(0, "%d %d %d", (pros::lcd::read_buttons() & LCD_BTN_LEFT) >> 2,
		                 (pros::lcd::read_buttons() & LCD_BTN_CENTER) >> 1,
		                 (pros::lcd::read_buttons() & LCD_BTN_RIGHT) >> 0);

		// WHEELS /////////////////////////////////////////////////////////////////////////

		// Get joystick values
		int leftY = master.get_analog(ANALOG_LEFT_Y);
		int rightX = master.get_analog(ANALOG_RIGHT_X);

		// Zeroing out stuff
		if(abs(left < 10)) leftY = 0;
		if(abs(right < 10)) rightX = 0;

		// Assign converted values to the wheels
		left_mtr = leftY + rightX;
		right_mtr = leftY - rightX;

		// Ring Intake ///////////////////////////////////////////////////////////////////////

		
		// Ring Director /////////////////////////////////////////////////////////////////////
		
		// This mechinism is going to be pretty deep in the robot, so we are going to need to
		// come up with some algorithm so that the robot can deal with this part of operations

		pros::delay(10);
	}
}