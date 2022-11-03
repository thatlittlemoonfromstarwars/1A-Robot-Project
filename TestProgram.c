void configureSensors();
task main()
{



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
