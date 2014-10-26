
all: hls_ip bitstream

bitstream: zynq_sys_wrapper.bit

zynq_sys_wrapper.bit: hls_ip
	vivado -mode batch -source system/vivado_build.tcl	
	rm *.log *.jou

hls_ip: hls_ip/solution/impl/ip/Imperial_OpenCV_image_filter_1_0.zip

hls_ip/solution/impl/ip/Imperial_OpenCV_image_filter_1_0.zip:
	vivado_hls hls_ip/script.tcl

clean:
	rm -r ./hls_ip/solution 
	rm -r ./tmp
	rm -r zynq_sys_wrapper.bit.bin	
	rm *.log *.jou
