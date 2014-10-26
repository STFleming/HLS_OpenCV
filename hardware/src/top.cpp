 #include "top.h"

 void image_filter(AXI_STREAM& input, AXI_STREAM& output, int rows, int cols) {
     //Create AXI streaming interfaces for the core
 #pragma HLS RESOURCE variable=input core=AXIS metadata="-bus_bundle INPUT_STREAM"
 #pragma HLS RESOURCE variable=output core=AXIS metadata="-bus_bundle OUTPUT_STREAM"

 #pragma HLS RESOURCE core=AXI_SLAVE variable=rows metadata="-bus_bundle CONTROL_BUS"
 #pragma HLS RESOURCE core=AXI_SLAVE variable=cols metadata="-bus_bundle CONTROL_BUS"
 #pragma HLS RESOURCE core=AXI_SLAVE variable=return metadata="-bus_bundle CONTROL_BUS"

 #pragma HLS INTERFACE ap_stable port=rows
 #pragma HLS INTERFACE ap_stable port=cols

     GRAY_IMAGE img_0(rows, cols);
     GRAY_IMAGE img_1(rows, cols);
     GRAY_IMAGE img_2(rows, cols);

 #pragma HLS dataflow
     hls::AXIvideo2Mat(input, img_0);
     hls::Sobel<1,0,3>(img_0, img_1);
     hls::Sobel<0,1,3>(img_1, img_2);
     hls::Mat2AXIvideo(img_2, output);
 }
