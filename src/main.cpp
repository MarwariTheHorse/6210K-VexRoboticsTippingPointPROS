#include "main.h"
#include <fstream>

// Motor + pneumatic port definitions
#define WHEEL_LEFT_F 2
#define WHEEL_LEFT_R 11

#define WHEEL_RIGHT_F 12
#define WHEEL_RIGHT_R 19

#define WHEEL_BACK_L 11
#define WHEEL_BACK_R 20

#define LIFT 4

#define GRIP 1

const bool DEBUG = false;
const bool LOGGING_RATE = 100; // ms * 10, plus execution per loop time. ie 100 results in data appox. every second
const bool TORQUE_THRESHOLD = 1.575;
int countRender = 0;

// Motors(port, reversed, gearset, encoderUnits, logger(implied))
okapi::Motor fLeftMotor(WHEEL_LEFT_F, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rLeftMotor(WHEEL_LEFT_R, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor fRightMotor(WHEEL_RIGHT_F, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rRightMotor(WHEEL_RIGHT_R, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor lBackMotor(WHEEL_BACK_L, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rBackMotor(WHEEL_BACK_R, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor lift(LIFT, true, okapi::AbstractMotor::gearset::red, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor grip(GRIP, true, okapi::AbstractMotor::gearset::red, okapi::AbstractMotor::encoderUnits::rotations);

// Motor Groups (For making the code simpler)
okapi::MotorGroup rightMotor({fLeftMotor, rLeftMotor});
okapi::MotorGroup leftMotor({fRightMotor, rRightMotor});
okapi::MotorGroup backMotor({lBackMotor, rBackMotor});

// Controllers
okapi::Controller master(okapi::ControllerId::master);

// Lift variables
int liftState;
int gripState = 1;
double gripHoldPosition;
bool prevL1;
bool prevL2;
bool prevR1;
bool prevR2;

void setLift()
{
	// Store controller state
	bool buttonL1 = master.getDigital(okapi::ControllerDigital::L1);
	bool buttonL2 = master.getDigital(okapi::ControllerDigital::L2);

	if(buttonL1 && !prevL1) liftState++; // Upper button
	if(buttonL2 && !prevL2) liftState--; // Lower button

	if(liftState < 0) liftState = 0;
	if(liftState > 2) liftState = 2;

	// Assign position with vel of 80
	double pos;
	switch(liftState){
		case 0: pos = 0; break;
		case 1: pos = .5; break;
		case 2: pos = 1.7; break;
	}
	lift.moveAbsolute(pos, 80);

	// Update
	prevL1 = buttonL1;
	prevL2 = buttonL2;
}

void setDTSpeeds()
{
	// Store joysticks range = [-1, 1]
	double joyLY = master.getAnalog(okapi::ControllerAnalog::leftY);
	double joyRX = master.getAnalog(okapi::ControllerAnalog::rightX);

	// Filter joysticks
	if(abs(joyLY) < .1){
		joyLY = 0;
	}

	if(abs(joyRX) < .1){
		joyRX = 0;
	}

	// Convert joysticks to wheel speeds
	double wheelLeftSpeed = joyLY - joyRX;
	double wheelRightSpeed = joyLY + joyRX;
	double wheelBackSpeed = (wheelLeftSpeed + wheelRightSpeed) / 2;

	// Filter wheel speeds (We got none right now)

	// Wheel speed assignments
	leftMotor.moveVelocity(wheelLeftSpeed * 600); // Speed is velocity pct * gearbox
	rightMotor.moveVelocity(wheelRightSpeed * 600);
	backMotor.moveVelocity(wheelBackSpeed * 600);
}

void setGrip(){
	// Store controller state
	bool buttonR1 = master.getDigital(okapi::ControllerDigital::R1);
	bool buttonR2 = master.getDigital(okapi::ControllerDigital::R2);

	// Upper button - Lift the grip
	if(buttonR1 && !prevR1){
		gripState = 1;
		grip.moveAbsolute(-2, 100);
	}
	// Lower button - Start lowering the lift
	if(buttonR2 && !prevR2){
		if(gripState != 0){
			gripState = 2;
			grip.moveVelocity(-100);
		}
	}
	// Torque threshold
	if(std::abs(grip.getTorque()) > TORQUE_THRESHOLD && gripState == 2){
		gripState = 0;
		gripHoldPosition = grip.getPosition() + .05; // Shift the grip out .05 rev so that the motor doesn't overheat
	}

	// Make sure the grip holds its position
	if(gripState == 0){
		grip.moveAbsolute(gripHoldPosition, 100);
	}

	// Update variables
	prevR1 = buttonR1;
	prevR2 = buttonR2;
}

void renderControllerDisplay()
{
	master.clear();
	master.setText(0, 0, "Running...");
	// master.setText(1, 0, "liftState: %f", liftState);
}

void renderBrainDisplay() {}

void initialize() {
	// Initialize stuff
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");
	
	// Tare grip
	grip.moveVelocity(100);
	while(std::abs(grip.getTorque()) < TORQUE_THRESHOLD){pros::delay(10);}
	grip.moveVelocity(0);
	pros::delay(100);
	grip.tarePosition();
	grip.moveAbsolute(-2, 100);

	// Tare lift
	lift.moveVelocity(-100);
	while(std::abs(lift.getTorque()) < TORQUE_THRESHOLD){pros::delay(10);}
	lift.moveVelocity(0);
	pros::delay(100);
	lift.tarePosition();
}

void competition_initialize() {}

void disabled() {}

void autonomous() {}

void opcontrol() {
	while (true) {
		setDTSpeeds();
		setLift();
		setGrip();

		countRender++;
		countRender %= 100; // 100 counts of 10 == 1000ms == 1s
		pros::delay(10);
	}
}