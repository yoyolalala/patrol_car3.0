#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "motor.h"
#include "math.h"
#include "encoder.h"
u8 i,len;
u16 inputEncoderVal;
extern int leftEncoderVal;
void setup()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	LED_Init();
	uart_init(115200);//��ʼ�����ڲ�����Ϊ115200
 	motorInit();
	Encoder_Init_TIM4();
	TIM2_Int_Init(499, 8399); //84Mhz/8400
}
int main(void)
{ 
	setup();
	motor1SetPct(70); //Խ�� ת��Խ��  70-38    30-130
    while(1) 
	{
		if(USART_RX_STA&0x8000)
		{
			inputEncoderVal = 0;
			len = USART_RX_STA & 0x3fff;
			for (i = 0; i< len;i++)
			{
				inputEncoderVal= inputEncoderVal + (USART_RX_BUF[i] - 0x30)*pow(10, len - i - 1);
				while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
			}
			USART_RX_STA = 0;
		//    setDesiredPoint(inputEncoderVal);
			printf("desiredVal: %d\r\n",inputEncoderVal);
		}
		printf("leftEncoderVal:%d\r\n",leftEncoderVal);
		printf("ok\r\n");
		ledToggle();
	}
}