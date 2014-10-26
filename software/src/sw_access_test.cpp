#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
//#include "opencv2/highgui.hpp"
#include "opencv2/highgui/highgui_c.h"


#include "xVDMA.h"
#include "ximage_filter.h"

#define INPUT_FRAME_ADDR 0x38400000
#define OUTPUT_FRAME_ADDR 0x3a284800 //Allow 32 Mb for the framebuffer 

#define HLS_ADDR 0x43C00000

//VDMA setup parameters
#define VDMAWidth 600
#define VDMAHeight 400
#define VDMAPixelWidth 4
#define VDMABaseAddr 0x43000000

#define MAP_SIZE 4000000UL
#define MAP_MASK (MAP_SIZE - 1)

void *setup_reserved_mem(uint input_address);
timespec diff(timespec start, timespec end);

int main()
{
        //cv::VideoCapture cap("http://iphone-streaming.ustream.tv/uhls/17074538/streams/live/iphone/playlist.m3u8"); 
        //if(!cap.isOpened())
        //{       std::cout << "Error no camera detected." << std::endl;
        //        return -1; }


	//Input container
	//cv::Mat inFrame; //Temporary input container
	//cap >> inFrame;

	struct timespec time1,time2;

	cv::Mat inFrame;
	inFrame = cv::imread("datalogger.jpg");
	cvtColor(inFrame, inFrame, CV_RGB2GRAY);
	inFrame.convertTo(inFrame, CV_32SC1);
	
	cv::Mat hw_inputFrame;
	hw_inputFrame = inFrame;
	hw_inputFrame.data =(uchar *) setup_reserved_mem(INPUT_FRAME_ADDR); 

	cv::Mat hw_outputFrame;
	hw_outputFrame = inFrame;
	hw_outputFrame.data = (uchar *) setup_reserved_mem(OUTPUT_FRAME_ADDR);	

	inFrame.copyTo(hw_inputFrame);

	XImage_filter HLSDevice;	
	HLSDevice = setup_ximage_filter(HLS_ADDR);
	XImage_filter_SetRows(&HLSDevice, inFrame.rows);
	XImage_filter_SetCols(&HLSDevice, inFrame.cols);

	xVDMA_info vdma;
	xVDMA_Init(&vdma, VDMABaseAddr, inFrame.cols, inFrame.rows, VDMAPixelWidth);
	
	clock_gettime(CLOCK_REALTIME, &time1);	
	XImage_filter_Start(&HLSDevice); //Kick it to start it			
	xVDMA_kick(&vdma, INPUT_FRAME_ADDR, OUTPUT_FRAME_ADDR);
	
	while(xVDMA_IsDone(&vdma)&0x00000001 == 0) {
    		//printf("Status: %u\n", xVDMA_IsDone(&vdma));
	}		

       clock_gettime(CLOCK_REALTIME, &time2);
       std::cout << diff(time1, time2).tv_sec << ":" << diff(time1, time2).tv_nsec << std::endl;	

	
	cv::Mat outFrame;
	hw_outputFrame.copyTo(outFrame);
	outFrame.convertTo(outFrame, CV_8UC1);
	imshow("Input", outFrame);	

while(1)
{	
	//cap >> inFrame;
	//cvtColor(inFrame, inFrame, CV_RGB2GRAY);
	//inFrame.convertTo(inFrame, CV_32SC1);
	//inFrame.copyTo(hw_inputFrame);
	//XImage_filter_Start(&HLSDevice); //Kick it to start it			
	//xVDMA_kick(&vdma, INPUT_FRAME_ADDR, OUTPUT_FRAME_ADDR);
	//while(xVDMA_IsDone(&vdma)&0x00000001 == 0) {}

	
	//hw_outputFrame.copyTo(outFrame);
	//outFrame.convertTo(outFrame, CV_8UC1);
	//imshow("Input", outFrame);	

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

void * setup_reserved_mem(uint input_address) //returns a pointer in userspace to the device
{
    void *mapped_base_reserved_mem;
    int memfd_reserved_mem;
    void *mapped_dev_base;
    off_t dev_base = input_address;

    memfd_reserved_mem = open("/dev/mem", O_RDWR | O_SYNC); //to open this the program needs to be run as root
        if (memfd_reserved_mem == -1) {
        printf("Can't open /dev/mem.\n");
        return NULL;
    }
    //printf("/dev/mem opened.\n"); 

    // Map one page of memory into user space such that the device is in that page, but it may not
    // be at the start of the page.

    mapped_base_reserved_mem = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd_reserved_mem, dev_base & ~MAP_MASK);
        if (mapped_base_reserved_mem == (void *) -1) {
        printf("Can't map the memory to user space.\n");
        return NULL;
    }
     //printf("Memory mapped at address %p.\n", mapped_base_reserved_mem); 

    // get the address of the device in user space which will be an offset from the base 
    // that was mapped as memory is mapped at the start of a page 

    mapped_dev_base = mapped_base_reserved_mem + (dev_base & MAP_MASK);
    return mapped_dev_base;
}


