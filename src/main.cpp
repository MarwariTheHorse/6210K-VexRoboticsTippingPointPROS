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

#define GPS 0
#define GPS_OFFSET_X 0
#define GPS_OFFSET_Y 0

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
pros::Gps gps(GPS, GPS_OFFSET_X, GPS_OFFSET_Y);

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

// TODOS:
// Improve the movement methods so that they are run by controllers, along with "SettledUntil" - Caleb
// Complete the auton - Joey
// Sensor fusion - Caleb
// Fancy brain interface w/KryptoKnAIghts logo

// Globals
char autonMode = 'N'; // Stands for none

int sgn(double d) // Mimimcs the mathematical sgn function
{
	if(d < 0){return -1;}
	if(d > 0){return 1;}
	return 0;
}

// Auton assist methods //

void driveViaIMU(double dist, double rotation)
{
	dist *= 39.3701 / (2.75 * PI); // To in. then to rev
	int aSpeed;
	int speed;
	okapi::EKFFilter kFilter;
	// reset all motor encoders to zero
	// 10000 units is equal to 56" of travel
	backMotor.tarePosition();
	leftMotor.tarePosition();
	rightMotor.tarePosition();
	int d = 0;
	if(d < dist){
		while (d < dist){
			speed = 500;
			aSpeed = (rotation - kFilter.filter(imu.get())) * 3;
			leftMotor.moveVelocity(speed - aSpeed);
			rightMotor.moveVelocity(speed + aSpeed);
			backMotor.moveVelocity(speed);
			pros::delay(5);
			d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
		}
	}else{
		while (d > dist){
			speed = -500;
			aSpeed = (rotation - kFilter.filter(imu.get())) * 3;
			leftMotor.moveVelocity(speed - aSpeed);
			rightMotor.moveVelocity(speed + aSpeed);
			pros::delay(5);
			d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
		}
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void driveViaTime(double ms, double vel, double rotation){
	double startTime = pros::millis();
	okapi::EKFFilter kFilter;
	while (pros::millis() - startTime < ms){
		int aSpeed = (rotation - kFilter.filter(imu.get())) * 3;
		leftMotor.moveVelocity(vel - aSpeed);
		rightMotor.moveVelocity(vel + aSpeed);
		backMotor.moveVelocity(vel);
		pros::delay(5);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

// UNTESTED
void driveViaGPS(double locx, double locy)
{
	// Data smoothing filters
	okapi::EKFFilter kFilterRot;
	okapi::EKFFilter kFilterDist;

	// Variables for the function
	double targetRotation;
	double xDiff = locx - gps.get_status().x;
	double yDiff = locy - gps.get_status().y;
	double dist = std::sqrt(std::pow(xDiff, 2) + std::pow(yDiff, 2));

	// Properly calculate the desired heading using inverse tangent (aka arctan())
	if (yDiff == 0 && sgn(xDiff) == 1) targetRotation = 0;
	else if (yDiff == 0 && sgn(xDiff) == -1) targetRotation = 180;
	else targetRotation = std::atan(yDiff/xDiff);
	if (sgn(yDiff) == -1) targetRotation += 180;

	// Create a PID distance controller
	auto distController = okapi::IterativeControllerFactory::posPID(.001, .0001, .0001);
	distController.setTarget(0);

	// Until we reach our destination, set the speed
	while(dist > .1){
		// Update the distance
		xDiff = locx - gps.get_status().x;
		yDiff = locy - gps.get_status().y;
		dist = kFilterDist.filter(std::sqrt(std::pow(xDiff, 2) + std::pow(yDiff, 2)));

		// Calculate our speed, which is a constant for now
		double speed = distController.step(dist);

		// Get the rotation from the gps
		double angularSpeed = (targetRotation - kFilterRot.filter(gps.get_rotation())) * 3; // NOTE: We can use gyro to maintain heading if gps sucks

		// Assign the calculated wheel values
		leftMotor.moveVelocity(speed - angularSpeed);
		rightMotor.moveVelocity(speed + angularSpeed);
		backMotor.moveVelocity(speed);

		pros::delay(5); // Delay for the other tasks
	}

	// Stop the robot after the movement is near the target location
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void turnViaIMU(double rotation)
{
	// Initialize things
	okapi::EKFFilter kFilter; // Kalman filter for IMU
	auto turnController = okapi::IterativeControllerFactory::posPID(.003, .0004, .0001); // PID for angular speed
	turnController.setTarget(rotation); // Prepare for the upcoming maneuver

	backMotor.moveVelocity(0); // There is not reason for the back motor to move when turning
	while(std::abs(rotation - kFilter.filter(imu.get())) > 3){ // We accept make range of 6 deg of error
		// Controller should give values based off of the previously filtered angle
		double controllerInput = kFilter.getOutput();
		// The resulting controller value will be used for turning speed
		double output = turnController.step(controllerInput);
		leftMotor.controllerSet(-output);
		rightMotor.controllerSet(output);
		pros::delay(5);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);

}

void grab()
{
	grip.moveAbsolute(-3.5, 100);
	pros::delay(750);
}

void ungrab() // NOTE: This has no wait, unlike the function above
{
	grip.moveAbsolute(-2, 80);
}

void liftMin() {lift.moveAbsolute(0, 90);}
void liftSmall() {lift.moveAbsolute(.2, 90);} // 0, .5, 1.7
void liftMax() {lift.moveAbsolute(1.85, 90);}
void liftScore() {lift.moveAbsolute(.5, 90);}
void liftHang() {lift.moveAbsolute(.9, 90);} // theoretically overshoots by .2

void scoreGoal()
{
	//Ensure lift is up
	liftMax();
	// Release Goal
	ungrab();
	// Lift the lift back to max
	liftMax();
}

// TODO: Pretty sure this is still UNTESTED / not ready
void judas()
{
	liftHang();
	while(lift.getPosition() > 1.4){
		pros::delay(10);
	}
	piston.set_value(true);
	while(lift.getPosition() > 1) pros::delay(10);
}

void skillsAuton()
{
	// Configure the GPS for skills
	gps.set_position(1.524, -1.0668, 90); // x, y, rot

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
	driveViaIMU(-1.8, 0);
}

void compRightAuton()
{
	// The line below is a filler line right now, but actually works quite nicely
	compLeftAuton();
}

void experimental()
{
	turnViaIMU(180);
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
		pros::delay(5);
	}
}