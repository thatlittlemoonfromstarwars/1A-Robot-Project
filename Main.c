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
void turnInPlace(int angle, int mot_pow); // NEEDS TO BE WRITTEN

void followLine(); // Sean

void followPathFromFile(); // Andor
int getCoordsFromFile(int* coordsX, int* coordsY);
float calcLength(int curX, int curY, int prevX, int prevY);
float calcAngle(int curX, int curY, int prevX, int prevY);

// calculation functions
int distToDeg(float dist);
float degToDist(int deg);

// constants
const float WHEEL_RAD = 2.75; // in cm
const int DOMINOS_AT_MAX_LOAD = 60;
const int MAX_COORDS = 50;
const int PIXELS_PER_CM = 15;
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
	int coordsX[MAX_COORDS];
	int coordsY[MAX_COORDS];
	int num_coords = getCoordsFromFile(coordsX, coordsY);

	// drive to first coord and turn
	float first_length = sqrt(pow(coordsX[0],2) + pow(coordsY[0],2))/PIXELS_PER_CM;
	int first_angle = atan(coordsY[0]/coordsX[0])*180/PI;

	turnInPlace(first_angle, 20);
	driveDist(50, first_length);

	// turn towards second point
	turnInPlace(calcAngle(coordsX[1], coordsY[1], coordsX[1]-coordsX[0], coordsY[1]-coordsY[0]), 20);

	int prevX = coordsX[1];
	int prevY = coordsY[1];

	for(int coord_index = 0; coord_index < num_coords; num_coords++)
	{
		float drive_length = calcLength(coordsX[coord_index], coordsY[coord_index], prevX, prevY)/PIXELS_PER_CM;
		// FOR TESTING ONLY
		driveDist(50, drive_length);
		if(coord_index != num_coords)
		{
			int angleToTurn = calcAngle(coordsX[coord_index+1], coordsY[coord_index+1], coordsX[coord_index+1]-coordsX[coord_index], coordsY[coord_index+1]-coordsY[coord_index])*180/PI;
			// FOR TESTING ONLY
			turnInPlace(angleToTurn, 20);
		}
	}

}

int getCoordsFromFile(int* coordsX, int* coordsY)
{
	TFileHandle fin;
	bool fileOkay = openReadPC(fin,"drive_coords.txt");

	int num_coords = 0;
	readIntPC(fin, num_coords);

	int tempX = 0;
	int tempY = 0;

	for(int read_index = 0; read_index < num_coords; read_index++)
	{
		readIntPC(fin, tempX);
		readIntPC(fin, tempY);
		coordsX[read_index] = tempX;
		coordsY[read_index] = tempY;
	}

	closeFilePC(fin);
	return num_coords;
}

float calcLength(int curX, int curY, int prevX, int prevY)
{
	return sqrt(pow(curX-prevX,2) + pow(curY-prevY, 2));
}

float calcAngle(int curX, int curY, int prevX, int prevY)
{
	return asin((curY*prevX - curX*prevY)/(pow(prevX, 2)+ pow(prevY, 2)));
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
