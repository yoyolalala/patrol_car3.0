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
#define kp 0.11 //Ѳ��ʱλ��PID����
#define kd 0.15
//#define INF 3.402823466e+38F
//u8 i,len; //����2����������
u16 dis;//�߹�����
u16 center;//���յ������е�����
u16 lastDiff=0;
u8 pct1=25;
u8 pct2=25;//Ѳ��ʱ�����ֳ�ʼת��
bool isBackStraight=false;
bool isBackBegin=false; //�Ƿ�ʼ�������
bool isUseCamera=true;//δ��ʼʶ��ץȡС��ʱ ��������ͷѲ��
float pidOutput;
extern int circle;
extern int leftEncoderVal;
extern int rightEncoderVal;
typedef enum
{
	findLine= 1,
	lostLine= 2,
	check= 3,
}runningState_Typedef;
runningState_Typedef runningState;
void setup()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);  //��ʼ����ʱ���� SYSCLOCK
	LED_Init();
	motorInit();
	uart1_init(115200);//���ڳ�ʼ��������ڵ����ʼ����
	uart2_init(38400);//����1������ݮ�ɽ������� ����2����λ����������
	Encoder1_Init_TIM4();
	Encoder2_Init_TIM3();
	TIM2_Int_Init(499, 8399); //0.05s
	servoPwmInit(99,2766);
}
void backStraight(u8 pct1,u8 pct2)
{
	static int pwmRight = 30;
	pidInit(0.01,pct2-1,pct2-99);//��ʼ��ˢ�¼�� ������
	
	setWeights(0.25, 0.0165, 0.001);
	isBackStraight=true;
	
	motor1SetPctback(pct1);//Խ�� ת��Խ��  70-33    30-116
	setDesiredPoint(leftEncoderVal);
	pidOutput=refresh(rightEncoderVal);
	pwmRight = - pidOutput + pwmRight;
	if(pwmRight > 99)
		pwmRight = 99;
	if(pwmRight < 0)
		pwmRight = 0;
	motor2SetPctback(pwmRight);
}
void forwardStraight(u8 pct1,u8 pct2)
{
	static int pwmRight = 30;
	pidInit(0.01,pct2-1,pct2-99);//��ʼ��ˢ�¼�� ������
	
	setWeights(0.246, 0.0165, 0.001);
	
	motor1SetPct(pct1);//Խ�� ת��Խ��  70-33    30-116
	setDesiredPoint(abs(leftEncoderVal));
	pidOutput=refresh(abs(rightEncoderVal));
	pwmRight = - pidOutput + pwmRight;
	if(pwmRight > 99)
		pwmRight = 99;
	if(pwmRight < 0)
		pwmRight = 0;
	motor2SetPct(pwmRight);
}
void setLeftPID(int leftDesiredEncoderVal)
{
	pidInit(0.01,pct1-1,pct1-99);
	setWeights(0.232, 0.0165, 0.001);
	motor1SetPct(pct1);
	setDesiredPoint(leftDesiredEncoderVal);
	pidOutput = refresh(abs(leftEncoderVal));
	pct1 = - pidOutput + pct1; 
	if(pct1 > 99)
		pct1=99;
	if(pct1 < 0)
		pct1 = 0;
	motor1SetPct(pct1);
}
void setLeftBackPID(int leftDesiredEncoderVal)
{
	pidInit(0.01,pct1-1,pct1-99);
	setWeights(0.232, 0.0165, 0.001);
	motor1SetPctback(pct1);
	setDesiredPoint(leftDesiredEncoderVal);
	pidOutput = refresh(abs(leftEncoderVal));
	pct1 = - pidOutput + pct1; 
	if(pct1 > 99)
		pct1=99;
	if(pct1 < 0)
		pct1 = 0;
	motor1SetPctback(pct1);
}
void setRightBackPID(int rightDesiredEncoderVal)
{
	pidInit(0.01,pct2-1,pct2-99);
	setWeights(0.232, 0.0165, 0.001);
	motor2SetPctback(pct2);
	setDesiredPoint(rightDesiredEncoderVal);
	pidOutput = refresh(abs(rightEncoderVal));
	pct2 = - pidOutput + pct2;
	if(pct2 > 99)
		pct2=99;
	if(pct2 < 0)
		pct2 = 0;
	motor2SetPctback(pct2);
}
void setRightPID(int rightDesiredEncoderVal)
{
	pidInit(0.01,pct2-1,pct2-99);
	setWeights(0.232, 0.0165, 0.001);
	motor2SetPct(pct2);
	setDesiredPoint(rightDesiredEncoderVal);
	pidOutput = refresh(abs(rightEncoderVal));
	pct2 = - pidOutput + pct2;
	if(pct2 > 99)
		pct2=99;
	if(pct2 < 0)
		pct2 = 0;
	motor2SetPct(pct2);
}
int main(void)
{ 
	setup();
	delay_ms(100);
    while(1)
	{
		if(USART1_RX_STA&0x8000)
		{
			runningState=USART_RX_BUF[0];
			center=USART_RX_BUF[1]*256+USART_RX_BUF[2];
			USART1_RX_STA = 0;
			/*printf("state:%d\r\n",runningState);
			printf("center:%d\r\n",center);
			printf("\r\n\r\n");*/
		}
		if(isUseCamera)
		{
			setServoDegree(38);//49���ȡ��λ�� 38Ϊ��������ͷ�ĳ�ʼλ�� 45���ˮƽ
		}
		if(isUseCamera && runningState==findLine)
		{ 
			u16 diff = abs(center - 320);
			//int16_t dDiff=diff-lastDiff;
			//lastDiff=diff;
			//if(diff > 149) //max=150.886
				//diff = 149;
			int a = 1.205*kp*diff;
			if(a > 30)
				a = 30;
			if(center == 320)
			{
				setLeftPID(30);
				setRightPID(30);
			}else if(center > 320) //�������߽�
			{
				setRightPID(30-a);//1.23
				setLeftPID(30+0.09*kp*diff);//0.18
			}else{ 
				setRightPID(30-0.09*kp*diff);
				setLeftPID(30-a);
				}
		}
		if(isUseCamera && runningState==lostLine)
		{
			continue;
		}
		if(isUseCamera && runningState==check)
		{
			setLeftPID(10);
			setRightPID(10);
			if(center <= 300)
			{	
				stopMotor();
				delay_ms(1000);//��У׼��ͣ1s��ʾУ׼
				isUseCamera=false;
				for(int i=38;i<49;i++)
					{setServoDegree(i);
					 delay_ms(100);
					}
				delay_ms(100);
				setLeftPID(10);
				setRightPID(10);
				delay_ms(1770);//����У׼��ֹͣ�� ֱ��һ�ξ��� ����ʱʱ�����
				stopMotor();//ǰ��һ�ξ��뵽ȡ����ʼ��
				for(int i=47;i>39;i--)//��ȡ�򵽿�ס��
				{
					setServoDegree(i);// ռ�ձȴ�35-49��49���ȡ���ʼ�� 33���� 40��ס��
					delay_ms(180); //180ms��ʱ
				}
				isBackBegin=true;
				delay_ms(500);
			}
		}
	/*	if(USART2_RX_STA&0x8000)
		{
			u16 receivedData;//���ڽ��յ���λ������ָ��
			USART2_RX_STA = 0;
			
			if(USART_RX_BUF[0]==1||USART_RX_BUF[0]=='1') //�Ӵ�������1 ��ʼ�������
			{
				isBackBegin=true; 
				printf("begin computing distance!!\r\n");
				for(int i=49;i>39;i--)//��ȡ�򵽿�ס��
					{
						setServoDegree(i);// ռ�ձȴ�35-50��50���ȡ���ʼ�� 33���� 40��ס��
						delay_ms(180); //180ms��ʱ
					}
				delay_ms(1000);
			}
		}*/
		while(isBackBegin)
		{
			delay_ms(500);
			isBackStraight = true;
			setLeftBackPID(30);
			setRightBackPID(27);
			dis=circle*PI*DIAMETER;
			if(dis>=190)
			{   
				setLeftBackPID(30);
				setRightBackPID(30);
				delay_ms(2500);
				stopMotor();
				isBackStraight=false;
				isBackBegin=false;
				for(int i=33;i<39;i++)
				{	setServoDegree(i);
					delay_ms(180);
				}
				setServoDegree(33);
			}
		}
		//printf("leftEncoderVal:%d\r\n",abs(leftEncoderVal));//�ض��崮��2��printf
		//printf("rightEncoderVal:%d\r\n\r\n",abs(rightEncoderVal));
		ledToggle();
	}
}