/*!
 * @brief     ZYNQ DMA Source File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_ZYNQ_DMA__
 * @{
 */

/*============= I N C L U D E S ============*/
#include "zynq_cdma.h"

/*============= D E F I N E S ==============*/

/* Source and Destination buffer for DMA transfer.
 */

/* Shared variables used to test the callbacks.
 */
volatile static int Done = 0;	/* Dma transfer is done */
volatile static int Error = 0;	/* Dma Bus Error occurs */

/*============= C O D E ====================*/
/*****************************************************************************/
/*
*
* This function does one simple transfer
*
* @param	InstancePtr is a pointer to the XAxiCdma instance
* @param	Length is the transfer length
* @param	Retries is how many times to retry on submission
*
* @return
*		- XST_SUCCESS if transfer is successful
*		- XST_FAILURE if either the transfer fails or the data has
*		  error
*
* @note		None
*
******************************************************************************/
int DoSimpleTransfer(XAxiCdma *InstancePtr, int Length, int Retries, int SourceAddr, int DestAddr)
{
	int Status;

	Done = 0;
	Error = 0;

	/* Try to start the DMA transfer
	 */
	while (Retries) {
		Retries -= 1;

		Status = XAxiCdma_SimpleTransfer(InstancePtr, (u32)SourceAddr, (u32)DestAddr, Length, Example_CallBack, (void *)InstancePtr);

		if (Status == XST_SUCCESS) {
			break;
		}
	}

	if (!Retries) {
		return XST_FAILURE;
	}

	/* Wait until the DMA transfer is done
	 */
	while (!Done && !Error) {
		/* Wait */
	}

	if (Error) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
* Callback function for the simple transfer. It is called by the driver's
* interrupt handler.
*
* @param	CallBackRef is the reference pointer registered through
*		transfer submission. In this case, it is the pointer to the
* 		driver instance
* @param	IrqMask is the interrupt mask the driver interrupt handler
*		passes to the callback function.
* @param	IgnorePtr is a pointer that is ignored by simple callback
* 		function
*
* @return	None
*
* @note		None
*
******************************************************************************/
void Example_CallBack(void *CallBackRef, u32 IrqMask, int *IgnorePtr)
{

	if (IrqMask & XAXICDMA_XR_IRQ_ERROR_MASK) {
		Error = TRUE;
	}

	if (IrqMask & XAXICDMA_XR_IRQ_IOC_MASK) {
		Done = TRUE;
	}

}

/*****************************************************************************/
/*
*
* This function disables the interrupt for the XAxiCdma device
*
* @param	IntcInstancePtr is the pointer to the instance of the INTC
* @param	IntrId is the interrupt Id for the XAxiCdma instance
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void DisableIntrSystem(XScuGic *IntcInstancePtr, u32 IntrId)
{

	/* Disconnect the interrupt
	 */
	XScuGic_Disable(IntcInstancePtr, IntrId);
	XScuGic_Disconnect(IntcInstancePtr, IntrId);
}

/*! @} */
