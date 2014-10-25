############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 2013 Xilinx Inc. All rights reserved.
############################################################
open_project hls_ip
set_top image_filter
add_files ./src/top.cpp
add_files ./src/top.h
open_solution "solution"
set_part {xc7z020clg484-1}
create_clock -period 10 -name default

#csim_design
csynth_design
#cosim_design -trace_level none
export_design -format ip_catalog -description "Example OpenCV style image processing core" -vendor "Imperial" -library "OpenCV" -version "1.0"
