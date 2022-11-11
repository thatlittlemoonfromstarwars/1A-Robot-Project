/*
Domino layer path follower
Sean Aitken, Henrique Engelke, Josh Morcombe, and Andor Siegers

v1.3

Assumptions:


Motor Ports:
A - left drive wheel
B - dispenser motor
C - gate motor
D - right drive wheel

Sensor Ports:
1 - color
2 - gyro
3 - touch
4 - ultrasonic

*/

#include "PC_FileIO.c";
void configureAllSensors();

// movement functions
void dropDomino(int &dropIndex, int &dominoCount); // Henrique
void openDoor();
void closeDoor();

void driveDist(int mot_pow, float dist);
void driveDistWhileDispensing(float dist, int &dropIndex, int &dominoCount);
void setDriveTrainSpeed(int speed);
void turnInPlace(int angle, int mot_pow); // NEEDS TO BE WRITTEN

// calculation functions
int distToDeg(float dist);

// constants
const float WHEEL_RAD = 2.75; // in cm
const int DOMINOS_AT_MAX_LOAD = 60;
const float DIST_BETWEEN_DOMINOS = 3.75; // in cm
const int DOOR_SIZE = 90; // degrees
const int DOOR_SPEED = 10;

const int TOUCH_PORT = S3;
const int GYRO_PORT = S2;
const int COLOR_PORT = S1;
const int ULTRASONIC_PORT = S4;

task main()
{
	configureAllSensors();

	// initialization for domino dropping
	nMotorEncoder(motorC)=0;
	nMotorEncoder(motorB)=0;
	int dropIndex = 0;
	int dominoCount = DOMINOS_AT_MAX_LOAD;

	driveDistWhileDispensing(30.0, dropIndex, dominoCount);
}

void configureAllSensors()
{
	SensorType[TOUCH_PORT] = sensorEV3_Touch;
	SensorType[GYRO_PORT] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorType[COLOR_PORT] = sensorEV3_Color;
	wait1Msec(50);
	SensorType[ULTRASONIC_PORT] = sensorEV3_Ultrasonic;
	wait1Msec(50);
	SensorMode[GYRO_PORT] = modeEV3Gyro_Calibration;
	wait1Msec(50);
	SensorMode[COLOR_PORT] = modeEV3Color_Color;
	wait1Msec(100);
	SensorMode[GYRO_PORT] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);
}

int distToDeg(float dist)
{
	return dist*180/PI/WHEEL_RAD;
}

void driveDist(int mot_pow, float dist) // input negative motor power for backwards
{
	setDriveTrainSpeed(mot_pow);
	nMotorEncoder[motorA] = 0;
	while(abs(nMotorEncoder[motorA]) < distToDeg(dist))
	{}
	setDriveTrainSpeed(0);
}

void driveDistWhileDispensing(float dist, int &dropIndex,int &dominoCount)
{
	nMotorEncoder[motorD] = 0;
	while(abs(nMotorEncoder[motorD]) < distToDeg(dist))
	{
		dropDomino(dropIndex, dominoCount);
	}
}

void turnInPlace(int angle, int mot_pow)
{
	int initialGyro = getGyroDegrees(GYRO_PORT);
	if(angle < 0)
	{
		motor[motorA] = mot_pow;
		motor[motorB] = -1*mot_pow;
		while(getGyroDegrees(GYRO_PORT) > initialGyro-angle)
		{}
	}
	else if(angle > 0)
	{
		motor[motorA] = -1*mot_pow;
		motor[motorB] = mot_pow;
		while(getGyroDegrees(GYRO_PORT) < initialGyro+angle)
		{}
	}

	setDriveTrainSpeed(0);
}

void setDriveTrainSpeed(int speed)
{
	motor[motorA] = motor[motorD] = -1*speed;
}

// Henrique's functions
void openDoor()
{

	motor[motorB] = DOOR_SPEED;
	while (nMotorEncoder(motorB)<DOOR_SIZE)
	{}
	motor[motorB] = 0;

	return;
}

void closeDoor()
{
	motor[motorB] = -1*DOOR_SPEED;
	while (nMotorEncoder(motorB)>5)
	{}
	motor[motorB] = 0;

	return;
}

void dropDomino(int &dropIndex, int &dominoCount)
{
	if (dropIndex == 0)
	{
		motor[motorC] = -15;
		while (nMotorEncoder(motorC) > -325)
		{}
		motor[motorC] = 0;
		dropIndex += 1;
	}
	else if (dropIndex == 1)
	{
		motor[motorC] = -15;
		while (nMotorEncoder(motorC) > -550)
		{}
		motor[motorC]= 0;

		dropIndex = 0;
		wait1Msec(100);

		motor[motorC] = 15;
		while (nMotorEncoder(motorC) < 100)
		{}
		motor[motorC] = 0;
	}
	wait1Msec(700);
	openDoor();
	dominoCount--;
	driveDist(15, DIST_BETWEEN_DOMINOS);
	closeDoor();
}
