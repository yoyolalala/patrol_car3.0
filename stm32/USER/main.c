#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "motor.h"
#include "math.h"
#include "encoder.h"
#include "PID.h"
#include "servo.h"
#include "stdbool.h"
#define PI 3.1415936
#define DIAMETER 62 //����ֱ��62mm
//#define INF 3.402823466e+38F
u8 pct1=70;
u8 pct2=70;
u8 i,len; //���ڽ���������
u16 inputEncoderVal=0; 
u16 dis;//�߹�����
bool isBackStraight=false;
bool isBackBegin=false; //�Ƿ�ʼ�������
bool isForwardStraight=false;
bool isForwardBegin=false; 
extern int circle;
extern int leftEncoderVal;
extern int rightEncoderVal;
extern float error;
extern float set_point;
float pidOutput;
void setup()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);  //��ʼ����ʱ���� SYSCLOCK
	LED_Init();
	rasLED_Init();
	motorInit();
	uart1_init(38400);//���ڳ�ʼ��������ڵ����ʼ����
	uart2_init(115200);
	Encoder1_Init_TIM4();
	Encoder2_Init_TIM3();
	TIM2_Int_Init(499, 8399); //0.05s
	servoPwmInit(99,2766);
	pidInit(0.01,pct1-1,pct1-99);//��ʼ��ˢ�¼�� ������
}
void backStraight(u8 pct1,u8 pct2)
{
	isBackStraight=true;
	
	motor1SetPctback(pct1);
	setDesiredPoint(leftEncoderVal);
	pidOutput=refresh(rightEncoderVal);
	if(leftEncoderVal<rightEncoderVal)
	{	 motor2SetPctback(pct2+pidOutput);
		 pct2=pct2+pidOutput;
	}
	else 
	{	motor2SetPctback(pct2-pidOutput);
		pct2=pct2-pidOutput;
	}
}
void forwardStraight(u8 pct1,u8 pct2)
{
	isForwardStraight=true;
	
	motor1SetPct(pct1);
	setDesiredPoint(leftEncoderVal);
	pidOutput=refresh(rightEncoderVal);
	if(leftEncoderVal<rightEncoderVal)
	{	 motor2SetPct(pct2+pidOutput);
		 pct2=pct2+pidOutput;
	}
	else 
	{	motor2SetPct(pct2-pidOutput);
		pct2=pct2-pidOutput;
	}
}
int main(void)
{ 
	setup();
    setServoDegree(60);// ռ�ձȴ�43-60��60��� Ϊ��������ͷ�ĳ�ʼλ��
	delay_ms(1000);
	//motor1SetPct(pct1);//Խ�� ת��Խ��  70-33    30-116
	//motor2SetPct(pct2);
	stopMotor();
	//backStraight(70,70);
	setWeights(0.42, 0.1, 0.001); //p�ٴ�һ��
    while(1) 
	{   
		if(USART1_RX_STA&0x8000) //mark1
		{
			len = USART1_RX_STA & 0x3fff; //mark2
			for (i = 0; i< len;i++)
			{
				inputEncoderVal=inputEncoderVal+(USART_RX_BUF[i] - 0x30)*pow(10, len - i - 1);
				while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
			}
			USART1_RX_STA = 0; //mark3
			//setDesiredPoint(inputEncoderVal);
			//printf("desiredVal: %d\r\n",inputEncoderVal);
			//inputEncoderVal=0;
			//printf("\r\n");
			//printf("set_point:%f\r\n",set_point);
		    //printf("leftEncoderVal:%d\r\n",leftEncoderVal);
			//pidOutput=refresh(leftEncoderVal);
			//printf("error:%f\r\n",error);
			//printf("pidOutput:%f\r\n",pidOutput);
			//motor1SetPct(pct1-pidOutput);
			//pct1=pct1-pidOutput;
		}
		if(inputEncoderVal==1) //�Ӵ�������1 ��ʼ�������
		{
			isBackBegin=true;
			printf("begin computing distance!!\r\n");
			inputEncoderVal=0;
		}
		while(isBackBegin)
		{   
			backStraight(60,60);
			dis=circle*PI*DIAMETER;
			if(dis>=100)
			{
				stopMotor();
				isBackStraight=false;
				isBackBegin=false;
				circle=0;
			}	
			else 
			{
				printf("leftEncoderVal:%d\r\n",leftEncoderVal);
				printf("rightEncoderVal:%d\r\n\r\n",rightEncoderVal);
			}
	    }
		printf("leftEncoderVal:%d\r\n",leftEncoderVal);
		printf("rightEncoderVal:%d\r\n\r\n",rightEncoderVal);
		ledToggle();
		rasLedToggle();
	}
}