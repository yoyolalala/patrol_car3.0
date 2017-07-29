#ifndef __USART_VCAN_H
#define __USART_VCAN_H
#include "sys.h"	
#include "usart.h"
void usart1_vcanInit(u32 bound);
void sendFloatCH1Oscilloscope(float data);
void sendU8CH1Oscilloscope(u8 data);
#endif