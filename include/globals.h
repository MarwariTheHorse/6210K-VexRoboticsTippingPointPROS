#include "main.h"
#define SPEED 200

// Motor + pneumatic port definitions
#define WHEEL_LEFT_F 14
#define WHEEL_LEFT_R 13

#define WHEEL_RIGHT_F 4
#define WHEEL_RIGHT_R 11

#define WHEEL_BACK_L 8
#define WHEEL_BACK_R 20

#define LIFT 19
#define HOOK 6
#define GRIP 'H'
#define HOOK_STOP 'G'

#define GYRO 7

#define GOAL_VISION 12
#define RAMP_VISON 5
#define GOAL_DETECT 'A'

#define GPS_PORT 9
#define GPS_OFFSET_X 0
#define GPS_OFFSET_Y 0

#define RED true
#define BLUE false

// Motors(port, reversed, gearset, encoderUnits, logger(implied))
extern okapi::Motor fLeftMotor;
extern okapi::Motor rLeftMotor;
extern okapi::Motor fRightMotor;
extern okapi::Motor rRightMotor;
extern okapi::Motor lBackMotor;
extern okapi::Motor rBackMotor;
extern okapi::Motor lift;
extern okapi::Motor hook;

// Motor Groups (For making the code simpler)
extern okapi::MotorGroup rightMotor;
extern okapi::MotorGroup leftMotor;
extern okapi::MotorGroup backMotor;

// Controllers
extern okapi::Controller master;

// Sensors
extern pros::Vision rampVision;
extern pros::Vision goalVision;
extern pros::Imu imu;
extern pros::GPS gps;
extern pros::ADIDigitalIn hookStop;
extern pros::ADIAnalogIn goalDetect;
extern pros::ADIUltrasonic echo;

// Pneumatics
extern pros::ADIDigitalOut grip;