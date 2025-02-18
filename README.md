# Example Loopback project on Zynq-7000 SoC

This project is the simplest test to know that you configured the DMA correctly: just send data to it and get them back. The only thing that resides in the PL along the DMA controller is a FIFO that connects the input stream to the output stream. The DMA is set up without any interrupts, so the software uses polling.

This project was developed on a Windows 11 machine with Vivado and Vitis 22.2. The SoC used is the Digilent [Cora Z7S](https://digilent.com/reference/programmable-logic/cora-z7/start?srsltid=AfmBOop1WbKCwSRevTnR1wBTyK89O_web_5MYcPAdbv43u49Iq8fW7U7) FPGA development board.

This repository contains the vivdao project that generated the hardware description files and the Vitis workspace with the software used. I decided not to leave the entire xilinx software support libraries out of this repository (even if they make it very large) so that the viewer of the repository can test that if she follows the steps I describe in the below section then similar project structures will emerge.

## VIVADO side

### Vivado IP integrator - Block Design 1

This is how the block design [design_1](./designs/vivado_2022.2/DMA_Loopback/DMA_Loopback.gen/sources_1/bd/design_1/) was made.

- insert a Zynq PS
  - in PS-PL configuration activate an AXI HP interface (I chose 32 bit data width)
  - from the schematic it has know that you neeed the following AXI interfaces:
    - a 32b AXI Master Port (usually the first one is `M_AXI_GP0`) which will connect and control the DMA via AXI interconnect
    - a high performance AXI Slave interface that as you see connects to the DDR memory controler (usually first one is `S_AXI_HP0`)
    - **do not run connection automation yet but you have to run board automation**
- add a processing system reset
- add an AXI DMA (the simple one)
  - make sure you have same stream data width as you specified above, both on the read and write channels
  - if you intent on having continuous data in memory you can disable scatter gather engine
  - careful with the buffer length because I had problems later with vitis - the number of valid bits it says, when raised as an exponent of two, must produce a number larger to the length of the buffer you want to transfer from the PS. _It is best to just leave it at 14_.
- run connection automation and block automation. This will most likely insert an AXI interconnect that you had not inserted. Beware the following:
  - the AXI Master interface in the Zynq must go throught an AXI _peripheral_ interconnect and connect to the DMA `S_AXI_LITE` port. This is the interface from which the PS has access and configures the DMA controller
  - the DMA controler master control ports, which are `M_AXI_MM2S` and `M_AXI_S2MM` must got through an AXI _memory_ interconnect and connect to the high performance AXI slave interface in the Zynq system. This interface is what makes the PS and the DMA controller talk to the DDR memory controler. If you had scatter gather then it should have its own master port in the DMA peripheral and be connected in a similar fashion.
- add a FIFO generator
  - make it AXI Stream and use common clock
  - `TDATA NUM BYTES` must be 4 bytes for 32 bit data
  - You want no `TUSER` so add 0 to `TUSER WIDTH`
  - Add `TLAST`
  - Connect the reset and the clock
- Connect the FIFO to the DMA 
  - Connect the SLAVE interface S_AXIS of the FIFO to  M_AXIS_MM2S interface on the DMA block.
  - Connect the master M_AXIS interface of the FIFO to the S_AXIS_S2MM in the DMA
- make sure you have connected the clocks to the clock line and the resets to the reset line
  - **WARNING**: *AXI reset is **active low***
- Run validation (checkbox)
- in the address editor there are some DMA addresses but nothing you cannot do on Vitis
- create the HDL wrapper and generate bitstream
- export hardware as .xsa file (include bitstream)

For reference the Block Design should look like the following picture:

![block_design](./images/Screenshot%202025-02-18%20164627.png)

_The final exported hardware had the following three versions:_
+ _[version 1](./designs/vivado_2022.2/DMA_Loopback/DMA_Loopback_v1.xsa) had left out the Zynq slave HP to DMA master interfaces_
+ _[version 2](./designs/vivado_2022.2/DMA_Loopback/DMA_Loopback_v2.xsa) had wrong DMA buffer length_
+ _[version 3](./designs/vivado_2022.2/DMA_Loopback/DMA_Loopback_v3.xsa) has DMA buffer length at 14_

<br>

## Vitis Side

This is the description of the DMA_Loopback application.

- you will probably need to configure the workspace a bit, I do not know how you left Vitis the last time. Make it point to the [vitis folder](./designs/vitis_2022.2/)
- create 'Application Project'
- Create new platform tab go and find your .xsa - make sure generate boot components is ticked
- go over the other tabs that are somewhat pre-configured. I will try to go with the empty template
- Just a reminder of the folders:
  - [`DMA_Loopback_v1`](./designs/vitis_2022.2/DMA_Loopback_v1/) is the hardware platform and you can find addresses and hardware info, and with right click even change the .xsa
  - [`DMA_Loopback`](./designs/vitis_2022.2/DMA_Loopback/) is the software source folder where you write your code
  - [`DMA_Loopback_System](./designs/vitis_2022.2/DMA_Loopback_system/) in Vitis seems to encompass both the other two directories and in VSCode it seems seperate. Either way it has project info.
- The empty template does not even have a `main.c` so we create it in the [`source`](./designs/vitis_2022.2/DMA_Loopback/src/) folder. All includes will be inserted by hand
- you can follow the comments on the code to know what to do

This is the main application but then I added some more to test some concepts using the same hardware platform. This means that the final [.xsa](./designs/vivado_2022.2/DMA_Loopback/DMA_Loopback_v3.xsa) used with the generated hardware platform ([`DMA_Loopback_v1`](./designs/vitis_2022.2/DMA_Loopback_v1/)) is the same but on top I add more applications (with the blue boxes like you insert the examples).

**_Using different data types in the transfer_**

The system application [`DMA_Loopback_datatypes`](./designs/vitis_2022.2/DMA_Loopback_datatypes/) has the same functionality as the above system application but this one utilizes single-precision floating point numbers (`float` types). An important difference is that `printf` is used for debugging instead of the lightweight `xil_printf` tha has no support for `float` datatypes.

**_Memory allocation with malloc_**

This system application [`DMA_Loopback_memorysafe`](./designs/vitis_2022.2/DMA_Loopback_memorysafe/) has the same functionality as the previous examples but makes no use of defines to access DDR memory. Instead it relies on the fact that in the [linker script](./designs/vitis_2022.2/DMA_Loopback_memorysafe/src/lscript.ld) the heap memory base is the beginning of the DDR memory. Thus every `malloc` or `calloc` call allocates memory on the heap, so as to say *dynamically in the DDR* which is incredibly useful in large programs, where we wish to avoid memory conflicts.
  
### Tips
- some files that are better to check out on Vitis cause they are GUI based, even though you can do mass changes on them here using VIM.
- when you create files in vitis it automatically fills in some boiler plate code
- in vitis you can choose what to build. Especially after a clean you can either build the platform or the application
- if you want to add some examples to help you go to the gui from the [`DMA_Loopback.prj`](./designs/vitis_2022.2/DMA_Loopback/DMA_Loopback.prj) and choose "Navigate to BSP Settings". This opens the [`platform.spr`](./designs/vitis_2022.2/DMA_Loopback_v1/platform.spr) GUI and in the box belwo it shows all available and initialized drivers, where you can import examples.