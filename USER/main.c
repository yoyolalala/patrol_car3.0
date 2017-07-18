#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "motor.h"
#include "math.h"
#include "encoder.h"
#include "PID.h"
#define INF 3.402823466e+38F
u8 pct1=70;
u8 i,len;
u16 inputEncoderVal=0;
extern int leftEncoderVal;
extern float error;
extern float set_point;
float pidOutput;
void setup()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数 SYSCLOCK
	LED_Init();
	uart_init(115200);//初始化串口波特率为115200
 	motorInit();
	Encoder_Init_TIM4();
	TIM2_Int_Init(499, 8399); //84Mhz/8400
	
	pidInit(0.01,pct1-1,pct1-99);//初始化刷新间隔 上下限
}
int main(void)
{ 
	setup();
	motor1SetPct(pct1); //越大 转速越慢  70-38    30-130
	delay_ms(1000);
	setWeights(0.42, 0.01, 0.0001); //
    while(1) 
	{ 
		if(USART_RX_STA&0x8000)
		{
			len = USART_RX_STA & 0x3fff;
			for (i = 0; i< len;i++)
			{
				inputEncoderVal=inputEncoderVal+(USART_RX_BUF[i] - 0x30)*pow(10, len - i - 1);
				while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
			}
			USART_RX_STA = 0;
			setDesiredPoint(inputEncoderVal);
			printf("desiredVal: %d\r\n",inputEncoderVal);
			inputEncoderVal=0;
			printf("\r\n");
			printf("set_point:%f\r\n",set_point);
		    printf("leftEncoderVal:%d\r\n",leftEncoderVal);
			pidOutput=refresh(leftEncoderVal);
			printf("error:%f\r\n",error);
			printf("pidOutput:%f\r\n",pidOutput);
			motor1SetPct(pct1-pidOutput);
			pct1=pct1-pidOutput;
			printf("\r\n");
		}
		printf("leftEncoderVal:%d\r\n",leftEncoderVal);
		ledToggle();
	}
}