#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/16
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

void motor1_TIM1_PWM_Init(u32 arr,u32 psc);
void motor2_TIM1_PWM_Init(u32 arr,u32 psc);
void motor1_TIM9_PWM_Init(u32 arr,u32 psc);
void motor2_TIM9_PWM_Init(u32 arr,u32 psc);
#endif