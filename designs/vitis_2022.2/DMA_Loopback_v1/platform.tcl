# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct D:\Yannos\FILES\PROGRAMS\FPGA\SoC\DMA_Loopback\designs\vitis_2022.2\DMA_Loopback_v1\platform.tcl
# 
# OR launch xsct and run below command.
# source D:\Yannos\FILES\PROGRAMS\FPGA\SoC\DMA_Loopback\designs\vitis_2022.2\DMA_Loopback_v1\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {DMA_Loopback_v1}\
-hw {D:\Yannos\FILES\PROGRAMS\FPGA\SoC\DMA_Loopback\designs\vivado_2022.2\DMA_Loopback\DMA_Loopback_v1.xsa}\
-out {D:/Yannos/FILES/PROGRAMS/FPGA/SoC/DMA_Loopback/designs/vitis_2022.2}

platform write
domain create -name {standalone_ps7_cortexa9_0} -display-name {standalone_ps7_cortexa9_0} -os {standalone} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform active {DMA_Loopback_v1}
domain active {zynq_fsbl}
domain active {standalone_ps7_cortexa9_0}
platform generate -quick
bsp reload
platform generate
platform clean
platform clean
platform clean
platform generate
platform clean
platform generate
platform clean
platform config -updatehw {D:/Yannos/FILES/PROGRAMS/FPGA/SoC/DMA_Loopback/designs/vivado_2022.2/DMA_Loopback/DMA_Loopback_v1.xsa}
platform generate
platform clean
platform clean
platform clean
platform active {DMA_Loopback_v1}
platform config -updatehw {D:/Yannos/FILES/PROGRAMS/FPGA/SoC/DMA_Loopback/designs/vivado_2022.2/DMA_Loopback/DMA_Loopback_v2.xsa}
platform generate
platform clean
platform config -updatehw {D:/Yannos/FILES/PROGRAMS/FPGA/SoC/DMA_Loopback/designs/vivado_2022.2/DMA_Loopback/DMA_Loopback_v3.xsa}
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform active {DMA_Loopback_v1}
bsp reload
platform clean
platform generate
platform clean
platform active {DMA_Loopback_v1}
platform generate
