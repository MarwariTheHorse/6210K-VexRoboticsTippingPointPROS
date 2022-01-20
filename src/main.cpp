#include "main.h"
#include <fstream>
#include "auton_assist_methods.h"
#include "globals.h"

// Constants
const bool TORQUE_THRESHOLD = 1.99;

// State variables
int liftState;
bool gripState;
bool hookState;
double gripHoldPosition;
bool initialized;

// Anti-doubles
bool prevUp;
bool prevB;

double lastVibrate = 0;

char autonMode = 'N'; // Stands for none

// Autons
void skillsAuton()
{
    // Get the red goal
	driveViaIMU(.3, 0);
	grab();
	driveViaIMU(-.4, 0);
	liftSmall();

	// Line up with the yellow goal and push in corner
	turnViaIMU(90);
	driveViaIMU(-2.1, 90);
	driveViaIMU(.9, 90);
	turnViaIMU(-45);

	//Score red
	liftMax();
	pros::delay(800);
	// driveViaTime correction fails when goal is high
	double time = pros::millis();
	while(pros::millis() - time < 3000){
		rightMotor.moveVelocity(500);
		leftMotor.moveVelocity(500);
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
	pros::delay(1000);
	// Use base of other ramp to get the goal in the grip
	driveViaTime(5000, 300, 90);
	grab();
	liftSmall();
	driveViaIMU(-.5, 90);

	// Go hang urself
	liftMax();
	pros::delay(800);
	driveViaTime(3000, 300, 90);
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
	driveViaIMU(3, 0);
	turnViaIMU(90);
}

// opcontrol
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

void setHook(){
	// Store controller state
	bool buttonUp = master.getDigital(okapi::ControllerDigital::up);

	// State changer
	if(buttonUp && !prevUp){
		hookState = !hookState;
	}
	if(!hookState){
		hook.moveAbsolute(0, 100);
	}
	if(hookState){
		hook.moveAbsolute(6.5, 100);
	}

	// Update variables
	prevUp = buttonUp;
}

// PROS-called functions
void initialize() {
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
	if(!initialized){
		// Initialize stuff
		pros::lcd::initialize();

		// Calibrate IMU
		master.setText(0, 0, "Calibrating...");
		imu.calibrate();
		while (imu.isCalibrating()){pros::delay(10);}
		master.clear();

		// Tare hook
		hook.moveVelocity(-100);
		while(!hookStop.get_value()){pros::delay(10);}
		hook.moveVelocity(0);
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
		pros::delay(1000);
		master.clear();
	}
	initialized = true;

	pros::delay(1000);

	while (true) {
		setVibrate();
		setGrip();
		setHook();
		setDTSpeeds(); // TODO: Add filters for this method
		setLift(); // TODO: Fix the lift limits
		pros::delay(10);
	}
}