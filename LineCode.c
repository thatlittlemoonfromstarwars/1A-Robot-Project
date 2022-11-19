#include "mindsensors-ev3smux.h"
#include "UW_sensorMux.c"

void configureAllSensors();

/*const int PISTON_POWER = 15;
const int PISTON_CYCLE_POWER = 15;
const int POSITION_2_ENC = -300;
const int POSITION_3_ENC = -500;
const int DOOR_POWER = 25;
const int DOOR_UP_LIMIT = 100;
const int DOOR_DOWN_LIMIT = 0;
const int DRIVE_CODE_POWER = -25;*/
const int dominoCount = 30;
const int MUX_WAIT = 10;

//C2 is left
//C1 is right

void configureAllSensors()
{
	SensorType[S3] = sensorEV3_Touch;
	SensorType[S2] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorType[S4] = sensorEV3_Ultrasonic;
	wait1Msec(50);
	SensorMode[S2] = modeEV3Gyro_Calibration;
	wait1Msec(50);

	SensorType[S1] = sensorEV3_GenericI2C;
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
	SensorMode[S2] = modeEV3Gyro_RateAndAngle;
	wait1Msec(50);
}

void followLine()
{
	time1[T1] = 0;
	int index = 0;
	int index2 = 0;
	int sensor1 = 0;
	int sensor2 = 0;

/*	while(true)
	{

		if(time1[T1] > index)
		{
			sensor1 = readMuxSensor(msensor_S1_1);
			index = time1[T1] + MUX_WAIT;

			if(sensor1 == (int) colorBlack)
			{
				displayClearTextLine(2);
				displayString(2, "Black");
			}
			else
			{
				displayClearTextLine(2);
				displayString(2, "Not Black");
			}

		}

		if(time1[T1] > index2)
		{
			sensor2 = readMuxSensor(msensor_S1_2);
			index2 = time1[T1] + MUX_WAIT+ 5;

			if(sensor2 == (int) colorBlack)
			{
				displayClearTextLine(4);
				displayString(4, "Black");
			}
			else
			{
				displayClearTextLine(4);
				displayString(4, "Not Black");
			}

		}
	}
}
*/

	while(dominoCount>0)
	{
		motor[motorA] = motor[motorD] = -10;

		if(time1[T1] > index)
		{
			sensor1 = readMuxSensor(msensor_S1_1);
			index = time1[T1] + MUX_WAIT;

			if(sensor1 == (int) colorBlack)
			{
				motor[motorD] = -5;
			}
		}

		if(time1[T1] > index2)
		{
			sensor2 = readMuxSensor(msensor_S1_2);
			index2 = time1[T1] + MUX_WAIT+ 5;

			if(sensor2 == (int) colorBlack)
			{
				motor[motorA] = -5;
			}
		}

		if(SensorValue(S4))
		{
			return;
		}
	}
}

task main()
{
	configureAllSensors();
	followLine();
}
