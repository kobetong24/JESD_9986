/*!
 * @brief     Helper HAL functions
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __AD7175_HAL_
 * @{
 */

#ifndef __AD7175_HAL_H__
#define __AD7175_HAL_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"

/*============= D E F I N E S ==============*/
#define SPI_IN_OUT_BUFF_SZ 0x00000004  /* bytes */

/*============= E X P O R T S ==============*/
#ifdef _cplusplus
extern "C" {
#endif

int32_t ad7175_hw_open(adi_ad7175_device_t *device);

int32_t ad7175_hw_close(adi_ad7175_device_t *device);

int32_t ad7175_sw_delay_us(adi_ad7175_device_t *device, uint32_t us);

int32_t ad7175_hw_reset(adi_ad7175_device_t *device);

int32_t ad7175_spi_reg_get(adi_ad7175_device_t *device, uint32_t reg, uint32_t *data, uint32_t size_bytes);

int32_t ad7175_spi_reg_set(adi_ad7175_device_t *device, uint32_t reg, uint32_t data);

#ifdef __cplusplus
}
#endif

#endif /*__AD7175_HAL_H__*/
/*! @} */
