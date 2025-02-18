/********************************************
 * Local Includes
 *******************************************/

#include <stdio.h>
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

#define PKT_ELEMENTS 32

#define TEST_START_VALUE 3.5
#define INCR 2.0

#define NUMBER_OF_TRANSFERS 10

/********************************************
 * Function decleration
 *******************************************/

// extern void xil_printf(const char *format, ...);
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
 * @return
 * - XST_SUCCESS if example finishes successfully
 *
 * - XST_FAILURE if example fails.
 ******************************************************************************/
int main()
{

    /*--------------------------------------------
     Local variables
    ---------------------------------------------*/

    int Status;
    int Index;
    float *TxBufferPtr;
    float *RxBufferPtr;
    float Value;

    printf("\r\n-----------------------------------------------------------------------\r\n");
    printf("\t DMA LOOPBACK EXAMPLE WITH VARIOUS DATATYPES \r\n");
    printf("-----------------------------------------------------------------------\r\n");

    /*--------------------------------------------
     Dma initialization
    ---------------------------------------------*/

    Status = dma_init();
    if (Status != XST_SUCCESS)
    {
        printf("Error in DMA initialization.\r\n Returned: %d\r\n", Status);
        return Status;
    }

    /*--------------------------------------------
     Creation of buffers and initialization
    ---------------------------------------------*/

    TxBufferPtr = (float *)TX_BUFFER_BASE;
    RxBufferPtr = (float *)RX_BUFFER_BASE;

    Value = TEST_START_VALUE;

    for (Index = 0; Index < PKT_ELEMENTS; Index++)
    {
        TxBufferPtr[Index] = Value;

        // Value = (float)((u32)(Value + INCR) & 0xFFFF);
        Value = Value + INCR;
    }

    /*--------------------------------------------
     Flush the cache
    ---------------------------------------------*/

    Xil_DCacheFlushRange((UINTPTR)TxBufferPtr, PKT_ELEMENTS * sizeof(float));
    Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, PKT_ELEMENTS * sizeof(float));

    printf("\r\nRx Buffer before DMA:\r\n");
    for (Index = 0; Index < PKT_ELEMENTS; Index++)
        printf("Rx[%d] = %f\r\n", Index, (float)(RxBufferPtr[Index]));
    printf("\r\n\r\n");

    /*--------------------------------------------
     Start the DMA transfer
    ---------------------------------------------*/

    // will try ten transfers
    for (Index = 0; Index < NUMBER_OF_TRANSFERS; Index++)
    {

        // transmission
        Status = XAxiDma_SimpleTransfer(&hAxiDma, (UINTPTR)RxBufferPtr, PKT_ELEMENTS * sizeof(float), XAXIDMA_DEVICE_TO_DMA);

        if (Status != XST_SUCCESS)
        {
            return XST_FAILURE;
        }

        // receive
        Status = XAxiDma_SimpleTransfer(&hAxiDma, (UINTPTR)TxBufferPtr, PKT_ELEMENTS * sizeof(float), XAXIDMA_DMA_TO_DEVICE);

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

    printf("\r\nRx Buffer after DMA:\r\n");
    for (Index = 0; Index < PKT_ELEMENTS; Index++)
        printf("Rx[%d] = %f\r\n", Index, (float)(RxBufferPtr[Index]));
    printf("\r\n\r\n");

    /* Test finishes successfully
     */

    printf("Successfully ran XAxiDma_SimplePoll Example\r\n");

    printf("\r\n-----------------------------------------------------------------------\r\n");
    printf("\t END OF EXAMPLE \r\n");
    printf("-----------------------------------------------------------------------\r\n");

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
    float *RxPacket;
    int Index = 0;
    float Value;

    RxPacket = (float *)RX_BUFFER_BASE;
    Value = TEST_START_VALUE;

    /* Invalidate the DestBuffer before receiving the data, in case the
     * Data Cache is enabled
     */
    Xil_DCacheInvalidateRange((UINTPTR)RxPacket, PKT_ELEMENTS * sizeof(float));

    for (Index = 0; Index < PKT_ELEMENTS; Index++)
    {
        if (RxPacket[Index] != Value)
            printf("Data error %d: %f instead of %f\r\n", Index, (float)RxPacket[Index], (float)Value);
        // Value = (float)((u32)(Value + INCR) & 0xFFFF);
        Value = Value + INCR;
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
