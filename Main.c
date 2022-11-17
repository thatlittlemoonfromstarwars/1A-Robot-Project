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

// one-time functions
void configureAllSensors();
bool selectMode();
void endProgram();

// high level functions
void followLine(bool &dropIndex, int &dominoCount); // Sean
void followPathFromFile(bool &dropIndex, int &dominoCount); // Andor
int getCoordsFromFile(Coord* coords);
void dropDomino(bool &dropIndex, int &dominoCount); // Henrique
void somethingInTheWay(int motor_power); // stops and informs the user to move the object in the way

// calculation functions
float calcLength(Coord nextCoord, Coord curCoord);
float calcAngle(Coord nextCoord, Coord curCoord);
int distToDeg(float dist);
float degToDist(int deg);

// movement functions
void setDriveTrainSpeed(int speed);
void driveDist(int mot_pow, float dist);
void turnInPlace(int angle, int mot_pow);
void stopAndKnock(); // Josh
void openDoor();
void closeDoor();

// constants
const float WHEEL_RAD = 2.75; // in cm
const int DOMINOS_AT_MAX_LOAD = 30;
const int MAX_COORDS = 50; // don't type 70 here
const int PIXELS_PER_CM = 15;
const float DIST_BETWEEN_DOMINOS = 3.75; // in cm
const int DIST_IN_FRONT_LIM = 5; // in cm
const int TIME_TO_PRESS = 10; // in seconds
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
	nMotorEncoder(DISPENSER_MOT_PORT) = 0;
	nMotorEncoder(DOOR_MOT_PORT) = 0;
	bool dropIndex = false; // false for back position, true for middle position
	int dominoCount = DOMINOS_AT_MAX_LOAD;
	wait1Msec(5000);
	if(selectMode())// false for line follow, true for file path
	{
		followLine(dropIndex, dominoCount);
	}
	else
	{
		followPathFromFile(dropIndex, dominoCount);
	}
}

//  ********************************** one-time functions *********************************************
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

bool selectMode()
{
	displayBigTextLine(5, "Choose Mode");
	displayBigTextLine(7, "Left - Follow Line");
	displayBigTextLine(9, "Right - Follow Path from File");

	while(!getButtonPress(buttonLeft) && !getButtonPress(buttonRight))
	{}

	// returns true if buttonRight is pressed (path from file mode)
	// returns false if buttonLeft is pressed (line follow mode)
	return getButtonPress(buttonRight);
}

void endProgram()
{
	setDriveTrainSpeed(0);
	time1[T1] = 0;
	while(time1[T1] < TIME_TO_PRESS*1000)
	{
		if(SensorValue[TOUCH_PORT])
			stopAndKnock();
	}
}

// ********************************** high level functions ************************************************
void followLine(bool &dropIndex, int &dominoCount) // Sean
{

}

void followPathFromFile(bool &dropIndex, int &dominoCount) // Andor
{
	// DO NOT DROP DOMINOES FOR FIRST INSTRUCTION
	Coord coords[MAX_COORDS];
	int num_coords = getCoordsFromFile(coords);

	// calculate how to get to starting Coord
	Coord origin;
	origin.x = 0;
	origin.y = 0;

	float first_length = calcLength(coords[0],origin)/PIXELS_PER_CM;
	int first_angle = atan2(coords[0].y, coords[0].x)*180/PI;

	// turn and drive to first coord
	turnInPlace(first_angle, 20);
	driveDist(50, first_length);

	// turn towards second point
	Coord point2adjusted;
	point2adjusted.x = coords[1].x-coords[0].x;
	point2adjusted.y = coords[1].y-coords[0].y;
	turnInPlace(calcAngle(coords[1], point2adjusted), 20);

	Coord curCoord;
	curCoord = coords[0];

	int coord_index = 1; // represents index of next coordinate
	while(coord_index < num_coords && dominoCount > 0)
	{
		Coord nextCoord;
		nextCoord = coords[coord_index];

		// do calculations
		float drive_length = calcLength(nextCoord, curCoord)/PIXELS_PER_CM;
		// TODO subtract lengths for turning radius

		if(coord_index != num_coords-1)
		{
			Coord nextCoord2;
			nextCoord2 = coords[coord_index+1];

			// calculate turn angle to next vector

			/* PROBABLY WRONG
			Coord nextCoord2Adj;
			nextCoord2Adj.x = nextCoord2.x-curCoord.x;
			nextCoord2Adj.y = nextCoord2.y-curCoord.y;

			int angleToTurn = calcAngle(nextCoord2, nextCoord2Adj)*180/PI;
			*/


			// FOR TESTING ONLY

			// https://math.stackexchange.com/questions/405024/determine-center-of-circle-if-radius-and-2-tangent-line-segments-are-given

			// update current point
			curCoord = nextCoord;
		}
		else
		{
			// TODO this has to be updated
			int angleToTurn = 0;
		}

		// drive length
		setDriveTrainSpeed(50);
		while(abs(nMotorEncoder(LEFT_MOT_PORT)) < drive_length)
		{
			// check for break conditions
			if(SensorValue[TOUCH_PORT])
			{
				stopAndKnock();
				return;
			}
			else if(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
			{
				somethingInTheWay(50);
			}

			// drop domino every DIST_BETWEEN_DOMINOS
			if((int)(degToDist(abs(nMotorEncoder(LEFT_MOT_PORT))*100)%((int)(DIST_BETWEEN_DOMINOS*100)) == 0))
			{
				dropDomino(dropIndex, dominoCount);
				setDriveTrainSpeed(50);
			}
		}

		// start turn
		// use motor encoder and arc length or just with gyro
		// if angle is 0, dont turn

		coord_index++;
	}
	endProgram();
}

int getCoordsFromFile(Coord* coords) // Andor
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

void dropDomino(bool &dropIndex, int &dominoCount) // Henrique
{
	setDriveTrainSpeed(0);
	closeDoor();
	if (!dropIndex)
	{
		motor[DISPENSER_MOT_PORT] = -15;
		while (nMotorEncoder(DISPENSER_MOT_PORT) > -325)
		{}
		motor[DISPENSER_MOT_PORT] = 0;
		dropIndex = true;
	}
	else
	{
		motor[DISPENSER_MOT_PORT] = -15;
		while (nMotorEncoder(DISPENSER_MOT_PORT) > -550)
		{}
		motor[DISPENSER_MOT_PORT]= 0;

		dropIndex = false;
		wait1Msec(100);

		motor[DISPENSER_MOT_PORT] = 15;
		while (nMotorEncoder(DISPENSER_MOT_PORT) < 100)
		{}
		motor[DISPENSER_MOT_PORT] = 0;
	}
	wait1Msec(700);
	openDoor();
	dominoCount--;
	// continue line or path follow after
}

void somethingInTheWay (int motor_power) // Josh
{
	// takes UltraSonic sensor port, max distance from an object and motor power.
	// Stops motors, displays message and plays a sound. continues when object is moved.
	while(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
	{
		setDriveTrainSpeed(0);
		eraseDisplay();
		displayString(5, "Please clear path ahead");
		playSound(soundBeepBeep); // can change later
	}
	ev3StopSound();
	setDriveTrainSpeed(motor_power);
}

// ********************************** calculation functions ***********************************************
float calcLength(Coord nextCoord, Coord curCoord)
{
	return sqrt(pow(nextCoord.x-curCoord.x,2) + pow(nextCoord.y-curCoord.y, 2));
}

float calcAngle(Coord nextCoord, Coord curCoord)
{
	return asin((nextCoord.y*curCoord.x - nextCoord.x*curCoord.y)/(pow(curCoord.x, 2)+ pow(curCoord.y, 2)));
}

int distToDeg(float dist)
{
	return dist*180/PI/WHEEL_RAD;
}

float degToDist(int deg)
{
	return deg*PI*WHEEL_RAD/180;
}

// ********************************** movement functions ***************************************************
void setDriveTrainSpeed(int speed)
{
	motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = -1*speed;
}

void driveDist(int mot_pow, float dist)
{
	// input negative motor power for backwards
	setDriveTrainSpeed(mot_pow);
	nMotorEncoder[LEFT_MOT_PORT] = 0;
	while(abs(nMotorEncoder[LEFT_MOT_PORT]) < distToDeg(dist))
	{}
	setDriveTrainSpeed(0);
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

void stopAndKnock() // Josh
{
	// moves backwards to knock over first domino

	nMotorEncoder(LEFT_MOT_PORT) = 0;
	setDriveTrainSpeed(-30);
	while(nMotorEncoder(LEFT_MOT_PORT) < (DIST_BETWEEN_DOMINOS-0.5)*180/PI/WHEEL_RAD)
	{}
	setDriveTrainSpeed(0);
	return;
}

void openDoor() // Henrique
{
	motor[DOOR_MOT_PORT] = DOOR_SPEED;
	while (nMotorEncoder(DOOR_MOT_PORT)<DOOR_ANG)
	{}
	motor[DOOR_MOT_PORT] = 0;
}

void closeDoor() // Henrique
{
	if(!nMotorEncoder(DOOR_MOT_PORT)<5)
	{
		motor[DOOR_MOT_PORT] = -1*DOOR_SPEED;
		while (nMotorEncoder(DOOR_MOT_PORT)>5)
		{}
		motor[DOOR_MOT_PORT] = 0;
	}
}
