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
 *  @file xaxicdma_bdring.c
* @addtogroup axicdma_v4_0
* @{
 *
 * Implementation for support on Scatter Gather (SG) transfers.
 * It includes the implementation of the BD ring API. There is only one BD ring
 * per DMA engine.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   04/18/10 First release
 * 2.01a rkv  01/25/11 Replaced with "\r\n" in place on "\n\r" in printf statements
 * 4.3   mi   09/21/16 Fixed compilation warnings
 * </pre>
 *
 *****************************************************************************/
#include "xaxicdma.h"
#include "xaxicdma_i.h"

/***************** Macros (Inline Functions) Definitions *********************/
/* The following macros are helper functions inside this file.
 */

/******************************************************************************
 * Move the BdPtr argument ahead an arbitrary number of BDs. Wrapping around
 * to the beginning of the ring if needed.
 *
 * We know a wraparound should occur if the new BdPtr is greater than
 * the high address in the ring.
 *
 * @param	InstancePtr is the ring BdPtr appears in
 * @param	BdPtr on input is the starting BD position and on output is the
 *			final BD position
 * @param	NumBd is the number of BD spaces to increment
 *
 * @note	The BdPtr will be changed if NumBd not zero.
 *
 *****************************************************************************/
#define XAXICDMA_RING_SEEKAHEAD(InstancePtr, BdPtr, NumBd)         \
{                                                                  \
    UINTPTR Addr = (UINTPTR)(void *)(BdPtr);                       \
                                                                   \
    Addr += ((InstancePtr)->BdSeparation * (NumBd));               \
    if (Addr > (InstancePtr)->LastBdAddr) {                        \
        Addr -= (InstancePtr)->BdRingTotalLen;                     \
    }                                                              \
                                                                   \
    (BdPtr) = (XAxiCdma_Bd *)(void *)Addr;                                  \
}

/******************************************************************************
 * Move the BdPtr argument backwards an arbitrary number of BDs. Wrapping
 * around to the end of the ring if needed.
 *
 * We know a wraparound should occur if the new BdPtr is less than
 * the base address in the ring.
 *
 * @param	InstancePtr is the ring BdPtr appears in
 * @param	BdPtr on input is the starting BD position and on output is the
 *			final BD position
 * @param	NumBd is the number of BD spaces to increment
 *
 * @note	The BdPtr will be changed if NumBd not zero.
 *
 *****************************************************************************/
#define XAXICDMA_RING_SEEKBACK(InstancePtr, BdPtr, NumBd)        \
{                                                                \
    UINTPTR Addr = (UINTPTR)(void *)(BdPtr);                                     \
                                                                 \
    Addr -= ((InstancePtr)->BdSeparation * (NumBd));             \
    if (Addr < (InstancePtr)->FirstBdAddr) {                     \
        Addr += (InstancePtr)->BdRingTotalLen;                   \
    }                                                            \
                                                                 \
    (BdPtr) = (XAxiCdma_Bd*)(void *)Addr;                                \
}

/*****************************************************************************/
/**
 * This function calculates how many BDs can be built using given number of
 * bytes of memory, according to alignment provided.
 *
 * @param	Alignment is the preferred alignment for the BDs
 * @param	Bytes is the number of bytes of memory to build BDs with
 * @param	BdBuffAddr is the buffer address allocated for the BDs. This
 *			is to check the alignment of the buffer to make sure the the
 *			buffer is aligned to the BD alignment. An invalid buffer
 *			address results in 0.
 *
 * @return	The number of BDs can be built. 0 means buffer address
 *			is not valid.
 *
 * @note	The application is responsible to align the buffer before pass
 *			it to this function.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingCntCalc(u32 Alignment, u32 Bytes, UINTPTR BdBuffAddr)
{

	/* The buffer alignment has to be taken account of. An unaligned buffer
	 * is invalid.
	 */
	if (BdBuffAddr & Alignment) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid buffer addr %x\r\n",
			(unsigned int)BdBuffAddr);
		return 0;
	}

	return ((uint32_t)(Bytes) / ((sizeof(XAxiCdma_Bd) + (Alignment - 1)) &
               ~(Alignment - 1)));
}

/*****************************************************************************/
/**
 * This function calculates how much memory is needed to build requested number
 * of BDs.
 *
 * @param	Alignment is the preferred alignment for the BDs
 * @param	NumBd is the number of BDs to be built
 *
 * @return	The number of bytes of memory needed to build the BDs
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingMemCalc(u32 Alignment, int NumBd)
{
	return (uint32_t)(((sizeof(XAxiCdma_Bd) + (Alignment - 1)) &
               ~(Alignment - 1)) * NumBd);
}

/*****************************************************************************/
/**
 * This function set the Current BD ptr of a BD
 *
 * @param       BdPtr is the pointer to the BD to set the Next BD ptr
 * @param       CurBdPtr is the valud of Current BD ptr
 *
 * @return      None
 *
 * @note        None.
 *
 *****************************************************************************/
void XAxiCdma_BdSetCurBdPtr(XAxiCdma *InstancePtr, UINTPTR CurBdPtr)
{
	XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CDESC_OFFSET,
                          (CurBdPtr & XAXICDMA_DESC_LSB_MASK));
    if (InstancePtr->AddrWidth > 32)
         XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CDESC_MSB_OFFSET,
			   UPPER_32_BITS(CurBdPtr));
        return;
}

/*****************************************************************************/
/**
 * This function grabs a copy of the current BD pointer from the hardware.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	The BD pointer in CDESC register
 *
 * @note	None.
 *
 *****************************************************************************/
XAxiCdma_Bd *XAxiCdma_BdRingGetCurrBd(XAxiCdma *InstancePtr)
{

	if (InstancePtr->AddrWidth > 32) {
		return (XAxiCdma_Bd *)(UINTPTR)((XAxiCdma_BdRead(InstancePtr->BaseAddr, XAXICDMA_CDESC_OFFSET)) |
		((uint64_t)(XAxiCdma_BdRead(InstancePtr->BaseAddr, XAXICDMA_CDESC_MSB_OFFSET)) << 32U));
	} else {
		return (XAxiCdma_Bd *)(UINTPTR)(XAxiCdma_BdRead(InstancePtr->BaseAddr, XAXICDMA_CDESC_OFFSET));
	}
}

/*****************************************************************************/
/**
 * This function gets the total number of BDs in the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	The total number of BDs for this instance
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingGetCnt(XAxiCdma *InstancePtr)
{
	return (uint32_t)InstancePtr->AllBdCnt;
}

/*****************************************************************************/
/**
 * This function gets the number of free BDs.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	The total number of free BDs for this instance
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingGetFreeCnt(XAxiCdma *InstancePtr)
{
	return InstancePtr->FreeBdCnt;
}

/*****************************************************************************/
/**
 * This function grabs a copy of the current BD pointer from the hardware.
 * It is normally used to prepare for the hardware reset. The snapshot of the
 * current BD pointer should be reloaded once the reset is done.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_BdRingSnapShotCurrBd(XAxiCdma *InstancePtr)
{
	XAxiCdma_Bd *BdPtr;

	BdPtr =	XAxiCdma_BdRingGetCurrBd(InstancePtr);

	InstancePtr->BdaRestart = XAxiCdma_BdRingNext(InstancePtr, BdPtr);

	return;
}

/*****************************************************************************/
/**
 * This function set the Tail BD ptr of a BD
 *
 * @param       BdPtr is the pointer to the BD to set the Next BD ptr
 * @param       Tail Bd is the value of Tail BD ptr
 *
 * @return      None
 *
 * @note        None.
 *
 *****************************************************************************/
void XAxiCdma_BdSetTailBdPtr(XAxiCdma *InstancePtr, UINTPTR TailBdPtr)
{
    XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_TDESC_OFFSET,
                      TailBdPtr & XAXICDMA_DESC_LSB_MASK);
    if (InstancePtr->AddrWidth > 32)
            XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_TDESC_MSB_OFFSET,
                              UPPER_32_BITS(TailBdPtr));
        return;
}


/*****************************************************************************/
/**
 * This function gets the next BD of the current BD on the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	BdPtr is the current BD
 *
 * @return	The next BD on the ring from the current BD, NULL if passed
 *			in BdPtr not valid.
 *
 * @note	None.
 *
 *****************************************************************************/
XAxiCdma_Bd *XAxiCdma_BdRingNext(XAxiCdma *InstancePtr, XAxiCdma_Bd *BdPtr)
{
	UINTPTR ReturnBd;

	/* Check whether the BD ptr is valid
	 * A BD ptr is not valid if:
	 * - It is outside of the BD memory range
	 * - It has invalid alignment
	 */
	if (((UINTPTR)BdPtr > InstancePtr->LastBdAddr)  ||
	    ((UINTPTR)BdPtr < InstancePtr->FirstBdAddr) ||
		((UINTPTR)BdPtr & (InstancePtr->BdSeparation - 1))) {

		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid BdPtr %x: %x/%x/%x\r\n",
		(UINTPTR)BdPtr, (UINTPTR)InstancePtr->FirstBdAddr,
			(UINTPTR)InstancePtr->LastBdAddr,
			(unsigned int)XAXICDMA_BD_MINIMUM_ALIGNMENT);
		ReturnBd = 0x0;
	}

	/* If the current BD is the last BD in the ring, return the first BD
	 */
	else if ((UINTPTR)BdPtr == InstancePtr->LastBdAddr) {
		ReturnBd = InstancePtr->FirstBdAddr;
	}
	else {
		ReturnBd = (UINTPTR)BdPtr + InstancePtr->BdSeparation;
	}

	 return (XAxiCdma_Bd *)ReturnBd;
}

/*****************************************************************************/
/**
 * This function gets the previous BD of the current BD on the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	BdPtr is the current BD
 *
 * @return	The previous BD on the ring from the current BD
 *
 * @note	None.
 *
 *****************************************************************************/
XAxiCdma_Bd *XAxiCdma_BdRingPrev(XAxiCdma *InstancePtr, XAxiCdma_Bd *BdPtr)
{
	UINTPTR ReturnBd;

	/* Check whether the BD ptr is valid
	 * A BD ptr is not valid if:
	 * - It is outside of the BD memory range
	 * - It has invalid alignment
	 */
	if (((UINTPTR)BdPtr > InstancePtr->LastBdAddr)  ||
	    ((UINTPTR)BdPtr < InstancePtr->FirstBdAddr) ||
		((UINTPTR)BdPtr & (InstancePtr->BdSeparation - 1))) {

		xdbg_printf(XDBG_DEBUG_ERROR, "Invalid BdPtr %x: %x/%x/%x\r\n",
			(UINTPTR)BdPtr, (UINTPTR)InstancePtr->FirstBdAddr,
			(UINTPTR)InstancePtr->LastBdAddr,
		    (unsigned int)XAXICDMA_BD_MINIMUM_ALIGNMENT);
		ReturnBd = 0x0;
	}

	/* If the current BD is the first BD in the ring, return the last BD
	 */
	else if ((UINTPTR)BdPtr == InstancePtr->FirstBdAddr) {
		ReturnBd = InstancePtr->LastBdAddr;
	}
	else {
		ReturnBd = (UINTPTR)BdPtr - InstancePtr->BdSeparation;
	}

	 return (XAxiCdma_Bd *)ReturnBd;
}

/*****************************************************************************/
/**
 * This function clones all BDs in the BD ring to be the same as the given
 * BD.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	TemplateBdPtr is the BD to be copied from
 *
 * @return
 *			- XST_SUCCESS for success
 *			- XST_DMA_SG_NO_LIST if there is no BD ring
 *			- XST_DEVICE_IS_STARTED if the hardware is running
 *			- XST_DMA_SG_LIST_ERROR is the BD ring is still in use
 *
 * @note	None.
 *
 *****************************************************************************/
LONG XAxiCdma_BdRingClone(XAxiCdma *InstancePtr, XAxiCdma_Bd * TemplateBdPtr)
{
	int Index;
	UINTPTR CurBd;
	XAxiCdma_Bd TmpBd;

	/* Can't do this function if there isn't a ring */
	if (InstancePtr->AllBdCnt == 0) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingClone: no bds\r\n");

		return XST_DMA_SG_NO_LIST;
	}

	/* Can't do this function with the channel running */
	if (XAxiCdma_IsBusy(InstancePtr)) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingClone: engine is running, "
			"cannot do\r\n");

		return XST_DEVICE_IS_STARTED;
	}

	/* Can't do this function with some of the BDs in use */
	if (InstancePtr->FreeBdCnt != InstancePtr->AllBdCnt) {

		xdbg_printf(XDBG_DEBUG_ERROR,
		    "BdRingClone: some bds already in use %d/%d\r\n",
		    InstancePtr->FreeBdCnt, InstancePtr->AllBdCnt);

		return XST_DMA_SG_LIST_ERROR;
	}

	/* Make a copy of the template then clear the status bits
	 */
	memcpy(&TmpBd, TemplateBdPtr, sizeof(XAxiCdma_Bd));

	XAxiCdma_BdClearSts(&TmpBd);

	/* Starting from the top of the ring, save BD.Next, BD.PhysAddr
	 * overwrite the entire BD with the template, then restore BD.Next
	 * and BD.PhysAddr
	 */
	for (Index = 0, CurBd = InstancePtr->FirstBdAddr;
		Index < InstancePtr->AllBdCnt;
		Index++, CurBd += InstancePtr->BdSeparation) {

		XAxiCdma_BdClone((XAxiCdma_Bd *)CurBd, &TmpBd);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function requests number of BDs from the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	NumBd is the number of BDs to request
 * @param	BdSetPtr is the pointer to the set of BDs returned
 *
 * @return
 *			- XST_SUCCESS for success
 *			- XST_INVALID_PARAM if requests non-positive number of BDs
 *			- XST_FAILURE if not enough free BDs available
 *
 * @note	None.
 *
 *****************************************************************************/
LONG XAxiCdma_BdRingAlloc(XAxiCdma *InstancePtr, int NumBd,
   XAxiCdma_Bd ** BdSetPtr)
{
	if (NumBd <= 0) {

		xdbg_printf(XDBG_DEBUG_ERROR,
		    "BdRingAlloc: negative BD number %d\r\n", NumBd);

		return XST_INVALID_PARAM;
	}

	/* Enough free BDs available for the request? */
	if (InstancePtr->FreeBdCnt < NumBd) {
		return XST_FAILURE;
	}

	/* Set the return argument and move FreeBdHead forward */
	*BdSetPtr = InstancePtr->FreeBdHead;
	XAXICDMA_RING_SEEKAHEAD(InstancePtr, InstancePtr->FreeBdHead, NumBd);
	InstancePtr->FreeBdCnt -= NumBd;
	InstancePtr->PreBdCnt += NumBd;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function tries to free the number of BDs back to the ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	NumBd is the number of BDs to return
 * @param	BdSetPtr is the set of BDs to be returned
 *
 * @return
 *			- XST_SUCCESS for success
 *			- XST_INVALID_PARAM if to free non-positive number of BDs
 *			- XST_FAILURE if BD ring management shows an error
 *
 * @note	None.
 *
 *****************************************************************************/
LONG XAxiCdma_BdRingUnAlloc(XAxiCdma *InstancePtr, int NumBd,
			 XAxiCdma_Bd * BdSetPtr)
{
	XAxiCdma_Bd *TmpBd;

	if (NumBd <= 0) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingUnAlloc: negative BD number "
			"%d\r\n", NumBd);

		return XST_INVALID_PARAM;
	}

	/* Enough BDs in the preprocessing state for the request? */
	if (InstancePtr->PreBdCnt < NumBd) {
		return XST_FAILURE;
	}

	/* The last BD in the BD set must has the FreeBdHead as its next BD.
	 * Otherwise, this is not a valid operation.
	 */
	TmpBd = BdSetPtr;
	XAXICDMA_RING_SEEKAHEAD(InstancePtr, TmpBd, NumBd);

	if (TmpBd != InstancePtr->FreeBdHead) {
		return XST_FAILURE;
	}

	/* Set the return argument and move FreeBdHead backward */
	XAXICDMA_RING_SEEKBACK(InstancePtr, InstancePtr->FreeBdHead, NumBd);
	InstancePtr->FreeBdCnt += NumBd;
	InstancePtr->PreBdCnt -= NumBd;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function returns the BDs back to the free pool of the BD ring.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	NumBd is the number of BDs to free
 * @param	BdSetPtr is the set of BDs to be freed
 *
 * @return
 *			- XST_SUCCESS for success
 *			- XST_INVALID_PARAM if number of BDs is negative
 *			- XST_DMA_SG_LIST_ERROR if the BD ring management has a problem
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_BdRingFree(XAxiCdma *InstancePtr, int NumBd,
		      XAxiCdma_Bd * BdSetPtr)
{
	if (NumBd < 0) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingFree: negative BDs%d\r\n",
			NumBd);

		return XST_INVALID_PARAM;
	}

	/* If the BD Set to free is empty, do nothing
	 */
	if (NumBd == 0) {
		return XST_SUCCESS;
	}

	/* Make sure we are in sync with XAxiCdma_BdRingFromHw() */
	if ((InstancePtr->PostBdCnt < NumBd) ||
	    (InstancePtr->PostBdHead != BdSetPtr)) {

		xdbg_printf(XDBG_DEBUG_ERROR, "BdRingFree: Error free BDs: "
		    "post count %d to free %d, PostBdHead %x to free ptr %x\r\n",
		    InstancePtr->PostBdCnt, NumBd,
		    (UINTPTR)InstancePtr->PostBdHead,
		    (UINTPTR)BdSetPtr);

		return XST_DMA_SG_LIST_ERROR;
	}

	/* Update pointers and counters */
	InstancePtr->FreeBdCnt += NumBd;
	InstancePtr->PostBdCnt -= NumBd;
	XAXICDMA_RING_SEEKAHEAD(InstancePtr, InstancePtr->PostBdHead, NumBd);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
 * This function tries to start the SG transfer that has been submitted to
 * the driver, however, not to the hardware yet, because the hardware was
 * doing a simple transfer at the time of submit.
 *
 * This function should be called when the simple transfer is done.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return
 *		- XST_SUCCESS if BD ring has been successfully started,
 *		SGWaiting will be 0
 *		- XST_FAILURE if BD ring cannot be be started, either because
 *		the hardware is simple only build or cannot switch to SG mode.
 *
 * @note	None.
 *
 *****************************************************************************/
int XAxiCdma_BdRingStartTransfer(XAxiCdma *InstancePtr)
{

	if (InstancePtr->SimpleOnlyBuild) {
		return XST_FAILURE;
	}

	/* If no BDs need to be transfered, we are done
	 */
	if (InstancePtr->HwBdCnt == 0) {
		return XST_SUCCESS;
	}

	/* See whether hardware is in simple mode
	 */
	if (XAxiCdma_IsSimpleMode(InstancePtr)) {
		/* Cannot switch to SG mode
		 */
		if ((InstancePtr->SimpleNotDone) ||
		    (XAxiCdma_SwitchMode(InstancePtr, XAXICDMA_SG_MODE)
			!= XST_SUCCESS)) {
			return XST_FAILURE;
		}
	}

	/* Now it is in SG mode, update the tail pointer to start the
	 * SG transfer
	 */
	XAxiCdma_BdSetTailBdPtr(InstancePtr,
                XAxiCdma_BdGetPhysAddr((XAxiCdma_Bd *)InstancePtr->HwBdTail));


	InstancePtr->SGWaiting = 0;

	return XST_SUCCESS;
}

/** @} */
