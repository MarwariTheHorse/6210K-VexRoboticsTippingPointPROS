#include <vector>
#include <iostream>
#include <string>
#include <tuple>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <limits>
#include "main.h"
#include "api.h"

const bool TRAIN = true;
// Wheel Definitions
okapi::Motor mWheelBackLeft(2);
okapi::Motor mWheelFrontLeft(14);
okapi::Motor mWheelFrontRight(9);
okapi::Motor mWheelBackRight(8);

// Controller and camera definitions + green is defined
// pros::Vision sCamera(4, pros::E_VISION_ZERO_CENTER);
// pros::vision_signature colorCode = sCamera.signature_from_utility(1, -4915, -4609, -4762, -5121, -4807, -4964, 11.000, 0);
pros::Imu sImu(5);
pros::Gps sGps(7);
okapi::Controller master(okapi::ControllerId::master);

using namespace std;
int sgn(double d) // Mimimcs the mathematical sgn function
{
	if(d < 0){return -1;}
	if(d > 0){return 1;}
	return 0;
}
std::fstream& GotoLine(std::fstream& file, unsigned int num){
    file.seekg(std::ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    return file;
}
void setDTSpeeds()
{
	// Store joysticks range = [-1, 1]
	double joyLY = master.getAnalog(okapi::ControllerAnalog::leftY);
	double joyRX = master.getAnalog(okapi::ControllerAnalog::rightX);
    double joyLX = master.getAnalog(okapi::ControllerAnalog::leftX);

	// Filter joysticks
	if(abs(joyLY) < .1){
		joyLY = 0;
	}

	if(abs(joyRX) < .1){
		joyRX = 0;
	}

	// Convert joysticks to wheel speeds
	double wheelFrontLeftSpeed = (joyLY + joyRX + joyLX) * .9;
	double wheelFrontRightSpeed = (joyRX - joyLY + joyLX) * .9;
	double wheelBackLeftSpeed = (joyRX + joyLY - joyLX) * .9;
    double wheelBackRightSpeed = (joyRX - joyLY - joyLX) * .9;

	// Wheel speed assignments
	mWheelBackLeft.moveVelocity(wheelFrontLeftSpeed * 200); // Speed is velocity pct * gearbox
	mWheelFrontRight.moveVelocity(wheelFrontRightSpeed * 200);
	mWheelBackRight.moveVelocity(wheelBackRightSpeed * 200);
    mWheelBackLeft.moveVelocity(wheelBackLeftSpeed * 200);
}

void copycat(){
    if(TRAIN){
        double joyLY = master.getAnalog(okapi::ControllerAnalog::leftY);
        double joyRX = master.getAnalog(okapi::ControllerAnalog::rightX);
        double joyLX = master.getAnalog(okapi::ControllerAnalog::leftX);
        // Filter joysticks
        if(abs(joyLY) < .1){
            joyLY = 0;
        }
        if(abs(joyRX) < .1){
            joyRX = 0;
        }
        double wheelFrontLeftSpeed = (joyLY + joyRX + joyLX) * .9;
        double wheelBackRightSpeed = (joyRX - joyLY - joyLX) * .9;
        int heading; //IMU
        int position; // Average of gps and motors
        int speed; // Tells copycat how fast we are going
        ofstream dataFile;
        auto start = chrono::high_resolution_clock::now(); //timestamp from movement
        // if pure copying doesn't work, time needs to be used to make a NN
        vector<tuple<int, tuple<int, int>, int>> data; // first heading, second position, third speed
        while(true){
            auto stop = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::seconds>(stop - start);
            int timestamp = duration.count();
            if(timestamp =.5){
                heading = sImu.get_heading();
                auto status = sGps.get_status();
                vector<tuple<int, int>> position;
                position.emplace_back(status.x, status.y);
                if(wheelBackRightSpeed < 5){
                    speed = abs(wheelFrontLeftSpeed);
                }else{
                    speed = (abs(wheelBackRightSpeed) + abs(wheelFrontLeftSpeed))/2;
                }
                data.emplace_back(heading, position, timestamp);
                dataFile.open("/usd/copycatData.txt");
                dataFile << heading << "\n"<< endl;
                dataFile << speed << "\n" << endl;
			    dataFile << status.x << "\n" << endl;
                dataFile << status.y << "\n" << endl;
			    dataFile.close();
            }
        }
    }else{
        fstream dataFile;
        dataFile.open("/usd/copycatData.txt", ios::in);
        if (!dataFile){
            cerr << "Unable to open file";
            exit(1);
        }
        string line;
        int i = 0;
        double angle;
        while(getline(dataFile, line)){
            if(i % 4 == 0){
                double angle = stod(line);
                double error = angle - sImu.get_rotation();
                int leftY = 0;
                int rightX = 0;
                int leftX = 0;
                while(fabs(error) > 10) // keeps turning until within 10 degrees of objective
                {
                    if (fabs(error) < 40){
                    // if within 40 degrees of objective, the motors start slowing
                    // and the speed never drops below 20
                    rightX = (2 * error);
                    } else {
                    // otherwise maintain fast turning speed of 90
                    rightX = 90 * sgn(error);
                    }
                    mWheelFrontLeft.moveVelocity((rightX + leftY + leftX) * .9 * 200);
                    mWheelFrontRight.moveVelocity((rightX - leftY + leftX) * .9 * 200);
                    mWheelBackLeft.moveVelocity((rightX + leftY - leftX) * .9 * 200);
                    mWheelBackRight.moveVelocity((rightX - leftY - leftX) * .9 * 200);
                    pros::delay(5);
                    error = angle - sImu.get_rotation();
                }
                // these next lines attempt to slow down the robot's rotational momentum
                // might be better just to put the motors into braking mode
                rightX = -5 * sgn(error);
                mWheelFrontLeft.moveVelocity((rightX + leftY + leftX) * .9 * 200);
                mWheelFrontRight.moveVelocity((rightX - leftY + leftX) * .9 * 200);
                mWheelBackLeft.moveVelocity((rightX + leftY - leftX) * .9 * 200);
                mWheelBackRight.moveVelocity((rightX - leftY - leftX) * .9 * 200);
                pros::delay(50);
                i++;
            }else if(i % 4 == 1){
                double velocity = stod(line);
                i++;
            }else if(i % 4 == 2){
                //Whole thing needs tuning
                int next = i + 1;
                int previous = i - 1;
                double locx = stod(line);
                GotoLine(dataFile, next);
                string y_line;
                dataFile >> y_line;
                double locy = stod(y_line);
                GotoLine(dataFile, previous);
                string speed_line;
                dataFile >> speed_line;
                double velocity = stod(speed_line);
                okapi::EKFFilter kFilterRot;
                okapi::EKFFilter kFilterDist;

                // Variables
                double targetRotation; // Rotation to hold throughout the movement
                auto status = sGps.get_status();
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

                std::stringstream ss;

                // Until we reach our destination, set the speed
                while(dist > .1){
                    // Update the distance
                    status = sGps.get_status();
                    xDiff = locx - status.x;
                    yDiff = locy - status.y;
                    dist = kFilterDist.filter(std::sqrt(std::pow(xDiff, 2) + std::pow(yDiff, 2)));

                    // Get the rotation from the gps
                    double angularSpeed = (targetRotation - kFilterRot.filter(sImu.get_rotation())) * 30; // NOTE: We can use gyro to maintain heading if gps sucks

                    // Assign the calculated wheel values
                    mWheelFrontLeft.moveVelocity((velocity - angularSpeed) * 200);
                    mWheelFrontRight.moveVelocity((velocity + angularSpeed) * 200);
                    mWheelBackRight.moveVelocity((velocity + angularSpeed) * 200);
                    mWheelBackLeft.moveVelocity((velocity - angularSpeed) * 200);
                    std::string str;
                    ss << targetRotation;
                    ss >> str;
                    master.setText(0, 0, str);

                    pros::delay(5); // Delay for the other tasks
                }

                // Stop the robot after the movement is near the target location
                mWheelFrontLeft.moveVelocity(0);
                mWheelFrontRight.moveVelocity(0);
                mWheelBackRight.moveVelocity(0);
                mWheelBackLeft.moveVelocity(0);
                i++;
                i++;
            }
        }
    } 

}
void initialize() {
     	pros::lcd::initialize();

	// Calibrate IMU
	master.setText(0, 0, "Calibrating...");
	sImu.reset();
	while (sImu.is_calibrating()){pros::delay(10);}
	master.clear();
}

void opcontrol() {
	setDTSpeeds();
    pros::delay(10);
    pros::lcd::register_btn0_cb(copycat);
    pros::lcd::register_btn1_cb(copycat);
    pros::lcd::register_btn2_cb(copycat);
}