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

#define PISTON 'H'

#define VISION 6
#define GYRO 8

const bool DEBUG = false;
const bool TORQUE_THRESHOLD = 1.575;
const bool LOGGING_RATE = 100; // ms * 10, plus execution per loop time. ie 100 results in data appox. every second

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

// Pneumatics
pros::ADIDigitalOut piston(PISTON);

// Controllers
okapi::Controller master(okapi::ControllerId::master);

// Sensors
pros::Vision vision (VISION);
okapi::IMU IMU(GYRO);

// Lift variables
int liftState;
int gripState = 1;
double gripHoldPosition;

// Anti-doubles
bool prevL1;
bool prevL2;
bool prevR1;
bool prevR2;
bool prevB;

double pistonTime1 = 0;
double pistonTime2 = -1000;
bool pistonState;

double lastVibrate = 0;

// Globals
char autonMode = 'N'; // Stands for none

// Auton assist methods //
void driveViaDist(double dist) // Untested
{
	dist *= 39.3701 / (2.75 * PI); // To in. then to rev
	backMotor.moveRelative(dist, 600);
	rightMotor.moveRelative(dist, 600);
	leftMotor.moveRelative(dist, 600);
	while(!leftMotor.isStopped()) pros::delay(10);
}

void driveViaIMU(double dist, double heading) // Untested TODO: get this from last year's code
{
	dist *= 39.3701 / (2.75 * PI); // To in. then to rev
	backMotor.moveRelative(dist, 80);
	rightMotor.moveRelative(dist, 80);
	leftMotor.moveRelative(dist, 80);
	if(IMU.get() + 1 > heading || IMU.get() - 1 < heading) pros::delay(1);
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

void driveViaTime(double ms, double vel){
	leftMotor.moveVelocity(vel);
	rightMotor.moveVelocity(vel);
	backMotor.moveVelocity(vel);
	pros::delay(ms);
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void turnViaIMUTo() // write from last year's code
{
}

void liftMin() {lift.moveAbsolute(0, 90);}
void liftSmall() {lift.moveAbsolute(.5, 90);} // 0, .5, 1.7
void liftMax() {lift.moveAbsolute(1.7, 90);}
void liftScore() {lift.moveAbsolute(.5, 90);}
void liftHang() {lift.moveAbsolute(.7, 90);} // needs tweaking

void scoreGoal()
{
	// Lower the lift
	liftScore();
	// Release the goal
	ungrab();
	// Lift the lift back to max
	liftMax();
}

void hang()
{
	liftHang();
	// TODO: Add a waitUntil lift is done here because otherwise, async
	piston.set_value(true);
}

void skillsAuton()
{
	/*
	//////////////////////
	// Grab nearby goal //
	//////////////////////

	// Drive to the goal
	const int BACK_WHEEL_DIST_1 = 100;
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

	hang();

*/

}

void compLeftAuton()
{
	driveViaDist(.5);
	pros::delay(750); // TODO: Replace this line with something involving getActualVelocity();
	driveViaDist(-.5);
}

void compForwardAuton()
{
	driveViaDist(1.5);
	pros::delay(750);
	grab();
	pros::delay(1000);
	driveViaDist(-1.5);
}

void compRightAuton()
{
	// The line below is a filler line right now, but actually works quite nicely
	compLeftAuton();
}

void setLift()
{
	if(master.getDigital(okapi::ControllerDigital::R1)) lift.moveVelocity(600);
	else if(master.getDigital(okapi::ControllerDigital::R2)) lift.moveVelocity(-600);
	else lift.moveVelocity(0);
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

void setPiston()
{
	bool buttonB = master.getDigital(okapi::ControllerDigital::B);
	if(buttonB && !prevB){
		pistonTime1 = pistonTime2;
		pistonTime2 = pros::millis();
		if(pistonTime2 - pistonTime1 < 500){
			pistonState = !pistonState;
			piston.set_value(pistonState ? 4095 : 0); // Set output to 5V or 0V
			pistonTime1 -= 2000; // Fudge the numbers to that it doesn't double trigger
			pistonTime2 -= 1000;
		}
	}
	prevB = buttonB;
}

void setVibrate(){
	if(pros::millis() - lastVibrate > 750 && lift.getPosition() > 1){
		master.rumble(".");
		lastVibrate = pros::millis();
	}
}

void renderControllerDisplay()
{
}

void renderBrainDisplay() {}

void initialize() {
	// Initialize stuff
	pros::lcd::initialize();
	pros::lcd::set_text(1, "TEST");
	
	// Tare grip
	grip.moveVelocity(100);
	while(std::abs(grip.getTorque()) < TORQUE_THRESHOLD){pros::delay(10);}
	grip.moveVelocity(0);
	pros::delay(100);
	grip.tarePosition();
	grip.moveAbsolute(-2, 100);

	lift.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);

	// Render the prompt
	master.setText(0, 0, "Select a mode:");
	master.setText(1, 0, "X:Auton/A:Comp");

	// Get the choice
	while(autonMode == 'N'){
		// Letter buttons
		if(master.getDigital(okapi::ControllerDigital::A)) autonMode = 'A';
		if(master.getDigital(okapi::ControllerDigital::B)) break;
		if(master.getDigital(okapi::ControllerDigital::X)) autonMode = 'X';
		if(master.getDigital(okapi::ControllerDigital::Y)) autonMode = 'Y';

		// Arrow buttons
		if(master.getDigital(okapi::ControllerDigital::left)) autonMode = '<';
		if(master.getDigital(okapi::ControllerDigital::right)) autonMode = '>';
		if(master.getDigital(okapi::ControllerDigital::up)) autonMode = '^';
		if(master.getDigital(okapi::ControllerDigital::down)) autonMode = 'V';
	}
	master.clear();
}

void competition_initialize()
{
}

void disabled() {}

void autonomous() {
	if(autonMode == 'X') skillsAuton();
	if(autonMode == '<') compLeftAuton();
	if(autonMode == '>') compRightAuton();
	if(autonMode == '^') compForwardAuton();
}

void opcontrol() {
	while (true) {
		setDTSpeeds();
		setLift();
		setGrip();
		setPiston();
		setVibrate();

		pros::delay(10);
	}
}