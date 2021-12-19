#include "main.h"
#include <fstream>
#include "auton_assist_methods.h"
#include "globals.h"

const bool DEBUG = false;
const bool TORQUE_THRESHOLD = 1.99;
const double LIFT_MAX_POSITION = 2.1; // 2.5 - 3.0
const double LIFT_MIN_POSITION = .09;
const bool LOGGING_RATE = 100; // ms * 10, plus execution per loop time. ie 100 results in data appox. every second

// State variables
int liftState;
bool gripState = false;
bool hookState;
double gripHoldPosition;

// Anti-doubles
bool prevUp;
bool prevB;

double lastVibrate = 0;

// Globals
char autonMode = 'N'; // Stands for none

// Autons
/*
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


}

void compLeftAuton()
{
	driveViaIMU(.5, 0);
	pros::delay(750); // TODO: Replace this line with something involving getActualVelocity();
	driveViaIMU(-.5, 0);
}

void compForwardAuton()
{
	driveViaIMU(1.5, 0);
	pros::delay(750);
	grab();
	pros::delay(1000);
	driveViaIMU(-1.5, 0);
}

void compRightAuton()
{
	// The line below is a filler line right now, but actually works quite nicely
	compLeftAuton();
}

// For screwing around
void experimental()
{
	gps.initialize_full(-1.2192, -1.2192, 90, 1, -1);
	driveViaGPS(1.2192, -1.2192);
}
*/
// opcontrol
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
	// Grip variables
	if(master.getDigital(okapi::ControllerDigital::L1) && gripState == true){
		grip.set_value(false);
		gripState = false;
		master.rumble(".");
	}
	if(master.getDigital(okapi::ControllerDigital::L2) && gripState == false){
		grip.set_value(true);
		gripState = true;
		master.rumble("-");
	}
}

void setVibrate(){
	if(pros::millis() - lastVibrate > 750 && lift.getPosition() > 1){
		master.rumble(".");
		lastVibrate = pros::millis();
	}
}

// TODO: Needs testing
void setHook(){
	// Store controller state
	bool buttonUp = master.getDigital(okapi::ControllerDigital::up);

	// State changer
	if(buttonUp && !prevUp){
		hookState = !hookState;
	}
	// Upper button - Lift the hook
	if(!hookState){
		hook.moveAbsolute(0, 100);
	}
	// Lower button - Start lowering the hook
	if(hookState){
		hook.moveAbsolute(1.5, 100); // TODO: Make this number more accurate
									 // Perhaps the strength inefficiency
									 // is a result of it not wanting to go further.
	}

	// Update variables
	prevUp = buttonUp;
}

// PROS-called functions
void initialize() {
	// Initialize stuff
	pros::lcd::initialize();

	// Calibrate IMU
	master.setText(0, 0, "Calibrating...");
	imu.calibrate();
	while (imu.isCalibrating()){pros::delay(10);}
	master.clear();

	// Tare lift
	lift.moveVelocity(-100);
	while(std::abs(lift.getTorque()) < TORQUE_THRESHOLD){pros::delay(10);}
	lift.moveVelocity(0);
	pros::delay(100);
	lift.tarePosition();

	// Tare hook
	// TODO: We need a sensor so that this thing can auto-zero without tearing itself apart
	hook.tarePosition();
	
	// Everything holds
	leftMotor.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
	rightMotor.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
	backMotor.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
	lift.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);

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

void competition_initialize()
{
}

void disabled() {}

void autonomous() {/*
	if(autonMode == 'X') skillsAuton();
	if(autonMode == '<') compLeftAuton();
	if(autonMode == '>') compRightAuton();
	if(autonMode == '^') compForwardAuton();
	if(autonMode == 'A') experimental();*/
}

void opcontrol() {
	while (true) {
		setDTSpeeds(); // TODO: Add filters for this method
		setLift(); // TODO: Fix the lift limits
		setVibrate();
		setGrip();
		setHook(); // TODO: Tune the numbers for this one
		pros::lcd::print(0, "%4.2f", hook.getPosition());
		updateFilters();
		pros::delay(5);
	}
}