#include "main.h"
#include "globals.h"

// Returns -1, 0, and 1 based on number direction
int sgn(double d) // Mimimcs the mathematical sgn function
{
	if(d < 0){return -1;}
	if(d > 0){return 1;}
	return 0;
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

// Returns a filtered x value from the gps
double averageGPSX(double ms){
	double startTime = pros::millis();
	double total;
	double count;
	while(pros::millis() - startTime < ms){
		total += gps.get_status().x;
		count++;
		pros::delay(5);
	}
	return total / count;
}

// Returns a filtered y value from the gps
double averageGPSY(double ms){
	double startTime = pros::millis();
	double total;
	double count;
	while(pros::millis() - startTime < ms){
		total += gps.get_status().y;
		count++;
		pros::delay(5);
	}
	return total / count;
}

// Returns a filtered rotation value from the gps
double averageGPSR(double ms){
	double startTime = pros::millis();
	double total;
	double count;
	while(pros::millis() - startTime < ms){
		total += gps.get_rotation();
		count++;
		pros::delay(5);
	}
	return total / count;
}

// Drive guided by IMU
void driveViaIMU(double dist, double rotation)
{
	dist *= 39.3701 / (2.75 * PI); // To in. then to rev

	// reset all motor encoders to zero
	backMotor.tarePosition();
	leftMotor.tarePosition();
	rightMotor.tarePosition();

	double d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;

	if(sgn(dist) > 0){
		while(std::fabs(dist - d) > .1){
			// Calculate base wheel speed
			d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
			double anglePCT = (imu.get_rotation() - rotation) * 4.5; // 4.5
			leftMotor.moveVelocity(SPEED + anglePCT); // Originally this + and the - below were swapped
			rightMotor.moveVelocity(SPEED - anglePCT);
			backMotor.moveVelocity(SPEED);
			pros::delay(5);
		}
	}else{
		while (std::fabs(dist - d) > .1){
			// Calculate base wheel speed
			d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
			double anglePCT = (imu.get_rotation() - rotation) * 4.5; // 4.5
			leftMotor.moveVelocity(-SPEED + anglePCT);
			rightMotor.moveVelocity(-SPEED - anglePCT);
			backMotor.moveVelocity(-SPEED);
			pros::delay(5);
		}
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void driveViaTime(double ms, double vel){
	leftMotor.moveVelocity(vel);
	rightMotor.moveVelocity(vel);
	backMotor.moveVelocity(vel);

	pros::delay(ms);

	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void driveToRamp(double time, double heading, bool isRedRamp){
	double startTime = pros::millis();
	while (pros::millis() - startTime < time){

		// Get the average of the two largest colors
		double rampCenter;
		if(isRedRamp){
			// Red
			rampCenter = (rampVision.get_by_sig(0, 1).x_middle_coord + rampVision.get_by_sig(1, 1).x_middle_coord) / 2;
		}else{
			// Blue
			rampCenter = (rampVision.get_by_sig(0, 2).x_middle_coord + rampVision.get_by_sig(1, 2).x_middle_coord) / 2;
		}

		// Set speed and aSpeed
		double aSpeed = ((heading - imu.get_rotation()) * 0.8) + ((rampCenter) * .13);

		// Go
		leftMotor.moveVelocity(SPEED + 3 * aSpeed);
		rightMotor.moveVelocity(SPEED - 3 * aSpeed);
		backMotor.moveVelocity(SPEED);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

// Drive guided by a single color signature
void driveViaSig(int sig){

	// reset all motor encoders to zero
	backMotor.tarePosition();
	leftMotor.tarePosition();
	rightMotor.tarePosition();

	double filteredGoalDetect = 3000;
	while (filteredGoalDetect > 2900){
		// Calculate base wheel speed
		filteredGoalDetect = filteredGoalDetect * .9 + goalDetect.get_value() * .1;
		double anglePCT = (goalVision.get_by_sig(0, sig).x_middle_coord * 15) / 100;

		leftMotor.moveVelocity(SPEED - 4.5 * anglePCT);
		rightMotor.moveVelocity(SPEED + 4.5 * anglePCT);
		backMotor.moveVelocity(SPEED);
		pros::delay(5);
	}
	grab();

	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void turnViaIMU(double heading)
{
	double error = heading - imu.get_rotation();
	int rotation;
	backMotor.moveVelocity(0);
	while(std::fabs(error) > 1) // keeps turning until within 10 degrees of objective
	{
		if (std::fabs(error) < 40){
		// if within 40 degrees of objective, the motors start slowing
		// and the speed never drops below 20
		rotation = (6 * error);
		} else {
		// otherwise maintain fast turning speed of 90
		rotation = 200 * sgn(error);
		}

		rightMotor.moveVelocity(rotation);
		leftMotor.moveVelocity(-rotation);

		pros::delay(5);
		error = heading - imu.get_rotation();
	}
	// these next lines attempt to slow down the robot's rotational momentum
	// might be better just to put the motors into braking mode
	rotation = -30 * sgn(error);
	leftMotor.moveVelocity(rotation);
	rightMotor.moveVelocity(-rotation);
	pros::delay(50);
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
}

// Auton lift methods
void liftMin() {lift.moveAbsolute(.189, 90);}
void liftSmall() {lift.moveAbsolute(.5, 90);} // 0, .5, 1.7
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
	hook.moveAbsolute(6.5, 100);
}	
