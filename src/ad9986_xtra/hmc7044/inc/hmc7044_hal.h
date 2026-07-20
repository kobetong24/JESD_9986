/*!
 * @brief     Helper HAL functions
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __HMC7044_HAL_
 * @{
 */

#ifndef __HMC7044_HAL_H__
#define __HMC7044_HAL_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"

/*============= D E F I N E S ==============*/
#define SPI_IN_OUT_BUFF_SZ 0x3

uint64_t gcd(uint64_t value1, uint64_t value2);
uint64_t lcm(uint64_t value1, uint64_t value2);

/*============= E X P O R T S ==============*/
#ifdef _cplusplus
extern "C" {
#endif

int32_t hmc7044_hw_open(adi_hmc7044_device_t *device);
int32_t hmc7044_hw_close(adi_hmc7044_device_t *device);

int32_t hmc7044_sw_delay_us(adi_hmc7044_device_t *device, uint32_t us);

int32_t hmc7044_hw_reset(adi_hmc7044_device_t *device);

int32_t hmc7044_spi_reg_get(adi_hmc7044_device_t *device,
        uint32_t reg, uint8_t *data);
int32_t hmc7044_spi_reg_set(adi_hmc7044_device_t *device,
        uint32_t reg, uint8_t data);

int32_t hmc7044_spi_reg_tbl_set(adi_hmc7044_device_t *device,
        adi_cms_reg_data_t *tbl, uint32_t count);

int32_t hmc7044_spi_reg_block_get(adi_hmc7044_device_t *device,
        const uint16_t address, uint8_t *data, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /*__HMC7044_HAL_H__*/
/*! @} */
