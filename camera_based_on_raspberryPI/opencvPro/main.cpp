#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include "base_linux_serial.h"
#define minPointNum 100
#define cmd1 0x0d
#define cmd2 0x0a
using namespace std;
using namespace cv;
bool isnotCross=1;
int lastX;
typedef enum
{
    findLine=1,
    lostLine=2,
    check=3,
}runningState_Typedef;
BaseLinuxSerial serial;
int main()
{
    serial.Open(0);
    serial.Init(115200);
    runningState_Typedef state;
    VideoCapture capture(1);
	
	if(!capture.isOpened())
	{
		capture.release();
		capture.open(0);
	}
	while(1)
	{
		if(serial.Open(0) != -1)
			break;
        if(serial.Open(1) == -1)
            break;
	}
	
    while(waitKey(30) < 0)
    {
        Mat img;
        capture>>img;
//double time = static_cast<double>(getTickCount()); 
//       cv::imshow("src", img);
        assert(!img.empty());
		Mat ranImg;
        ranImg=img(Range(240,480),Range(80,560));
        cvtColor(ranImg,ranImg,CV_BGR2GRAY);
//imshow("img",ranImg);

       Mat binImg;
       double binThres=threshold(ranImg,binImg,99,255,THRESH_OTSU|THRESH_BINARY_INV);
//        cout<<"binThres"<<binThres<<endl;
	    vector<vector<Point> >contours;
       findContours(binImg,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
       for(int i=0;i<contours.size();)
       {
         if(contours[i].size()<minPointNum)
           {
              contours.erase(contours.begin()+i);
           }
          else i++;
      }

       vector<vector<Point> >afterPoly(contours.size());
       for(int i=0;i<(int)contours.size();i++)
           approxPolyDP(contours[i],afterPoly[i],40,true);

       Mat dst(binImg.size(),CV_8U,Scalar(255));
       drawContours(dst,afterPoly,-1,Scalar(0),2);
//imshow("test",dst);
        
		if(binThres>130)
		{
			state=lostLine;
			char a[3];
			a[0]=state;
			a[1]=cmd1;
			a[2]=cmd2;
			serial.SendMsg(a,3,'1');
           cout<<state<<endl<<"bin>130->LostLine"<<endl;
			continue;
		}
		if(afterPoly.size()==0)
       {
           state=lostLine;
           char a[3];
           a[0]=state;
           a[1]=cmd1;
           a[2]=cmd2;
           serial.SendMsg(a,3,'1');
            
	       cout<<state<<endl<<"findNoContours->LostLine"<<endl;
           continue;
       }
       else isnotCross=isContourConvex(afterPoly[0]);
       if(afterPoly.size()==1&&isnotCross==1)
       {
           char a[5];
//			bool index=0;
           RotatedRect rectFindline=minAreaRect(afterPoly[0]);
 //      cout<<rectFindline.angle<<endl;
//			if(rectFindline.size.width>rectFindline.size.height)
//				index=1;
//			else index=0;
//cout<<index;
//            if(rectFindline.angle <= -3.3 && index == 0)
//            {
//                state = leftLine;
//           }else if(rectFindline.angle > -50 && index == 1)
//               state = rightLine;
           state = findLine;
           a[0]=state;
           a[1]=(int)(rectFindline.center.x) /256;
           a[2]=(int)(rectFindline.center.x) %256;
           a[3]=cmd1;
           a[4]=cmd2;
			serial.SendMsg(a,5,'1');
//time = ((double)getTickCount()-time)/getTickFrequency();
//cout<<time;
           cout<<state<<endl<<rectFindline.center.x<<endl;
           lastX=rectFindline.center.x;
           continue;
       }

       if(isnotCross==0)
       {
           state=lostLine;
           char a[3];
           a[0]=state;
           a[1]=cmd1;
           a[2]=cmd2;
           serial.SendMsg(a,3,'1');
           cout<<state<<endl<<"cross!"<<endl;
           continue;
       }
       if(afterPoly.size()==3&&isnotCross==1)
       {
           state=check;
           char a[5];
           RotatedRect rectCheck1=minAreaRect(afterPoly[0]);
          // RotatedRect rectCheck2=minAreaRect(afterPoly[1]);
          // RotatedRect rectCheck3=minAreaRect(afterPoly[2]);
           a[0]=state;
           a[1]=(int)(rectCheck1.center.y) /256;
           a[2]=(int)(rectCheck1.center.y) %256;
           a[3]=cmd1;
           a[4]=cmd2;
       //    a[3]=(int)(rectCheck1.center.x) /256;
       //    a[4]=(int)(rectCheck1.center.x) %256;
           serial.SendMsg(a,5,'1');
           cout<<state<<endl<<rectCheck1.center.y<<endl;
           continue;
       }
       if(afterPoly.size()==2&&isnotCross==1)
       {
           state=findLine;
           char a[5];
           RotatedRect rectOne=minAreaRect(afterPoly[0]);
           RotatedRect rectTwo=minAreaRect(afterPoly[1]);
           int error1=fabs(lastX-rectOne.center.x);
           int error2=fabs(lastX-rectTwo.center.x);
           if(error1>error2)
              {
               lastX=rectTwo.center.x;
              }
           else
              {
               lastX=rectOne.center.x;
              }
           a[0]=state;
           a[1]=(int)(lastX) /256;
           a[2]=(int)(lastX) %256;
           a[3]=cmd1;
           a[4]=cmd2;
           serial.SendMsg(a,5,'1');
           cout<<state<<endl<<lastX<<endl;
           continue;
       }
    }
    return 0;
}