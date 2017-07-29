#include "usart_vcan.h"
#define cmd 3
void usart1_vcanInit(u32 bound)
{
	uart1_init(bound);
}
void sendFloatCH1Oscilloscope(float datax)
{
	union 
	{
		float data;
		u8 c[sizeof(datax)];
	}dataBuf;
	dataBuf.data=datax;
	USART1_Write(cmd);
	USART1_Write(~cmd);
	
	for(int i=0;i<sizeof(datax);i++)
		USART1_Write(dataBuf.c[i]);
	
	USART1_Write(~cmd);
	USART1_Write(cmd);
}
void sendU8CH1Oscilloscope(u8 data)
{
	USART1_Write(cmd);
	USART1_Write(~cmd);
	
	USART1_Write(data);
	
	USART1_Write(~cmd);
	USART1_Write(cmd);
}