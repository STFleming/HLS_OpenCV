#Tcl script that builds a Zynq project using Vivado
#It connects the IP core geneated by GOS and hooks it into the 
#AXI SoC interconnect
#


set current_dir [pwd]
set fpga_part "xc7z020clg484-1"
set temp_prj_name "temp"
set board_name "xilinx.com:zynq:zc702:1.0"
set synth_language "VHDL"
set sim_language "VHDL"
set bd_name "zynq_sys"
set hls_dir "$current_dir/hls_ip/solution/impl/ip"


create_project $temp_prj_name $current_dir/tmp/$temp_prj_name -part $fpga_part -force
set_property board $board_name [current_project]
set_property target_language $synth_language [current_project]
set_property simulator_language $sim_language [current_project]

#Create the block design now that the project is created

create_bd_design "${bd_name}"

#Add the processing system
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0
endgroup
#run block automation on the processing system
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" apply_board_preset "1" }  [get_bd_cells processing_system7_0]

#Add the output directory where the GOS IP core has been created
set_property ip_repo_paths $hls_dir [current_fileset]
update_ip_catalog

#Add the AXI VDMA core
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_vdma:6.2 axi_vdma_0
endgroup
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config {Master "/processing_system7_0/M_AXI_GP0" }  [get_bd_intf_pins axi_vdma_0/S_AXI_LITE]
set_property offset 0x43C00000 [get_bd_addr_segs {processing_system7_0/Data/SEG_axi_vdma_0_Reg}]

#Insert our newly generated vivadoHLS core
startgroup
create_bd_cell -type ip -vlnv Imperial:OpenCV:image_filter:1.0 image_filter_0
endgroup

#Connecting the vivado hls core and the processing system
#AXI slave for configuration
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config {Master "/processing_system7_0/M_AXI_GP0" }  [get_bd_intf_pins image_filter_0/S_AXI_CONTROL_BUS]
set_property offset 0x43000000 [get_bd_addr_segs {processing_system7_0/Data/SEG_image_filter_0_Reg}]

#Connecting the streaming interfaces
connect_bd_intf_net [get_bd_intf_pins image_filter_0/INPUT_STREAM] [get_bd_intf_pins axi_vdma_0/M_AXIS_MM2S] 
connect_bd_intf_net [get_bd_intf_pins image_filter_0/OUTPUT_STREAM] [get_bd_intf_pins axi_vdma_0/S_AXIS_S2MM]

#Expose the HP slave ports
startgroup
set_property -dict [list CONFIG.PCW_USE_S_AXI_HP0 {1} CONFIG.PCW_USE_S_AXI_HP1 {1}] [get_bd_cells processing_system7_0]
endgroup

#Connecting up the VDMA masters
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config {Master "/axi_vdma_0/M_AXI_MM2S" }  [get_bd_intf_pins processing_system7_0/S_AXI_HP0]
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config {Master "/axi_vdma_0/M_AXI_S2MM" }  [get_bd_intf_pins processing_system7_0/S_AXI_HP1]
#Connecting up the VDMA clocks
connect_bd_net -net [get_bd_nets processing_system7_0_FCLK_CLK0] [get_bd_pins axi_vdma_0/m_axis_mm2s_aclk] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net -net [get_bd_nets processing_system7_0_FCLK_CLK0] [get_bd_pins axi_vdma_0/s_axis_s2mm_aclk] [get_bd_pins processing_system7_0/FCLK_CLK0]

#Save the current board design
save_bd_design

#Create System Wrapper
make_wrapper -files [get_files  $current_dir/tmp/$temp_prj_name/$temp_prj_name.srcs/sources_1/bd/$bd_name/${bd_name}.bd] -top
add_files -norecurse $current_dir/tmp/$temp_prj_name/$temp_prj_name.srcs/sources_1/bd/$bd_name/hdl/${bd_name}_wrapper.vhd
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1

#Build the bitstream
reset_run synth_1
reset_run impl_1
launch_runs synth_1
wait_on_run synth_1
launch_runs impl_1
wait_on_run impl_1
launch_runs impl_1 -to_step write_bitstream
wait_on_run impl_1

#Cleaning up
file copy -force $current_dir/tmp/$temp_prj_name/$temp_prj_name.runs/impl_1/${bd_name}_wrapper.bit $current_dir/
# DT10 : For some reason this was all failing, I think it is cygwin's fault
#file delete -force  $current_dir/tmp
#file delete -force $current_dir/ps_clock_registers.log 
#file delete -force $current_dir/vivado.log
#file delete -force $current_dir/vivado.jou
