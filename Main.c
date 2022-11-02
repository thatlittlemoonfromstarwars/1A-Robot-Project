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
3 - colour
4 - ultrasonic
*/
void configureAllSensors();
void dispenseDomino();
void followLine();
void navFromCoords();
void calcPath();

task main()
{
	motor[motorA] = motor[motorD] = 50;
	wait1Msec(5000000000);
}
