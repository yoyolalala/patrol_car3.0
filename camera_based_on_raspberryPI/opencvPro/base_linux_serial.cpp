#include "base_linux_serial.h"
#include <stdlib.h>
#include <fcntl.h>  
#include <termios.h> 
#include <unistd.h> //Unix 标准函数定义
#include <iostream>   
#include <vector>  
#include <sstream>  
#include <fstream>
#include <iomanip>

#ifndef LINUX_SERIAL_COMMON_BASE_DEFINE_H_
#define LINUX_SERIAL_COMMON_BASE_DEFINE_H_
#define SUCCESSFUL 1
#define FAILED -1
#endif
using namespace std;  


char BaseLinuxSerial::frame_header1_ = 0xAA;
char BaseLinuxSerial::frame_header2_ = 0xDD;
//char BaseLinuxSerial::m_frameHeader3 = 0xEE;
int BaseLinuxSerial::headerLength_ = 8;

/*********************************************************************** 
* 名称：         Open 
* 功能：         打开指定串口并返回状态 
* 入口参数：     portName:串口号(/dev/ttyS0,/dev/ttyS1,/dev/ttyS2) 
* 返回值：       正确打开返回1  错误返回-1 
************************************************************************/  

int BaseLinuxSerial::Open(const char *portName)
{
    fd_ = ::open(portName,O_RDWR|O_NOCTTY);//阻塞模式打开
    if(fd_ == -1)
    {
        cout << "Can't Open serial port" << *portName << endl;
        return FAILED;
    }    
    cout << "success to Open " << portName << endl;
    return SUCCESSFUL;
}  

/*********************************************************************** 
* 名称：         Open 
* 功能：         打开串口并返回状态 
* 入口参数：     uartNum:串口号 打开uartNum串口号 
* 返回值：       正确返回1  错误返回-1 
************************************************************************/  
int BaseLinuxSerial::Open(int uartNum) 
{  
    stringstream stream;
    string uartName = "/dev/ttyUSB";
    string str;

    stream.clear();
    stream << uartNum;
    stream >> str;
    str = uartName + str;
    fd_ = ::open(str.c_str(),O_RDWR|O_NOCTTY);//阻塞模式打开
    if(fd_ < 0)
    {
        cout << "fail to Open " << str << endl;
        return FAILED;
    }
    cout << "success to Open " << str << endl;
    return SUCCESSFUL;
}  

/*************************************************************************** 
* 名称：                       Init 
* 功能：                       设置串口数据位，停止位，校验位 
* 入口参数：                   baud               波特率 2400 4800 9600 115200 
*                             flow_ctrl          数据流控制 0 1 2 
*                             databits           数据位  7或8 
*                             stopbits           停止位  1或2 
*                             parity             校验类型  0 1 2 
* 返回值：                    正确返回1        错误返回-1 
*****************************************************************************/  
int BaseLinuxSerial::Init(int baud ,int flow_ctrl ,int databits ,int stopbits ,int parity )  
{      
    typedef std::pair< int,int> Baud;
    vector<Baud> baud_list;
    baud_list.push_back(Baud(1200,B1200));
    baud_list.push_back(Baud(1800,B1800));
    baud_list.push_back(Baud(2400,B2400));
    baud_list.push_back(Baud(4800,B4800));
    baud_list.push_back(Baud(9600,B9600));
    baud_list.push_back(Baud(19200,B19200));
    baud_list.push_back(Baud(38400,B38400));
    baud_list.push_back(Baud(57600,B57600));
    baud_list.push_back(Baud(115200,B115200));

    struct termios options;

    /*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将他们保存与options，该函数还可以测试配置是否正确，
      该串口是否可用等，若调用成功，函数返回0,若调用失败，返回-1    */
    if(tcgetattr(fd_,&options) !=0)
    {
        cout << "设置串口失败" <<endl;
        return FAILED;
    }

    for(vector< Baud >::iterator iter=baud_list.begin();iter!=baud_list.end();++iter)
    {
        if(iter->first == baud)
        {
            cfsetispeed(&options,iter->second);
            cfsetospeed(&options,iter->second);
			cout << "baud set " << baud << endl;
			break;
        }
		if(iter == baud_list.end() -1)
		{
			cout << "baud set error,please check baud" << endl;
			return FAILED;
		}
    }

    //修改控制模式 保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式 使得能够从串口中读输入数据
    options.c_cflag |= CREAD;
	options.c_lflag&= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
	options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    //设置数据流控制
    switch (flow_ctrl)
    {
    case 0 : //不使用流控制
        options.c_cflag &= ~CRTSCTS;
        break;
    case 1 : //使用硬件流控制
        options.c_cflag |= CRTSCTS;
        break;
    case 2 : //使用软件流控制
        options.c_cflag |= IXON |IXOFF |IXANY;
        break;
    default :
        cout << "数据流参数错误" << endl;
        return FAILED;
    }

    //设置数据位
    options.c_cflag &= ~CSIZE;   //屏蔽其他标志位
    switch(databits)
    {
    case 5 :
        options.c_cflag |= CS5;
        break;
    case 6 :
        options.c_cflag |= CS6;
        break;
    case 7 :
        options.c_cflag |= CS7;
        break;
    case 8 :
        options.c_cflag |= CS8;
        break;
    default :
        cout << "数据位参数错误" << endl;
        return FAILED;
    }

    //设置校验位
    switch(parity)
    {
    case 0 : //无奇偶校验位
        options.c_cflag &= ~PARENB;
        //options.c_iflag &= ~INPCK;
        break;
    case 1 : //奇校验
        options.c_cflag |= (PARODD | PARENB);

        //options.c_iflag |= INPCK;
        break;
    case 2 : //偶校验
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~PARODD;
        //options.c_iflag |= INPCK;
        break;
    case 3 : //设置为空格
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;
    default :
        cout << "校验位参数错误" << endl;
        return FAILED;
    }

    //设置停止位
 
   switch(stopbits)
    {
    case 1 :
        options.c_cflag &= ~CSTOPB;
        break;
    case 2 :
        options.c_cflag |= CSTOPB;
        break;
    default :
        cout << "停止位参数错误" << endl;
        return FAILED;
    }

    //修改输出模式 原始数据输出
    options.c_oflag &= ~OPOST;

    
    options.c_cc[VTIME] = 0; // 读取一个字符等待0S
    options.c_cc[VMIN ] = 0; // 

    //如果发生数据溢出，接受数据，但是不再读取
    tcflush(fd_,TCIFLUSH);

    //将修改后的数据设置写入串口
    if(tcsetattr(fd_,TCSANOW,&options) != 0)  //立即生效
    {
        cout << "com set error" << endl;
        return FAILED;
    }
	
	if(pthread_create(&pid_,NULL,&BaseLinuxSerial::pthreadHelp,this) != 0)
	{
		cout << "create new pthread failed" << endl;
		return FAILED;
	}
    return SUCCESSFUL;
}


/*************************************************************************
* 
***********************************************************************/

void BaseLinuxSerial::ReceivePthread()
{
	while(!is_close_)
	{
	//	cout << "receive pthread run" << endl;
		if(ReceiveMsg() < 0)
		{
			cout << "receive msg error" << endl;
		}
	}
}

/*********************************************************************
********************************************************************/
void *BaseLinuxSerial::pthreadHelp(void *arg)
{
	((BaseLinuxSerial *)arg)->ReceivePthread();
	return (void *)NULL;
}
/********************************************************************** 
* 名称：                                        ReceiveMsg 
* 功能：                                        接受串口数据 
* 入口参数:                                     void 
*                                                
* 返回值:                                       正确返回实际接受字符数   错误返回-1 
*************************************************************************/  

int BaseLinuxSerial::ReceiveMsg()  
{  
    int len,fs_sel;
    fd_set fs_read;

    FD_ZERO(&fs_read);
    FD_SET(fd_,&fs_read);

    //
    fs_sel = select(fd_+1,&fs_read,NULL,NULL,NULL);
    if(fs_sel)
    {	
//		char *headerPtr = new char[BaseLinuxSerial::headerLength_];
//    	len = readn(headerPtr,BaseLinuxSerial::headerLength_);
//		if((len != BaseLinuxSerial::headerLength_)
//			|| (headerPtr[0] != BaseLinuxSerial::frame_header1_)
//			|| (headerPtr[1] != BaseLinuxSerial::frame_header2_) )
//		{
//	    	delete [] headerPtr;
//		//	cout << "1" << endl;
//	    	return FAILED;
//		}
//        char typeByte = headerPtr[2];
//		char checkByte = headerPtr[3];
        int msgLen = 12;
//		memcpy(&msgLen,&headerPtr[4],sizeof(msgLen));
//		delete [] headerPtr;
		char *msgPtr = new char[msgLen+1];
		len = readn(msgPtr,msgLen+1);
		if((len-1) != msgLen)
		{
	    	delete [] msgPtr;
		//	cout << "2" << endl;
	    	return FAILED;
		}
//		int i_checkByte = 0;
//		for(int i = 0;i < msgLen;++i)
//		{
//	    	i_checkByte +=msgPtr[i];
//		}
//		char c_checkByte = i_checkByte;
//		if(checkByte != c_checkByte)
//		{
//		//	cout << checkByte << endl;
//			//cout << msgPtr << endl;
//	    	delete [] msgPtr;
//		//	cout << "3" << endl;
//	    	return FAILED;
//		}
	msgPtr[msgLen] = '\0';
	pthread_mutex_lock(&mutex_);
        receive_data_.type_byte = 0;
        receive_data_.receive_buf = msgPtr;
        receive_data_.receive_buf_length = msgLen; 
        
	ofstream serial_reco;
        unsigned char recv_data[12];
        for(int i = 0; i < 12; ++i)
        {
            recv_data[i] = static_cast<unsigned char>(receive_data_.receive_buf[i]);
            cout << (int)recv_data[i] << endl;
        }
        HandleBuffer(recv_data);

        new_data_ = true;
		pthread_mutex_unlock(&mutex_);
        return len;
    }
	//cout << "4" << endl;
    return FAILED;
}  

/**************************************************************************
**************************************************************************/

int BaseLinuxSerial::readn(char *vptr,size_t n) const
{
    size_t nleft;
    ssize_t nread;
    char *ptr = vptr;
    nleft = n;
    while(nleft > 0)
    {
		if((nread = read(fd_,ptr,nleft)) < 0)
        {
            if(errno == EINTR)
				nread = 0;
	    	else 
			return FAILED; 	    
		}
		else if(nread == 0)
	    	continue;
		nleft -= nread;
		ptr += nread;
    }
    return (n-nleft);
}
/****************************************************************************** 
* 名称：                      SendMsg 
* 功能：                      发送数据 
* 入口参数：                  SendMsg 发送缓存区 
*                             msgLen  
* 返回值：                   正确返回实际发送字符数   错误返回-1 
*******************************************************************************/  

int BaseLinuxSerial::SendMsg(const char* SendMsg,int msgLen,char type) const  
{  
    int len = 0;
    char *sendBufPtr = new char[msgLen];
//    int checkNum = 0;++
//    for(int i = 0;i < msgLen;++i)
//    {
//		checkNum += SendMsg[i];
//    }
//    char checkByte = checkNum;
//    sendBufPtr[0] = BaseLinuxSerial::frame_header1_;
//    sendBufPtr[1] = BaseLinuxSerial::frame_header2_;
//    sendBufPtr[2] = type;
//    sendBufPtr[3] = checkByte;
    memcpy(sendBufPtr,SendMsg,12);
//    memcpy(&sendBufPtr[4+sizeof(msgLen)],SendMsg,msgLen);
//    sendBufPtr[msgLen] = '\r';
//    cout << SendMsg << endl;
//	 << sendBuf.size() << endl;
  //  cout << fd_<< endl;  
    len = write(fd_, sendBufPtr, msgLen);  //返回实际串口发送字符数
    usleep(10000);  //10ms

	delete [] sendBufPtr; 
    if( len < 0)
    { 
		cout << "errno=" << errno << endl;
		return FAILED;
    }
    else if((len-BaseLinuxSerial::headerLength_-1) == msgLen)
    {
    //	cout << "success send data" << endl;
        return msgLen;
    }

    tcflush(fd_,TCOFLUSH);
    return FAILED;
}  
/*************************************************************************
*************************************************************************/
// void BaseLinuxSerial::ClearRcvBuf(void)
// {
// 	pthread_mutex_lock(&mutex_);
// 	receive_buffer_.clear();
// 	pthread_mutex_unlock(&mutex_);
// }

bool BaseLinuxSerial::IsNewData()
{
	pthread_mutex_lock(&mutex_);
	bool ret = new_data_;
	pthread_mutex_unlock(&mutex_);
	return ret;
}

BaseLinuxSerial::ReceiveData BaseLinuxSerial::receive_data()
{
	pthread_mutex_lock(&mutex_);
	ReceiveData receive_data = receive_data_;
    new_data_ = false;
	pthread_mutex_unlock(&mutex_);
	return receive_data;
}

pthread_t BaseLinuxSerial::pid() const
{
	return pid_;	
}
/******************************************************************************** 
* 名称：                    ~BaseLinuxSerial 
* 功能：                    析构函数  关闭串口 
* 入口参数：                void 
* 返回值：                  void 
*********************************************************************************/  

BaseLinuxSerial::~BaseLinuxSerial()  
{  
    is_close_ = 1;
    sleep(1);
    close(fd_);
}

int BaseLinuxSerial::fd()
{
	return fd_;
}

void BaseLinuxSerial::HandleBuffer(unsigned char* rcv_buf)
{
    static ofstream correct_file;
    static float correct_data_A[4][14] = {};
    static float correct_data_B[4][14] = {};
    static const string plf_name[2] = { "A机构", "B机构" };
    static const string style_name[2] = { "落台", "打球" };
    static const string pillar_name[7] = { "近台", "最左台", "次左台", "中台", "次右台", "最右台", "远台" };
    if(rcv_buf[0] == 0xe0 && rcv_buf[11] == 0x0f)
    {
        correct_file.open("//home//pi//Desktop//correct.txt", ios::app);
        if(!correct_file)
            cout << "failed to open file" << endl;

        int which_plf = ((rcv_buf[1] & 0xf0) >> 4) - 1;
        int which_style = (rcv_buf[1] & 0x0f) - 1;
        int which_pillar = rcv_buf[2] - 1;
        string this_plf = plf_name[which_plf];
        string this_style = style_name[which_style];
        string this_pillar = pillar_name[which_pillar];
        float pitch = (float)(rcv_buf[3] * 256 + rcv_buf[4] - 20000) / 100;
        float yaw = (float)(rcv_buf[5] * 256 + rcv_buf[6] - 20000) / 100;
        int big_speed = (rcv_buf[7] * 256 + rcv_buf[8]) * 10;
        int small_speed = (rcv_buf[9] * 256 + rcv_buf[10]) * 10;
        //A big + small - B big- small+
        if (((rcv_buf[1] & 0xf0) >> 4 )== 1)
            small_speed = -small_speed;
        else
            big_speed = -big_speed;

        cout << this_plf << this_pillar << this_style << "	"
            << " pitch: " << setw(4) << pitch
            << " yaw: " << setw(4) << yaw
            << " 大轮: " << setw(4) << big_speed
            << " 小轮: " << setw(4) << small_speed << endl;

        correct_file << this_plf << this_pillar << this_style << "	"
                     << " pitch: " << setw(4) << pitch
                     << " yaw: " << setw(4) << yaw
                     << " 大轮: " << setw(4) << big_speed
                     << " 小轮: " << setw(4) << small_speed << endl;

        if (!(pitch == 0 && yaw == 0 && big_speed == 0 && small_speed == 0)) //回A点
        {
            if (which_plf == 0)
            {
                correct_data_A[0][which_pillar + which_style * 7] = pitch;
                correct_data_A[1][which_pillar + which_style * 7] = yaw;
                correct_data_A[2][which_pillar + which_style * 7] = big_speed;
                correct_data_A[3][which_pillar + which_style * 7] = small_speed;
            }
            else
            {
                correct_data_B[0][which_pillar + which_style * 7] = pitch;
                correct_data_B[1][which_pillar + which_style * 7] = yaw;
                correct_data_B[2][which_pillar + which_style * 7] = big_speed;
                correct_data_B[3][which_pillar + which_style * 7] = small_speed;
            }
        }
        else
        {
            static bool writed = false;
            if (!writed)
            {
                for (int j = 0; j < 4; j++)
                {
                    correct_file << '{';
                    for (int k = 0; k < 14; k++)
                    {
                        correct_file << setw(4) << correct_data_A[j][k] << ',';
                    }
                    correct_file << '}'  << ',' << endl;
                }
                correct_file << endl;
                for (int j = 0; j < 4; j++)
                {
                    correct_file << '{';
                    for (int k = 0; k < 14; k++)
                    {
                        correct_file << setw(4) << correct_data_B[j][k] << ',';
                    }
                    correct_file << '}' << ',' << endl;
                }
                cout << "已输出数组" << endl;
                writed = true;
	        system("sleep 5 && sudo shutdown now &");
		exit(0);
            }
        }
        correct_file.close();
    }
}
