/*!
 * @brief     Helper HAL functions
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __AD7175_HAL__
 * @{
 */

/*============= I N C L U D E S ============*/
#include <stdarg.h>
#include <stdlib.h>
#include "adi_ad7175.h"
#include "ad7175_hal.h"

/*============= D E F I N E S ==============*/

/*============= C O D E ====================*/
int32_t ad7175_hw_open(adi_ad7175_device_t *device)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (device->hal_info.hw_open != ADI_INVALID_POINTER) {
        err = device->hal_info.hw_open(device->hal_info.user_data);
        if (err != API_CMS_ERROR_OK) {
            return API_CMS_ERROR_HW_OPEN;
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t ad7175_hw_close(adi_ad7175_device_t *device)
{
    int32_t err;

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (device->hal_info.hw_close != ADI_INVALID_POINTER) {
        err = device->hal_info.hw_close(device->hal_info.user_data);
        if (err != API_CMS_ERROR_OK) {
            return API_CMS_ERROR_HW_CLOSE;
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t ad7175_sw_delay_us(adi_ad7175_device_t *device, uint32_t us)
{
    int32_t err;

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (device->hal_info.delay_us == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_DELAYUS_PTR;
    }
    err = device->hal_info.delay_us(device->hal_info.user_data, us);
    if (err != API_CMS_ERROR_OK ) {
        return API_CMS_ERROR_DELAY_US;
    }

    return API_CMS_ERROR_OK;
}

int32_t ad7175_spi_reg_set(adi_ad7175_device_t *device, uint32_t reg, uint32_t data)
{
    int32_t err;
    uint8_t in_data[4] = {0};
    uint8_t out_data[5] = {0};

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if (device->hal_info.spi_xfer == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_XFER_PTR;
    }
    in_data[0] = 0;
    in_data[1] = (reg & 0x3F);  
    in_data[2] = (uint8_t)((data >>  8) & 0xFF); /* msb */
    in_data[3] = (uint8_t)((data >>  0) & 0xFF); /* lsb */
    err = device->hal_info.spi_xfer(device->hal_info.user_data, in_data, out_data, SPI_IN_OUT_BUFF_SZ);
    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_SPI_XFER;
    }

    return API_CMS_ERROR_OK;
}

int32_t ad7175_spi_reg_get(adi_ad7175_device_t *device, uint32_t reg, uint32_t *data, uint32_t size_bytes)
{
    int32_t err;
    uint8_t in_data[4] = {0};
    uint8_t out_data[5] = {0};

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if (device->hal_info.spi_xfer == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_XFER_PTR;
    }
    in_data[0] = 0;
    in_data[1] = (reg & 0x3F) | 0x40;

    err = device->hal_info.spi_xfer(device->hal_info.user_data, in_data, out_data, size_bytes);
    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_SPI_XFER;
    }
    if (size_bytes == 3) {
        *data = (out_data[4]) + (out_data[3] << 8) + (out_data[2] << 16);
    }
    if (size_bytes == 2) {
        *data = (out_data[3]) + (out_data[2] << 8);
    }
    if (size_bytes == 1) {
        *data = out_data[2];
    }

    return API_CMS_ERROR_OK;
}

/*! @} */