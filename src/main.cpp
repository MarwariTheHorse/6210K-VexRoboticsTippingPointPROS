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

okapi::Motor mWheelBackLeft(20);
okapi::Motor mWheelFrontLeft(11);
okapi::Motor mWheelFrontRight(1);
okapi::Motor mWheelBackRight(9);
okapi::Controller master(okapi::ControllerId::master);

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

		// x-drive software
		float controllerY = master.getAnalog(okapi::ControllerAnalog::leftY) * 100;
		float controllerX = master.getAnalog(okapi::ControllerAnalog::leftX) * 100;
		float controllerR = master.getAnalog(okapi::ControllerAnalog::rightX) * 100;

		// Zero out the channels
		if(controllerY < 10 && controllerY > -10) controllerY = 0;
		if(controllerX < 10 && controllerX > -10) controllerX = 0;
		if(controllerR < 10 && controllerR > -10) controllerR = 0;

		double magnitude = sqrt((controllerX * controllerX) + (controllerY * controllerY));
	    double direction;
	    if(controllerX > 0){
	      direction = (2 * okapi::pi) + asin(controllerY/magnitude) - (okapi::pi/4);
	    }else{
	      direction = okapi::pi - asin(controllerY/magnitude) - (okapi::pi/4);
	    }

	    // Assign wheel speeds
	    mWheelFrontLeft.moveVelocity(controllerR + controllerX + controllerY);
	    mWheelFrontRight.moveVelocity(controllerR + controllerX - controllerY);
	    mWheelBackLeft.moveVelocity(controllerR - controllerX + controllerY);
	    mWheelBackRight.moveVelocity(controllerR - controllerX - controllerY);

		pros::delay(10);
	}
}