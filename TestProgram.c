#include "mindsensors-ev3smux.h"

void configureAllSensors();

const int PISTON_POWER = 15;
const int PISTON_CYCLE_POWER = 15;
const int POSITION_2_ENC = -300;
const int POSITION_3_ENC = -500;
const int DOOR_POWER = 25;
const int DOOR_UP_LIMIT = 100;
const int DOOR_DOWN_LIMIT = 0;
const int DRIVE_CODE_POWER = -25;
const int dominoCount = 30;
//const string PUSH_MOTOR = "motorC"; //change type
//const string DOOR_MOTOR = "motorB"; //change type

void configureAllSensors()
{
	SensorType[S3] = sensorEV3_Touch;
	SensorType[S2] = sensorEV3_Gyro;
	wait1Msec(50);
///	SensorType[S1] = sensorEV3_Color;
//	wait1Msec(50);
	SensorType[S4] = sensorEV3_Ultrasonic;
	wait1Msec(50);
	SensorMode[S2] = modeEV3Gyro_Calibration;
	wait1Msec(50);
//	SensorMode[S1] = modeEV3Color_Color;
//	wait1Msec(100);
	SensorMode[S2] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);
}

int followLine()
{
	while(dominoCount>0)
	{
		motor[MotorA] = motor[motorD] = 10;

		if(SensorValue[S3]==1)
		{
			return 1;
		}
		while(SensorValue[msensor_S1_1]==(int)colorBlack)
		{
			motor[motorA]= 0;
		}
		motor[MotorA] = motor[motorD] = 10;
		while(SensorValue[msensor_S1_2]==(int)colorBlack)
		{
			motor[motorD] = 0;
		}
		motor[MotorA] = motor[motorD] = 10;
	}
}

task main()
{

}
