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
//u8 i,len; //����2����������
u16 dis;//�߹�����
u16 center;//���յ������е�����
u8 pct1=50;
u8 pct2=50;//Ѳ��ʱ�����ֳ�ʼת��
bool isBackStraight=false;
bool isBackBegin=false; //�Ƿ�ʼ�������
bool isForwardStraight=false;
bool isUseCamera=true;//δ��ʼʶ��ץȡС��ʱ ��������ͷѲ��
bool isSecondCheck=false;//�Ƿ��ǵڶ��μ�⵽check��
float pidOutput;
extern int circle;
extern int leftEncoderVal;
extern int rightEncoderVal;
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
	motorInit();
	uart1_init(38400);//���ڳ�ʼ��������ڵ����ʼ����
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
int main(void)
{ 
	setup();
	delay_ms(100);
	stopMotor();
    while(1)
	{
		if(USART1_RX_STA&0x8000)
		{
			rasLedToggle();//PD4ָʾ����˸ 32�н��յ���ݮ������
			runningState=USART_RX_BUF[0];
			center=USART_RX_BUF[1]*256+USART_RX_BUF[2];
			USART1_RX_STA = 0;
			/*printf("state:%d\r\n",runningState);
			printf("center:%d\r\n",center);
			printf("\r\n\r\n");*/
		}
		if(isUseCamera)
		{
			setServoDegree(35);//49���ȡ��λ�� 35Ϊ��������ͷ�ĳ�ʼλ��
		}
		if(isUseCamera && runningState==findLine)
		{
			static int pwmRight = 50;
			pidInit(0.01,pct2-1,pct2-99);//��ʼ��ˢ�¼�� ������
			setWeights(0.04,0.001,0);
			
			motor1SetPct(pct1);
			motor2SetPct(pct2);
			setDesiredPoint(320);
			pidOutput = refresh(center);//�������߽�ʱ center>320 pidOutputΪ��
			
			pwmRight = pidOutput + pwmRight;
			if(pwmRight > 99)
				pwmRight = 99;
			if(pwmRight < 0)
				pwmRight = 0;
			if(320<center) //�������߽� ��Ҫ�ӿ������ٶ�
				motor2SetPctback(pwmRight);
			else motor1SetPct(pwmRight);
		}else if(isUseCamera&&runningState==cross)
		{
			forwardStraight(50,50);
		}else if(isUseCamera && runningState==check && isSecondCheck==false)
		{	
			forwardStraight(50,50);
			if(center<=260)
			{	runningState=findLine;
				isSecondCheck=true;
			}
		}else if(isUseCamera && runningState==check && isSecondCheck==true)
		{
			forwardStraight(50,50);
			if(center <= 260)
			{	
				stopMotor();
				delay_ms(1000);//��У׼��ͣ1s��ʾУ׼
				isUseCamera=false;
				setServoDegree(49);
				//forwardStraight(30,30);
				delay_ms(200);//����У׼��ֹͣ�� ֱ��һ�ξ��� ����ʱʱ�����
				stopMotor();//ǰ��һ�ξ��뵽ȡ����ʼ��
				for(int i=48;i>39;i--)//��ȡ�򵽿�ס��
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
		}
		
		while(isBackBegin)
		{
			delay_ms(500);
			backStraight(30,30);
			dis=circle*PI*DIAMETER;
			if(dis>=194)
			{   
				backStraight(30,30);
				delay_ms(335);
				stopMotor();
				isBackStraight=false;
				isBackBegin=false;
				for(int i=33;i<41;i++)
				{	setServoDegree(i);
					delay_ms(180);
				}
			}
		}*/
		//printf("leftEncoderVal:%d\r\n",abs(leftEncoderVal));//�ض��崮��2��printf
		//printf("rightEncoderVal:%d\r\n\r\n",abs(rightEncoderVal));
		ledToggle();
	}
}