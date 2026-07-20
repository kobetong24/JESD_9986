/*!
 * @brief     Output Clock Distributor Support API implementation
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __HMC7044_OUTPUT_CH_API__
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_utils.h"
#include "adi_hmc7044.h"
#include "hmc7044_hal.h"
#include "hmc7044_reg.h"

/*============= D E F I N E S ==============*/

/*============= C O D E ====================*/
int32_t adi_hmc7044_output_config_set(adi_hmc7044_device_t *device, uint8_t output_ch,
	    adi_hmc7044_op_source_e output_sel, uint16_t ch_div, uint8_t mode, uint8_t enable)
{
    int32_t err;
    uint16_t reg_addr;
    uint8_t reg_val;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if ((output_ch >= HMC7044_NOF_OP_CH) || (ch_div > HMC7044_CH_DIV_MAX)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if ((output_sel >= HMC7044_OP_SIG_INVALID) || (mode > 1 ) || (enable > 1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    reg_addr = (HMC7044_CLK_OP_CTRL_0_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    err = adi_hmc7044_device_spi_register_get(device, reg_addr, &reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    if (!enable) {
        reg_val &= 0xFE;
        err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
         return API_CMS_ERROR_OK;
    }
    reg_val &= ~HMC7044_CLK_OP_HIGH_PERFORM_EN;
    reg_val |= mode ? HMC7044_CLK_OP_HIGH_PERFORM_EN : 0;
    reg_val |= enable;
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    reg_addr = (HMC7044_CLK_OP_CTRL_1_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    reg_val = (uint8_t) (ch_div & 0xFF);
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_addr = (HMC7044_CLK_OP_CTRL_2_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    reg_val = (uint8_t) ((ch_div >> 8) & 0xFF);
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_addr = (HMC7044_CLK_OP_CTRL_7_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    reg_val = 0x0;
    reg_val |= HMC7044_CLK_OP_MUX_SEL(output_sel);
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_output_driver_config_set(adi_hmc7044_device_t *device,
        uint8_t output_ch, adi_hmc7044_op_driver_config_t *config)
{
    int32_t  err;
    uint16_t reg_addr;
    uint8_t  reg_val = 0x0;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (output_ch >= HMC7044_NOF_OP_CH)  {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if ((config->mode != SIGNAL_CML)  &&
        (config->mode != SIGNAL_LVPECL) &&
        (config->mode != SIGNAL_LVDS) &&
        (config->mode != SIGNAL_CMOS)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if ((config->impedance != ADI_CMS_NO_INTERNAL_RESISTOR) &&
        (config->impedance != ADI_CMS_INTERNAL_RESISTOR_100_OHM) &&
        (config->impedance != ADI_CMS_INTERNAL_RESISTOR_50_OHM) ) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if ((config->dynamic_driver_en > 1 ) ||
        (config->force_mute_en > 1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    reg_addr = (HMC7044_CLK_OP_CTRL_8_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    if (config->impedance == ADI_CMS_INTERNAL_RESISTOR_100_OHM) {
        reg_val |= HMC7044_CLK_OP_DRIVER_IMPEDANCE(0x1);
    } else if (config->impedance == ADI_CMS_INTERNAL_RESISTOR_50_OHM) {
        reg_val |= HMC7044_CLK_OP_DRIVER_IMPEDANCE(0x3);
    }
    reg_val |= config->dynamic_driver_en ? HMC7044_CLK_OP_DYNAMIC_DR_EN   : 0x0;
    reg_val |= config->force_mute_en     ? HMC7044_CLK_OP_FORCE_MUTE(0x2) : 0x0;
    switch (config->mode) {
    case SIGNAL_CML:
        reg_val |= HMC7044_CLK_OP_DRIVER_MODE(0x0);
        break;
    case SIGNAL_LVPECL:
        reg_val |= HMC7044_CLK_OP_DRIVER_MODE(0x1);
        break;
    case SIGNAL_LVDS:
        reg_val |= HMC7044_CLK_OP_DRIVER_MODE(0x2);
        break;
    case SIGNAL_CMOS:
    default:
        reg_val |= HMC7044_CLK_OP_DRIVER_MODE(0x3);
        break;
    }

    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_output_enable_set(adi_hmc7044_device_t *device, uint8_t output_ch, uint8_t en)
{
    int32_t  err;
    uint16_t reg_addr;
    uint8_t  reg_val = 0x0;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if ((output_ch >= HMC7044_NOF_OP_CH) || (en >1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    reg_addr = (HMC7044_CLK_OP_CTRL_0_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    err = adi_hmc7044_device_spi_register_get(device, reg_addr, &reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_val &= !HMC7044_CLK_OP_EN;
    reg_val |= en? HMC7044_CLK_OP_EN : 0x0;
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_output_delay_set(adi_hmc7044_device_t *device,
        uint8_t output_ch, uint8_t coarse_adj, uint8_t fine_adj)
{
    int32_t  err;
    uint16_t reg_addr;
    uint8_t  reg_val = 0x0;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if (output_ch >= HMC7044_NOF_OP_CH) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if ((coarse_adj >= HMC7044_CLK_OP_COURSE_DELAY(ADI_UTILS_ALL)) ||
        (fine_adj   >= HMC7044_CLK_OP_FINE_DELAY(ADI_UTILS_ALL))) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    reg_addr = (HMC7044_CLK_OP_CTRL_4_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    reg_val = coarse_adj;
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    reg_addr = (HMC7044_CLK_OP_CTRL_3_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    reg_val = fine_adj;
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_output_sync_config_set(adi_hmc7044_device_t *device, uint8_t output_ch,
        uint8_t startup_mode, uint8_t slip_mode_en, uint8_t sync_mode_en)
{
    int32_t  err;
    uint16_t reg_addr;
    uint8_t  reg_val = 0x0;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if (output_ch >= HMC7044_NOF_OP_CH) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if ((startup_mode > 1) ||
        (slip_mode_en > 1) ||
        (sync_mode_en >1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    reg_addr = (HMC7044_CLK_OP_CTRL_0_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    err = adi_hmc7044_device_spi_register_get(device, reg_addr, &reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_val &= ~(HMC7044_CLK_OP_SYNC_EN |
                 HMC7044_CLK_OP_SLIP_EN |
                 HMC7044_CLK_OP_STARTUP_MODE(ADI_UTILS_ALL));
    reg_val |= startup_mode ? HMC7044_CLK_OP_STARTUP_MODE(0x3) : HMC7044_CLK_OP_STARTUP_MODE(0x0);
    reg_val |= slip_mode_en ? HMC7044_CLK_OP_SLIP_EN : 0;
    reg_val |= sync_mode_en ? HMC7044_CLK_OP_SYNC_EN : 0;
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_output_multi_slip_config_set(adi_hmc7044_device_t *device,
        uint8_t output_ch, uint8_t multi_slip_en, uint16_t slip_delay)
{
    int32_t  err;
    uint16_t reg_addr;
    uint8_t  reg_val = 0x0;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }

    if (output_ch >= HMC7044_NOF_OP_CH) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if ((multi_slip_en > 1) || (slip_delay > HMC7044_SLIP_DELAY_MAX)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    reg_addr = (HMC7044_CLK_OP_CTRL_0_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
    err = adi_hmc7044_device_spi_register_get(device, reg_addr, &reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_val &= ~(HMC7044_CLK_OP_MULTI_SLIP_EN);
    reg_val |= multi_slip_en ? HMC7044_CLK_OP_MULTI_SLIP_EN : 0;
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    if (multi_slip_en) {
        reg_val = (uint8_t) (slip_delay & 0xFF);
        reg_addr = (HMC7044_CLK_OP_CTRL_5_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
        err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
        reg_val = (uint8_t) ((slip_delay >> 8) &0xF);
        reg_addr = (HMC7044_CLK_OP_CTRL_6_REG + (output_ch * HMC7044_CLK_OP_CTRL_OFFSET));
        err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
        if (err != API_CMS_ERROR_OK) {
            return err;
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_output_performance_set(adi_hmc7044_device_t *device, uint8_t enable)
{
	int32_t  err;
	if (device == ADI_INVALID_POINTER) {
		return API_CMS_ERROR_INVALID_HANDLE_PTR;
	}

	if (!enable)
		return API_CMS_ERROR_OK;

	if (err = adi_hmc7044_device_spi_register_set(device, 0x009f, 0x4d), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_device_spi_register_set(device, 0x00a0, 0xdf), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_device_spi_register_set(device, 0x00a5, 0x6), err != API_CMS_ERROR_OK)
		return err;
	if (err =  adi_hmc7044_device_spi_register_set(device, 0x00a8, 0x6), err != API_CMS_ERROR_OK)
		return err;
	if (err =  adi_hmc7044_device_spi_register_set(device, 0x00b0, 0x4), err != API_CMS_ERROR_OK)
		return err;

	return API_CMS_ERROR_OK;
}
/*! @! */
