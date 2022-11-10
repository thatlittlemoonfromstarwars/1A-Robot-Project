void configureAllSensors();

const int PISTON_POWER = 75;
const int PISTON_CYCLE_POWER = 25;
const int POSITION_2_ENC = -300;
const int POSITION_3_ENC = -400;
const int DOOR_POWER = 25;
const int DOOR_UP_LIMIT = 100;
const int DOOR_DOWN_LIMIT = 0;
const int DRIVE_CODE_POWER = 25;
const type PUSH_MOTOR = "motorC"; //change type
const type DOOR_MOTOR = "motorB"; //change type


task main()
{
	bool piston_arm = false;
	bool door = false;
	bool drive = false;

	displayString(5, "Press enter for piston arm");
	displayString(7, "Press left for door");
	displayString(9, "Press Right for drive forward");

	configureAllSensors();
	
	while(true)
	{
		while(true)
		{
			if(getButtonPress(buttonEnter))
			{
				piston_arm = true;
				break;
			}
			else if(getButtonPress(buttonLeft))
			{
				door = true;
				break;
			}
			else if(getButtonPress(buttonRight))
			{
				drive = true;
				break;
			}
		}
		while(getButtonPress(buttonAny))
		{}

		eraseDisplay();

		if(piston_arm)
		{
			displayString(5, "Press Down for Position 1");
			displayString(7, "Press Enter for Position 2");
			displayString(9, "Press Up for position 3");
			displayString(11, "Press Right for cycle");
			displayString(13, "Press left to break");
			
			nMotorEncoder(PUSH_MOTOR) = 0;

			while(true)
			{
				while(!getButtonPress(buttonAny))
				{}

				if(getButtonPress(buttonDown))
				{
					motor[PUSH_MOTOR] = PISTON_POWER;
					while (nMotorEncoder(PUSH_MOTOR) < 0)
					{}
					motor[PUSH_MOTOR] = 0;
				}
				if(getButtonPress(buttonEnter))
				{
					motor[PUSH_MOTOR] = -PISTON_POWER;
					while (nMotorEncoder(PUSH_MOTOR) > POSITION_2_ENC)
					{}
					motor[PUSH_MOTOR] = 0;
				}
				if(getButtonPress(buttonEnter))
				{
					motor[PUSH_MOTOR] = -PISTON_POWER;
					while (nMotorEncoder(PUSH_MOTOR) > POSITION_3_ENC)
					{}
					motor[PUSH_MOTOR] = 0;
				}
				//cycle
				if(getButtonPress(buttonRight))
				{
					eraseDisplay();
					displayString(7, "Press Enter to break");
					
					while(!getButtonPress(buttonEnter))
					{
						if(!getButtonPress(buttonEnter))
						{
							motor[PUSH_MOTOR] = PISTON_CYCLE_POWER;
							while (nMotorEncoder(PUSH_MOTOR) < 0)
							{
								if(getButtonPress(buttonEnter))
								{
									break;
								}
							}
							motor[PUSH_MOTOR] = 0;
						}
						
						if(!getButtonPress(buttonEnter))
						{
							motor[PUSH_MOTOR] = -PISTON_CYCLE_POWER;
							while (nMotorEncoder(PUSH_MOTOR) > POSITION_2_ENC)
							{
								if(getButtonPress(buttonEnter))
								{
									break;
								}
							}
							motor[PUSH_MOTOR] = 0;
						}
						
						if(!getButtonPress(buttonEnter))
						{
							motor[PUSH_MOTOR] = -PISTON_CYCLE_POWER;
							while (nMotorEncoder(PUSH_MOTOR) > POSITION_3_ENC)
							{
								if(getButtonPress(buttonEnter))
								{
									break;
								}
							}
							motor[PUSH_MOTOR] = 0;
						}
					}
				}
				//cycle
				
				if(getButtonPress(buttonLeft))
				{
					break;
				}
				while(getButtonPress(buttonAny))
				{}
			}
			eraseDisplay();
		}
		
		else if(door)
		{
			displayString(7, "Press enter to stop");
			nMoterEncoder[DOOR_MOTOR] = 0;

			while(!getButtonPress(buttonEnter))
			{
				motor[DOOR_MOTOR] = DOOR_POWER;
				while(nMotorEncoder(DOOR_MOTOR) < DOOR_UP_LIMIT)
				{}

				motor[DOOR_MOTOR] = -DOOR_POWER;
				while(nMotorEncoder(DOOR_MOTOR) > DOOR_DOWN_LIMIT)
				{}
			}
			motor[DOOR_MOTOR] = 0;
			eraseDisplay();

		}
		else if(drive)
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
			eraseDisplay();
			displayString(7, "Press enter to stop");
			while(!getButtonPress(buttonEnter))
			{}
			motor[motorA] = motor[PUSH_MOTOR] = 0;
		}
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
