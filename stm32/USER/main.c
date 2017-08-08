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
#define DIAMETER 62 //车轮直径62mm
#define kp 0.11 //巡线时位置PID参数
#define kd 0.15
//#define INF 3.402823466e+38F
//u8 i,len; //串口2接收数据用
u16 dis;//走过距离
u16 center;//接收到的线中点坐标
u16 lastDiff=0;
u8 pct1=25;
u8 pct2=25;//巡线时左右轮初始转速
bool isBackStraight=false;
bool isBackBegin=false; //是否开始计算距离
bool isUseCamera=true;//未开始识别抓取小球时 启动摄像头巡线
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init(168);  //初始化延时函数 SYSCLOCK
	LED_Init();
	motorInit();
	uart1_init(115200);//串口初始化必须放在电机初始化后
	uart2_init(38400);//串口1用于树莓派接收数据 串口2向上位机发送数据
	Encoder1_Init_TIM4();
	Encoder2_Init_TIM3();
	TIM2_Int_Init(499, 8399); //0.05s
	servoPwmInit(99,2766);
}
void backStraight(u8 pct1,u8 pct2)
{
	static int pwmRight = 30;
	pidInit(0.01,pct2-1,pct2-99);//初始化刷新间隔 上下限
	
	setWeights(0.25, 0.0165, 0.001);
	isBackStraight=true;
	
	motor1SetPctback(pct1);//越大 转速越慢  70-33    30-116
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
	pidInit(0.01,pct2-1,pct2-99);//初始化刷新间隔 上下限
	
	setWeights(0.246, 0.0165, 0.001);
	
	motor1SetPct(pct1);//越大 转速越慢  70-33    30-116
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
			setServoDegree(38);//49最低取球位置 38为启用摄像头的初始位置 45舵机水平
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
			}else if(center > 320) //右轮离线近
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
				delay_ms(1000);//在校准点停1s以示校准
				isUseCamera=false;
				for(int i=38;i<49;i++)
					{setServoDegree(i);
					 delay_ms(100);
					}
				delay_ms(100);
				setLeftPID(10);
				setRightPID(10);
				delay_ms(1770);//车在校准点停止后 直行一段距离 由延时时间决定
				stopMotor();//前行一段距离到取球点初始点
				for(int i=47;i>39;i--)//从取球到卡住球
				{
					setServoDegree(i);// 占空比从35-49。49最低取球初始点 33掉球 40卡住球
					delay_ms(180); //180ms延时
				}
				isBackBegin=true;
				delay_ms(500);
			}
		}
	/*	if(USART2_RX_STA&0x8000)
		{
			u16 receivedData;//串口接收到上位机发送指令
			USART2_RX_STA = 0;
			
			if(USART_RX_BUF[0]==1||USART_RX_BUF[0]=='1') //从串口输入1 开始计算距离
			{
				isBackBegin=true; 
				printf("begin computing distance!!\r\n");
				for(int i=49;i>39;i--)//从取球到卡住球
					{
						setServoDegree(i);// 占空比从35-50。50最低取球初始点 33掉球 40卡住球
						delay_ms(180); //180ms延时
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
		//printf("leftEncoderVal:%d\r\n",abs(leftEncoderVal));//重定义串口2的printf
		//printf("rightEncoderVal:%d\r\n\r\n",abs(rightEncoderVal));
		ledToggle();
	}
}