task main()
{
    motor[motorA] = motor[motorD] = 50;
    wait1Msec(5000);
    motor[motorA] = motor[motorD] = 0;
}