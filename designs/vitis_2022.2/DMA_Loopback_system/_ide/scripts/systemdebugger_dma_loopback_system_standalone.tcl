# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: D:\Yannos\FILES\PROGRAMS\FPGA\SoC\DMA_Loopback\designs\vitis_2022.2\DMA_Loopback_system\_ide\scripts\systemdebugger_dma_loopback_system_standalone.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source D:\Yannos\FILES\PROGRAMS\FPGA\SoC\DMA_Loopback\designs\vitis_2022.2\DMA_Loopback_system\_ide\scripts\systemdebugger_dma_loopback_system_standalone.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent Cora Z7 - 7007S 210370B9F0DCA" && level==0 && jtag_device_ctx=="jsn-Cora Z7 - 7007S-210370B9F0DCA-13723093-0"}
fpga -file D:/Yannos/FILES/PROGRAMS/FPGA/SoC/DMA_Loopback/designs/vitis_2022.2/DMA_Loopback/_ide/bitstream/DMA_Loopback_v3.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw D:/Yannos/FILES/PROGRAMS/FPGA/SoC/DMA_Loopback/designs/vitis_2022.2/DMA_Loopback_v1/export/DMA_Loopback_v1/hw/DMA_Loopback_v3.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source D:/Yannos/FILES/PROGRAMS/FPGA/SoC/DMA_Loopback/designs/vitis_2022.2/DMA_Loopback/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow D:/Yannos/FILES/PROGRAMS/FPGA/SoC/DMA_Loopback/designs/vitis_2022.2/DMA_Loopback/Debug/DMA_Loopback.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con
