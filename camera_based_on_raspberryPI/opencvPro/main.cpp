#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include "base_linux_serial.h"
#define minPointNum 300
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
//BaseLinuxSerial serial;
int main()
{

    //serial.Open(0);
    //serial.Init(115200);
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
            approxPolyDP(contours[i],afterPoly[i],25,true);

        Mat dst(binImg.size(),CV_8U,Scalar(255));
        drawContours(dst,afterPoly,-1,Scalar(0),2);



        isnotCross=isContourConvex(afterPoly[0]);
        cout<<(int)isnotCross<<endl;
        imshow("test", dst);
        if(afterPoly.size()==1&&isnotCross==1)
        {
            state=findLine;
            char a[3];
            RotatedRect rectFindline=minAreaRect(afterPoly[0]);
            a[0]=state;
            a[1]=(int)(rectFindline.center.x) /256;
            a[2]=(int)(rectFindline.center.x) %256;
         //   serial.SendMsg(a,3,'1');
            cout<<rectFindline.center.x<<endl<<state<<endl;
            lastX=rectFindline.center.x;
        }

        if(isnotCross==0)
        {
            state=cross;
            cout<<state<<endl;
            char a[1];
            a[0]=state;
        //    serial.SendMsg(a,1,'2');
        }
        if(afterPoly.size()==3&&isnotCross==1)
        {
            state=check;
            char a[5];
            RotatedRect rectCheck1=minAreaRect(afterPoly[0]);
           // RotatedRect rectCheck2=minAreaRect(afterPoly[1]);
           // RotatedRect rectCheck3=minAreaRect(afterPoly[2]);
            a[0]=state;
            a[1]=(int)(rectCheck1.center.x) /256;
            a[2]=(int)(rectCheck1.center.x) %256;
            a[3]=(int)(rectCheck1.center.y) /256;
            a[4]=(int)(rectCheck1.center.y) %256;
        //    serial.SendMsg(a,5,'1');
            cout<<rectCheck1.center.x<<endl<<rectCheck1.center.y<<endl;
            cout<<state<<endl;
        }
        if(afterPoly.size()==2&&isnotCross==1)
        {
            state=findLine;
            char a[1];
            RotatedRect rectOne=minAreaRect(afterPoly[0]);
            RotatedRect rectTwo=minAreaRect(afterPoly[1]);
            int error1=fabs(lastX-rectOne.center.x);
            int error2=fabs(lastX-rectTwo.center.x);
            if(error1>error2)
               {
                a[0]=rectTwo.center.x;
                lastX=rectTwo.center.x;
               }
            else
               {
                a[0]=rectOne.center.x;
                lastX=rectOne.center.x;
               }
            cout<<lastX;
        }
     waitKey(30);
    }
    return 0;
}
//   double time0= static_cast<double>(getTickCount());//单位为s
//   time0=((double)getTickCount()-time0)/getTickFrequency();
//   cout<<time0;
