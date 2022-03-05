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

// Anti-doubles
bool prevUp;
bool prevB;

double lastVibrate = 0;

char autonMode = 'N'; // Stands for none

// Autons
void skillsAuton()
{
	// THE STRATEGY
	// 1. Grab the red goal and score it on the other side, pushing the yellow in the way
	// @ 40
	// 2. Grab the unmoved small yellow scoring on the red platform
	// @ 80
	// 3. Grab the yellow we pushed out of the way earlier and score that.
	// @ 120
	// 3. Grab the nearby blue and score that
	// @ 160
	// 4. Grab the tall yellow and hang
	// @ 230

    // Get the red goal
	liftMin();
	driveViaIMU(.3, 0);
	grab();
	driveViaIMU(-.2, 0); // was -.4
	liftSmall();

	// Line up with the yellow goal and push in corner
	turnViaIMU(93);
	driveViaIMU(-2.1, 93);
	
	// Line up with the bridge and get around the base
	driveViaIMU(.3, 93); 
	liftMax();
	turnViaIMU(-52.5);
	pros::delay(500);
	driveViaIMU(.5, -52.5); // Get near platform
	driveViaTime(2000, 200); // Make sure we are around the base
	
	// Score red
	liftScore();
	pros::delay(300);
	ungrab();

	//Get off of platform
	driveViaTime(600, -90);
	liftMax();
	pros::delay(600);
	driveViaIMU(-.9, -90);

	// Line up with far yellow and grab
	liftMin();
	turnViaIMU(10);
	pros::delay(600);
	driveViaSig(3);
	grab();
	pros::delay(400);
	turnViaIMU(33); // Make sure we have not been thrown off
	driveViaIMU(-.4, 33);

	// Turn around and get to platfrom
	liftSmall();
	turnViaIMU(-128.5);
	liftMax();
	pros::delay(600);
	driveViaTime(4000, 200);
	pros::delay(300);

	// Score the first yellow then back up from the bridge
	ungrab();
	driveViaIMU(-.2, -90);
	liftMin();
	driveViaIMU(-.7, -90);

	// Align with and get the og yellow
	turnViaIMU(-145);
	driveViaIMU(1.9, -145);
	grab();
	pros::delay(400);
	driveViaIMU(-1.8, -145);

	// turn and score og yellow then back up
	liftMax();
	turnViaIMU(-90);
	pros::delay(500);
	driveViaTime(2000, 200);
	liftScore();
	pros::delay(300);
	ungrab();
	driveViaTime(600, -90);
	liftMax();
	pros::delay(300);
	driveViaIMU(-.5, -90);
	liftMin();

	turnViaIMU(-170);
	driveViaTime(3000, 200);
	grab();
	turnViaIMU(-180);
	driveViaIMU(-.3, -180);
	liftMax();
	turnViaIMU(-300);
	driveViaIMU(2, -300);
	driveViaTime(2000, 200);
	pros::delay(300);
	driveViaIMU(-2, -270);
	pros::delay(300);
	driveViaTime(4000, 200);
	liftScore();
	pros::delay(300);
	ungrab();
	driveViaTime(600, -90);
	liftMax();
	driveViaIMU(-.5, -270);



	/*
	turnViaIMU(-178);
	driveViaTime(2000, 200);
	grab();
	turnViaIMU(-180);
	driveViaIMU(-.5, -180);
	turnViaIMU(-120);
	driveViaIMU(4000, 200);
	pros::delay(300);
	driveViaTime(3000, -200);
	pros::delay(600);
	driveViaTime(3000, 200);
	liftScore();
	judas();




	// driveToRamp(2000, true); // 2000 ms, isRedRamp

	// Score the second yellow that we deal with
	liftScore();
	pros::delay(300);
	ungrab();

	// Get off of platform
	driveViaTime(1000, -90);
	liftMax();
	pros::delay(600);
	driveViaIMU(-.5, -90);

	// Get the yellow we pushed out of the way earlier
	turnViaIMU(-180);
	driveViaIMU(1, -180);
	turnViaIMU(-90);
	driveViaSig(.3, 3);
	grab();

	// Reverse of above
	driveViaIMU(-.3, -90);
	turnViaIMU(-180);
	liftMax();
	driveViaIMU(-1, -180);
	turnViaIMU(-90);

	// Score the yellow
	driveViaTime(1000, 600); // Make sure we are around the base
	liftScore();
	pros::delay(300);
	ungrab();

	//Get off of platform
	driveViaTime(1000, -90);
	liftMax();
	pros::delay(600);
	driveViaIMU(-.5, -90);

	// Get the blue from down south
	turnViaIMU(-180);
	liftMin();
	driveViaIMU(1.7, -180); // get near the blue
	driveViaSig(1, 2); // Finish the journey w/ the camera
	grab();

	// get to the bridge
	turnViaIMU(50);
	driveViaIMU(6, 50);
	driveToRamp(2000, false);

	// score the blue
	liftScore();
	pros::delay(300);
	ungrab();

	//Get off of platform
	driveViaTime(1000, -90);
	liftMax();
	pros::delay(600);
	driveViaIMU(-.5, 90);

	// Get the tall yellow
	turnViaIMU(90);
	driveViaSig(1, 3);
	grab();

	// Score and pull a judas
	turnViaIMU(90);
	driveToRamp(2000, false);
	liftScore();
	judas();

	*/

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
	driveViaIMU(.7, 0);
	grab();
	pros::delay(500);
	driveViaIMU(-.5, 0);
}

// For screwing around
void experimental() // 2900 is the magic number
{
	liftMin();
	driveViaSig(3);
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
	leftMotor.moveVelocity(wheelLeftSpeed * SPEED); // Speed is velocity pct * gearbox
	rightMotor.moveVelocity(wheelRightSpeed * SPEED);
	backMotor.moveVelocity(wheelBackSpeed * SPEED);
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
	static bool initialized = false; // This line only runs once, no matter how many function calls
	if(!initialized){
		// Initialize stuff
		pros::lcd::initialize();

		// Configure the goal vision sensor
		pros::vision_signature_s_t sigGoalRed = goalVision.signature_from_utility(1, 5607, 8193, 6900, -793, -297, -545, 3.7, 0);
		pros::vision_signature_s_t sigGoalBlue = goalVision.signature_from_utility(2, -2909, -2315, -2612, 8851, 10215, 9533, 10.5, 0);
		pros::vision_signature_s_t sigGoalYellow = goalVision.signature_from_utility(3, 431, 745, 588, -3343, -3041, -3192, 8.2, 0);

		goalVision.set_signature(1, &sigGoalRed);
		goalVision.set_signature(2, &sigGoalBlue);
		goalVision.set_signature(3, &sigGoalYellow);

		rampVision.set_signature(1, &sigGoalRed);
		rampVision.set_signature(2, &sigGoalBlue);

		goalVision.set_exposure(33);

		// Calibrate IMU
		master.setText(0, 0, "Calibrating...");
		imu.reset(); // This line is blocking so there is no reason to wait after
		goalDetect.calibrate();
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
		master.clear();
		pros::delay(1000);
		initialized = true;
	}

	while (true) {
		setVibrate();
		setGrip();
		setHook();
		setDTSpeeds();
		setLift();
		pros::delay(10);
	}
}
