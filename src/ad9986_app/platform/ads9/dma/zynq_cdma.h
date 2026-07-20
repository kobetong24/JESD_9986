

/******************************************************************************
*
* Copyright (C) 2010 - 2017 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xaxicdma_example_simple_intr.c
 *
 * This file demonstrates how to use the xaxicdma driver on the Xilinx AXI
 * CDMA core (AXICDMA) to transfer packets in simple transfer mode through
 * interrupt.
 *
 * Modify the NUMBER_OF_TRANSFER constant to have different number of simple
 * transfers done in this test.
 *
 * This example assumes that the system has an interrupt controller.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options for the example, also
 * comment out the "#undef DEBUG" in xdebug.h. You need to rebuild your
 * software executable.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 *  . Updated the debug print on type casting to avoid warnings on u32. Cast
 *    u32 to (unsigned int) to use the %x format.
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   07/30/10 First release
 * 2.01a rkv  01/28/11 Changed function prototype of XAxiCdma_SimpleIntrExample
 * 		       to a function taking arguments interrupt instance,device
 * 		       instance,device id,device interrupt id
 *		       Added interrupt support for Cortex A9
 * 2.01a srt  03/05/12 Modified interrupt support for Zynq.
 * 		       Modified Flushing and Invalidation of Caches to fix CRs
 *		       648103, 648701.
 * 4.3   ms   01/22/17 Modified xil_printf statement in main function to
 *            ensure that "Successfully ran" and "Failed" strings are
 *            available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * </pre>
 *
 ****************************************************************************/

#ifndef __ZYNQ_CDMA_H_
#define __ZYNQ_CDMA_H_



#include "xaxicdma.h"
#include "xdebug.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xparameters_ps.h"

#ifndef __MICROBLAZE__
//#include "xpseudo_asm_gcc.h"
#endif

#ifdef __arm__
#include "xreg_cortexa9.h"
#endif

/******************** Constant Definitions **********************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define DMA_CTRL_DEVICE_ID 	XPAR_AXICDMA_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define DMA_CTRL_IRPT_INTR	XPAR_FABRIC_AXICDMA_0_VEC_ID

//#define BUFFER_BYTESIZE		1024	///* Length of the buffers (bytes) for DMA transfer */    //(XPAR_AXI_CDMA_0_M_AXI_DATA_WIDTH * XPAR_AXI_CDMA_0_M_AXI_MAX_BURST_LEN)			//64

#define NUMBER_OF_TRANSFERS	1	/* Number of simple transfers to do */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#if (!defined(DEBUG))
#endif

int xaxicdma_example_simple_intr_main();
int XAxiCdma_SimpleIntrTransfer(XScuGic *IntcInstancePtr, XAxiCdma *InstancePtr, u16 DeviceId, u32 IntrId, u32 SourceAddr, u32 DestAddr, u32 TranferSize_Byte, u32 keyhole_enable, u32 keyhole_write);
int DoSimpleTransfer(XAxiCdma *InstancePtr, int Length, int Retries, int SourceAddr, int DestAddr);
void Example_CallBack(void *CallBackRef, u32 IrqMask, int *IgnorePtr);
void DisableIntrSystem(XScuGic *IntcInstancePtr, u32 IntrId);


#endif /* __ZYNQ_CDMA_H_ */
