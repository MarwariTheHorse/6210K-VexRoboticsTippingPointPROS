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
okapi::IMU imu(GYRO);

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

int sgn(double d)
{
	if(d < 0){return -1;}
	if(d > 0){return 1;}
	return 0;
}

// Auton assist methods //
void driveViaDist(double dist)
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
	int rotation;
	int speed;
	// reset all motor encoders to zero
	// 10000 units is equal to 56" of travel
	backMotor.tarePosition();
	leftMotor.tarePosition();
	rightMotor.tarePosition();
	int d = 0;
	if(d < dist){
		while (d < dist){
			speed = 240;
			rotation = (heading - imu.get()) * 3;
			leftMotor.moveVelocity(speed - rotation);
			rightMotor.moveVelocity(speed + rotation);
			backMotor.moveVelocity(speed);
			pros::delay(5);
			d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
		}
	}else{
		while (d > dist){
			speed = -240;
			rotation = (heading - imu.get()) * 3;
			leftMotor.moveVelocity(speed - rotation);
			rightMotor.moveVelocity(speed + rotation);
			pros::delay(5);
			d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
		}
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void driveViaTime(double ms, double vel, double heading){
	double startTime = pros::millis();
	while (pros::millis() - startTime < ms){
		int rotation = (heading - imu.get()) * 3;
		leftMotor.moveVelocity(vel - rotation);
		rightMotor.moveVelocity(vel + rotation);
		backMotor.moveVelocity(vel);
		pros::delay(5);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void turnViaIMU(double heading)
{
	double error = heading - imu.get();
	int rotation;
	backMotor.moveVelocity(0);
	while(std::fabs(error) > 10) // keeps turning until within 10 degrees of objective
	{
		if (std::fabs(error) < 40){
		// if within 40 degrees of objective, the motors start slowing
		// and the speed never drops below 20
		rotation = (6 * error);
		} else {
		// otherwise maintain fast turning speed of 90
		rotation = 270 * sgn(error);
		}

		rightMotor.moveVelocity(rotation);
		leftMotor.moveVelocity(-rotation);

		pros::delay(5);
		error = heading - imu.get();
		std::string imuMeasurement = std::to_string(imu.get());
		master.setText(0, 0, "IMU:" + imuMeasurement);
	}
	// these next lines attempt to slow down the robot's rotational momentum
	// might be better just to put the motors into braking mode
	rotation = -15 * sgn(error);
	leftMotor.moveVelocity(rotation);
	rightMotor.moveVelocity(-rotation);
	pros::delay(50);
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
}

void grab() // NOTE: Grip should be in holding, allowing it to grip via this simple piece of code
{
	grip.moveAbsolute(-3.5, 100);
	pros::delay(750);
}

void ungrab() // TODO: Write this code
{
	grip.moveAbsolute(-2, 80);
}

void liftMin() {lift.moveAbsolute(0, 90);}
void liftSmall() {lift.moveAbsolute(.2, 90);} // 0, .5, 1.7
void liftMax() {lift.moveAbsolute(1.7, 90);}
void liftScore() {lift.moveAbsolute(.5, 90);}
void liftHang() {lift.moveAbsolute(.7, 90);} // needs tweaking

void scoreGoal()
{
	//Ensure lift is up
	liftMax();
	// Release Goal
	ungrab();
	// Lift the lift back to max
	liftMax();
}

void judas()
{
	liftHang();
	while(lift.getPosition() > .7){
		pros::delay(10);
	}
	// TODO: Add a waitUntil lift is done here because otherwise, async
	piston.set_value(true);
}
// Uncommented is for 11/20 tournament and scores 110 points
void skillsAuton()
{
	//////////////////////
	// Grab nearby goal //
	//////////////////////

	// Drive to the goal
	driveViaIMU(.1, 0);

	// Grab the goal
	grab();

	// Lift the goal a little
	driveViaIMU(-.4, 0);
	ungrab();
	driveViaIMU(.1, 0);
	grab();
	lift.moveAbsolute(.2, 100); //make it not scrape the ground so it can move

	/////////////////////////////
	// Drive to the other side //
	/////////////////////////////
	// Turn towards gap between yellow
	turnViaIMU(-45);

	// Drive to the gap
	driveViaIMU(.8, -45); // Previously 1

	// Turn to perpendiculars
	turnViaIMU(-90);

	// Drive to other side of goal
	driveViaIMU(1.2, -90);
	// Turn to ramp
	turnViaIMU(-60);

	///////////////////////////////
	// Swipe goal out of the way (De-tilt the ramp) //
	///////////////////////////////

	// Hit the ramp
	driveViaTime(2000, 500);
/*
	// De-tilt and back up to center on the ramp
	lift.moveRelative(.5, 100);
	driveViaDist(-.5);
	turnViaIMU(-90);

	///////////////////////////////////
	// Score goal 1 in bridge center //
	///////////////////////////////////

	// Lift goal then approach
	liftMax();
	driveViaTime(500, 100);

	// Score
	scoreGoal();
*/
/* This section is the 11/20 auton
	//////////////////////////////
	// Get tall yellow and judas //
	//////////////////////////////

	driveViaIMU(-.5, -90);
	turnViaIMU(180);
	liftMin();
	driveViaIMU(.5, 90);
	pros::delay(10);
	grab();
	liftMax();
	pros::delay(10);
	leftMotor.moveVelocity(100); // So that it runs up against the ramp and enables the ability to judas
	rightMotor.moveVelocity(100);
	backMotor.moveVelocity(100);
	pros::delay(1000);
	judas();
	leftMotor.moveVelocity(0); // Don't kill the motors
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
	*/
	
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