#include "main.h"
#include <fstream>

// Motor + pneumatic port definitions
#define WHEEL_LEFT_V 1
#define WHEEL_LEFT_H 2
#define WHEEL_RIGHT_V 3
#define WHEEL_RIGHT_H 4
#define WHEEL_BACK_L 5
#define WHEEL_BACK_R 6

#define LOCK_LEFT 7
#define LOCK_RIGHT 8
#define LIFT_LEFT 9
#define LIFT_RIGHT 10

const bool DEBUG = false;
const bool LOGGING_RATE = 100; // ms * 10, plus execution per loop time. ie 100 results in data appox. every second

// Motors(port, reversed, gearset, encoderUnits, logger(implied))
okapi::Motor vLeftMotor(WHEEL_LEFT_V, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations)
okapi::Motor hLeftMotor(WHEEL_LEFT_H, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations)

okapi::Motor vRightMotor(WHEEL_RIGHT_V, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations)
okapi::Motor hRightMotor(WHEEL_RIGHT_H, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations)

okapi::Motor lBackMotor(WHEEL_BACK_L, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations)
okapi::Motor rBackMotor(WHEEL_BACK_R, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations)

// Motor Groups (For making the code simpler)
okapi::MotorGroup leftMotor({*vLeftMotor, *hLeftMotor})
okapi::MotorGroup rightMotor({*vRightMotor, *hRightMotor})
okapi::MotorGroup backMotor({*lBackMotor, *rBackMotor})

// Controllers
okapi::Controller master(okapi::ControllerId::master);

// Cameras
pros::Vision sCamera(2, pros::E_VISION_ZERO_CENTER);
pros::vision_signature colorCode = sCamera.signature_from_utility(1, -4275, -3275, -3774, -7043, -5763, -6402, 2.400, 0);

/**
 * Runs when program is started. Blocks everything else.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");
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

void setDTSpeeds(){
	// Store joysticks range = [-1, 1]
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
	leftMotor.moveVelocity(wheelLeftSpeed * 600) // Speed is velocity pct * gearbox
	rightMotor.moveVelocity(wheelRightSpeed * 600)
	backMotor.moveVelocity(wheelBackSpeed * 600)
}

/**
 * For operator control. Automatically runs after initialize if not connected 
 * to a field controller or etc.
 */
void opcontrol() {
	while (true) {
		setDTSpeeds();
		pros::delay(10);
	}
}