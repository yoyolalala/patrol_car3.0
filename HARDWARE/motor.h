#pragma once
#include "pwm.h"
void motorInit(void);
void motor1SetPct(u8 pct1); //pctȡֵ��0��99
void motor1SetPctback(u8 pct1);
void motor2SetPct(u8 pct2);
void motor2SetPctback(u8 pct2);
void stopMotor();
// MOTOR_H