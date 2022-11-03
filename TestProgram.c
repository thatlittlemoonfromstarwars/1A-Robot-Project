void configureSensors();


task main()
{
bool piston_arm = false;
bool door = false;
bool drive = false;
const int PISTON_CODE_POWER = 75;
const int DOOR_POWER = 10;
const int DOOR_UP_LIMIT = 720;
const int DOOR_DOWN_LIMIT = 0;
const int DRIVE_CODE_POWER = 25;

displayString(5, "Press enter for piston arm");
displayString(7, "Press left for door");
displayString(9, "Press Right for drive forward");


while(!getButtonPress(buttonAny))
{}

if(getButtonPress(buttonEnter))
{
	piston_arm = true;
}
if(getButtonPress(buttonLeft))
{
	door = true;
}
if(getButtonPress(buttonRight))
{
	drive = true;
}
while(getButtonPress(buttonAny))
{}

displayString.clear();

if(piston_arm)
{
	displayString(5, "Press down for 5% power");
	displayString(7, "Press left for 25% power");
	displayString(9, "press enter for code power");
	displayString(11, "Press right for 50% power");
	displayString(13, "Press up for 100% power");

	while(!getButtonPress(buttonAny))
	{}

	if(getButtonPress(buttonDown))
	{
		motor[motorB] = 5;
	}
	if(getButtonPress(buttonLeft))
	{
		motor[motorB] = 25;
	}
	if(getButtonPress(buttonEnter))
	{
		motor[motorB] = PISTON_CODE_POWER;
	}
	if(getButtonPress(buttonRight))
	{
		motor[motorB] = 50;
	}
	if(getButtonPress(buttonUp))
	{
		motor[motorB] = 100;
	}

	wait1Msec(1000)
	displayString.clear();
	displayString(7, "Press enter to stop");
	while(!getButtonPress(buttonEnter))
	{}
	motor[motorB] = 0;
}


if(door)
{
	displayString(7, "Press enter to stop");

	while(!getButtonPress(buttonEnter))
	{
		motor[motorC] = DOOR_POWER;
		while(nMotorEncoder[motorC] < DOOR_UP_LIMIT)
		{}

		motor[motorC] = -DOOR_POWER;
		while(nMotorEncoder[motorC] > DOOR_DOWN_LIMIT)
		{}
	}
	motor[motorC] = 0;
	displayString.clear();
}


if(drive)
{
	displayString(5, "Press down for 5% power");
	displayString(7, "Press left for 25% power");
	displayString(9, "press enter for code power");
	displayString(11, "Press right for 50% power");
	displayString(13, "Press up for 100% power");

	while(!getButtonPress(buttonAny))
	{}

	if(getButtonPress(buttonDown))
	{
		motor[motorA] = motor[motorD] = 5;
	}
	if(getButtonPress(buttonLeft))
	{
		motor[motorA] = motor[motorD] = 25;
	}
	if(getButtonPress(buttonEnter))
	{
		motor[motorA] = motor[motorD] = DRIVE_CODE_POWER;
	}
	if(getButtonPress(buttonRight))
	{
		motor[motorA] = motor[motorD] = 50;
	}
	if(getButtonPress(buttonUp))
	{
		motor[motorA] = motor[motorD] = 100;
	}

	wait1Msec(1000);
	displayString.clear();
	displayString(7, "Press enter to stop");
	while(!getButtonPress(buttonEnter))
	{}
	motor[motorA] = motor[motorB] = 0;
}


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
