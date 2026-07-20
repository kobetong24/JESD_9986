/*!
 * @brief     PS7 ADS9CFG Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_PS7_ADS9CFG__
 * @{
 */

#ifndef PS7_ADS9CFG_H
#define PS7_ADS9CFG_H


/*============= I N C L U D E S ============*/
#include "common.h"
#include <stdio.h>


/*============= D E F I N E S ==============*/
#define MZED_CFG_ADS9

#define ADS9CFG_MODE 0
#define ADS9CFG_DONE 1
#define ADS9CFG_INITB 2
#define ADS9CFG_PGMB 3

#define ADS9C2C_STATUS 4
#define ADS9C2C_ADS9_STATUS 4
#define ADS9C2C_MZED_STATUS 5
#define ADS9C2C_MZED_INTF_ERROR 6
#define ADS9C2C_MZED_LINK_ERROR 7
#define ADS9C2C_SRESETN 8
#define MZEDC2C_MRESETN_SW 9
#define MZEDC2C_MRESETN_SEL 10
#define MZED_PG_C2M_MZ 11


/*============= E X P O R T S ==============*/
void ps7_gpio_wr(uint8_t start_bit, uint8_t num_of_bits, uint32_t value);
uint32_t ps7_gpio_rd(uint8_t start_bit, uint8_t num_of_bits);
uint32_t ps7_spi_xfer(uint32_t *data, uint32_t no_of_bytes);
#ifdef MZED_CFG_ADS9
int32_t ps7_ads9fpga(void);
#endif
int32_t ps7_ads9setup(void);
int32_t ps7_ads9setup_resync(void);



#endif

/*! @} */

