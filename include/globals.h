#include "main.h"

// Motors(port, reversed, gearset, encoderUnits, logger(implied))
extern okapi::Motor fLeftMotor;
extern okapi::Motor rLeftMotor;
extern okapi::Motor fRightMotor;
extern okapi::Motor rRightMotor;
extern okapi::Motor lBackMotor;
extern okapi::Motor rBackMotor;
extern okapi::Motor lift;
extern okapi::Motor grip;

// Motor Groups (For making the code simpler)
extern okapi::MotorGroup rightMotor;
extern okapi::MotorGroup leftMotor;
extern okapi::MotorGroup backMotor;

// Pneumatics
extern pros::ADIDigitalOut piston;

// Controllers
extern okapi::Controller master;

// Sensors
extern pros::Vision vision;
extern okapi::IMU imu;
extern pros::Gps gps;