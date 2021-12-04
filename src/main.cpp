#include "main.h"
#include <fstream>
#include "auton_assist_methods.h"
#include "globals.h"

const bool DEBUG = false;
const bool TORQUE_THRESHOLD = 1.575;
const double LIFT_MAX_POSITION = 2.1; // 2.5 - 3.0
const double LIFT_MIN_POSITION = .1;
const bool LOGGING_RATE = 100; // ms * 10, plus execution per loop time. ie 100 results in data appox. every second

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

void skillsAuton()
{
	// Configure the GPS for skills

	//////////////////////
	// Grab nearby goal //
	//////////////////////

	// Drive to the goal
	driveViaIMU(.1, 0);

	// Grab the goal
	grab();

	// Get better grip
	driveViaIMU(-.45, 0);
	ungrab();
	driveViaIMU(.05, 0);
	grab();
	lift.moveAbsolute(.2, 100); //make it not scrape the ground so we can move

	/////////////////////////////
	// Drive to the other side //
	/////////////////////////////
	// Turn towards gap between yellow
	turnViaIMU(-45);

	// Drive to the gap
	driveViaIMU(.8, -45); // Previously 1

	// Turn to perpendicular
	turnViaIMU(-90);

	// Drive to other side of goal
	driveViaIMU(1, -90);
	// Turn to ramp
	turnViaIMU(-50);

	///////////////////////////////
	// Swipe goal out of the way (De-tilt the ramp) //
	///////////////////////////////

	// Hit the ramp
	driveViaTime(2000, 400, -50);
	lift.moveRelative(.7, 100);
	while(lift.getPosition() < .65) pros::delay(10);

	// De-tilt and back up to center on the ramp
	driveViaIMU(-.75, -50);
	turnViaIMU(-90);
	driveViaTime(1000, 100, -90);
	///////////////////////////////////
	// Score goal 1 in bridge center //
	///////////////////////////////////

	// Lift goal then approach
	liftMax();
	while(lift.getPosition() < 1.75) pros::delay(10);
	driveViaTime(3000, 600, -90);

	// Slap that bad boy onto the ramp
	scoreGoal();

	// This section is the 11/20 auton
	//////////////////////////////
	// Get tall yellow and judas //
	//////////////////////////////

	// Grab yellow
	driveViaIMU(-.4, -90);
	turnViaIMU(90);
	liftMin();
	while(lift.getPosition() > .1) pros::delay(10);
	driveViaIMU(.75, 90);
	pros::delay(10);
	grab();

	// Lift a bit
	lift.moveAbsolute(.7, 90);

	//Drive forward some
	driveViaIMU(.4, 90);

	//Yeet those stupid rings outta the way
	turnViaIMU(0);
	driveViaIMU(1, 0);
	pros::delay(1000);
	driveViaIMU(-.8, 0);
	turnViaIMU(90);
	
	// Judas
	liftMax();
	pros::delay(10);
	leftMotor.moveVelocity(600); // So that it runs up against the ramp and enables the ability to judas
	rightMotor.moveVelocity(600);
	backMotor.moveVelocity(600);
	pros::delay(3000);
	judas();
	leftMotor.moveVelocity(0); // Don't kill the motors
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
	lift.setBrakeMode(okapi::AbstractMotor::brakeMode::coast);
	lift.moveVelocity(0);
	pros::delay(60000);
	
/*
FUTURE AUTON FOR AFTER 11/20
	/////////////////////////////////////////////////
	// Turn around, grab one yellow, and score it //
	/////////////////////////////////////////////////

	// Go to the goal

	// Lower lift

	// Approach, grip, and lift goal
	

	// Do a 180

	// Lift goal approach the ramp

	// Score the  goal
	
	////////////////////////////////////////////////////////
	// Turn around, grab blue by the auton line //
	////////////////////////////////////////////////////////
	//Back up and turn
	
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
	// judas on the bridge //
	////////////////////////

	judas();

*/

}

void compLeftAuton()
{
	driveViaIMU(.5, 0);
	pros::delay(750); // TODO: Replace this line with something involving getActualVelocity();
	driveViaIMU(-.5, 0);
}

void compForwardAuton()
{
	driveViaIMU(1.8, 0);
	pros::delay(500); // Originally 750
	grab();
	pros::delay(250); // Originally not here
	pros::delay(1000);
	leftMotor.moveVelocity(-300);
	rightMotor.moveVelocity(-300);
	backMotor.moveVelocity(-300);
	pros::delay(2000);
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void compRightAuton()
{
	// The line below is a filler line right now, but actually works quite nicely
	compLeftAuton();
}

void experimental()
{
	gps.initialize_full(-1.2192, -1.2192, 90, 1, -1);
	driveViaGPS(1.2192, -1.2192);
}

void setLift()
{
	if(master.getDigital(okapi::ControllerDigital::R1) && lift.getPosition() < LIFT_MAX_POSITION) lift.moveVelocity(600);
	else if(master.getDigital(okapi::ControllerDigital::R2) && lift.getPosition() > LIFT_MIN_POSITION) lift.moveVelocity(-600);
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
	double wheelBackSpeed = joyLY;

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
		grip.moveAbsolute(-2, 100);
	}
	// Lower button - Start lowering the lift
	if(buttonL2 && !prevL2){
		grip.moveAbsolute(-3.5, 100);
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

void initialize() {
	// Initialize stuff
	pros::lcd::initialize();

	// Calibrate IMU
	master.setText(0, 0, "Calibrating...");
	imu.calibrate();
	while (imu.isCalibrating()){pros::delay(10);}
	master.clear();
	
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

	// Everything holds
	leftMotor.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
	rightMotor.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
	backMotor.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
	lift.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
	grip.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);

	// Render the prompt
	master.setText(0, 0, "Select a mode:");

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

// Not a clue on what someone would use thing dumb thing for
void competition_initialize()
{
}

void disabled() {}

void autonomous() {
	if(autonMode == 'X') skillsAuton();
	if(autonMode == '<') compLeftAuton();
	if(autonMode == '>') compRightAuton();
	if(autonMode == '^') compForwardAuton();
	if(autonMode == 'A') experimental();
}

void opcontrol() {
	while (true) {
		setDTSpeeds();
		setLift();
		setGrip();
		setPiston();
		setVibrate();
		updateFilters();
		pros::delay(5);
	}
}