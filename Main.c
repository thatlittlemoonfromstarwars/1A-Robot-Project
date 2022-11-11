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

typedef struct
{
	int x;
	int y;

} Coord;

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
void turnInPlace(int angle, int mot_pow);

void followLine(); // Sean

void followPathFromFile(); // Andor
int getCoordsFromFile(Coord* coords);
float calcLength(Coord nextCoord, Coord curCoord);
float calcAngle(Coord nextCoord, Coord curCoord);

// calculation functions
int distToDeg(float dist);
float degToDist(int deg);

// constants
const float WHEEL_RAD = 2.75; // in cm
const int DOMINOS_AT_MAX_LOAD = 60;
const int MAX_COORDS = 50; // don't type 70 here
const int PIXELS_PER_CM = 15;
const float DIST_BETWEEN_DOMINOS = 3.75; // in cm
const int DOOR_ANG = 90; // degrees
const int DOOR_SPEED = 10;

const int TOUCH_PORT = S3;
const int GYRO_PORT = S2;
const int COLOR_PORT = S1;
const int ULTRASONIC_PORT = S4;

const int RIGHT_MOT_PORT = motorD;
const int LEFT_MOT_PORT = motorA;
const int DOOR_MOT_PORT = motorB;
const int DISPENSER_MOT_PORT = motorC;

task main()
{
	configureAllSensors();
	// initialization for domino dropping
	nMotorEncoder(DISPENSER_MOT_PORT)=0;
	nMotorEncoder(DOOR_MOT_PORT)=0;
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
	Coord coords[MAX_COORDS];
	int num_coords = getCoordsFromFile(coords);

	// drive to first coord and turn
	float first_length = sqrt(pow(coords[0].x,2) + pow(coords[0].y,2))/PIXELS_PER_CM;
	int first_angle = atan2(coords[0].y, coords[0].x)*180/PI;

	turnInPlace(first_angle, 20);
	driveDist(50, first_length);

	// turn towards second point
	Coord point2adjusted;
	point2adjusted.x = coords[1].x-coords[0].x;
	point2adjusted.y = coords[1].y-coords[0].y;
	turnInPlace(calcAngle(coords[1], point2adjusted), 20);

	Coord curCoord;
	curCoord = coords[1];

	for(int coord_index = 0; coord_index < num_coords; num_coords++)
	{
		float drive_length = calcLength(coords[coord_index], curCoord)/PIXELS_PER_CM;
		// FOR TESTING ONLY
		driveDist(50, drive_length);
		if(coord_index != num_coords-1)
		{
			Coord nextCoord;
			nextCoord = coords[coord_index+1];
			Coord nextCoordAdj;
			nextCoordAdj.x = nextCoord.x-curCoord.x;
			nextCoordAdj.y = nextCoord.y-curCoord.y;
			int angleToTurn = calcAngle(nextCoord, nextCoordAdj)*180/PI;
			// FOR TESTING ONLY
			turnInPlace(angleToTurn, 20);

			// update current point
			curCoord = nextCoord;
		}

	}

}

int getCoordsFromFile(Coord* coords)
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
		coords[read_index].x = tempX;
		coords[read_index].y = tempY;
	}

	closeFilePC(fin);
	return num_coords;
}

float calcLength(Coord nextCoord, Coord curCoord)
{
	return sqrt(pow(nextCoord.x-curCoord.x,2) + pow(nextCoord.y-curCoord.y, 2));
}

float calcAngle(Coord nextCoord, Coord curCoord)
{
	return asin((nextCoord.y*curCoord.x - nextCoord.x*curCoord.y)/(pow(curCoord.x, 2)+ pow(curCoord.y, 2)));
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
	nMotorEncoder[LEFT_MOT_PORT] = 0;
	while(abs(nMotorEncoder[LEFT_MOT_PORT]) < distToDeg(dist))
	{}
	setDriveTrainSpeed(0);
}

void driveDistWhileDispensing(int mot_pow, int dist, int &dropIndex,int &dominoCount)
{
	nMotorEncoder[RIGHT_MOT_PORT] = 0;
	while(abs(nMotorEncoder[RIGHT_MOT_PORT]) < distToDeg(dist))
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
		motor[LEFT_MOT_PORT] = mot_pow;
		motor[DOOR_MOT_PORT] = -1*mot_pow;
		while(getGyroDegrees(GYRO_PORT) > initialGyro-angle)
		{}
	}
	else if(angle > 0)
	{
		motor[LEFT_MOT_PORT] = -1*mot_pow;
		motor[DOOR_MOT_PORT] = mot_pow;
		while(getGyroDegrees(GYRO_PORT) < initialGyro+angle)
		{}
	}

	setDriveTrainSpeed(0);
}

// Josh - takes motor power, a distance in encoded degrees and the gyro sensor port.
// moves forward, turns 180 degrees, moves forward again to knock down first domino.
void stopAndKnock (int motor_power, int enc_limit) // TODO update with built in functions
{
	nMotorEncoder[LEFT_MOT_PORT] = 0;
	motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = motor_power;

	while(nMotorEncoder[LEFT_MOT_PORT] < enc_limit)
	{}

	motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = 0;

	resetGyro(GYRO_PORT);

	motor[LEFT_MOT_PORT] = motor_power;
	motor[RIGHT_MOT_PORT] = -1*motor_power;

	while(getGyroDegrees(GYRO_PORT) < 180)
	{}

	nMotorEncoder[LEFT_MOT_PORT] = 0;

	motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = 0;
	motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = motor_power;

	while(nMotorEncoder[LEFT_MOT_PORT] < enc_limit)
	{}

	motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = 0;

}

//Josh - takes UltraSonic sensor port, max distance from an object and motor power.
//Stops motors, displays message and plays a sound. continues when object is moved.
void somethingInTheWay (int ULTRASONIC_PORT, float max_dist, int motor_power)
{
	while(SensorValue[ULTRASONIC_PORT] < max_dist)
	{
		motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = 0;
		// TODO find function to clear display
		displayString(5, "Please clear path ahead");
		playSound(soundBeepBeep); // can change later
	}
	ev3StopSound();
	motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = motor_power;
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
	motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = -1*speed;
}

// Henrique's functions
void openDoor()
{
	motor[DOOR_MOT_PORT] = DOOR_SPEED;
	while (nMotorEncoder(DOOR_MOT_PORT)<DOOR_ANG)
	{}
	motor[DOOR_MOT_PORT] = 0;

	return;
}

void closeDoor()
{
	motor[DOOR_MOT_PORT] = -1*DOOR_SPEED;
	while (nMotorEncoder(DOOR_MOT_PORT)>5)
	{}
	motor[DOOR_MOT_PORT] = 0;

	return;
}

void dropDomino(int &dropIndex, int &dominoCount)
{
	if (dropIndex == 0)
	{
		motor[DISPENSER_MOT_PORT] = -15;
		while (nMotorEncoder(DISPENSER_MOT_PORT) > -325)
		{}
		motor[DISPENSER_MOT_PORT] = 0;
		dropIndex += 1;
	}
	else if (dropIndex == 1)
	{
		motor[DISPENSER_MOT_PORT] = -15;
		while (nMotorEncoder(DISPENSER_MOT_PORT) > -550)
		{}
		motor[DISPENSER_MOT_PORT]= 0;

		dropIndex = 0;
		wait1Msec(100);

		motor[DISPENSER_MOT_PORT] = 15;
		while (nMotorEncoder(DISPENSER_MOT_PORT) < 100)
		{}
		motor[DISPENSER_MOT_PORT] = 0;
	}
	wait1Msec(700);
	openDoor();
	dominoCount--;
	driveDist(15, DIST_BETWEEN_DOMINOS);
	closeDoor();
}
