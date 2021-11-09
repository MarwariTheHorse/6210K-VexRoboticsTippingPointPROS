#include "main.h"
#include <fstream>

// Motor + pneumatic port definitions
#define WHEEL_LEFT_F 2
#define WHEEL_LEFT_R 13

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

// Sensors
okapi:: // TODO: Add the sensors

// Lift variables
int liftState;
int gripState = 1;
double gripHoldPosition;
bool prevL1;
bool prevL2;
bool prevR1;
bool prevR2;

// Globals
char autonMode = 'N'; // Stands for none

// Auton assist methods //
void driveViaDist(double dist)
{
	dist *= 39.3701 / (3 * pros::pi); // To in. then to rev
	backMotor.moveRelative(dist, 80);
	rightMotor.moveRelative(dist, 80);
	leftMotor.moveRelative(dist, 80);
	while(!leftMotor.isStopped()) pros::delay(10);
}

void driveViaIMU(double dist, double heading) // TODO: get this from last year's code
{
	dist *= 39.3701 / (3 * pros::pi); // To in. then to rev
}

void grab() // NOTE: Grip should be in holding, allowing it to grip via this simple piece of code
{
	grip.moveVelocity(-100);
	if(std::abs(grip.getTorque()) > TORQUE_THRESHOLD){
		grip.moveVelocity(0);
	}
}

void ungrab() // TODO: Write this code
{
	grip.moveAbsolute(-2, 80);
}

void driveViaGPS()
{

}

void driveViaTime(double ms, double vel){
	leftMotor.moveVelocity(vel);
	rightMotor.moveVelocity(vel);
	backMotor.moveVelocity(vel);
	pros::delay(ms);
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void turnViaIMUTo()
{
	// TODO: Find and use turning code from last year
}

void liftMin() lift.moveAbsolute(0);
void liftSmall() lift.moveAbsolute(.5); // 0, .5, 1.7
void liftMax() lift.moveAbsolute(1.7);
void liftScore() lift.moveAbsolute(.5);

void scoreGoal()
{
	// Lower the lift
	liftScore();
	// Release the goal
	ungrab();
	// Lift the lift back to max
	liftMax();
}

void skillsAuton()
{
	//////////////////////
	// Grab nearby goal //
	//////////////////////

	// Drive to the goal
	const BACK_WHEEL_DIST_1 = 100;
	driveViaIMU(BACK_WHEEL_DIST_1); // 100mm = 10cm

	// Grab the goal
	grab();

	// Lift the goal a little
	liftSmall();

	/////////////////////////////
	// Drive to the other side //
	/////////////////////////////

	// Back up 
	driveViaIMU(BACK_WHEEL_DIST_1*2);

	// Turn towards gap between yellow
	turnViaIMUTo(-90);

	// Drive to the gap
	driveViaGPS(3.5);

	// Turn to perpendicular
	turnViaIMUTo(0);

	// Drive to other side of goal
	driveViaIMU(3.0);

	// Turn to goal
	turnViaIMUTo(-90);

	///////////////////////////////
	// Swipe goal out of the way //
	///////////////////////////////

	// Lift our goal up as we approach the annoying goal
	liftMax();

	// Hit the goal
	driveViaTime(500);

	// Lower goal (Manual code that we only use once)

	// Rotate to swipe the annoying goal off the ramp
	turnViaIMUTo(-45);

	///////////////////////////////////
	// Score goal 1 in bridge center //
	///////////////////////////////////

	// Alighn with scoring spot
	driveViaIMU(1.5);
	turnViaIMUTo(-90);

	// Lift goal then approach
	liftMax();
	driveViaTime(500);

	// Score
	scoreGoal(); // TODO: Lower goal, release goal

	/////////////////////////////////////////////////
	// Turn around, grab tall yellow, and score it //
	/////////////////////////////////////////////////

	// Do a 180
	turnViaIMUTo(90);

	// Lower lift
	liftMin();

	// Approach, grip, and lift goal
	driveViaIMU(1.5);
	grab();
	liftSmall();

	// Do a 180
	turnViaIMUTo(-90);

	// Lift goal approach the ramp
	liftMax();

	// Score the tall goal

	////////////////////////////////////////////////////////
	// Turn around, grab another yellow, and score it too //
	////////////////////////////////////////////////////////

	// Back up

	// Turn towards a yellow goal

	// Approach, grip, then lift goal a litte

	///////////////////////////////////////////////////////////////////
	// Grab nearby color goal, drive to the other side, and score it //
	///////////////////////////////////////////////////////////////////

	// Turn towards the color goal

	// Approach, grip, then lift the goal a litte

	// Turn and approach the ramp

	// Lower lift and score

	/////////////////////////////////////////
	// Grab the last yellow and score that //
	/////////////////////////////////////////

	// Back up then turn towards the final yellow

	// Approach, grip, and lift the yellow

	// Turn around and approach the ramp

	// Lower the lift and score

	////////////////////////
	// Hang on the bridge //
	////////////////////////

	// Lower the arm and apply pneumatic brake

}

void setLift()
{
	if(master.getDigital(okapi::ControllerDigital::R1)) lift.moveVelocity(600);
	else if(master.getDigital(okapi::ControllerDigital::R2)) lift.moveVelocity(-600);
	else lift.moveVelocity(0);

	// // Store controller state
	// bool buttonR1 = master.getDigital(okapi::ControllerDigital::R1);
	// bool buttonR2 = master.getDigital(okapi::ControllerDigital::R2);

	// if(buttonR1 && !prevR1) liftState++; // Upper button
	// if(buttonR2 && !prevR2) liftState--; // Lower button

	// if(liftState < 0) liftState = 0;
	// if(liftState > 2) liftState = 2;

	// // Assign position with vel of 80
	// double pos;
	// switch(liftState){
	// 	case 0: pos = 0; break;
	// 	case 1: pos = .5; break;
	// 	case 2: pos = 1.7; break;
	// }
	// lift.moveAbsolute(pos, 80);

	// // Update
	// prevR1 = buttonR1;
	// prevR2 = buttonR2;
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
	bool buttonL1 = master.getDigital(okapi::ControllerDigital::L1);
	bool buttonL2 = master.getDigital(okapi::ControllerDigital::L2);

	// Upper button - Lift the grip
	if(buttonL1 && !prevL1){
		gripState = 1;
		grip.moveAbsolute(-2, 100);
	}
	// Lower button - Start lowering the lift
	if(buttonL2 && !prevL2 && gripState != 0){
		gripState = 2;
		grip.moveVelocity(-100);
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
	prevL1 = buttonL1;
	prevL2 = buttonL2;
}

void renderControllerDisplay()
{
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

	// // Tare lift
	// lift.moveVelocity(-100);
	// while(std::abs(lift.getTorque()) < TORQUE_THRESHOLD){pros::delay(10);}
	// lift.moveVelocity(0);
	// pros::delay(100);
	// lift.tarePosition();
	lift.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
}

void competition_initialize()
{
	// Render the prompt
	master.setText(0, 0, "Select a mode:");
	master.setText(1, 0, "X:Auton");

	// Get the choice
	while(autonMode == 'N'){
		if(master.getDigital(okapi::ControllerDigital::X)){
			autonMode = 'X';
		}
		if(master.getDigital(okapi::ControllerDigital::B)){
			break;
		}
	}
	master.clear();
}

void disabled() {}

void autonomous() {
	if(autonMode = 'X') skillsAuton();
}


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