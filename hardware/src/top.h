 #ifndef _TOP_H_
 #define _TOP_H_

 #include "hls_video.h"

 // maximum image size
 #define MAX_WIDTH  1920
 #define MAX_HEIGHT 1080

 // typedef video library core structures
 typedef hls::stream<ap_axiu<32,1,1,1> >               AXI_STREAM;
 typedef hls::Scalar<3, unsigned char>                 RGB_PIXEL;
 typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_32SC1>    GRAY_IMAGE;

 // top level function for HW synthesis
 void image_filter(AXI_STREAM& src_axi, AXI_STREAM& dst_axi, int rows, int cols);

 #endif
