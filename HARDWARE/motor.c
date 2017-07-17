#include "motor.h"
void motorInit(void)
{
	TIM3_PWM_Init(99,83);	//pwm 10khz
}
void motor1SetPct(u8 pct1)
{
	TIM_SetCompare4(TIM3, pct1); 
}
//void motor2SetPct(u8 pct2)
void stopMotor()
{
	motor1SetPct(0);
}