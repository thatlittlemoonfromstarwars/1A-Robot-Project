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
float calcLength(Coord &nextCoord, Coord &curCoord);
int calcAngle(Coord &curCoord, Coord &coord1, Coord &coord2);
int distToDeg(float dist);
float degToDist(int deg);

// movement functions
void setDriveTrainSpeed(int speed);
void driveDist(int mot_pow, float dist);
void driveWhileDropping(int mot_pow, float dist, bool &dropIndex, int &dominoCount);
void turnInPlace(int angle, int mot_pow);
void turnWhileDropping(int angle, int speed, bool &dropIndex, int &dominoCount);
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
const float TURN_RAD = 30; //in cm - needs to be more than 6.75cm
const int TIME_TO_PRESS = 10; // in seconds
const int DOOR_ANG = 90; // degrees
const int DOOR_SPEED = 10;

const int TOUCH_PORT = S2;
const int GYRO_PORT = S3;
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
	//wait1Msec(5000);

	if(selectMode())// false for line follow, true for file path
	{
		followPathFromFile(dropIndex, dominoCount);
	}
	else
	{
		followLine(dropIndex, dominoCount);
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
	displayTextLine(5, "Choose Mode");
	displayTextLine(7, "Left - Follow Line");
	displayTextLine(9, "Right - Follow Path from File");

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
	Coord coords[3];
	coords[0].x = 105;
	coords[0].y = 116;
	coords[1].x = 513;
	coords[1].y = 120;
	coords[2].x = 520;
	coords[2].y = 254;
	//Coord coords[MAX_COORDS];
	//int num_coords = getCoordsFromFile(coords);
	int num_coords = 3;

	Coord origin;
	origin.x = 0;
	origin.y = 0;

	// calculate how to get to starting Coord
	float first_length = calcLength(coords[0],origin)/PIXELS_PER_CM;
	int first_angle = atan2(coords[0].y, coords[0].x)*180/PI;

	// turn and drive to first coord
	turnInPlace(first_angle, 20);
	driveDist(50, first_length);

	// turn towards second point
	turnInPlace(calcAngle(origin,coords[0],coords[1]), 20);

	Coord curCoord;
	curCoord.x = coords[0].x;
	curCoord.y = coords[0].y;

	int angleToTurn = 0;

	int coord_index = 1; // represents index of next coordinate
	while(coord_index < num_coords && dominoCount > 0)
	{
		// do calculations
		Coord coord1;
		coord1.x = coords[coord_index].x;
		coord1.y = coords[coord_index].y;

		float drive_length = calcLength(curCoord, coord1)/PIXELS_PER_CM;
		// TODO subtract lengths for turning radius

		if(coord_index != num_coords-1)
		{
			Coord coord2;
			coord2.x = coords[coord_index+1].x;
			coord2.y = coords[coord_index+1].y;

			angleToTurn = calcAngle(curCoord, coord1, coord2);

			// update coordinates
			curCoord.x = coord1.x;
			curCoord.y = coord1.y;
		}
		else
		{
			// TODO this has to be updated
			angleToTurn = 0;
		}

		// drive length
		driveWhileDropping(50, drive_length, dropIndex, dominoCount);
		// turn
		turnWhileDropping(angleToTurn, 20, dropIndex, dominoCount);
		coord_index++;
	}
	endProgram();
}

int getCoordsFromFile(Coord* coords) // Andor
{
	TFileHandle fin;
	bool fileOkay = openReadPC(fin,"coords.txt");

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
float calcLength(Coord &nextCoord, Coord &curCoord)
{
	return sqrt(pow(nextCoord.x-curCoord.x,2) + pow(nextCoord.y-curCoord.y, 2));
}

int calcAngle(Coord &curCoord, Coord &coord1, Coord &coord2)
{
	Coord origin;
	origin.x = 0;
	origin.y = 0;
	Coord adjCoord;
	adjCoord.x = coord2.x-coord1.x;
	adjCoord.y = coord2.y-coord1.y;
	return acos((coord1.x*adjCoord.x+coord1.y*adjCoord.y)/(calcLength(coord1,origin)*calcLength(adjCoord, origin)))*180/PI;

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

void driveWhileDropping(int mot_pow, float dist, bool &dropIndex, int &dominoCount)
{
	setDriveTrainSpeed(mot_pow);
	nMotorEncoder[LEFT_MOT_PORT] = 0;
	while(abs(nMotorEncoder(LEFT_MOT_PORT)) < distToDeg(dist))
	{
		// check for break conditions
		if(SensorValue[TOUCH_PORT])
		{
			stopAndKnock();
			return;
		}
		else if(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
		{
			somethingInTheWay(mot_pow);
		}

		// drop domino every DIST_BETWEEN_DOMINOS
		if((int)(degToDist(abs(nMotorEncoder(LEFT_MOT_PORT))*100)%((int)(DIST_BETWEEN_DOMINOS*100)) == 0))
		{
			dropDomino(dropIndex, dominoCount);
			setDriveTrainSpeed(mot_pow);
			while((int)(degToDist(abs(nMotorEncoder(LEFT_MOT_PORT))*100)%((int)(DIST_BETWEEN_DOMINOS*100)) == 0))
			{}
		}
	}
}

void turnInPlace(int angle, int mot_pow)
{
	int initialGyro = getGyroDegrees(GYRO_PORT);
	if(angle < 0)
	{
		// turn left
		motor[LEFT_MOT_PORT] = mot_pow;
		motor[RIGHT_MOT_PORT] = -mot_pow;
		while(getGyroDegrees(GYRO_PORT) > initialGyro+angle)
		{
			int gyroDeg = getGyroDegrees(GYRO_PORT);
		}
	}
	else if(angle > 0)
	{
		// turn right
		motor[LEFT_MOT_PORT] = -mot_pow;
		motor[RIGHT_MOT_PORT] = mot_pow;
		while(getGyroDegrees(GYRO_PORT) < initialGyro+angle)
		{
			int gyroDeg = getGyroDegrees(GYRO_PORT);
		}
}

	setDriveTrainSpeed(0);
}

void turnWhileDropping(int angle, int speed, bool &dropIndex, int &dominoCount)
{
	// https://math.stackexchange.com/questions/4310012/calculate-the-turning-radius-turning-circle-of-a-two-wheeled-car

	float const TURN_RATIO = (TURN_RAD-13.5)/TURN_RAD;

	int initialGyro = getGyroDegrees(GYRO_PORT);
	if(angle > 0)
	{
		// turn Right
		motor[LEFT_MOT_PORT] = -speed*TURN_RATIO;
		motor[RIGHT_MOT_PORT] = -speed;
		nMotorEncoder(RIGHT_MOT_PORT) = 0;
		while(getGyroDegrees(GYRO_PORT) < initialGyro+angle)
		{
			if((int)(degToDist(abs(nMotorEncoder(RIGHT_MOT_PORT))*100)%((int)(DIST_BETWEEN_DOMINOS*100)) == 0))
			{
				dropDomino(dropIndex, dominoCount);
				motor[LEFT_MOT_PORT] = -speed*TURN_RATIO;
				motor[RIGHT_MOT_PORT] = -speed;
				while((int)(degToDist(abs(nMotorEncoder(RIGHT_MOT_PORT))*100)%((int)(DIST_BETWEEN_DOMINOS*100)) == 0))
				{}
			}
		}
	}
	else if(angle < 0)
	{
		// turn left
		motor[LEFT_MOT_PORT] = -speed;
		motor[RIGHT_MOT_PORT] = -speed*TURN_RATIO;
		nMotorEncoder(LEFT_MOT_PORT) = 0;
		while(getGyroDegrees(GYRO_PORT) > initialGyro+angle)
		{
			if((int)(degToDist(abs(nMotorEncoder(LEFT_MOT_PORT))*100)%((int)(DIST_BETWEEN_DOMINOS*100)) == 0))
			{
				dropDomino(dropIndex, dominoCount);
				motor[LEFT_MOT_PORT] = -speed;
				motor[RIGHT_MOT_PORT] = -speed*TURN_RATIO;
				while((int)(degToDist(abs(nMotorEncoder(LEFT_MOT_PORT))*100)%((int)(DIST_BETWEEN_DOMINOS*100)) == 0))
				{}
			}
		}
	}
}

void stopAndKnock() // Josh
{
	// moves backwards to knock over first domino

	nMotorEncoder(LEFT_MOT_PORT) = 0;
	setDriveTrainSpeed(-30);
	while(nMotorEncoder(LEFT_MOT_PORT) < distToDeg(DIST_BETWEEN_DOMINOS-0.5))
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
