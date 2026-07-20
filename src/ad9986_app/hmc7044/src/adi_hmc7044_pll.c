/*!
 * @brief     DUAL PLL Clock Multiplier Support API implementation
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __HMC7044_PLL_API__
 * @{
 */

/*============= I N C L U D E S ============*/
#include <stdlib.h>
#include "adi_utils.h"
#include "adi_hmc7044.h"
#include "hmc7044_hal.h"
#include "hmc7044_reg.h"

/*============= C O D E ====================*/
int32_t adi_hmc7044_input_reference_set(adi_hmc7044_device_t *device,
        uint8_t clk_in, uint8_t config, uint8_t enable)
{
    int32_t err;
    uint16_t reg_addr;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if ((clk_in >= (HMC7044_NOF_CLK_IN + HMC7044_NOF_OSC_IN)) || (enable > 1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if (((config & IPBUFFER_HIGH_Z_MODE_EN)     && (config & IPBUFFER_AC_COUPLED_MODE_EN)) ||
        ((config & IPBUFFER_HIGH_Z_MODE_EN)     && (config & IPBUFFER_LVPECL_MODE_EN)) ||
        ((config & IPBUFFER_AC_COUPLED_MODE_EN) && (config & IPBUFFER_LVPECL_MODE_EN)) ||
        ((config > IPBUFFER_CONFIG_MAX))) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    reg_addr = ((clk_in * HMC7044_CLK_IP_BUFF_OFFSET) + HMC7044_CLK_IP_BUFF_BASE_REG);
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, ((config <<1) | (enable)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_enable_input_reference_set(adi_hmc7044_device_t *device, uint8_t clk_in, uint8_t enable)
{
	int32_t err;
	uint8_t reg_val = 0x00;
	uint16_t reg_addr = ((clk_in * HMC7044_CLK_IP_BUFF_OFFSET) + HMC7044_CLK_IP_BUFF_BASE_REG);
	if (err = adi_hmc7044_device_spi_register_get(device, reg_addr, &reg_val), err != API_CMS_ERROR_OK) {
		return err;
	}

	reg_val |= enable;
	err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}

	return API_CMS_ERROR_OK;	
}

int32_t adi_hmc7044_input_reference_get(adi_hmc7044_device_t *device,  uint8_t *status)
{
    int32_t err;
    uint8_t reg;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (err = adi_hmc7044_device_spi_register_get(device, 0x82, &reg), err != API_CMS_ERROR_OK) {
		return err;
    }
    
    *status = (reg & 0x18);
	return API_CMS_ERROR_OK;    
}

int32_t adi_hmc7044_input_reference_priority_set(adi_hmc7044_device_t *device,
        uint8_t priority[4], uint8_t nof_ref)
{
    int32_t err;
    uint8_t i;
    uint8_t reg_val = 0x0;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if ((priority == ADI_INVALID_POINTER) || 
        (nof_ref > HMC7044_NOF_CLK_IN) || 
        (nof_ref < 1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    for (i=0; i<HMC7044_NOF_CLK_IN; i++) {
        if (i < nof_ref) {
            reg_val |= priority[i] << (2*i);
        } else {
            reg_val |= priority[nof_ref-1] << (2*i);
        }
    }
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_CLK_IP_PRIORITY_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_input_reference_los_config_set(adi_hmc7044_device_t *device,
        uint8_t timer_cycles, uint8_t prescaler_bypass, uint8_t vcxo_prescaler_en)
{
    int32_t err;
    uint8_t reg_val = 0x0;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (timer_cycles > HMC7044_PLL1_LOS_VALID_TIMER(ADI_UTILS_ALL)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if ((prescaler_bypass > 1) || (vcxo_prescaler_en > 1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_LOS_TIMER_CTRL_REG, timer_cycles);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    reg_val |= prescaler_bypass  ? HMC7044_LOS_INPUT_PRESCALER_BYPASS : 0x0;
    reg_val |= vcxo_prescaler_en ? HMC7044_LOS_VCXO_PRESCALER_EN : 0x0;
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_LOS_MODE_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_input_reference_prescaler_config_set(adi_hmc7044_device_t *device,
        uint8_t clk_in, uint8_t lcm_div)
{
    int32_t err;
    uint8_t reg_addr;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if ((clk_in >= HMC7044_NOF_CLK_IN) || (lcm_div < 1)) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    reg_addr = (clk_in * HMC7044_CLKINX_PRESCALER_OFFSET) + HMC7044_CLKINX_PRESCALER_BASE_REG;
    err = adi_hmc7044_device_spi_register_set(device, reg_addr, lcm_div);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_input_reference_oscin_prescaler_config_set(adi_hmc7044_device_t *device, uint8_t lcm_div)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if (lcm_div < 1) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_OSCIN_PRESCALER_REG, lcm_div);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll1_config_set(adi_hmc7044_device_t *device, uint16_t r_div, uint16_t n_div)
{
    int32_t err;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
        
    if ((r_div < 1) || (n_div < 1 ))  {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_R_DIV_LSB_REG, ((uint8_t) (r_div & 0xFF)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_R_DIV_MSB_REG, ((uint8_t) ((r_div >>8) & 0xFF)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_N_DIV_LSB_REG, ((uint8_t) (n_div & 0xFF)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_N_DIV_MSB_REG, ((uint8_t) ((n_div >>8) & 0xFF)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll2_config_set(adi_hmc7044_device_t *device, uint8_t freq_dbl_en, uint16_t r_div, uint16_t n_div)
{
    int32_t err;
    uint8_t reg_val = 0x0;
    if (device == ADI_INVALID_POINTER) {
        return API_CMS_ERROR_INVALID_HANDLE_PTR;
    }
    if ((r_div < 1) || (n_div < 1) || (r_div > HMC7044_PLL2_R_DIV_MAX ))  {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if (freq_dbl_en > 1)  {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL2_R_DIV_LSB_REG, ((uint8_t) (r_div & 0xFF)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL2_R_DIV_MSB_REG, ((uint8_t) ((r_div >>8) & 0xFF)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL2_N_DIV_LSB_REG, ((uint8_t) (n_div & 0xFF)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL2_N_DIV_MSB_REG, ((uint8_t) ((n_div >>8) & 0xFF)));
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
    reg_val = freq_dbl_en ? HMC7044_PLL2_FREQ_DOUBLER_EN : 0x0;
    err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL2_FREQ_DOUBLER_REG, reg_val);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_device_pll_lock_status_get(adi_hmc7044_device_t *device, uint8_t *status)
{
	uint8_t reg_val = 0x0;
	int32_t err;

	if (status == NULL)
	   return API_CMS_ERROR_NULL_PARAM;

	*status = 0;
	if (err = adi_hmc7044_device_spi_register_get(device, 0x7D, &reg_val), err != API_CMS_ERROR_OK)
	    return err;       

	if (reg_val & 0x1)
	    *status |= HMC7044_PLL2_LOCK_ST;
	if (reg_val &  0x8)
	    *status |= HMC7044_PLL1_AND_PLL2_LOCK_ST;
	
	if (err = adi_hmc7044_device_spi_register_get(device, 0x7C, &reg_val), err != API_CMS_ERROR_OK)
		return err;
        if (reg_val & 0x20) 
	    *status |= HMC7044_PLL1_LOCK_ST;
	return API_CMS_ERROR_OK;    
}

int32_t adi_hmc7044_pll1_holdover_exit_ctrl_set(adi_hmc7044_device_t *device, adi_hmc_pll1_holdover_config_t *config) 
{
	int32_t err;
	uint8_t reg_val = (config->exit_action << 2) | config ->exit_criteria;
	err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_HOLDOVER_EXIT_CTRL_REG, reg_val);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}
	
	err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_HOLDOVER_DAC_CTRL_REG, config ->holdover_dac);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}
	reg_val = (config ->adc_tracking << 3) | (config ->quick_mode << 2) | config ->holdover_bw;
	err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_HOLDOVER_ADC_CTRL_REG, reg_val);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}

	return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll1_charge_pump_ctrl_set(adi_hmc7044_device_t *device, uint8_t charge_pump)
{
	int32_t err;
	err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_CHARGE_PUMP_CRTL_REG, charge_pump);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}

	return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll1_lock_detect_set(adi_hmc7044_device_t *device, uint8_t slip_use, uint8_t lock_detect_timer)
{
	int32_t err;
	uint8_t reg_val = (slip_use << 5) | lock_detect_timer;
	err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_LOCK_DETECT_CTRL_REG, reg_val);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}

	return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll1_reference_switch_ctrl_set(adi_hmc7044_device_t *device, uint8_t bypass_debounce, uint8_t manual_switch, uint8_t holdover_dac_use, uint8_t autorevert_switch, uint8_t automode_switch)
{
	int32_t err;
	uint8_t reg_val = (bypass_debounce << 5) | (manual_switch << 3) | (holdover_dac_use << 2) | (autorevert_switch << 1) | automode_switch; 
	err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_REF_SWITCH_CTRL_REG, reg_val);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}

	return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll1_holdoff_time_ctrl_set(adi_hmc7044_device_t *device, uint8_t hold_off_time)
{
	int32_t err;
	err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL1_HOLDOFF_TIME_CRTL_REG, hold_off_time);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}

	return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll2_charge_pump_ctrl_set(adi_hmc7044_device_t *device, uint8_t charge_pump)
{
	int32_t err;
	err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL2_CHARGE_PUMP_CRTL_REG, charge_pump);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}

	return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_oscout_path_config_set(adi_hmc7044_device_t *device, uint8_t divider, uint8_t enable)
{
	int32_t err;
	uint8_t reg_val = (divider << 1) | enable;
	err = adi_hmc7044_device_spi_register_set(device, HMC7044_PLL2_OSCOUT_PATH_CTRL_REG, reg_val);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}

	return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_oscout_driver_config_set(adi_hmc7044_device_t *device, uint8_t oscout_ch, adi_hmc7044_op_driver_config_t *config)
{
	int32_t  err;
	uint16_t reg_addr;
	uint8_t  reg_val = 0x0;
	if (device == ADI_INVALID_POINTER) {
		return API_CMS_ERROR_INVALID_HANDLE_PTR;
	}
	if (oscout_ch >= HMC7044_NOF_OP_CH) {
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
        (config->impedance != ADI_CMS_INTERNAL_RESISTOR_50_OHM)) {
		return API_CMS_ERROR_INVALID_PARAM;
	}
	if (config->dynamic_driver_en > 1) {
		return API_CMS_ERROR_INVALID_PARAM;
	}
	reg_addr = (HMC7044_OSCOUT1_CTRL_REG + (oscout_ch * HMC7044_OSC_OP_CTRL_OFFSET));
	if (config->impedance == ADI_CMS_INTERNAL_RESISTOR_100_OHM) {
		reg_val |= HMC7044_OSC_OP_DRIVER_IMPEDANCE(0x1);
	}
	else if (config->impedance == ADI_CMS_INTERNAL_RESISTOR_50_OHM) {
		reg_val |= HMC7044_OSC_OP_DRIVER_IMPEDANCE(0x3);
	}
	reg_val |= config->dynamic_driver_en ? 0x01 : 0x0;
	switch (config->mode) {
	case SIGNAL_CML:
		reg_val |= HMC7044_OSC_OP_DRIVER_MODE(0x0);
		break;
	case SIGNAL_LVPECL:
		reg_val |= HMC7044_OSC_OP_DRIVER_MODE(0x1);
		break;
	case SIGNAL_LVDS:
		reg_val |= HMC7044_OSC_OP_DRIVER_MODE(0x2);
		break;
	case SIGNAL_CMOS:
	default:
		reg_val |= HMC7044_OSC_OP_DRIVER_MODE(0x3);
		break;
	}

	err = adi_hmc7044_device_spi_register_set(device, reg_addr, reg_val);
	if (err != API_CMS_ERROR_OK) {
		return err;
	}

	return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_distribution_clk_config(adi_hmc7044_device_t *device, uint64_t ref_clk_freq_hz, 
        uint64_t pfd1_freq_hz, uint64_t dist_freq_hz, uint64_t pfd2_freq_hz)
{       
    int32_t  err;
    uint64_t R1, N1, N2, R2, Ra;    
    
    if (ref_clk_freq_hz > 800e6 || ref_clk_freq_hz < 0.00015e6) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if (dist_freq_hz == 0 || pfd1_freq_hz == 0) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    
    if (ref_clk_freq_hz % (pfd1_freq_hz * 4) == 0) {
        Ra = ref_clk_freq_hz / pfd1_freq_hz / 4;
        if (Ra > 65535) {
            return API_CMS_ERROR_ERROR;
        }
    } else {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    
    R1 = ref_clk_freq_hz / pfd1_freq_hz / 4;
    N2 = dist_freq_hz / pfd2_freq_hz;
    N1 = 245.76e6 / pfd1_freq_hz / 2;
    R2 = 2 * 122.88e6 / pfd2_freq_hz;
    
    err = adi_hmc7044_pll1_config_set(device, R1, N1);
    if ( err != API_CMS_ERROR_OK) {
        return err; 
    }
    err = adi_hmc7044_pll2_config_set(device, 0x0, R2, N2);
    if (err!= API_CMS_ERROR_OK) {
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_hmc7044_pll_config(adi_hmc7044_device_t *device, adi_hmc7044_clk_in_e ref_ch, uint64_t ref_clk_freq_hz, uint64_t fvcxo_clk_freq_hz, uint64_t fpfd1_freq_hz, uint64_t fvco_freq_hz){
	int32_t err;
	uint64_t flcm_clk_hz, pfd1_clk_hz, pfd2_clk_hz, vcxo_prescaler, pll2ref_clk_hz;
	uint64_t R1, N1, N2, R2;    
	uint8_t  hmc_priority[] = { 0, 1, 2, 3 };
	
	/*range check*/
	if (device == ADI_INVALID_POINTER) {
		return API_CMS_ERROR_INVALID_HANDLE_PTR;
	}

	if (ref_ch > HMC7044_CLK_IN_ALL) {
		return API_CMS_ERROR_INVALID_HANDLE_PTR;
	}
	
	if ((ref_clk_freq_hz > HMC7044_REF_CLK_FREQ_HZ_MAX || ref_clk_freq_hz < HMC7044_REF_CLK_FREQ_HZ_MIN)) {
		return API_CMS_ERROR_INVALID_PARAM;
	}

	if (fvcxo_clk_freq_hz < HMC7044_VCXO_CLK_FREQ_HZ_MIN || fvcxo_clk_freq_hz > HMC7044_VCXO_CLK_FREQ_HZ_MAX) {
		return API_CMS_ERROR_INVALID_PARAM;
	}
	
	/*enable clkin*/
	if (err = adi_hmc7044_input_reference_set(device, 0, IPBUFFER_INTERNAL_100_OHM_EN | IPBUFFER_AC_COUPLED_MODE_EN, (ref_ch & HMC7044_CLK_IN_0)), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_input_reference_set(device, 1, IPBUFFER_INTERNAL_100_OHM_EN | IPBUFFER_AC_COUPLED_MODE_EN, (ref_ch & HMC7044_CLK_IN_1) >> 1), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_input_reference_set(device, 2, IPBUFFER_INTERNAL_100_OHM_EN | IPBUFFER_AC_COUPLED_MODE_EN, (ref_ch & HMC7044_CLK_IN_2) >> 2), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_input_reference_set(device, 3, IPBUFFER_INTERNAL_100_OHM_EN | IPBUFFER_AC_COUPLED_MODE_EN, (ref_ch & HMC7044_CLK_IN_3) >> 3), err != API_CMS_ERROR_OK)
		return err;	
	if (err = adi_hmc7044_input_reference_priority_set(device, hmc_priority, 4), err != API_CMS_ERROR_OK)
		return err;

	/*calculate fLCM, typical range to be 30~40 MHz*/
	flcm_clk_hz = ref_clk_freq_hz;

	/*scale flcm to 30-40 range*/
	if (ref_clk_freq_hz > HMC7044_PLL1REF_CLK_FREQ_HZ_MAX || ref_clk_freq_hz < HMC7044_PLL1REF_CLK_FREQ_HZ_MIN) {
		uint8_t div = ref_clk_freq_hz / HMC7044_PLL1REF_CLK_FREQ_HZ_MIN;
		flcm_clk_hz = ref_clk_freq_hz / div;
	}
	/*calculate vcxo prescaler*/
	if (fvcxo_clk_freq_hz % flcm_clk_hz != 0) {
		return API_CMS_ERROR_ERROR;
	}
	vcxo_prescaler = fvcxo_clk_freq_hz / flcm_clk_hz;
		
	if (err = adi_hmc7044_input_reference_los_config_set(device, 7, 0, 0), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_input_reference_prescaler_config_set(device, 0, vcxo_prescaler), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_input_reference_prescaler_config_set(device, 1, vcxo_prescaler), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_input_reference_prescaler_config_set(device, 2, vcxo_prescaler), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_input_reference_prescaler_config_set(device, 3, vcxo_prescaler), err != API_CMS_ERROR_OK)
		return err;
	if (err = adi_hmc7044_input_reference_oscin_prescaler_config_set(device, vcxo_prescaler), err != API_CMS_ERROR_OK)
		return err;
	
	/*calculate fPFD1*/
	pfd1_clk_hz = gcd(flcm_clk_hz, fvcxo_clk_freq_hz);
	if (fvcxo_clk_freq_hz / 65535 > pfd1_clk_hz) {
		return API_CMS_ERROR_ERROR;
	}
	
	/* calculate pll2ref*/
	pll2ref_clk_hz = fvcxo_clk_freq_hz <= 125e6 ? 2 * fvcxo_clk_freq_hz : fvcxo_clk_freq_hz;
		
	/*calculate fpfd2*/
	pfd2_clk_hz = gcd(fvco_freq_hz, pll2ref_clk_hz);
	
	/*calculate R1, N1, R2, N2*/
	R1 = flcm_clk_hz / pfd1_clk_hz;
	N1 = fvcxo_clk_freq_hz / pfd1_clk_hz;
	R2 = pll2ref_clk_hz / pfd2_clk_hz;
	N2 = fvco_freq_hz / pfd2_clk_hz;
	
	/*pll config*/
	if (err = adi_hmc7044_pll1_config_set(device, R1, N1), err != API_CMS_ERROR_OK) 
		return err; 
	if (err = adi_hmc7044_pll2_config_set(device, 0x0, R2, N2), err != API_CMS_ERROR_OK) 
		return err;
	if (err = adi_hmc7044_output_performance_set(device, 1), err != API_CMS_ERROR_OK)
		return err;
	return API_CMS_ERROR_OK;
}

/*! @! */
