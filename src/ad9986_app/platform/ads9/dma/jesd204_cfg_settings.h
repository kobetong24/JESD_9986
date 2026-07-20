/*!
 * @brief     JESD204 CFG SETTINGS Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_JESD204_CFG_SETTINGS__
 * @{
 */

#ifndef JESD204_CFG_SETTINGS_H
#define JESD204_CFG_SETTINGS_H




/*============= I N C L U D E S ============*/





/*============= D E F I N E S ==============*/
//GTY RX Usage Defines
#define NUM_RX_LANE 8
#define NUM_RX_LINK 2
#define NUM_RX_PHY 1
#define NUM_DEFRAMER 2
#define LANES_PER_DEFRAMER {8, 4}
#define LANES_PER_RX_LINK {8, 4}

//GTY TX Usage Defines
#define NUM_TX_LANE 8
#define NUM_TX_LINK 2
#define NUM_TX_PHY 1
#define NUM_FRAMER 2
#define NUM_ENCODER 1
#define LANES_PER_TX_LINK {8, 4}

//Memory Defines
#define NUM_MEM 2
#define LOG2_MEM_WIDTH_BYTES 7
#define HMC_LINK_RATE HMC_15G
#define HMC_CHAINING_EN 0
#define GT_DRP_CLK_RATE_MHZ 50

//FIFO defines
#define RX_DATA_FIFO_SIZE 512
#define TX_DATA_FIFO_SIZE 512
#define RX_AXI_FIFO_SIZE  512
#define TX_AXI_FIFO_SIZE  512

// If ENABLE_RX_CDR_HOLD is defined, RX CDR hold is enabled after JESD204 PHY reset done
// #define ENABLE_RX_CDR_HOLD

// If enabled, sets the value of SYNC~ DURING RX equalization
#define ENABLE_SYNCN_FORCE_DURING_EQ 0
// The value of SYNC~ to set during RX equalization, if ENABLE_SYNCN_FORCE_DURING_EQ is enabled
#define SYNCN_FORCE_DURING_EQ_VALUE 1


#define LANES_PER_RX_PHY (NUM_RX_LANE/NUM_RX_PHY)
#define QUADS_PER_RX_PHY ((LANES_PER_RX_PHY+3)/4)
#define NUM_RX_QUAD (QUADS_PER_RX_PHY*NUM_RX_PHY)
#define DEFRAMERS_PER_LINK (NUM_DEFRAMER/NUM_RX_LINK)

#define LANES_PER_TX_PHY (NUM_TX_LANE/NUM_TX_PHY)
#define QUADS_PER_TX_PHY ((LANES_PER_TX_PHY+3)/4)
#define NUM_TX_QUAD (QUADS_PER_TX_PHY*NUM_TX_PHY)
#define LANES_PER_ENCODER (NUM_TX_LANE/NUM_ENCODER)
#define FRAMERS_PER_LINK   (NUM_FRAMER/NUM_TX_LINK)

#define MAX_NUM_LINK ((NUM_RX_LINK > NUM_TX_LINK) ? NUM_RX_LINK : NUM_TX_LINK)

#define HMC_MEM_MASK ((NUM_MEM == 1) ? (HMC_CHAINING_EN ? 0x5 : 0x1) : (HMC_CHAINING_EN ? 0xF : 0x3))

#define RX_DATA_DMA_MAX_CNT ((RX_DATA_FIFO_SIZE/2)-5)
#define TX_DATA_DMA_MAX_CNT ((TX_DATA_FIFO_SIZE/2)-5)
#define RX_AXI_DMA_MAX_CNT  ((RX_AXI_FIFO_SIZE/2)-5)
#define TX_AXI_DMA_MAX_CNT  ((TX_AXI_FIFO_SIZE/2)-5)



/*============= E X P O R T S ==============*/



#endif

/*! @} */
