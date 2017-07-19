#include "motor.h"
void motorInit(void)
{
	motor1_TIM1_PWM_Init(99,167);	//pwm 10khz
	motor2_TIM1_PWM_Init(99,167);	//pwm 10khz
}
void motor1SetPct(u8 pct1)
{
	TIM_SetCompare4(TIM1, pct1); 
}
void motor2SetPct(u8 pct2)
{
	TIM_SetCompare1(TIM1, pct2);
}
void stopMotor()
{
	motor1SetPct(99);
	motor2SetPct(99);//µÁ—π0.4V
}