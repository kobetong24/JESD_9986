/*!
 * @brief     Helper HAL functions
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __HMC7044_HAL__
 * @{
 */

/*============= I N C L U D E S ============*/
#include <stdarg.h>
#include <stdlib.h>
#include "adi_hmc7044.h"
#include "hmc7044_hal.h"

/*============= D E F I N E S ==============*/

/*============= C O D E ====================*/
int32_t hmc7044_hw_open(adi_hmc7044_device_t *device)
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

int32_t hmc7044_hw_close(adi_hmc7044_device_t *device)
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

int32_t hmc7044_sw_delay_us(adi_hmc7044_device_t *device, uint32_t us)
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

int32_t hmc7044_hw_reset(adi_hmc7044_device_t *device)
{
    int32_t err;

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (device->hal_info.reset_pin_ctrl == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_RESET_CTRL_PTR;
    }
    if (device->hal_info.delay_us == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_DELAYUS_PTR;
    }

    err = device->hal_info.reset_pin_ctrl(device->hal_info.user_data, 0x1);
    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_RESET_PIN_CTRL;
    }

    err = hmc7044_sw_delay_us(device, HMC7044_HW_RESET_PERIOD_US);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    err = device->hal_info.reset_pin_ctrl(device->hal_info.user_data, 0x0);
    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_RESET_PIN_CTRL;
    }

    return API_CMS_ERROR_OK;
}

int32_t hmc7044_spi_reg_get(adi_hmc7044_device_t *device , uint32_t reg, uint8_t *data)
{
    int32_t err;
    uint8_t in_data[SPI_IN_OUT_BUFF_SZ] = {0};
    uint8_t out_data[SPI_IN_OUT_BUFF_SZ] = {0};

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if (device->hal_info.spi_xfer == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_XFER_PTR;
    }
    if (data == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_NULL_PARAM;
    }

    in_data[0] = (((reg >> 8) & 0x1F) | 0x80);
    in_data[1] = (reg & 0xFF);
    err = device->hal_info.spi_xfer(device->hal_info.user_data, in_data, out_data, SPI_IN_OUT_BUFF_SZ);
    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_SPI_XFER;
    }
    *data = out_data[2];

    return API_CMS_ERROR_OK;
}

int32_t hmc7044_spi_reg_set(adi_hmc7044_device_t *device, uint32_t reg, uint8_t data)
{
    int32_t err;
    uint8_t in_data[SPI_IN_OUT_BUFF_SZ] = {0};
    uint8_t out_data[SPI_IN_OUT_BUFF_SZ] = {0};

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if (device->hal_info.spi_xfer == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_XFER_PTR;
    }
    in_data[0] = ((reg >> 8) & 0x1F);
    in_data[1] = (reg & 0xFF);
    in_data[2] = data;
    err = device->hal_info.spi_xfer(device->hal_info.user_data, in_data, out_data, SPI_IN_OUT_BUFF_SZ);
    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_SPI_XFER;
    }

    return API_CMS_ERROR_OK;
}

int32_t hmc7044_spi_reg_block_get(adi_hmc7044_device_t *device,
    const uint16_t address, uint8_t *data, uint32_t count)
{
    int err;
    uint16_t i =0;

    for (i = 0; i<count; i++) {
        err = hmc7044_spi_reg_get(device, (address + i), &data[i]);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t hmc7044_spi_reg_tbl_set(adi_hmc7044_device_t *device,
    adi_cms_reg_data_t *tbl, uint32_t count)
{
    uint16_t i =0;
    int err;

    for (i = 0; i<count; i++) {
        err = hmc7044_spi_reg_set(device, tbl[i].reg, tbl[i].val);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
    }

    return API_CMS_ERROR_OK;
}

uint64_t gcd(uint64_t value1, uint64_t value2)
{
	uint64_t maxvalue;
	while (value1 != 0 && value2 != 0) {
		if (value1 > value2){
			value1 %= value2;
		}
		else {
			value2 %= value1;
		}
	}
	maxvalue = value1 > value2 ? value1 : value2;
	return maxvalue;
}

uint64_t lcm(uint64_t value1, uint64_t value2)
{
	if (value1 == 0 && value2 == 0) {
		return 0;
	}
	else {
		return (value1 * value2) / gcd(value1, value2);
	}
}

/*! @} */
