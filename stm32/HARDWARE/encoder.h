#ifndef __ENCODER_H
#define __ENCODER_H
#include <sys.h>	 
#include "stdbool.h"
#define ENCODER_TIM_PERIOD (u16)(65535)   //���ɴ���65535 ��ΪF103�Ķ�ʱ����16λ�ġ�
void Encoder1_Init_TIM4(void);
void Encoder2_Init_TIM3(void);
void TIM4_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM2_Int_Init(u16 arr, u16 psc);
void TIM2_IRQHandler(void);
#endif
