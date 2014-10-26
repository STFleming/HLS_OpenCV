#include "pthread.h"
#include <unistd.h>
#include <stdio.h> 
#include <sys/mman.h>
 
#define xVDMACount 64
#define xVDMAMapLen xVDMACount*4
 
typedef struct
{
    unsigned int baseAddr;
    int vdmaHandler;
    int width;
    int height;
    int pixelLength;
    int fbLength;
    unsigned int* vdmaVirtualAddress;
 
    pthread_mutex_t lock;
} xVDMA_info;
 
int xVDMA_Init(xVDMA_info *info, unsigned int baseAddr, int width, int height, int pixelLength);
void xVDMA_UnInit(xVDMA_info *info);
unsigned int xVDMA_Get(xVDMA_info *info, int num);
void xVDMA_Set(xVDMA_info *info, int num, unsigned int val);
void xVDMA_Start(xVDMA_info *info, unsigned int inadr);
void xVDMA_OutStart(xVDMA_info *info, unsigned int adr, int circular);
int xVDMA_IsRunning(xVDMA_info *info);
int xVDMA_IsDone(xVDMA_info *info);
void xVDMA_Disp(xVDMA_info *info, char *str, int num);
void xVDMA_kick(xVDMA_info *info, unsigned int inadr, unsigned int outadr); 
 
 
 
int xVDMA_Init(xVDMA_info *info, unsigned int baseAddr, int width, int height, int pixelLength)
{
    info->baseAddr=baseAddr;
    info->width=width;
    info->height=height;
    info->pixelLength=pixelLength;
    info->fbLength=pixelLength*width*height;
    info->vdmaHandler = open("/dev/mem", O_RDWR);
    info->vdmaVirtualAddress = (unsigned int*)mmap(NULL, xVDMAMapLen, PROT_READ | PROT_WRITE, MAP_SHARED, info->vdmaHandler, (off_t)info->baseAddr);
    if(info->vdmaVirtualAddress == MAP_FAILED)
    {
     perror("vdmaVirtualAddress mapping for absolute memory access failed.\n");
     return -1;
    }
 
    return 0;
}
 
void xVDMA_UnInit(xVDMA_info *info)
{
    munmap((void *)info->vdmaVirtualAddress, xVDMAMapLen);
    close(info->vdmaHandler);
}
 
unsigned int xVDMA_Get(xVDMA_info *info, int num)
{
    if(num>=0 && num<xVDMACount)
    {
        return info->vdmaVirtualAddress[num];
    }
    return 0;
}
 
void xVDMA_Set(xVDMA_info *info, int num, unsigned int val)
{
    if(num>=0 && num<xVDMACount)
    {
        info->vdmaVirtualAddress[num]=val;
    }
}
 

void xVDMA_kick(xVDMA_info *info, unsigned int inadr, unsigned int outadr)
{

   xVDMA_Set(info, 0x00/4, 1); //Start the MM2S (input).
   xVDMA_Set(info, 0x30/4, 1); //Start the S2MM (output). 	
 
   xVDMA_Set(info, 0x5C/4, inadr); //Address for MM2S (input).
   xVDMA_Set(info, 0xAC/4, outadr); //Address for S2MM (output). 

   xVDMA_Set(info, 0x58/4, info->pixelLength*info->width); //MM2S stride (input).
   xVDMA_Set(info, 0xA8/4, info->pixelLength*info->width); //S2MM stride (output) 
   
   xVDMA_Set(info, 0x54/4, info->pixelLength*info->width); //Horizontal size MM2S (input).
   xVDMA_Set(info, 0xA4/4, info->pixelLength*info->width); //Horizontal size S2MM (output).

   xVDMA_Set(info, 0x50/4, info->height); //Vertical size MM2S (input).
   xVDMA_Set(info, 0xA0/4, info->height); //Vertical size S2MM (output). 

   return;
}

void xVDMA_OutStart(xVDMA_info *info, unsigned int adr, int circular)
{
    xVDMA_Disp(info, "status ", 0x04/4);
    xVDMA_Disp(info, "control ", 0x00/4);
 
    xVDMA_Set(info, 0x00/4, circular==1?4+2:4);  // MM2S_DMACR: reset
 
    while((xVDMA_Get(info, 0x00/4)&4)==4); // wait for reset end
 
    xVDMA_Disp(info, "status ", 0x04/4);
    xVDMA_Set(info, 0x04/4, 0xffffffff);  // S2MM_DMASR: remove errors
    xVDMA_Disp(info, "status ", 0004/4);
 
    usleep(100000);
 
    xVDMA_Set(info, 0x18/4, 1);  // MM2S_FRMSTORE: 1
 
    xVDMA_Set(info, 0x00/4, circular==1?2+1:1);  // S2MM_DMACR: RS
    // wait for run
    while((xVDMA_Get(info, 0x00/4)&1)==0 || (xVDMA_Get(info, 0x04/4)&1)==1)
    {
        xVDMA_Disp(info, "status ", 0004/4);
        xVDMA_Disp(info, "control ", 0x00/4);
    }
 
    xVDMA_Set(info, 0x28/4, 0); // this alters s2mm park ptr
 
    xVDMA_Set(info, 0x5C/4, adr); //adr1
//  xVDMA_Set(info, 0x60/4, adr); //adr2
 
    xVDMA_Set(info, 0x58/4, info->pixelLength*info->width);  // stride length in bytes
 
    xVDMA_Set(info, 0x54/4, info->pixelLength*info->width);  // length in bytes
 
    xVDMA_Set(info, 0x50/4, info->height);  // height and start
 
    xVDMA_Disp(info, "status ", 0x04/4);
    xVDMA_Disp(info, "control ", 0x00/4);
    xVDMA_Disp(info, "Park", 0x28/4);
}
 
void xVDMA_Start(xVDMA_info *info, unsigned int inadr)
{
    xVDMA_Set(info, 0x30/4, 64+4);  // S2MM_DMACR: sof=tuser, reset
 
    while((xVDMA_Get(info, 0x30/4)&4)==4); // wait for reset end
 
    xVDMA_Disp(info, "status ", 0x34/4);
    xVDMA_Set(info, 0x34/4, 0xffffffff);  // S2MM_DMASR: remove errors
    xVDMA_Disp(info, "status ", 0x34/4);
 
 
    xVDMA_Set(info, 0x30/4, 64+1);  // S2MM_DMACR: sof=tuser, RS
// wait for run
    while((xVDMA_Get(info, 0x30/4)&1)==0 || (xVDMA_Get(info, 0x34/4)&1)==1)
    {
        xVDMA_Disp(info, "status ", 0x34/4);
        xVDMA_Disp(info, "control ", 0x30/4);
    }
 
    xVDMA_Set(info, 0x5C/4, inadr);
    xVDMA_Set(info, 0x28/4, 0);
 
    xVDMA_Set(info, 0xA8/4, info->pixelLength*info->width);  // stride length in bytes
 
    xVDMA_Set(info, 0xA4/4, info->pixelLength*info->width);  // length in bytes
 
    xVDMA_Set(info, 0xA0/4, info->height);  // height and start
    xVDMA_Disp(info, "status ", 0x34/4);
    xVDMA_Disp(info, "control ", 0x30/4);
}
 
int xVDMA_IsRunning(xVDMA_info *info)
{
    return (xVDMA_Get(info, 0x34/4)&1)==1;
}
 
int xVDMA_IsDone(xVDMA_info *info)
{
    return (xVDMA_Get(info, 0x34/4));
}
 
void xVDMA_Disp(xVDMA_info *info, char *str, int num)
{
    printf("%s(%02x)=%08x\n", str, num, xVDMA_Get(info, num));
}
