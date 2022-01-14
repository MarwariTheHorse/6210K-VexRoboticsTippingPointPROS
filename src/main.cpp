#include "main.h"
#include <fstream>
#include "auton_assist_methods.h"
#include "globals.h"

// Constants
const bool TORQUE_THRESHOLD = 1.99;
const double LIFT_MAX_POSITION = 2.1;
const double LIFT_MIN_POSITION = .09;

// State variables
int liftState;
bool gripState;
bool hookState;
double gripHoldPosition;
bool initializing;

// Anti-doubles
bool prevUp;
bool prevB;

double lastVibrate = 0;

char autonMode = 'N'; // Stands for none

// Autons
void skillsAuton()
{
	// Get the red goal
	driveViaIMU(.1, 0);
	grab();
	driveViaIMU(-.4, 0);
	liftSmall();

	// Line up with the yellow goal and push
	turnViaIMU(-45);
	driveViaIMU(.525, -45); //.8
	turnViaIMU(-90);
	driveViaIMU(1.7, -90);
	pros::delay(1000);

	//Score red
	driveViaIMU(-.3, -90);
	pros::delay(800);
	turnViaIMU(-55);
	liftMax();
	pros::delay(800);
	double time = pros::millis();
	while(pros::millis() - time < 3000){
		leftMotor.moveVelocity(500);
		rightMotor.moveVelocity(500);
		backMotor.moveVelocity(500);
	}
	liftScore();
	pros::delay(300);
	ungrab();

	//Get off of platform
	driveViaTime(1000, -90, -90);
	liftMax();
	pros::delay(600);
	driveViaIMU(-.5, -90);

	// Line up with tall yellow and grab
	turnViaIMU(90);
	liftMin();
	driveViaIMU(.6, 90);
	grab();
	
	// Go hang urself
	liftMax();
	pros::delay(800);
	driveViaTime(3000, 50, 90);
	liftScore();
	judas();

	// From now on we need to see if we need the special maneuver to move rings out
	// of the way and this can only be acheived through testing. Sooo...
	// TODO: Test and see if we can hang with rings jammed between the bot and the ramp base
}
	
void compLeftAuton()
{
	driveViaIMU(.5, 0);
	pros::delay(750);
	driveViaIMU(-.5, 0);
}

void compForwardAuton()
{
	driveViaIMU(1.75, 0);
	grab();
	pros::delay(500);
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
		hook.moveAbsolute(7, 100); // TODO: Make this number more accurate
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
	initializing = true;
	while(autonMode == 'N'){
		// Letter buttons
		if(master.getDigital(okapi::ControllerDigital::A)) autonMode = 'A';
		if(master.getDigital(okapi::ControllerDigital::B)) break;
		if(master.getDigital(okapi::ControllerDigital::X)) autonMode = 'X';
		if(master.getDigital(okapi::ControllerDigital::Y)) autonMode = 'Y';

		// Arrow buttons
		if(master.getDigital(okapi::ControllerDigital::left)) autonMode = '<';
		if(master.getDigital(okapi::ControllerDigital::right)) autonMode = '>';
		if(master.getDigital(okapi::ControllerDigital::L1)) autonMode = '^';
		if(master.getDigital(okapi::ControllerDigital::down)) autonMode = 'V';
	}
	pros::delay(1000);
	initializing = false;

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
	if(autonMode == 'A') experimental();
}

void opcontrol() {
	while (true) {
		if(!initializing){
			setDTSpeeds(); // TODO: Add filters for this method
			setLift(); // TODO: Fix the lift limits
			setVibrate();
			setGrip();
			setHook(); // TODO: Tune the numbers for this one
		}
		pros::delay(5);
	}
}