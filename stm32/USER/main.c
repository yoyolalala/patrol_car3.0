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
u8 pct1=70;
u8 pct2=70; //初始化电机占空比
u8 i,len; //串口2接收数据用
u16 dis;//走过距离
u16 center;//接收到的线中点坐标
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
	pidInit(0.01,pct2-1,pct2-99);//初始化刷新间隔 上下限
}
void backStraight(u8 pct1,u8 pct2)
{
	setWeights(0.425, 0.175, 0.001);//只装底盘后退时完美PID参数0.425  0.16  0.001
	isBackStraight=true;
	
	motor1SetPctback(pct1);
	setDesiredPoint(leftEncoderVal);
	pidOutput=refresh(rightEncoderVal);
	if(leftEncoderVal<rightEncoderVal)
	{	 motor2SetPctback(pct2-pidOutput); //pidOutput为负值
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
	//motor1SetPct(70);//越大 转速越慢  70-33    30-116
	//motor2SetPct(70);//起始转速
    while(1)
	{
		if(USART1_RX_STA&0x8000)
		{
			rasLedToggle();//PD4指示灯闪烁 32有接收到树莓派数据
			runningState=USART_RX_BUF[0];
			center=USART_RX_BUF[1]*256+USART_RX_BUF[2];
			USART1_RX_STA = 0;
			printf("state:%d\r\n",runningState);
			printf("center:%d\r\n",center);
			printf("\r\n\r\n");
		}
		if(isUseCamera)
		{
			setServoDegree(35);//35为启用摄像头的初始位置
		}
		if(isUseCamera&&runningState==findLine)
		{
			setWeights(0,0,0);
			
			motor1SetPctback(pct1);
			setDesiredPoint(320);
			pidOutput=refresh(center);
			if(320<center) //车身向右偏 需要加快右轮速度
			{	 motor2SetPctback(pct2+pidOutput); //pidOutput为负值
				 pct2=pct2+pidOutput;
			}
			else 
			{	motor2SetPctback(pct2-pidOutput);
				pct2=pct2-pidOutput;
			}
		}else if(isUseCamera&&runningState==cross)
		{
			forwardStraight(60,60);
		}else if(isUseCamera&&runningState==check&&isSecondCheck==false)
		{	
			forwardStraight(60,60);
			if(center>=138)
			{	runningState=findLine;
				isSecondCheck=true;
			}
		}else if(isUseCamera&&runningState==check&&isSecondCheck==true)
		{
			forwardStraight(60,60);
			if(center>=138)
			{	
				stopMotor();
				delay_ms(1000);//在校准点停1s以示校准
				isUseCamera=false;
				forwardStraight(50,50);
				delay_ms(500);//车在校准点停止后 直行一段距离 由延时时间决定
				stopMotor();//前行一段距离到取球点初始点
				for(int i=50;i>39;i--)//从取球到卡住球
				{
					setServoDegree(i);// 占空比从35-50。50最低取球初始点 33掉球 40卡住球
					delay_ms(180); //180ms延时
				}
				isBackBegin=true;
			}
		}
		if(USART2_RX_STA&0x8000)
		{
			u16 receivedData;//串口接收到上位机发送指令
			len = USART2_RX_STA & 0x3fff;
			for (i = 0; i< len;i++)
			{
				receivedData=receivedData+(USART_RX_BUF[i] - 0x30)*pow(10, len - i - 1);
				while (USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);
			}
			USART2_RX_STA = 0;
			
			if(receivedData==1) //从串口输入1 开始计算距离
			{
				isBackBegin=true; 
				printf("begin computing distance!!\r\n");
				receivedData=0;
			}
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
				setServoDegree(33);
			}
			else 
			{
				printf("leftEncoderVal:%d\r\n",leftEncoderVal);
				printf("rightEncoderVal:%d\r\n\r\n",rightEncoderVal);
			}
		}
		//printf("leftEncoderVal:%d\r\n",leftEncoderVal);//重定义串口2的printf
		//printf("rightEncoderVal:%d\r\n\r\n",rightEncoderVal);
		ledToggle();
	}
}