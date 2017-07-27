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
u8 pct2=70; //��ʼ�����ռ�ձ�
u8 i,len; //���ڽ���������
u16 receivedData;//���ڽ��յ���ݮ�ɵ�����
u16 dis;//�߹�����
int dataNum=0;//�ж���ݮ�ɽ������ݵ�����
u8 rxData[2];//�����洢����λ�õ�����
u16 center;//���յ������е�����
bool isBackStraight=false;
bool isBackBegin=false; //�Ƿ�ʼ�������
bool isForwardStraight=false;
bool isUseCamera=true;//δ��ʼʶ��ץȡС��ʱ ��������ͷѲ��
float pidOutput;
extern int circle;
extern int leftEncoderVal;
extern int rightEncoderVal;
//extern float error;
//extern float set_point;
typedef enum
{
	findLine=1,
	cross=2,
	check=3,
}runningState_Typedef;
runningState_Typedef runningState;
void setup()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);  //��ʼ����ʱ���� SYSCLOCK
	LED_Init();
	rasLED_Init();
	tmpLED_Init();
	motorInit();
	uart1_init(115200);//���ڳ�ʼ��������ڵ����ʼ����
	uart2_init(38400);//����1������ݮ�ɽ������� ����2����λ����������
	Encoder1_Init_TIM4();
	Encoder2_Init_TIM3();
	TIM2_Int_Init(499, 8399); //0.05s
	servoPwmInit(99,2766);
	pidInit(0.01,pct2-1,pct2-99);//��ʼ��ˢ�¼�� ������
}
void backStraight(u8 pct1,u8 pct2)
{
	setWeights(0.425, 0.16, 0.001);//ֻװ���̺���ʱ����PID����
	isBackStraight=true;
	
	motor1SetPctback(pct1);
	setDesiredPoint(leftEncoderVal);
	pidOutput=refresh(rightEncoderVal);
	if(leftEncoderVal<rightEncoderVal)
	{	 motor2SetPctback(pct2-pidOutput); //pidOutputΪ��ֵ
		 pct2=pct2-pidOutput;
	}
	else 
	{	motor2SetPctback(pct2+pidOutput);
		pct2=pct2+pidOutput;
	}
}
void forwardStraight(u8 pct1,u8 pct2)
{
	setWeights(0.46, 0.17, 0.001);
	motor1SetPct(pct1);
	setDesiredPoint(abs(leftEncoderVal));
	pidOutput=refresh(abs(rightEncoderVal));
	if(abs(leftEncoderVal)<abs(rightEncoderVal))
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
	delay_ms(100);
	//motor1SetPct(pct1);//Խ�� ת��Խ��  70-33    30-116
	//motor2SetPct(pct2);
	stopMotor();
	//backStraight(70,70);
    while(1) 
	{   
		if(isUseCamera)
		{
			setServoDegree(60);// ռ�ձȴ�43-60��60��� Ϊ��������ͷ�ĳ�ʼλ��
		}
		if(USART2_RX_STA&0x8000) //
		{
			tmpLedToggle();
			rasLedToggle();//PD4ָʾ����˸ 32�н��յ���ݮ������
			len = USART2_RX_STA & 0x3fff; //
			for (i = 0; i< len;i++)
			{
				receivedData=receivedData+(USART_RX_BUF[i] - 0x30)*pow(10, len - i - 1);
				while (USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);//
			}
			USART2_RX_STA = 0;//
			
			if(dataNum==0 & receivedData==1)
			{
				runningState=findLine;
			}else if(dataNum==0 & receivedData==2)
			{
				runningState=cross;
			}else if(dataNum==0 & receivedData==3)
			{
				runningState=check;
			}
			
			if(dataNum==1)
			{
				rxData[0]=receivedData;
			}else if(dataNum==2)
			{
				rxData[1]=receivedData;
			    dataNum=-1;
				center=rxData[0]*256+rxData[1];
			}
			dataNum++;
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
		if(receivedData==1) //�Ӵ�������1 ��ʼ�������
		{
			isBackBegin=true; 
			printf("begin computing distance!!\r\n");
			receivedData=0;
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
	    printf("leftEncoderVal:%d\r\n",leftEncoderVal);//�ض��崮��2��printf
		printf("rightEncoderVal:%d\r\n\r\n",rightEncoderVal);
		ledToggle();
	}
}