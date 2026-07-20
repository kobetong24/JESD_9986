/*!
 * @brief     CDMA API Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_CDMA_API__
 * @{
 */

#ifndef CDMAAPI_H
#define CDMAAPI_H

/*============= I N C L U D E S ============*/

/*============= D E F I N E S ==============*/
#define DDRTOHMC               1
#define HMCTODDR               2
#define DMA_1MSIZE             0x100000

/*============= E X P O R T S ==============*/
int cdmatransfer_ddrtobram(uint32_t ddr_addr, uint32_t size);
int cdmatransfer_bramtoddr(uint32_t ddr_addr, uint32_t size);
int cdmatransfer_ddrtohmc(uint32_t ddr_addr, uint32_t hmc_addr, uint32_t size);
int ddmatransfer_hmctoddr(uint32_t ddr_addr, uint32_t hmc_addr, uint32_t size);

int cdma_start(uint32_t base_addr, uint32_t size, int op_type);
int cdma_inprogress(int op_type, uint8_t* pdata, uint32_t data_size);
int cdma_end(void);

#endif

/*! @} */

