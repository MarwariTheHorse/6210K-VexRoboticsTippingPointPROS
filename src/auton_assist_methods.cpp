#include "main.h"
#include "globals.h"
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