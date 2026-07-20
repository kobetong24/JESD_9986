/*!
 * @brief     SIMPLE DMA REGS Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_SIMPLE_DMA_REGS__
 * @{
 */

#ifndef _DEF_SIMPLE_DMA_REGS_H
#define _DEF_SIMPLE_DMA_REGS_H


/*============= I N C L U D E S ============*/
#if defined(_LANGUAGE_C) || (defined(__GNUC__) && !defined(__ASSEMBLER__))
#include <stdint.h>
#endif /* _LANGUAGE_C */




/*============= D E F I N E S ==============*/
#ifndef __ADI_GENERATED_DEF_HEADERS__
#define __ADI_GENERATED_DEF_HEADERS__    1
#endif

#define __ADI_HAS_SIMPLE_DMA__     1

/* ============================================================================================================================
        
   ============================================================================================================================ */

/* ============================================================================================================================
        SIMPLE_DMA0
   ============================================================================================================================ */
#define REG_SIMPLE_DMA0_CONTROL_RESET        0x00000000            /*      Reset Value for CONTROL  */
#define REG_SIMPLE_DMA0_CONTROL              0x00000000
#define REG_SIMPLE_DMA0_STATUS_RESET         0x00000000            /*      Reset Value for STATUS  */
#define REG_SIMPLE_DMA0_STATUS               0x00000001
#define REG_SIMPLE_DMA0_WR_CONTROL_RESET     0xFFFF0000            /*      Reset Value for WR_CONTROL  */
#define REG_SIMPLE_DMA0_WR_CONTROL           0x00000002
#define REG_SIMPLE_DMA0_RD_CONTROL_RESET     0xFFFF0000            /*      Reset Value for RD_CONTROL  */
#define REG_SIMPLE_DMA0_RD_CONTROL           0x00000003
#define REG_SIMPLE_DMA0_WR_STATUS_RESET      0x00000000            /*      Reset Value for WR_STATUS  */
#define REG_SIMPLE_DMA0_WR_STATUS            0x00000004
#define REG_SIMPLE_DMA0_RD_STATUS_RESET      0x00000000            /*      Reset Value for RD_STATUS  */
#define REG_SIMPLE_DMA0_RD_STATUS            0x00000005
#define REG_SIMPLE_DMA0_WR_ADDR_RESET        0x00000000            /*      Reset Value for WR_ADDR  */
#define REG_SIMPLE_DMA0_WR_ADDR              0x00000006
#define REG_SIMPLE_DMA0_WR_CNT_RESET         0x00000000            /*      Reset Value for WR_CNT  */
#define REG_SIMPLE_DMA0_WR_CNT               0x00000007
#define REG_SIMPLE_DMA0_RD_ADDR_RESET        0x00000000            /*      Reset Value for RD_ADDR  */
#define REG_SIMPLE_DMA0_RD_ADDR              0x00000008
#define REG_SIMPLE_DMA0_RD_CNT_RESET         0x00000000            /*      Reset Value for RD_CNT  */
#define REG_SIMPLE_DMA0_RD_CNT               0x00000009

/* ============================================================================================================================
        SIMPLE_DMA Register BitMasks, Positions & Enumerations 
   ============================================================================================================================ */
/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_CONTROL                   Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_CONTROL_RESET         1
#define BITP_SIMPLE_DMA_CONTROL_HALT          0
#define BITM_SIMPLE_DMA_CONTROL_RESET        0x00000002
#define BITM_SIMPLE_DMA_CONTROL_HALT         0x00000001

/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_STATUS                    Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_STATUS_HALT_DONE      0
#define BITM_SIMPLE_DMA_STATUS_HALT_DONE     0x00000001

/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_WR_CONTROL                Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_WR_CONTROL_MAX_WR    16
#define BITP_SIMPLE_DMA_WR_CONTROL_WR_START   0
#define BITM_SIMPLE_DMA_WR_CONTROL_MAX_WR    0xFFFF0000
#define BITM_SIMPLE_DMA_WR_CONTROL_WR_START  0x00000001

/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_RD_CONTROL                Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_RD_CONTROL_MAX_RD    16
#define BITP_SIMPLE_DMA_RD_CONTROL_RD_CYCLIC  1
#define BITP_SIMPLE_DMA_RD_CONTROL_RD_START   0
#define BITM_SIMPLE_DMA_RD_CONTROL_MAX_RD    0xFFFF0000
#define BITM_SIMPLE_DMA_RD_CONTROL_RD_CYCLIC 0x00000002
#define BITM_SIMPLE_DMA_RD_CONTROL_RD_START  0x00000001

/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_WR_STATUS                 Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_WR_STATUS_WR_DONE     0
#define BITM_SIMPLE_DMA_WR_STATUS_WR_DONE    0x00000001

/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_RD_STATUS                 Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_RD_STATUS_RD_DONE     0
#define BITM_SIMPLE_DMA_RD_STATUS_RD_DONE    0x00000001

/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_WR_ADDR                   Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_WR_ADDR_WR_ADDR       0
#define BITM_SIMPLE_DMA_WR_ADDR_WR_ADDR      0xFFFFFFFF

/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_WR_CNT                    Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_WR_CNT_WR_CNT         0
#define BITM_SIMPLE_DMA_WR_CNT_WR_CNT        0xFFFFFFFF

/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_RD_ADDR                   Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_RD_ADDR_RD_ADDR       0
#define BITM_SIMPLE_DMA_RD_ADDR_RD_ADDR      0xFFFFFFFF

/* -------------------------------------------------------------------------------------------------------------------------
          SIMPLE_DMA_RD_CNT                    Pos/Masks         Description
   ------------------------------------------------------------------------------------------------------------------------- */
#define BITP_SIMPLE_DMA_RD_CNT_RD_CNT         0
#define BITM_SIMPLE_DMA_RD_CNT_RD_CNT        0xFFFFFFFF




/*============= E X P O R T S ==============*/



#endif	/* end ifndef _DEF_SIMPLE_DMA_REGS_H */

/*! @} */
