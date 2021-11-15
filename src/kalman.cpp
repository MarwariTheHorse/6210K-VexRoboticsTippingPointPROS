#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
#include "main.h"
#include "api.h"

// Wheel Definitions
okapi::Motor mWheelBackLeft(2);
okapi::Motor mWheelFrontLeft(14);
okapi::Motor mWheelFrontRight(9);
okapi::Motor mWheelBackRight(8);

// Controlle and camera definitions + green is defined
pros::Vision sCamera(4, pros::E_VISION_ZERO_CENTER);
pros::vision_signature colorCode = sCamera.signature_from_utility(1, -4915, -4609, -4762, -5121, -4807, -4964, 11.000, 0);
pros::Imu sImu(5);
pros::Gps sGps(7);
void initialize() {
     
}

