#include <stdio.h>

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
#define OUTPUT_FRAME_ADDR 0x3a284800

#define HLS_ADDR 0x43C00000

//VDMA setup parameters
#define VDMAWidth 600
#define VDMAHeight 400
#define VDMABaseAddr 0x43000000

#define MAP_SIZE 4000000UL
#define MAP_MASK (MAP_SIZE - 1)

void *setup_reserved_mem(uint input_address);


int main()
{
	printf("----------------------------\nTesting the reserved memory\n----------------------------\n\n");

	cv::VideoCapture cap("http://iphone-streaming.ustream.tv/uhls/17074538/streams/live/iphone/playlist.m3u8"); 
	if(!cap.isOpened())
	{	std::cout << "Error no camera detected." << std::endl;
		return -1; }
	

	//Input container
	cv::Mat inFrame; //Temporary input container
	//inFrame = cv::imread("datalogger.jpg"); //Read the input just to get size
	cap >> inFrame;	

	cvtColor(inFrame, inFrame, CV_RGB2GRAY);
	//In case we want to change size
	inFrame.setTo(cv::Scalar(255)); //Set the image ot white?

	//Convert the inFrame to the format that is recognised by the hardware
	inFrame.convertTo(inFrame, CV_32SC1);
	
	cv::Mat hw_inputFrame = inFrame.clone();
	hw_inputFrame.data =(uchar *) setup_reserved_mem(OUTPUT_FRAME_ADDR); //Point the input container to the reserved RAM
	inFrame.copyTo(hw_inputFrame);		

	//Output container
	cv::Mat hwOutputFrame = inFrame.clone(); 
	hwOutputFrame.data = (uchar *) setup_reserved_mem(OUTPUT_FRAME_ADDR);	
	//cvtColor(hwOutputFrame, hwOutputFrame, CV_RGB2RGBA);

	cv::Mat outFrame;
	hwOutputFrame.copyTo(outFrame);
	//outFrame.convertTo(outFrame, CV_8UC3); //Convert into the right output format
	imshow("Output", outFrame); //displaying the output frame

	//Display the input
	cv::Mat theInputFrame = inFrame.clone();
	theInputFrame.data = (uchar *) setup_reserved_mem(INPUT_FRAME_ADDR);
	cv::Mat disp_InputFrame;
	theInputFrame.copyTo(disp_InputFrame);
	disp_InputFrame.convertTo(disp_InputFrame, CV_8UC1);
	imshow("Input", disp_InputFrame);	

	//uint8_t *input_img = (uint8_t*)setup_reserved_mem(INPUT_FRAME_ADDR);
	//uint8_t *output_img = (uint8_t*)setup_reserved_mem(OUTPUT_FRAME_ADDR);
	//int j=0;
	//int i=0;
	//int diff_count = 0;

while(1)
{
	
	theInputFrame.copyTo(disp_InputFrame);
	disp_InputFrame.convertTo(disp_InputFrame, CV_8UC1);
	hwOutputFrame.copyTo(outFrame);
	outFrame.convertTo(outFrame, CV_8UC1);
	imshow("Input", disp_InputFrame);
	imshow("Output", outFrame); //displaying the output frame
	

	//usleep(100000);

	//cvtColor(outFrame, outFrame, CV_RGBA2RGB);

	//We need to implement our own copying function to do this so that the alpha channel is preserved.
/*	int rows, cols;
	int red=0, green=0, blue=0;
	for (cols = 0; cols < hwOutputFrame.cols; cols++)
	{
		red =0; green = 0; blue = 0;
		for(rows=0; rows < hwOutputFrame.rows; rows++)
		{
		  red += hwOutputFrame.at<cv::Vec3b>(rows,cols)[0];
		  green += hwOutputFrame.at<cv::Vec3b>(rows,cols)[1];
		  blue += hwOutputFrame.at<cv::Vec3b>(rows,cols)[2];
		}  
		if(red == 0) {std::cout << "RED MISSING" << std::endl;}
		if(green == 0) {std::cout << "GREEN MISSING" << std::endl;}
		if(blue == 0) {std::cout << "BLUE MISSING" << std::endl;}
		std::cout << cols << "\trt=" << red << "\tgt=" << green << "\trb=" << blue << std::endl;

	}
*/
        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       {
            break;
       }
}

	return 0;
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


