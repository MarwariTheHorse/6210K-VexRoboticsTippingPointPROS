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

	// TODO: Add PID
	if(d < dist){
		while (d < dist){
			speed = 500;
			aSpeed = (rotation - kFilter.filter(imu.get())) * 15; // Was at 30
			leftMotor.moveVelocity(speed - aSpeed);
			rightMotor.moveVelocity(speed + aSpeed);
			backMotor.moveVelocity(speed);
			pros::delay(5);
			d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
		}
	}else{
		while (d > dist){
			speed = -500;
			aSpeed = (rotation - kFilter.filter(imu.get())) * 15; // Was at 30
			leftMotor.moveVelocity(speed - aSpeed);
			rightMotor.moveVelocity(speed + aSpeed);
			backMotor.moveVelocity(speed);
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
		int aSpeed = (rotation - kFilter.filter(imu.get())) * 30;
		leftMotor.moveVelocity(vel - aSpeed);
		rightMotor.moveVelocity(vel + aSpeed);
		backMotor.moveVelocity(vel);
		pros::delay(5);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

// NEEDS TUNING
void driveViaGPS(double locx, double locy)
{
	// Data smoothing filters
	okapi::EKFFilter kFilterRot;
	okapi::EKFFilter kFilterDist;

	// Variables
	double targetRotation; // Rotation to hold throughout the movement
	auto status = gps.get_status();
	double xDiff = locx - status.x;
	double yDiff = locy - status.y;
	double dist = std::sqrt(std::pow(xDiff, 2) + std::pow(yDiff, 2));

	// Update the target angle
	if (xDiff == 0) targetRotation = 90;
	else targetRotation = std::atan(yDiff/xDiff) * 180 / PI;
	
	targetRotation = -(targetRotation + 90);
	if (sgn(xDiff) == -1) targetRotation += 180;
	targetRotation *= -1;

	// Create a PID distance controller
	// auto distController = okapi::IterativeControllerFactory::posPID(.001, .0001, .0001);
	// distController.setTarget(0);

	// Until we reach our destination, set the speed
	while(dist > .1){
		// Update the distance
		status = gps.get_status();
		xDiff = locx - status.x;
		yDiff = locy - status.y;
		dist = kFilterDist.filter(std::sqrt(std::pow(xDiff, 2) + std::pow(yDiff, 2)));

		// Calculate our speed, which is a constant for now
		double speed = 500; // distController.step(dist) + 10; // 10 is added for testing's sake

		// Get the rotation from the gps
		double angularSpeed = (targetRotation - kFilterRot.filter(imu.get())) * 30; // NOTE: We can use gyro to maintain heading if gps sucks

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
	grip.set_value(true);
	pros::delay(300);
}

void ungrab() // NOTE: This has no wait, unlike the function above
{
	grip.set_value(false);
}

void liftMin() {lift.moveAbsolute(0, 90);}
void liftSmall() {lift.moveAbsolute(.4, 90);} // 0, .5, 1.7
void liftMax() {lift.moveAbsolute(1.75, 90);}
void liftScore() {lift.moveAbsolute(1, 90);}
void liftHang() {lift.moveAbsolute(1, 90);} // theoretically overshoots by .2

void scoreGoal()
{
	//Ensure lift is up
	liftScore();
	// Release Goal
	ungrab();
	// Lift the lift back to max
	liftMax();
}

// TODO: Pretty sure this is still UNTESTED / not ready
void judas()
{
	hook.moveAbsolute(7, 100);
}	
