#include "motor.h"
void motorInit(void)
{
	motor1_TIM1_PWM_Init(99,167);	//pwm 10khz
	motor2_TIM1_PWM_Init(99,167);	//pwm 10khz
}
void motor1SetPct(u8 pct1)
{
	TIM_SetCompare4(TIM1, pct1); 
	TIM_SetCompare1(TIM9, 99); 
}
void motor2SetPct(u8 pct2)
{
	TIM_SetCompare1(TIM1, pct2);
	TIM_SetCompare2(TIM9, 99); 
}
void stopMotor()
{
	motor1SetPct(99);
	motor2SetPct(99);
}
void motor1SetPctback(u8 pct1)
{
	TIM_SetCompare4(TIM1,99); 
	TIM_SetCompare1(TIM9,pct1); 
}
void motor2SetPctback(u8 pct2)
{
	TIM_SetCompare1(TIM1, 99);
	TIM_SetCompare2(TIM9, pct2); 
}