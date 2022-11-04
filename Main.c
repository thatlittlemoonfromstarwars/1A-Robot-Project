/*
Domino layer path follower
Sean Aitken, Henrique Engelke, Josh Morcombe, and Andor Siegers

v1.0

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

void dispenseDomino(); // Henrique
void stopAndKnock(); // Josh??
void objectInTheWay(); // stops and informs the user to move the object in the way
void driveDist(int mot_pow, float dist);
void driveDistWhileDispensing(int mot_pow, float dist);
void setDriveTrainSpeed(int speed);

void followLine(); // Sean

void followPathFromFile(); // Andor
void driveToStartLocation(); // Andor

// calculation functions
void calcPath(); // Andor
float calcModulus(int x1, int x2);
int calcAngle(int x1, int x2, int y1, int y2);
int calcLength(int x1, int x2, int y1, int y2);
int distToDeg(float dist);
float degToDist(int deg);

// constants
const float WHEEL_RAD = 2.75; // in cm
const int DOMINOS_AT_MAX_LOAD = 60;
const float DIST_BETWEEN_DOMINOS = 3.6; // in cm

task main()
{

	configureAllSensors();
	int dominoCount = DOMINOS_AT_MAX_LOAD;
}

void configureAllSensors()
{
	SensorType[S3] = sensorEV3_Touch;
	SensorType[S2] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorType[S1] = sensorEV3_Color;
	wait1Msec(50);
	SensorType[S4] = sensorEV3_Ultrasonic;
	wait1Msec(50);
	SensorMode[S2] = modeEV3Gyro_Calibration;
	wait1Msec(50);
	SensorMode[S1] = modeEV3Color_Color;
	wait1Msec(100);
	SensorMode[S2] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);
}

void driveDist(int mot_pow, float dist) // input negative motor power for backwards
{
	setDriveTrainSpeed(mot_pow);
	nMotorEncoder[motorA] = 0;
	while(abs(nMotorEncoder[motorA]) < distToDeg(dist))
	{}
	setDriveTrainSpeed(0);
}

void driveDistWhileDispensing(int mot_pow, int dist)
{

}

float calcModulus(int x1, int x2)
{
	return sqrt(pow(x1,2) + pow(x2,2));
}

int calcAngle(int x1, int x2, int y1, int y2)
{
	return acos((x1*y1 + x2*y2)/(calcModulus(x1,x2)*calcModulus(y1,y2)));
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
