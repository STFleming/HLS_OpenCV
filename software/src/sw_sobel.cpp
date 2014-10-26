#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <iostream>
#include "opencv2/opencv.hpp"

timespec diff(timespec start, timespec end);

int main()
{
	
        struct timespec time1,time2;
	
	cv::Mat inFrame; 
	inFrame = cv::imread("datalogger.jpg");	

	cv::Mat x_sobel;
	cv::Mat y_sobel;
	cv::Mat outFrame;

	cvtColor(inFrame, inFrame, CV_RGB2GRAY);
	inFrame.convertTo(inFrame, CV_8UC1);

        clock_gettime(CLOCK_REALTIME, &time1);	
	
	cv::Sobel(inFrame, x_sobel, CV_8U, 1, 0, 3);
	cv::Sobel(x_sobel, y_sobel, CV_8U, 0, 1, 3);

        clock_gettime(CLOCK_REALTIME, &time2);
        std::cout << diff(time1, time2).tv_sec << ":" << diff(time1, time2).tv_nsec << std::endl;
	
	cv::imshow("output", y_sobel); 
		
while(1)
{	
	

        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       {
            break;
       }
}

	return 0;
}

timespec diff(timespec start, timespec end)
{
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}


