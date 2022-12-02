/*
Waterloo Engineering Expeller of Dominos Main Program

Sean Aitken, Henrique Engelke, Josh Morcombe, and Andor Siegers

v1.7

Assumptions:
- more than 3 instructions will be in instruction file
- no more than 100 instructions will be given to the robot
- robot is fully loaded at program start, with exactly 30 dominoes
  in the hopper
- door is closed, dispenser arm is all the way back at program start
- if user is selecting line follow mode, it must be placed on a line of adequate length
  with white on either side
- if user is selecting file follow mode, a file of the correct format must be
  loaded on the robot

Motor Ports:
A - left drive wheel
B - dispenser motor
C - gate motor
D - right drive wheel

Sensor Ports:
1 - MUX
2 - gyro
3 - touch
4 - ultrasonic
*/

#include "PC_FileIO.c";
#include "mindsensors-ev3smux.h"
#include "UW_sensorMux.c"

typedef struct
{
	bool is_ang;
	int val;

} Instr;

// one-time functions
void configureAllSensors(bool mode);
bool selectMode();
void endProgram();

// high level functions
void followLine(bool &drop_index, int &domino_count); // Sean
void followPathFromFile(bool &drop_index, int &domino_count); // Andor
int getInstrFromFile(Instr* all_instr);
void dropDomino(bool &drop_index, int &domino_count); // Henrique
void somethingInTheWay(int motor_power); // stops and informs the user to move the object in the way
void somethingInTheWay (int left_mot_pow, int right_mot_pow);

// calculation functions
int distToDeg(float dist);
float degToDist(int deg);
float average(int value1, int value2);


// movement functions
void setDriveTrainSpeed(int speed);
void driveDist(float dist,int mot_pow);
void driveWhileDropping(float dist, int mot_pow, bool &drop_index, int &domino_count, float &dist_since_last_dom); // Andor
void turnInPlace(int angle, int mot_pow);
void turnWhileDropping(int angle, int speed, bool &drop_index, int &domino_count, float &dist_since_last_dom); // Andor
void stopAndKnock(); // Josh
void openDoor();
void closeDoor();

// constants
const float WHEEL_RAD = 2.75; // in cm
const int DOMINOS_AT_MAX_LOAD = 30;
const int MAX_INSTR = 100;
const float PIXELS_PER_CM = 5.0;
const float DIST_BETWEEN_DOMINOS = 3.75; // in cm
const float DIST_BET_DOM_TURNING = 5.5; // in cm
const int DRIVE_SPEED = 20; // for path from file mode
const int DIST_IN_FRONT_LIM = 20; // in cm
const float TURN_RAD = 20; // in cm - needs to be more than 6.75cm
const int TIME_TO_PRESS = 10; // in seconds
const int DOOR_ANG = 90; // degrees
const int DOOR_SPEED = 50;
const int DROP_WAIT = 500; // in milliseconds
const int MUX_WAIT = 10;
const int DISPENSER_SPEED = -30;
const int DISPENSER_POS0 = 80;
const int DISPENSER_POS1 = -370;
const int DISPENSER_POS2 = -530;
const int KNOCK_SPEED = -15;

// port assignments
const int TOUCH_PORT = S2;
const int GYRO_PORT = S3;
const int MULTIPLEXER_PORT = S1;
const int ULTRASONIC_PORT = S4;

const int RIGHT_MOT_PORT = motorD;
const int LEFT_MOT_PORT = motorA;
const int DOOR_MOT_PORT = motorB;
const int DISPENSER_MOT_PORT = motorC;

task main()
{
	// initialization for domino dropping
	nMotorEncoder(DISPENSER_MOT_PORT) = 0;
	nMotorEncoder(DOOR_MOT_PORT) = 0;
	bool drop_index = false; // false for back position, true for middle position
	int domino_count = DOMINOS_AT_MAX_LOAD;

	if(selectMode())// false for line follow, true for file path
	{
		followPathFromFile(drop_index, domino_count);
	}
	else
	{
		followLine(drop_index, domino_count);
	}
}

//  ********************************** one-time functions *********************************************
void configureAllSensors(bool mode)
{
	SensorType[TOUCH_PORT] = sensorEV3_Touch;
	SensorType[GYRO_PORT] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorType[ULTRASONIC_PORT] = sensorEV3_Ultrasonic;
	wait1Msec(50);
	SensorMode[GYRO_PORT] = modeEV3Gyro_Calibration;
	wait1Msec(50);
	SensorMode[GYRO_PORT] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);

	// if line follow mode is selected, configure sensors required for
	// this mode
	if(!mode)
	{
		SensorType[MULTIPLEXER_PORT] = sensorEV3_GenericI2C;
		wait1Msec(100);

		if (!initSensorMux(msensor_S1_1, colorMeasureColor))
		{
			displayString(2,"Failed to configure colour1");
			return;
		}
		wait1Msec(50);
		if (!initSensorMux(msensor_S1_2, colorMeasureColor))
		{
			displayString(4,"Failed to configure colour2");
			return;
		}
		wait1Msec(50);
	}
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
	bool mode = getButtonPress(buttonRight);
	configureAllSensors(mode);
	wait1Msec(700);
	return mode;
}

void endProgram()
{
	setDriveTrainSpeed(0);
	time1[T1] = 0;
	// wait for user to press touch sensor
	while(time1[T1] < TIME_TO_PRESS*1000)
	{
		if(SensorValue[TOUCH_PORT])
			stopAndKnock();
	}
	stopAllTasks();
}

// ********************************** high level functions ************************************************
void followLine(bool &drop_index, int &domino_count) // Sean
{
	time1[T2] = 0;
	int index = 0;
	int index2 = 0;
	int sensor1 = 0;
	int sensor2 = 0;
	int domino_encoder_spacing = distToDeg(DIST_BETWEEN_DOMINOS);

	openDoor();

	while((domino_count>0)&&(SensorValue(TOUCH_PORT) == 0))
	{

		if((SensorValue[ULTRASONIC_PORT]) < (DIST_IN_FRONT_LIM))
		{
			somethingInTheWay(0);
		}

	 	if((average(nMotorEncoder[RIGHT_MOT_PORT],nMotorEncoder[LEFT_MOT_PORT])) > domino_encoder_spacing)
		{
			dropDomino(drop_index, domino_count);
 			nMotorEncoder[RIGHT_MOT_PORT] = nMotorEncoder[LEFT_MOT_PORT] = 0;
	 	}

		motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = -10;

		if(time1[T2] > index)
		{
			sensor1 = readMuxSensor(msensor_S1_1);
			index = time1[T2] + MUX_WAIT;

			if(sensor1 == (int) colorBlack)
			{
				motor[RIGHT_MOT_PORT] = 0;
			}
		}

		if(time1[T2] > index2)
		{
			sensor2 = readMuxSensor(msensor_S1_2);
			index2 = time1[T2] + MUX_WAIT + 5;

			if(sensor2 == (int) colorBlack)
			{
				motor[LEFT_MOT_PORT] = 0;
			}
		}
	}
	if(SensorValue(TOUCH_PORT))
	{
		stopAndKnock();
	}
	endProgram();
}

void followPathFromFile(bool &drop_index, int &domino_count) // Andor
{
	Instr all_instr[MAX_INSTR];
	float dist_since_last_dom = 0;

	int num_instr = getInstrFromFile(all_instr);

	int num_turns = 0;
	int instr_index = 0;

	// drive to starting position
	while(num_turns < 2)
	{
		if(all_instr[instr_index].is_ang)
		{
			num_turns++;
			turnInPlace(all_instr[instr_index].val, 20);
		}
		else
		{
			driveDist(all_instr[instr_index].val/PIXELS_PER_CM, 50);
		}
		instr_index++;
	}

	while(instr_index < num_instr && domino_count > 0)
	{
		// loop through all instructions

		if(all_instr[instr_index].is_ang)
		{
			// turn
			turnWhileDropping(all_instr[instr_index].val, DRIVE_SPEED, drop_index, domino_count, dist_since_last_dom);
		}
		else
		{
			// drive length
			driveWhileDropping(all_instr[instr_index].val/PIXELS_PER_CM, DRIVE_SPEED, drop_index, domino_count, dist_since_last_dom);
		}
		instr_index++;
	}
	endProgram();
}

int getInstrFromFile(Instr* all_instr) // Andor
{
	// open file and initialize variables
	TFileHandle fin;
	bool fileOkay = openReadPC(fin,"instr.txt");

	int num_instr = 0;
	readIntPC(fin, num_instr);

	int temp_is_ang_int = 0;
	bool temp_is_ang = false;
	int temp_val = 0;

	for(int read_index = 0; read_index < num_instr; read_index++)
	{
		// read in instruction
		readIntPC(fin, temp_is_ang_int);
		if(temp_is_ang_int == 0)
		{
			temp_is_ang = false;
		}
		else
		{
			temp_is_ang = true;
		}

		readIntPC(fin, temp_val);
		all_instr[read_index].is_ang = temp_is_ang;
		all_instr[read_index].val = temp_val;
	}

	closeFilePC(fin);
	return num_instr;
}

void dropDomino(bool &drop_index, int &domino_count) // Henrique
{
	setDriveTrainSpeed(0);
	closeDoor();

	// moves dispenser arm to next position, depending on current
	// position
	if (!drop_index)
	{
		motor[DISPENSER_MOT_PORT] = DISPENSER_SPEED;
		while (nMotorEncoder(DISPENSER_MOT_PORT) > DISPENSER_POS1)
		{
			// scan for touch press
			if(SensorValue[TOUCH_PORT])
			{
				motor[DISPENSER_MOT_PORT] = 0;
				stopAndKnock();
			}
		}
		motor[DISPENSER_MOT_PORT] = 0;
		drop_index = true;
		wait1Msec(DROP_WAIT);
	}
	else
	{
		motor[DISPENSER_MOT_PORT] = DISPENSER_SPEED;
		while (nMotorEncoder(DISPENSER_MOT_PORT) > DISPENSER_POS2)
		{
			if(SensorValue[TOUCH_PORT])
			{
				motor[DISPENSER_MOT_PORT] = 0;
				stopAndKnock();
			}
		}
		motor[DISPENSER_MOT_PORT]= 0;

		drop_index = false;
		wait1Msec(100);

		// reset arm to initial position
		motor[DISPENSER_MOT_PORT] = -DISPENSER_SPEED;
		while (nMotorEncoder(DISPENSER_MOT_PORT) < DISPENSER_POS0)
		{
			if(SensorValue[TOUCH_PORT])
			{
				motor[DISPENSER_MOT_PORT] = 0;
				stopAndKnock();
			}
		}
		motor[DISPENSER_MOT_PORT] = 0;
	}
	openDoor();
	domino_count--;
}

void somethingInTheWay (int motor_power) // Josh
{
	// Stops motors, displays message and plays a sound. Exits when object is moved.
	while(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
	{
		setDriveTrainSpeed(0);
		eraseDisplay();
		displayString(5, "Please clear path ahead");
		playSound(soundBeepBeep);
	}
	ev3StopSound();
	setDriveTrainSpeed(motor_power);
}

void somethingInTheWay (int left_mot_pow, int right_mot_pow)
{
	// same as apove, just with 2 motor inputs to accomodate the 
	// use of this function in turns
	while(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
	{
		setDriveTrainSpeed(0);
		eraseDisplay();
		displayString(5, "Please clear path ahead");
		playSound(soundBeepBeep);
	}
	ev3StopSound();
	motor[LEFT_MOT_PORT] = left_mot_pow;
	motor[RIGHT_MOT_PORT] = right_mot_pow;
}

// ********************************** calculation functions ***********************************************
int distToDeg(float dist)
{
	// takes a distance and converts it to motor encoder clicks
	// using wheel radius
	return dist*180/PI/WHEEL_RAD;
}

float degToDist(int deg)
{
	// converts degrees to motor encoder clicks using wheel radius
	return deg*PI*WHEEL_RAD/180;
}

float average(int value1, int value2)
{
	// returns average of two fucntions
	return (abs(value1 + value2)/2.0);
}

// ********************************** movement functions ***************************************************
void setDriveTrainSpeed(int speed)
{
	// accomodates the backwards mounting of drive motors
	motor[LEFT_MOT_PORT] = motor[RIGHT_MOT_PORT] = -1*speed;
}

void driveDist(float dist, int mot_pow)
{
	// drives specified distance without dropping dominoes
	setDriveTrainSpeed(mot_pow);
	nMotorEncoder[LEFT_MOT_PORT] = 0;
	while(abs(nMotorEncoder[LEFT_MOT_PORT]) < distToDeg(dist))
	{
		// check for break conditions
		if(SensorValue[TOUCH_PORT])
		{
			stopAndKnock();
		}
		else if(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
		{
			somethingInTheWay(mot_pow);
		}
	}
	setDriveTrainSpeed(0);
}

void driveWhileDropping(float dist, int mot_pow, bool &drop_index, int &domino_count, float &dist_since_last_dom)
{
	// drives specified distance while dropping dominos at consistent intervals
	setDriveTrainSpeed(mot_pow);
	nMotorEncoder[LEFT_MOT_PORT] = 0;
	nMotorEncoder[RIGHT_MOT_PORT] = 0;
	while(degToDist(abs(nMotorEncoder(LEFT_MOT_PORT))) < dist && domino_count > 0)
	{
		// check for break conditions
		if(SensorValue[TOUCH_PORT])
		{
			stopAndKnock();
		}
		else if(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
		{
			somethingInTheWay(mot_pow);
		}

		// drop domino every DIST_BETWEEN_DOMINOS
		if(degToDist(abs(nMotorEncoder(RIGHT_MOT_PORT))) + dist_since_last_dom >= DIST_BETWEEN_DOMINOS)
		{
			dist_since_last_dom = 0;
			nMotorEncoder(RIGHT_MOT_PORT) = 0;
			dropDomino(drop_index, domino_count);
			setDriveTrainSpeed(mot_pow);
		}
	}
	// allows for a smooth transition in the domino path between driving linearly and turning
	dist_since_last_dom = degToDist(abs(nMotorEncoder(RIGHT_MOT_PORT)));
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
			// check for break conditions
			if(SensorValue[TOUCH_PORT])
			{
				stopAndKnock();
			}
			else if(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
			{
				somethingInTheWay(mot_pow, -mot_pow);
			}
		}
	}
	else if(angle > 0)
	{
		// turn right
		motor[LEFT_MOT_PORT] = -mot_pow;
		motor[RIGHT_MOT_PORT] = mot_pow;
		while(getGyroDegrees(GYRO_PORT) < initialGyro+angle)
		{
			// check for break conditions
			if(SensorValue[TOUCH_PORT])
			{
				stopAndKnock();
			}
			else if(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
			{
				somethingInTheWay(-mot_pow, mot_pow);
			}
		}
}

	setDriveTrainSpeed(0);
}

void turnWhileDropping(int angle, int speed, bool &drop_index, int &domino_count, float &dist_since_last_dom)
{
	// some concepts taken from:
	// https://math.stackexchange.com/questions/4310012/calculate-the-turning-radius-turning-circle-of-a-two-wheeled-car

	// turns the robot through a specific radius while dropping dominoes
	float const TURN_RATIO = (TURN_RAD-13.5)/TURN_RAD;
	int initialGyro = getGyroDegrees(GYRO_PORT);
	if(angle > 0)
	{
		// turn Right
		motor[LEFT_MOT_PORT] = -speed;
		motor[RIGHT_MOT_PORT] = -speed*TURN_RATIO;
		nMotorEncoder(LEFT_MOT_PORT) = 0;
		while(getGyroDegrees(GYRO_PORT) < initialGyro+angle && domino_count > 0)
		{
			// check for break conditions
			if(SensorValue[TOUCH_PORT])
			{
				stopAndKnock();
			}
			else if(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
			{
				somethingInTheWay(-speed, -speed*TURN_RATIO);
			}

			if(degToDist(abs(nMotorEncoder(LEFT_MOT_PORT))) + dist_since_last_dom >= DIST_BET_DOM_TURNING)
			{
				// drops domino if correct spacing is reached
				dist_since_last_dom = 0;
				nMotorEncoder(LEFT_MOT_PORT) = 0;
				dropDomino(drop_index, domino_count);
				motor[LEFT_MOT_PORT] = -speed;
				motor[RIGHT_MOT_PORT] = -speed*TURN_RATIO;
			}
		}
		dist_since_last_dom = degToDist(abs(nMotorEncoder(LEFT_MOT_PORT)));
	}
	else if(angle < 0)
	{
		// turn left
		motor[LEFT_MOT_PORT] = -speed*TURN_RATIO;
		motor[RIGHT_MOT_PORT] = -speed;
		nMotorEncoder(RIGHT_MOT_PORT) = 0;
		while(getGyroDegrees(GYRO_PORT) > initialGyro+angle && domino_count > 0)
		{
			// check for break conditions
			if(SensorValue[TOUCH_PORT])
			{
				stopAndKnock();
			}
			else if(SensorValue[ULTRASONIC_PORT] < DIST_IN_FRONT_LIM)
			{
				somethingInTheWay(-speed*TURN_RATIO, -speed);
			}
			if(degToDist(abs(nMotorEncoder(RIGHT_MOT_PORT))) + dist_since_last_dom >= DIST_BET_DOM_TURNING)
			{
				// drops domino if correct spacing is reached
				dist_since_last_dom = 0;
				nMotorEncoder(RIGHT_MOT_PORT) = 0;
				dropDomino(drop_index, domino_count);
				motor[LEFT_MOT_PORT] = -speed*TURN_RATIO;
				motor[RIGHT_MOT_PORT] = -speed;
			}
		}
		dist_since_last_dom = degToDist(abs(nMotorEncoder(RIGHT_MOT_PORT)));
	}
}

void stopAndKnock() // Josh
{
	// moves backwards, knocking over first domino
	nMotorEncoder(LEFT_MOT_PORT) = 0;
	setDriveTrainSpeed(KNOCK_SPEED);
	while(nMotorEncoder(LEFT_MOT_PORT) < distToDeg(DIST_BETWEEN_DOMINOS-0.5))
	{}
	setDriveTrainSpeed(0);
	stopAllTasks();
}

void openDoor() // Henrique
{
	motor[DOOR_MOT_PORT] = DOOR_SPEED;
	while (nMotorEncoder(DOOR_MOT_PORT)<DOOR_ANG)
	{
		// check for break conditions
		if(SensorValue[TOUCH_PORT])
		{
			motor[DOOR_MOT_PORT] = 0;
			stopAndKnock();
		}
	}
	motor[DOOR_MOT_PORT] = 0;
}

void closeDoor() // Henrique
{
	if(!nMotorEncoder(DOOR_MOT_PORT)<5)
	{
		motor[DOOR_MOT_PORT] = -1*DOOR_SPEED;
		while (nMotorEncoder(DOOR_MOT_PORT)>5)
		{
			// check for break conditions
			if(SensorValue[TOUCH_PORT])
			{
				motor[DOOR_MOT_PORT] = 0;
				stopAndKnock();
			}
		}
		motor[DOOR_MOT_PORT] = 0;
	}
}