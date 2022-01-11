#include "main.h"

// Motor + pneumatic port definitions
#define WHEEL_LEFT_F 14
#define WHEEL_LEFT_R 13

#define WHEEL_RIGHT_F 12
#define WHEEL_RIGHT_R 11

#define WHEEL_BACK_L 8
#define WHEEL_BACK_R 20

#define LIFT 19
#define HOOK 6
#define GRIP 'H'

#define VISION 6
#define GYRO 7

#define GPS_PORT 1
#define GPS_OFFSET_X 1
#define GPS_OFFSET_Y -1

okapi::Motor fLeftMotor(WHEEL_LEFT_F, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rLeftMotor(WHEEL_LEFT_R, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor fRightMotor(WHEEL_RIGHT_F, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rRightMotor(WHEEL_RIGHT_R, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor lBackMotor(WHEEL_BACK_L, true, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rBackMotor(WHEEL_BACK_R, false, okapi::AbstractMotor::gearset::blue, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor lift(LIFT, true, okapi::AbstractMotor::gearset::red, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor hook(HOOK, true, okapi::AbstractMotor::gearset::red, okapi::AbstractMotor::encoderUnits::rotations);

// Motor Groups (For making the code simpler)
okapi::MotorGroup rightMotor({fLeftMotor, rLeftMotor});
okapi::MotorGroup leftMotor({fRightMotor, rRightMotor});
okapi::MotorGroup backMotor({lBackMotor, rBackMotor});

// Controllers
okapi::Controller master(okapi::ControllerId::master);

// Sensors
pros::Vision vision (VISION);
okapi::IMU imu(GYRO);
pros::GPS gps(GPS_PORT, GPS_OFFSET_X, GPS_OFFSET_Y);

// Pneumatics
pros::ADIDigitalOut grip(GRIP);