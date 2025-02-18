/********************************************
 * Local Includes
 *******************************************/

#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"

/********************************************
 * Local defines
 *******************************************/

#define DMA_DEV_ID XPAR_AXIDMA_0_DEVICE_ID

#define MEM_BASE_ADDR 0x01000000

#define TX_BUFFER_BASE (MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE (MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH (MEM_BASE_ADDR + 0x004FFFFF)

#define MAX_PKT_LEN 0x20

#define TEST_START_VALUE 0x1
#define INCR 1

#define NUMBER_OF_TRANSFERS 10

/********************************************
 * Function decleration
 *******************************************/

extern void xil_printf(const char *format, ...);
int dma_init(void);
static int CheckData(void);

/********************************************
 * Global instances
 *******************************************/

/** The XAxiDma driver instance data.
 * An instance must be allocated for each DMA engine in use.
 * Basically a DMA handler.
 * MUST BE GLOBAL
 */

XAxiDma hAxiDma;

/*****************************************************************************/
/**
 * The entry point for this example. It invokes the example function,
 * and reports the execution status.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if example finishes successfully
 *		- XST_FAILURE if example fails.
 *
 * @note		None.
 *
 ******************************************************************************/
int main()
{

    /*--------------------------------------------
     Local variables
    ---------------------------------------------*/

    int Status;
    int Index;
    u8 *TxBufferPtr;
    u8 *RxBufferPtr;
    u8 Value;

    xil_printf("\r\n-----------------------------------------------------------------------\r\n");
    xil_printf("\t DMA LOOPBACK EXAMPLE \r\n");
    xil_printf("-----------------------------------------------------------------------\r\n");

    /*--------------------------------------------
     Dma initialization
    ---------------------------------------------*/

    Status = dma_init();
    if (Status != XST_SUCCESS)
    {
        xil_printf("Error in DMA initialization.\r\n Returned: %d\r\n", Status);
        return Status;
    }

    /*--------------------------------------------
     Creation of buffers and initialization
    ---------------------------------------------*/

    TxBufferPtr = (u8 *)TX_BUFFER_BASE;
    RxBufferPtr = (u8 *)RX_BUFFER_BASE;

    Value = TEST_START_VALUE;

    for (Index = 0; Index < MAX_PKT_LEN; Index++)
    {
        TxBufferPtr[Index] = Value;

        Value = (Value + INCR) & 0xFF;
    }

    /*--------------------------------------------
     Flush the cache
    ---------------------------------------------*/

    Xil_DCacheFlushRange((UINTPTR)TxBufferPtr, MAX_PKT_LEN);
    Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, MAX_PKT_LEN);

    xil_printf("\r\nRx Buffer before DMA:\r\n");
    for (Index = 0; Index < MAX_PKT_LEN; Index++)
        xil_printf("Rx[%d] = %d\r\n", Index, (int)(RxBufferPtr[Index]));
    xil_printf("\r\n\r\n");

    /*--------------------------------------------
     Start the DMA transfer
    ---------------------------------------------*/

    // will try ten transfers
    for (Index = 0; Index < NUMBER_OF_TRANSFERS; Index++)
    {

        // transmission
        Status = XAxiDma_SimpleTransfer(&hAxiDma, (UINTPTR)RxBufferPtr, MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);

        if (Status != XST_SUCCESS)
        {
            return XST_FAILURE;
        }

        // receive
        Status = XAxiDma_SimpleTransfer(&hAxiDma, (UINTPTR)TxBufferPtr, MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

        if (Status != XST_SUCCESS)
        {
            return XST_FAILURE;
        }

        while ((XAxiDma_Busy(&hAxiDma, XAXIDMA_DEVICE_TO_DMA)) || (XAxiDma_Busy(&hAxiDma, XAXIDMA_DMA_TO_DEVICE)))
        {
            /* Wait */
        }

        // poll to wait to finish - no interrupts set
        Status = CheckData();
        if (Status != XST_SUCCESS)
        {
            return XST_FAILURE;
        }
    }

    xil_printf("\r\nRx Buffer after DMA:\r\n");
    for (Index = 0; Index < MAX_PKT_LEN; Index++)
        xil_printf("Rx[%d] = %d\r\n", Index, (int)(RxBufferPtr[Index]));
    xil_printf("\r\n\r\n");

    /* Test finishes successfully
     */

    xil_printf("Successfully ran XAxiDma_SimplePoll Example\r\n");

    xil_printf("\r\n-----------------------------------------------------------------------\r\n");
    xil_printf("\t END OF EXAMPLE \r\n");
    xil_printf("-----------------------------------------------------------------------\r\n");

    return XST_SUCCESS;
}

/*****************************************************************************
 * This function checks data buffer after the DMA transfer is finished.
 *
 * @param	None
 *
 * @return
 * - XST_SUCCESS if validation is successful.
 *
 * - XST_FAILURE otherwise.
 */
static int CheckData(void)
{
    u8 *RxPacket;
    int Index = 0;
    u8 Value;

    RxPacket = (u8 *)RX_BUFFER_BASE;
    Value = TEST_START_VALUE;

    /* Invalidate the DestBuffer before receiving the data, in case the
     * Data Cache is enabled
     */
    Xil_DCacheInvalidateRange((UINTPTR)RxPacket, MAX_PKT_LEN);

    for (Index = 0; Index < MAX_PKT_LEN; Index++)
    {
        if (RxPacket[Index] != Value)
        {
            xil_printf("Data error %d: %x/%x\r\n",
                       Index, (unsigned int)RxPacket[Index],
                       (unsigned int)Value);

            return XST_FAILURE;
        }
        Value = (Value + INCR) & 0xFF;
    }

    return XST_SUCCESS;
}

/*****************************************************************************
 * Initialize the AXI DMA Engine
 *
 * @param handler a pointer to the pointer to a struct of type `XAxiDma`
 *
 * @return DMA status
 */
int dma_init(void)
{
    int status;

    XAxiDma_Config *dmaConfig_p; // pointer to configuration struct

    /*--------------------------------------------
     Fill-up configuration struct
     - the define can be found in:
        + xparameters.h
        + "Hardware Platform Specification" GUI

     WARNING: Not sure if I have scatter-gather
    ---------------------------------------------*/

    dmaConfig_p = XAxiDma_LookupConfig(DMA_DEV_ID);
    if (!dmaConfig_p)
        return XST_FAILURE;

    /*--------------------------------------------
     Initialize the handler with
     the configuration struct

     WARNING: Not sure if I have scatter-gather
    ---------------------------------------------*/

    status = XAxiDma_CfgInitialize(&hAxiDma, dmaConfig_p);
    if (status != XST_SUCCESS)
        return status;

    /*--------------------------------------------
     Make sure you have no scatter gather
    ---------------------------------------------*/

    if (XAxiDma_HasSg(&hAxiDma))
        return XST_FAILURE;

    /*--------------------------------------------
     Disable interrupts, this is a polling example
    ---------------------------------------------*/

    XAxiDma_IntrDisable(&hAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    XAxiDma_IntrDisable(&hAxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

    return XST_SUCCESS;
}
