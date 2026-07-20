/*!
 * @brief     Standalone Linux Application Helper Header file
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_AD9986_APP__
 * @{
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "adi_cms_api_common.h"
#include "adi_ad9986.h"
#include "adi_ads9.h"
#include "adi_hmc7044.h"
#include "adi_ad7175.h"
#include "ads9.h"

/*============= M A C R O S====================*/

/*============= C O D E ====================*/

/*===== A P P  J E S D  S T A T U S  &  C O N F I G =====*/
/**
 * @ingroup APP JESD STATUS & CONFIG
 * @brief App Level API. \n Show JESD link status after configuration
 * 
 * @param device    Pointer to the device structure
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details.
 */
int32_t app_show_link_status(adi_ad9986_device_t *device);

/**
 * @ingroup APP JESD STATUS & CONFIG
 * @brief App Level API. \n Run JRX PRBS PHY test
 *        Configure links and enable prior to running this test 
 * 
 * @param device    Pointer to the device structure
 * @param prbs      PRBS pattern @see adi_cms_jesd_prbs_pattern_e for options
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details.
 */
int32_t app_test_phy_prbs(adi_ad9986_device_t *device, adi_cms_jesd_prbs_pattern_e prbs);

/**
 * @ingroup APP JESD STATUS & CONFIG
 * @brief App Level API. \n Calculate tx lane rate using uc settings
 * 
 * @param clock_hz                  Clock settings for selected use case
 * @param jesd_rx_param             Jesd Rx parameters for selected use case
 * @param transmit_interp           Main and channelizer DUC interpolation for selected use case
 * @param calc_app_jrx_lane_rate    Pointer to variable to store calculated lane rate value
 * 
 */
void app_calc_tx_lane_rate(uint64_t clock_hz[4], adi_cms_jesd_param_t *jesd_rx_param, uint8_t transmit_interp[2], uint64_t *calc_app_jrx_lane_rate);

/**
 * @ingroup APP JESD STATUS & CONFIG
 * @brief App Level API. \n Calculate rx lane rate using uc settings
 * 
 * @param clock_hz                  Clock settings for selected use case
 * @param jesd_tx_param             Jesd Tx parameters for selected use case
 * @param jesd_tx_chip_dcm_ratio    Coarse and fine DDC decimation for selected use case
 * @param calc_app_jtx_lane_rate    Pointer to variable to store calculated lane rate value
 * 
 */
void app_calc_rx_lane_rate(uint64_t clock_hz[4], adi_cms_jesd_param_t jesd_tx_param[2], uint8_t jesd_tx_chip_dcm_ratio[2], uint64_t calc_app_jtx_lane_rate[2]);

/**
 * @ingroup APP JESD STATUS & CONFIG
 * @brief App Level API. \n Load pre-saved CTLE coefficients from file to bypass JESD RX 204C calibration
 * 
 * @param device    Pointer to the device structure
 * @param filename  Name of txt file to load coefficients
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details.
 */
int32_t app_cal_ctle_manual_config_load(adi_ad9986_device_t *device, char filename[]);

/**
 * @ingroup APP JESD STATUS & CONFIG
 * @brief App Level API. \n Save CTLE coefficients to file
 * 
 * @param device    Pointer to the device structure
 * @param filename  Name of txt file to save coefficients
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details.
 */
int32_t app_cal_ctle_manual_config_save(adi_ad9986_device_t *device, char filename[]);

/**
 * @ingroup APP JESD STATUS & CONFIG
 * @brief App Level API. \n Calculate nco frequency in Hz from frequency tuning word
 * 
 * @param freq          ADC or DAC clock frequency in Hz
 * @param ftw           Frequency tuning word of NCO
 * @param nco_shift     Pointer to variable to store calculated nco shift value
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details.
 */
int32_t app_calc_nco_freq_get(uint64_t freq, uint64_t ftw, double *nco_shift);

/**
 * @ingroup APP JESD STATUS & CONFIG
 * @brief App Level API. \n Write eye scan data points to file 
 * 
 * @param device            Pointer to the device structure
 * @param mode              Lane rate mode @see adi_ad9986_deser_mode_e for options
 * @param lane              Lane that data is collected on
 * @param spo_size          SPO size of collected data
 *                          Quarter rate: 32, Half rate: 64
 * @param eye_scan_data     Collected eye scan data to save to file
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details.
 */
int32_t app_test_eye_scan_write_to_file(adi_ad9986_device_t *device, adi_ad9986_deser_mode_e mode, uint8_t lane, uint8_t spo_size, int16_t eye_scan_data[]);

/**
 * @ingroup APP JESD STATUS & CONFIG
 * @brief App Level API. \n Run JRX eye scan for quarter rate and half rate modes
 * 
 * @param device            Pointer to the device structure
 * @param jesd_rx_param     JESD Rx parameters for selected use case
 * @param uc                Selected use case
 * @param mode              Lane rate mode @see adi_ad9986_deser_mode_e for options
 * @param prbs              PRBS test mode pattern @see adi_cms_jesd_prbs_pattern_e for options
 * @param lane              Lane that data is collected on
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details.
 */
int32_t app_test_eye_scan(adi_ad9986_device_t *device, adi_cms_jesd_param_t *jesd_rx_param, int32_t uc, adi_ad9986_deser_mode_e mode, adi_cms_jesd_prbs_pattern_e prbs, uint8_t lane);


/*===== A P P  C L O C K I N G =====*/
/**
 * @ingroup APP CLOCKING
 * @brief App Level API. \n Select clock source for sysref signal
 * 
 * @param sysref_clk    Pointer to clock source for SYSREF signal
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details.
 */
int32_t app_sysref_clk_src_sel(void *sysref_clk);


/*===== A P P  P O W E R =====*/
/**
 * @ingroup APP POWER
 * @brief App Level API. \n Collect power measurements from AD7175 ADC
 * 
 * @param ad7175_dev    Pointer to the ad7175 device structure
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details.
 */
int32_t app_collect_power_measurements(adi_ad7175_device_t *ad7175_dev);
