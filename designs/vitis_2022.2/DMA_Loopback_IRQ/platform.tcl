# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct D:\Yannos\FILES\PROGRAMS\FPGA\SoC\DMA_Loopback\designs\vitis_2022.2\DMA_Loopback_IRQ\platform.tcl
# 
# OR launch xsct and run below command.
# source D:\Yannos\FILES\PROGRAMS\FPGA\SoC\DMA_Loopback\designs\vitis_2022.2\DMA_Loopback_IRQ\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {DMA_Loopback_IRQ}\
-hw {D:\Yannos\FILES\PROGRAMS\FPGA\SoC\DMA_Loopback\designs\vivado_2022.2\DMA_Loopback_IRQ\DMA_Loopback_IRQ_v1.xsa}\
-proc {ps7_cortexa9_0} -os {standalone} -out {D:/Yannos/FILES/PROGRAMS/FPGA/SoC/DMA_Loopback/designs/vitis_2022.2}

platform write
platform generate -domains 
platform active {DMA_Loopback_IRQ}
platform generate
domain active {zynq_fsbl}
bsp reload
domain active {standalone_domain}
bsp reload
platform active {DMA_Loopback_IRQ}
bsp reload
platform generate -domains 
platform clean
platform generate
