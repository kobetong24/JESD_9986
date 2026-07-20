/*!
 * @brief     Device level API implementation
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __AD7175_DEVICE_API__
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_utils.h"
#include "adi_ad7175.h"
#include "ad7175_hal.h"
#include "ad7175_reg.h"

/*============= D A T A ====================*/
static uint8_t ad7175_api_revision[3] = {1,0,0};

/*============= C O D E ====================*/

int32_t adi_ad7175_device_hw_open(adi_ad7175_device_t *device)
{
    int32_t err;

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (device->hal_info.spi_xfer == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_XFER_PTR;
    }
    if (device->hal_info.delay_us == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_DELAYUS_PTR;
    }

    err = ad7175_hw_open(device);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ad7175_device_hw_close(adi_ad7175_device_t *device)
{
    int32_t err;

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    err = ad7175_hw_close(device);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ad7175_device_spi_register_set(adi_ad7175_device_t *device, uint16_t addr, uint32_t val)
{
    return ad7175_spi_reg_set(device, addr, val);
}

int32_t adi_ad7175_device_spi_register_get(adi_ad7175_device_t *device, uint16_t addr, uint32_t *val, uint32_t size_bytes)
{
    return ad7175_spi_reg_get(device, addr, val, size_bytes);
}

int32_t adi_ad7175_device_api_revision_get(adi_ad7175_device_t *device, uint8_t *rev_major,
        uint8_t *rev_minor, uint8_t *rev_rc)
{
    int32_t err;

    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    err = API_CMS_ERROR_OK;
    if (rev_major != ADI_INVALID_POINTER) {
        *rev_major = ad7175_api_revision[0];
    } else {
        err = API_CMS_ERROR_INVALID_PARAM;
    }
    if (rev_minor != ADI_INVALID_POINTER) {
        *rev_minor = ad7175_api_revision[1];
    } else {
        err = API_CMS_ERROR_INVALID_PARAM;
    }
    if(rev_rc != ADI_INVALID_POINTER) {
        *rev_rc = ad7175_api_revision[2];
    } else {
        err = API_CMS_ERROR_INVALID_PARAM;
    }

    return err;
}

int32_t adi_ad7175_device_id_get(adi_ad7175_device_t *device, uint32_t *id)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    err = ad7175_spi_reg_get(device, AD7175_ID_REG, id, 0x00000002);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ad7175_device_channel_config_set(adi_ad7175_device_t *device, uint8_t chan, uint8_t ch_enable, adi_ad7175_setup_sel_e setup, adi_ad7175_ain_e ainpos, adi_ad7175_ain_e ainneg)
{        
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (chan > 0xf || ch_enable > 0x1 || setup > 0x7 || ainpos > 0x10 || ainneg > 0x10) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    err = ad7175_spi_reg_set(device, (chan | 0x10), ((ch_enable << 15) | setup | (ainpos << 5) | ainneg));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ad7175_device_channel_run(adi_ad7175_device_t *device, uint8_t chan, uint8_t ref_en, adi_ad7175_clk_sel_e clk_sel, 
adi_ad7175_adc_mode_e adc_mode, adi_ad7175_prog_delay_e delay, uint8_t hide_delay, uint8_t sing_cyc) {
    
    int32_t err;
    uint8_t i;
    uint32_t reg_val = 0;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    for (i = 0; i < 8; i++){
        /* Save Current Channel Settings */
        err = ad7175_spi_reg_get(device, (i | 0x10), &reg_val, 0x00000002);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
        if(i == chan){
            /* Enable Channel */
            err = ad7175_spi_reg_set(device, (i | 0x10), (CH_MAP_REG_CHEN | reg_val));
            if (err != API_CMS_ERROR_OK) {
                return err;
            }
        }
        else {
            /* Disable */
            reg_val &= 0x7FFF;
            err = ad7175_spi_reg_set(device, (i | 0x10), reg_val);
            if (err != API_CMS_ERROR_OK) {
              return err;
            }
        }
    }

    /* Trigger Conversion */
    err = adi_ad7175_device_adc_mode_config_set(device, ref_en, clk_sel, adc_mode, delay, hide_delay, sing_cyc);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_ad7175_device_setup_config_set(adi_ad7175_device_t *device, adi_ad7175_setup_sel_e setup, adi_ad7175_setup_ref_sel_e ref_sel, uint32_t setup_config)
{
    int32_t err;
    uint32_t reg_val = 0;
    uint32_t config_input = 0x0000;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (setup > 0x0007 || ref_sel > 0x04 || setup_config > 0xFFFF) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    config_input = (ref_sel << 4) | (setup_config);
    err = ad7175_spi_reg_set(device, (setup | 0x20), config_input);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = ad7175_spi_reg_get(device, (setup | 0x20), &reg_val, 0x00000002);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ad7175_device_adc_mode_config_set(adi_ad7175_device_t *device, uint8_t ref_en, adi_ad7175_clk_sel_e clk_sel, adi_ad7175_adc_mode_e adc_mode, adi_ad7175_prog_delay_e delay, 
uint8_t hide_delay, uint8_t sing_cyc)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (adc_mode < 0x0000 || adc_mode > 0x0007) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if ((clk_sel > 0x3) || (delay > 0x7) || (sing_cyc > 1) || (hide_delay > 1) || (ref_en > 1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    err = ad7175_spi_reg_set(device, AD7175_ADC_MODE_REG, (ADC_MODE_REG_CLKSEL(clk_sel) | ADC_MODE_REG_MODE(adc_mode) | ADC_MODE_REG_DELAY(delay) | (sing_cyc << 13) | (hide_delay << 14) | (ref_en << 15)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ad7175_device_intf_config_set(adi_ad7175_device_t *device, uint32_t intf_mode)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (intf_mode < 0x00 || intf_mode > 0x1FFF) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    err = ad7175_spi_reg_set(device, AD7175_IF_MODE_REG, (intf_mode));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ad7175_device_data_get(adi_ad7175_device_t *device, uint32_t *data_status)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    err = ad7175_spi_reg_get(device, AD7175_DATA_REG, data_status, 3);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ad7175_device_run_data_conv_get(adi_ad7175_device_t *device, uint32_t *data_status, uint8_t channel)
{
    int32_t err;
    uint8_t i;
    uint32_t ready=0, timeout=20;

    for (i = 0; i <= timeout; i++) {
        /* Read the value of the Status Register */
        err = ad7175_spi_reg_get(device, AD7175_STATUS_REG, &ready, 1);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
        if ((ready >> 7) == 0) {
            break;
        }
    }
    if (i == timeout) {
        return API_CMS_ERROR_DELAY_US;
    }
    else {
        /* Check for errors in Status register */
       if ((ready & 0x70) | ((ready & 0x0f) != channel)) {
            return API_CMS_ERROR_ERROR;
       }
        /* Get data */
       err = adi_ad7175_device_data_get(device, data_status);
       if (err != API_CMS_ERROR_OK) {
            return err;
       }
    }
    return API_CMS_ERROR_OK; 
}

/*! @} */