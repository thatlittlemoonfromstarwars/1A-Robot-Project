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
const int COLOR_SENSOR_PORT = 1
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

void configureLineFollower()
{
	tMSEV3 muxedSensor[2];

	tEV3SensorTypeMode typeMode[2] = {colorMeasureColor, colorMeasureColor};

	case colorMeasureColor:
		displayTextLine(i*2, "Chan[%d]: Color", i+1);
		displayTextLine(i*2 + 1, "Color: %d", muxedSensor[i].color);
		break;

	case colorMeasureColor:
		displayTextLine(i*2, "Chan[%d]: Color", i+1);
		displayTextLine(i*2 + 1, "Color: %d", muxedSensor[i].color);
			break;
}

task main()
{


}
