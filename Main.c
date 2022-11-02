/*
Domino layer path follower

v1.0

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

task main()
{
	const int MAX_DOMINO_LOAD = 60;

	configureAllSensors();
	int dominoCount = MAX_DOMINO_LOAD;
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

