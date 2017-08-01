#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include "base_linux_serial.h"
#define minPointNum 300
#define cmd1 0x0d
#define cmd2 0x0a
using namespace std;
using namespace cv;
bool isnotCross=1;
int lastX;
typedef enum
{
    findLine=1,
    cross=2,
    check=3,
}runningState_Typedef;
BaseLinuxSerial serial;
int main()
{
    serial.Open(0);
    serial.Init(38400);
    runningState_Typedef state;
    VideoCapture capture(0);
    while(1)
    {
        Mat img;
        capture>>img;
        assert(!img.empty());
        cvtColor(img,img,CV_BGR2GRAY);

        Mat binImg;
        threshold(img,binImg,99,255,THRESH_OTSU|THRESH_BINARY_INV);

        vector<vector<Point> >contours;
        findContours(binImg,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);

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
            approxPolyDP(contours[i],afterPoly[i],50,true);

        Mat dst(binImg.size(),CV_8U,Scalar(255));
        drawContours(dst,afterPoly,-1,Scalar(0),2);

        isnotCross=isContourConvex(afterPoly[0]);

//        imshow("test", dst);
        if(afterPoly.size()==1&&isnotCross==1)
        {
            state=findLine;
            char a[5];
            RotatedRect rectFindline=minAreaRect(afterPoly[0]);
            a[0]=state;
            a[1]=(int)(rectFindline.center.x) /256;
            a[2]=(int)(rectFindline.center.x) %256;
            a[3]=cmd1;
            a[4]=cmd2;
            serial.SendMsg(a,5,'1');

            cout<<state<<endl<<rectFindline.center.x<<endl;
            lastX=rectFindline.center.x;
        }

        if(isnotCross==0)
        {
            state=cross;
            char a[3];
            a[0]=state;
            a[1]=cmd1;
            a[2]=cmd2;
            serial.SendMsg(a,3,'1');
            cout<<state<<endl;
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
            cout<<state<<endl<<rectCheck1.center.x<<endl<<rectCheck1.center.y<<endl;
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
        }
//     waitKey(30);
    }
    return 0;
}
//   double time0= static_cast<double>(getTickCount());//单位为s
//   time0=((double)getTickCount()-time0)/getTickFrequency();
//   cout<<time0;
