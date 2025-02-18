#include <stdlib.h>

/********************************************
 * Local Includes
 *******************************************/

#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xil_mem.h"

/********************************************
 * Local defines
 *******************************************/

#define DMA_DEV_ID XPAR_AXIDMA_0_DEVICE_ID

#define PKT_ELEMENTS 32

#define TEST_START_VALUE 1
#define INCR 1

#define NUMBER_OF_TRANSFERS 10

/********************************************
 * Function decleration
 *******************************************/

extern void xil_printf(const char *format, ...);
int dma_init(void);
static int CheckData(int32_t *rx_buff_base);

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
    int32_t *txBuff;
    int32_t *rxBuff;
    int32_t Value;

    xil_printf("\r\n-----------------------------------------------------------------------\r\n");
    xil_printf("\t DMA LOOPBACK MEMORY SAFE EXAMPLE \r\n");
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

    txBuff = (int32_t *)calloc(sizeof(int32_t), PKT_ELEMENTS);
    if (txBuff == NULL)
    {
        xil_printf("Error in allocation of txBuff\r\n");
        return XST_FAILURE;
    }
    rxBuff = (int32_t *)calloc(sizeof(int32_t), PKT_ELEMENTS);
    if (rxBuff == NULL)
    {
        xil_printf("Error in allocation of rxBuff\r\n");
        return XST_FAILURE;
    }

    Value = TEST_START_VALUE;

    for (Index = 0; Index < PKT_ELEMENTS; Index++)
    {
        txBuff[Index] = Value;

        Value = Value + INCR;
    }

    /*--------------------------------------------
     Flush the cache
    ---------------------------------------------*/

    Xil_DCacheFlushRange((UINTPTR)txBuff, PKT_ELEMENTS * sizeof(int32_t));
    Xil_DCacheFlushRange((UINTPTR)rxBuff, PKT_ELEMENTS * sizeof(int32_t));

    xil_printf("\r\nRx Buffer before DMA:\r\n");
    for (Index = 0; Index < PKT_ELEMENTS; Index++)
        xil_printf("Rx[%d] = %u\r\n", Index, (rxBuff[Index]));
    xil_printf("\r\n\r\n");

    /*--------------------------------------------
     Start the DMA transfer
    ---------------------------------------------*/

    // will try ten transfers
    for (Index = 0; Index < NUMBER_OF_TRANSFERS; Index++)
    {

        // transmission
        Status = XAxiDma_SimpleTransfer(&hAxiDma, (UINTPTR)rxBuff, PKT_ELEMENTS * sizeof(int32_t), XAXIDMA_DEVICE_TO_DMA);
        if (Status != XST_SUCCESS)
            return Status;

        // receive
        Status = XAxiDma_SimpleTransfer(&hAxiDma, (UINTPTR)txBuff, PKT_ELEMENTS * sizeof(int32_t), XAXIDMA_DMA_TO_DEVICE);
        if (Status != XST_SUCCESS)
            return Status;

        while ((XAxiDma_Busy(&hAxiDma, XAXIDMA_DEVICE_TO_DMA)) || (XAxiDma_Busy(&hAxiDma, XAXIDMA_DMA_TO_DEVICE)))
        {
            /* Wait */
        }

        // poll to wait to finish - no interrupts set
        Status = CheckData(rxBuff);
        if (Status != XST_SUCCESS)
        {
            return XST_FAILURE;
        }
    }

    xil_printf("\r\nRx Buffer after DMA:\r\n");
    for (Index = 0; Index < PKT_ELEMENTS; Index++)
        xil_printf("Rx[%d] = %u\r\n", Index, rxBuff[Index]);
    xil_printf("\r\n\r\n");

    xil_printf("Successfully ran XAxiDma_SimplePoll Example\r\n");

    /*--------------------------------------------
     Memory de-allocations
    ---------------------------------------------*/

    free(txBuff);
    free(rxBuff);

    xil_printf("\r\n-----------------------------------------------------------------------\r\n");
    xil_printf("\t END OF EXAMPLE \r\n");
    xil_printf("-----------------------------------------------------------------------\r\n");

    return XST_SUCCESS;
}

/*****************************************************************************
 * This function checks data buffer after the DMA transfer is finished.
 *
 * @param
 *
 * @return
 * - XST_SUCCESS if validation is successful.
 *
 * - XST_FAILURE otherwise.
 */
static int CheckData(int32_t *rx_buff_base)
{
    int Index = 0;
    int32_t Value;

    Value = TEST_START_VALUE;

    /* Invalidate the DestBuffer before receiving the data, in case the
     * Data Cache is enabled
     */
    Xil_DCacheInvalidateRange((UINTPTR)rx_buff_base, PKT_ELEMENTS * sizeof(int32_t));

    for (Index = 0; Index < PKT_ELEMENTS; Index++)
    {
        if (rx_buff_base[Index] != Value)
        {
            xil_printf("Data error %d: %u instead of %u\r\n", Index, rx_buff_base[Index], Value);
            return XST_FAILURE;
        }
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
