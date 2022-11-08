/*
Domino layer path follower
Sean Aitken, Henrique Engelke, Josh Morcombe, and Andor Siegers

v1.1

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

void configureAllSensors();
void selectMode(bool &mode);

// movement functions
void dropDomino(int &dropIndex, int &dominoCount); // Henrique
void openDoor();
void closeDoor();

void stopAndKnock(int motor_power, int enc_limit); // Josh
void somethingInTheWay(); // stops and informs the user to move the object in the way

void driveDist(int mot_pow, float dist);
void driveDistWhileDispensing(int mot_pow, float dist, int &dropIndex, int &dominoCount);
void setDriveTrainSpeed(int speed);

void followLine(); // Sean

void followPathFromFile(); // Andor
void driveToStartLocation(); // Andor

// calculation functions
int distToDeg(float dist);
float degToDist(int deg);

// constants
const float WHEEL_RAD = 2.75; // in cm
const int DOMINOS_AT_MAX_LOAD = 60;
const float DIST_BETWEEN_DOMINOS = 3.6; // in cm
const int DOOR_SIZE = 170; // degrees
const int DOOR_SPEED = 75;
const int TOUCH_PORT = S3;
const int GYRO_PORT = S2;
const int COLOR_PORT = S1;
const int ULTRASONIC_PORT = S4;

task main()
{
	configureAllSensors();
	// initialization for domino dropping
	nMotorEncoder(motorB)=0;
  int dropIndex = 0;
  int dominoCount = DOMINOS_AT_MAX_LOAD;
  bool mode = false; // false for line follow, true for file path
  selectMode(mode);
  if(mode)
  {
  	followLine();
	}
	else
	{
		followPathFromFile();
	}
}

void followLine()
{

}

void followPathFromFile()
{
 // DO NOT DROP DOMINOES FOR FIRST INSTRUCTION
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

void selectMode(bool &mode)
{
	displayBigTextLine(5, "Choose Mode");
	displayBigTextLine(7, "Left - Follow Line");
	displayBigTextLine(9, "Right - Follow Path from File");

	while(!getButtonPress(buttonLeft) && !getButtonPress(buttonRight))
	{}

	if(getButtonPress(buttonLeft))
	{
		mode = false;
	}
	else if(getButtonPress(buttonRight))
	{
		mode = true;
	}

}

void driveDist(int mot_pow, float dist) // input negative motor power for backwards
{
	setDriveTrainSpeed(mot_pow);
	nMotorEncoder[motorA] = 0;
	while(abs(nMotorEncoder[motorA]) < distToDeg(dist))
	{}
	setDriveTrainSpeed(0);
}

void driveDistWhileDispensing(int mot_pow, int dist, int &dropIndex,int &dominoCount)
{
	nMotorEncoder[motorD] = 0;
	while(nMotorEncoder[motorD] < distToDeg(dist))
	{
		if(dominoCount == 0)
		{
			// TODO
		}
		dropDomino(dropIndex, dominoCount);
	}

}

//Josh - takes motor power, a distance in encoded degrees and the gyro sensor port.
//moves forward, turns 180 degrees, moves forward again to knock down first domino.
void stopAndKnock (int motor_power, int enc_limit) // TODO update with built in functions
{
	nMotorEncoder[motorA] = 0;
	motor[motorA] = motor[motorD] = motor_power;

	while(nMotorEncoder[motorA] < enc_limit)
	{}

	motor[motorA] = motor[motorD] = 0;

	resetGyro(GYRO_PORT);

	motor[motorA] = motor_power;
	motor[motorD] = -1*motor_power;

	while(getGyroDegrees(GYRO_PORT) < 180)
	{}

	nMotorEncoder[motorA] = 0;

	motor[motorA] = motor[motorD] = 0;
	motor[motorA] = motor[motorD] = motor_power;

	while(nMotorEncoder[motorA] < enc_limit)
	{}

	motor[motorA] = motor[motorD] = 0;

}

//Josh - takes UltraSonic sensor port, max distance from an object and motor power.
//Stops motors, displays message and plays a sound. continues when object is moved.
void somethingInTheWay (int ULTRASONIC_PORT, float max_dist, int motor_power)
{
	while(SensorValue[ULTRASONIC_PORT] < max_dist)
	{
		motor[motorA] = motor[motorD] = 0;
		// TODO find function to clear display
		displayString(5, "Please clear path ahead");
		playSound(soundBeepBeep); // can change later
	}
		ev3StopSound();
		motor[motorA] = motor[motorD] = motor_power;
}

void followPathFromFile()
{
	// assumes robot starts on top left of domino placement area, facing right

}

int distToDeg(float dist)
{
	return dist*180/PI/WHEEL_RAD;
}

float degToDist(int deg)
{
	return deg*PI*WHEEL_RAD/180;
}

void setDriveTrainSpeed(int speed)
{
	motor[motorA] = motor[motorD] = speed;
}

// Henrique's functions
void openDoor()
{
	nMotorEncoder(motorC) = 0;
	motor[motorC] = DOOR_SPEED;
	while (nMotorEncoder(motorC)<DOOR_SIZE)
	{}
	motor[motorC] = 0;

	return;
}

void closeDoor()
{
	motor[motorC] = -1*DOOR_SPEED;
	while (nMotorEncoder(motorC)>5)
	{}
	motor[motorC] = 0;

	return;
}

void dropDomino(int &dropIndex, int &dominoCount)
{
	if (dropIndex == 0)
	{
		motor[motorB] = 15;
		while (nMotorEncoder(motorB)<130)
		{}
		motor[motorB] = 0;
		dropIndex += 1;
	}
	if (dropIndex == 1)
	{
		motor[motorB] = 15;
		while (nMotorEncoder(motorB)<160)
		{}
		motor[motorB]= 0;
		dropIndex += 1;
	}

	if (dropIndex == 2)
	{
		motor[motorB] = 15;
		while (nMotorEncoder(motorB)<220)
		{}
		motor[motorB] = 0;
		wait1Msec(100);
		motor[motorB] = -15;
		while (nMotorEncoder(motorB)<0)
		{}
		motor[motorB] = 0;
	}
	openDoor();
	dominoCount--;
	driveDist(15, DIST_BETWEEN_DOMINOS);
	closeDoor();
}
