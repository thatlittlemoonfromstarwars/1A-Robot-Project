/*
Domino layer path follower
Sean Atiken, Henrique Engelke, Josh Morcombe, and Andor Siegers

v1.0

Assumptions:


Motor Ports:
A - left drive wheel
B - dispenser motor
C - gate motor
D - right drive wheel

Sensor Ports:
1 - touch
2 - gyro
3 - color
4 - ultrasonic


*/

void configureAllSensors();
void dispenseDomino();
void followLine(); // Sean
void followPathFromFile(); // Andor
void driveToStartLocation(); // Andor
void calcPath(); // Andor
float calcModulus(int x1, int x2);
int calcAngle(int x1, int x2, int y1, int y2);
int calcLength(int x1, int x2, int y1, int y2);

task main()
{
	const int DOMINOS_AT_MAX_LOAD = 60;

	configureAllSensors();
	int dominoCount = DOMINOS_AT_MAX_LOAD;
}

void configureAllSensors()
{
	SensorType[S1] = sensorEV3_Touch;
	SensorType[S2] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorType[S3] = sensorEV3_Color;
	wait1Msec(50);
	SensorType[S4] = sensorEV3_Ultrasonic;
	wait1Msec(50);
	SensorMode[S2] = modeEV3Gyro_Calibration;
	wait1Msec(50);
	SensorMode[S3] = modeEV3Color_Color;
	wait1Msec(100);
	SensorMode[S2] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);
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
