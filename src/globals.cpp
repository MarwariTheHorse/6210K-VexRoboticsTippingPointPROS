#include "main.h"

// Motor + pneumatic port definitions
#define WHEEL_LEFT_F 5
#define WHEEL_LEFT_R 14

#define WHEEL_RIGHT_F 11
#define WHEEL_RIGHT_R 13

#define WHEEL_BACK_L 8
#define WHEEL_BACK_R 20

#define LIFT 19
#define HOOK 6
#define GRIP 'H'
#define HOOK_STOP 'A'

#define GYRO 7

#define GOAL_VISION 12
#define RAMP_VISON 5
#define GOAL_DETECT 'G'

#define GPS_PORT 9
#define GPS_OFFSET_X 0
#define GPS_OFFSET_Y 0

#define RED true
#define BLUE false

okapi::Motor fLeftMotor(WHEEL_LEFT_F, false, okapi::AbstractMotor::gearset::green, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rLeftMotor(WHEEL_LEFT_R, false, okapi::AbstractMotor::gearset::green, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor fRightMotor(WHEEL_RIGHT_F, true, okapi::AbstractMotor::gearset::green, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rRightMotor(WHEEL_RIGHT_R, true, okapi::AbstractMotor::gearset::green, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor lBackMotor(WHEEL_BACK_L, true, okapi::AbstractMotor::gearset::green, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor rBackMotor(WHEEL_BACK_R, false, okapi::AbstractMotor::gearset::green, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor lift(LIFT, true, okapi::AbstractMotor::gearset::red, okapi::AbstractMotor::encoderUnits::rotations);
okapi::Motor hook(HOOK, true, okapi::AbstractMotor::gearset::red, okapi::AbstractMotor::encoderUnits::rotations);

// Motor Groups (For making the code simpler)
okapi::MotorGroup rightMotor({fLeftMotor, rLeftMotor});
okapi::MotorGroup leftMotor({fRightMotor, rRightMotor});
okapi::MotorGroup backMotor({lBackMotor, rBackMotor});

// Controllers
okapi::Controller master(okapi::ControllerId::master);

// Sensors
pros::Vision goalVision(GOAL_VISION, pros::E_VISION_ZERO_CENTER);
pros::Vision rampVision(RAMP_VISON, pros::E_VISION_ZERO_CENTER);
pros::Imu imu(GYRO);
pros::GPS gps(GPS_PORT, GPS_OFFSET_X, GPS_OFFSET_Y);
pros::ADIAnalogIn goalDetect(GOAL_DETECT);
pros::ADIUltrasonic echo(5, 6);

// ADI Stop sensor
pros::ADIDigitalIn hookStop(HOOK_STOP);

// Pneumatics
pros::ADIDigitalOut grip(GRIP);