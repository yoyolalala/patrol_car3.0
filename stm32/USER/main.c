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
//#define INF 3.402823466e+38F
//u8 i,len; //串口2接收数据用
u16 dis;//走过距离
u16 center;//接收到的线中点坐标
u8 pct1=50;
u8 pct2=50;//巡线时左右轮初始转速
bool isBackStraight=false;
bool isBackBegin=false; //是否开始计算距离
bool isForwardStraight=false;
bool isUseCamera=true;//未开始识别抓取小球时 启动摄像头巡线
bool isSecondCheck=false;//是否是第二次检测到check点
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init(168);  //初始化延时函数 SYSCLOCK
	LED_Init();
	rasLED_Init();
	motorInit();
	uart1_init(38400);//串口初始化必须放在电机初始化后
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
int main(void)
{ 
	setup();
	delay_ms(100);
	stopMotor();
    while(1)
	{
		if(USART1_RX_STA&0x8000)
		{
			rasLedToggle();//PD4指示灯闪烁 32有接收到树莓派数据
			runningState=USART_RX_BUF[0];
			center=USART_RX_BUF[1]*256+USART_RX_BUF[2];
			USART1_RX_STA = 0;
			/*printf("state:%d\r\n",runningState);
			printf("center:%d\r\n",center);
			printf("\r\n\r\n");*/
		}
		if(isUseCamera)
		{
			setServoDegree(35);//49最低取球位置 35为启用摄像头的初始位置
		}
		if(isUseCamera && runningState==findLine)
		{
			static int pwmRight = 50;
			pidInit(0.01,pct2-1,pct2-99);//初始化刷新间隔 上下限
			setWeights(0.04,0.001,0);
			
			motor1SetPct(pct1);
			motor2SetPct(pct2);
			setDesiredPoint(320);
			pidOutput = refresh(center);//左轮离线近时 center>320 pidOutput为负
			
			pwmRight = pidOutput + pwmRight;
			if(pwmRight > 99)
				pwmRight = 99;
			if(pwmRight < 0)
				pwmRight = 0;
			if(320<center) //左轮离线近 需要加快右轮速度
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
				delay_ms(1000);//在校准点停1s以示校准
				isUseCamera=false;
				setServoDegree(49);
				//forwardStraight(30,30);
				delay_ms(200);//车在校准点停止后 直行一段距离 由延时时间决定
				stopMotor();//前行一段距离到取球点初始点
				for(int i=48;i>39;i--)//从取球到卡住球
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
		//printf("leftEncoderVal:%d\r\n",abs(leftEncoderVal));//重定义串口2的printf
		//printf("rightEncoderVal:%d\r\n\r\n",abs(rightEncoderVal));
		ledToggle();
	}
}