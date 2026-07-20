/*!
 * @brief     SIMPLE DMA Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_SIMPLE_DMA__
 * @{
 */

#ifndef SIMPLE_DMA_H
#define SIMPLE_DMA_H


/*============= I N C L U D E S ============*/
#include <stdint.h>



/*============= D E F I N E S ==============*/
typedef struct {
   uint32_t base;
   uint32_t log2_data_width_bytes;
} simple_dma_inst;




/*============= E X P O R T S ==============*/
/*!
   \brief Reset DMA
   \param inst SIMPLE_DMA driver instance
*/
void simple_dma_reset(simple_dma_inst * inst);

/*!
   \brief Halt DMA - Wait for outstanding transactions to complete
   \param inst SIMPLE_DMA driver instance
*/
void simple_dma_halt(simple_dma_inst * inst);

/*!
   \brief Run DMA
   \param inst SIMPLE_DMA driver instance
   \param is_write Nonzero if write, zero if read
   \param addr Starting byte address of transfer
   \param len Length of transfer in bytes, must be a multiple of inst->data_width_bytes
   \param cyclic If 1, rerun transfer indefinitely. Only supported for read transfers
   \param Maximum outstanding writes or reads allowed, if WR/RD_AVAIL_EN IP parameter is set
*/
void simple_dma_run(simple_dma_inst * inst, uint8_t is_write, uint64_t addr, uint64_t len, uint8_t cyclic, uint16_t max_outstanding);

/*!
   \brief Check if write or read DMA is done. If so, clear done flag
   \param inst SIMPLE_DMA driver instance
   \param is_write Nonzero if write, zero if read
   \return Nonzero if done, otherwise zero
*/
uint8_t simple_dma_check_done(simple_dma_inst * inst, uint8_t is_write);

#endif

/*! @} */
