#include "main.h"
#include <fstream>

// Motor + pneumatic port definitions
#define WHEEL_LEFT_F 11
#define WHEEL_LEFT_R 12

#define WHEEL_RIGHT_F 2
#define WHEEL_RIGHT_R 1

#define WHEEL_BACK_L 19
#define WHEEL_BACK_R 9

#define LIFT 3

#define GRIP 4

const bool DEBUG = false;
const bool LOGGING_RATE = 100; // ms * 10, plus execution per loop time. ie 100 results in data appox. every second
int countRender = 0;

// Motors(port, reversed, gearset, encoderUnits, logger(implied))
okapi::Motor fLeftMotor(WHEEL_LEFT_F, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rLeftMotor(WHEEL_LEFT_R, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor fRightMotor(WHEEL_RIGHT_F, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rRightMotor(WHEEL_RIGHT_R, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor lBackMotor(WHEEL_BACK_L, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rBackMotor(WHEEL_BACK_R, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor lift(LIFT, false, okapi::AbstractMotor::gearset::red, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor grip(GRIP, false, okapi::AbstractMotor::gearset::red, okapi::AbstractMotor::encoderUnits::rotations);

// Motor Groups (For making the code simpler)
okapi::MotorGroup rightMotor({fLeftMotor, rLeftMotor});
okapi::MotorGroup leftMotor({fRightMotor, rRightMotor});
okapi::MotorGroup backMotor({lBackMotor, rBackMotor});

// Controllers
okapi::Controller master(okapi::ControllerId::master);

// Lift variables
int liftState;
int gripState;
bool prevL1;
bool prevL2;
bool prevR1;
bool prevR2;

void setLift()
{
	// Store controller state
	bool buttonL1 = master.getDigital(okapi::ControllerDigital::L1);
	bool buttonL2 = master.getDigital(okapi::ControllerDigital::L2);

	// Upper button
	if(buttonL1 && !prevL1){
		prevL1 = true;
		switch(liftState){
			case 0: liftState = 1; break;
			case 1: // Same as below
			case 2: liftState = 3; break;
			case 3: liftState = 2; break;
		}
	}

	// Lower button
	if(buttonL2 && !prevL2){
		prevL2 = true;
		switch(liftState){
			case 0: liftState = 2; break;
			case 1:	// Same as below
			case 2: liftState = 0; break;
			case 3: liftState = 1; break;
		}
	}

	// Assign position with vel of 80
	double pos;
	switch(liftState){
		case 0: pos = 0; break;
		case 1: pos = .5; break;
		case 2: pos = 1.3; break;
		case 3: pos = 1.5; break;
	}
	lift.moveAbsolute(pos, 80)

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
	if(buttonR1 && !prevR1){gripState = 1;}
	// Lower button - lower the grip
	if(buttonR2 && !prevR2){gripState = 2;}
	// Torque threshold TODO: Write this code
	if(){gripState = 0;}

	// Set actual motor speeds TODO: Write this code
	if(gripState == 1){
		grip.moveVelocity(100 * skjsakljf) // TODO: Replace gibberish by red gearbox multiplier,
		// TODO: Why do we have to multiply by kdjfa;kfjads if we pass the gearbox type into the constructor???
	}
	if(gripState == 2){
		grip.moveVelocity(100 * skjsakljf) // TODO: Replace gibberish by red gearbox multiplier,
	}
	if(gripState == 0){

	}

	// Update variables
	prevR1 = buttonR1;
	prevR2 = buttonR2;
}

void renderControllerDisplay()
{
	master.clear();
	master.setText(0, 0, "Running...");
	master.setText(1, 0, "liftState: %f", liftState);
	pros::delay(1000);
}

void renderBrainDisplay() {}

void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");
}

void competition_initialize() {}

void disabled() {}

void autonomous() {}

void opcontrol() {
	while (true) {
		setDTSpeeds();
		setLift();
		setGrip();
		if(countRender == 0){
			renderControllerDisplay();
			renderBrainDisplay();
		}

		countRender++;
		countRender % 100; // 100 counts of 10 == 1000ms == 1s
		pros::delay(10);
	}
}