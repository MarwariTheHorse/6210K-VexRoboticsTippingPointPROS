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

	// reset all motor encoders to zero
	backMotor.tarePosition();
	leftMotor.tarePosition();
	rightMotor.tarePosition();

	double d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
	while (std::fabs(dist - d) > .3){
		// Calculate base wheel speed
		d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
		double anglePCT = (imu.get() * .5) / 100;

		leftMotor.moveVelocity(sgn(dist) * 600 * (1 - sgn(dist) * anglePCT));
		rightMotor.moveVelocity(sgn(dist) * 600 * (1 + sgn(dist) * anglePCT));
		backMotor.moveVelocity(sgn(dist) * 600);
		pros::delay(5);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void driveViaTime(double ms, double vel, double rotation){
	double startTime = pros::millis();
	okapi::EKFFilter kFilter;
	while (pros::millis() - startTime < ms){
		int aSpeed = (rotation - kFilter.filter(imu.get())) * 20;
		leftMotor.moveVelocity(vel - aSpeed);
		rightMotor.moveVelocity(vel + aSpeed);
		backMotor.moveVelocity(vel);
		pros::delay(5);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void driveToRamp(double time, bool isRedRamp){
	double startTime = pros::millis();
	while (pros::millis() - startTime){
		// Get the two largest colors
		pros::vision_object_s_t stuff [2];
		if(isRedRamp){
			// Red
			stuff[0] = rampVision.get_by_sig(0, 1); // First index
			stuff[1] = rampVision.get_by_sig(1, 1) // Second index
		}else{
			// Blue
			stuff[0] = rampVision.get_by_sig(0, 2); // First index
			stuff[1] = rampVision.get_by_sig(1, 2) // Second index
		}

		// Get the location between the colors
		double rampCenter = (stuff[0] + stuff[1]) / 2;

		// Set speed and aSpeed
		double speed = 600;
		double aSpeed = imu.get() * 3;

		// Go
		leftMotor.moveVelocity(speed - aSpeed);
		rightMotor.moveVelocity(speed + aSpeed);
		backMotor.moveVelocity(speed);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void driveViaSig(double dist, int sig){
	dist *= 39.3701 / (2.75 * PI); // To in. then to rev

	// reset all motor encoders to zero
	backMotor.tarePosition();
	leftMotor.tarePosition();
	rightMotor.tarePosition();

	double d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;

	while (std::fabs(dist - d) > .3){
		// Calculate base wheel speed
		d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
		double anglePCT = (goalVision.get_by_sig(0, sig).x_middle_coord * .5) / 100;

		leftMotor.moveVelocity(sgn(dist) * 600 * (1 - sgn(dist) * anglePCT));
		rightMotor.moveVelocity(sgn(dist) * 600 * (1 + sgn(dist) * anglePCT));
		backMotor.moveVelocity(sgn(dist) * 600);
		pros::delay(5);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void turnViaIMU(double heading)
{
	auto turnController = okapi::IterativeControllerFactory::posPID(.5, .1, .075); // PID for angular speed int aSpeed; int speed; okapi::EKFFilter kFilter;
	turnController.setTarget(heading);
	okapi::EKFFilter turnFilter;

	while (std::fabs(heading - turnFilter.filter(imu.get())) > .5){
		double aSpeed = turnController.step(turnFilter.getOutput());
		leftMotor.moveVelocity(-600 * aSpeed);
		rightMotor.moveVelocity(600 * aSpeed);
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
void liftMax() {lift.moveAbsolute(2, 90);}
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

void judas()
{
	hook.moveAbsolute(7, 100);
}	
