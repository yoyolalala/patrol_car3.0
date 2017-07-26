#ifndef BaseLinuxSerial_H
#define BaseLinuxSerial_H  

#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <string>

using namespace std;

const char kCmd = '1';
const char kData =  '2';
const char kFile = '3';
const char kFileAck = '4';
  

class BaseLinuxSerial  
{  
public:  
    BaseLinuxSerial():fd_(-1),pid_(-1),is_close_(false),new_data_(false)
	{
		pthread_mutex_init(&mutex_,NULL);
	}

	static char frame_header1_;
	static char frame_header2_;
//	static char m_frameHeader3;
	static int headerLength_;
	static void *pthreadHelp(void* arg);
	typedef struct 
	{
		char type_byte;
        char *receive_buf;
		int receive_buf_length;
	} ReceiveData;
   	int Open(const char *portName);  
   	int Open(int uartNum);   
   	int Init(int baud = 115200,int flow_ctrl = 0,int databits = 8,int stopbits = 1,int parity = 0);
   	int SendMsg(const char* SendMsg,int msgLen,char type)const;   
	// void ClearRcvBuf();
	bool IsNewData();
	ReceiveData receive_data();
	pthread_t pid()const;
   	~BaseLinuxSerial(); 
    int fd();
      
private:  
    int fd_;  //串口设备描述符  
	pthread_t pid_;
	pthread_mutex_t mutex_;
	ReceiveData receive_data_;
	bool is_close_;
	bool new_data_;
	int readn(char *vptr,size_t n)const;
	void ReceivePthread();
    int ReceiveMsg();  
    void HandleBuffer(unsigned char* rcv_buf);
};

#endif 
