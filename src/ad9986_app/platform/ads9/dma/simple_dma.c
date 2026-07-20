/*!
 * @brief     SIMPLE DMA Source File
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

/*============= I N C L U D E S ============*/
#include "simple_dma.h"
#include "simple_dma_regs.h"
#include "yoda_util.h"
#include "util.h"

/*============= D E F I N E S ==============*/
#define WTT_FIELD_LEN   16
#define NUM_BD          512

/*============= C O D E ====================*/
void simple_dma_reset(simple_dma_inst * inst) {
    YODA_WR_FIELD(inst->base, REG_SIMPLE_DMA0_CONTROL, 1, SIMPLE_DMA_CONTROL_RESET);
}

void simple_dma_halt(simple_dma_inst * inst) {
    
    // Assert halt
    YODA_WR_FIELD(inst->base, REG_SIMPLE_DMA0_CONTROL, 1, SIMPLE_DMA_CONTROL_HALT);

    // Wait for halt done
    while(!(YODA_RD_FIELD(inst->base, REG_SIMPLE_DMA0_STATUS, SIMPLE_DMA_STATUS_HALT_DONE)));
}

void simple_dma_run(simple_dma_inst * inst, uint8_t is_write, uint64_t addr, uint64_t len, uint8_t cyclic, uint16_t max_outstanding) {
    
    // Address
    YODA_WR(inst->base, (is_write ? REG_SIMPLE_DMA0_WR_ADDR : REG_SIMPLE_DMA0_RD_ADDR), addr);

    // Count
    YODA_WR(inst->base, (is_write ? REG_SIMPLE_DMA0_WR_CNT : REG_SIMPLE_DMA0_RD_CNT), ((len>>(inst->log2_data_width_bytes))-1));

    // Start and cyclic setting
    YODA_WR(inst->base, (is_write ? REG_SIMPLE_DMA0_WR_CONTROL : REG_SIMPLE_DMA0_RD_CONTROL), (BITM_SIMPLE_DMA_RD_CONTROL_RD_START | (cyclic ? BITM_SIMPLE_DMA_RD_CONTROL_RD_CYCLIC : 0) | (((uint32_t)max_outstanding) << BITP_SIMPLE_DMA_RD_CONTROL_MAX_RD)));
}

uint8_t simple_dma_check_done(simple_dma_inst * inst, uint8_t is_write) 
{  
    uint8_t done = YODA_RD_FIELD(inst->base, (is_write ? REG_SIMPLE_DMA0_WR_STATUS : REG_SIMPLE_DMA0_RD_STATUS), SIMPLE_DMA_WR_STATUS_WR_DONE);
    if(done) {
        YODA_WR_FIELD(inst->base, (is_write ? REG_SIMPLE_DMA0_WR_STATUS : REG_SIMPLE_DMA0_RD_STATUS), 0, SIMPLE_DMA_WR_STATUS_WR_DONE);
    }
    
    return done;
}


/*! @} */
