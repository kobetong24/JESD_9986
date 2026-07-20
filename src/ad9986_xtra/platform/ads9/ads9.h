/*!
 * @brief     ADS9 platform configuration header file.
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __PLATFORM_ADS9__
 * @{
 */
#ifndef __ADS9_H__
#define __ADS9_H__

/*============= I N C L U D E S ============*/
#include "adi_ad9986.h"
#include "adi_hmc7044.h"
#include "adi_ad7175.h"

/*============= D E F I N E S ==============*/

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

int32_t ads9_hw_open(const char *log_file);
int32_t ads9_hw_close();
int32_t ads9_log_write(void *user_data, int32_t log_type, const char *comment, va_list argp);
int32_t ads9_spi_xfer_ad9986(void *user_data, uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes);
int32_t ads9_spi_xfer_hmc7044(void *user_data, uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes);
int32_t ads9_spi_xfer_ad7175(void *user_data, uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes);
int32_t ads9_wait_us(void *user_data, uint32_t time_us);
int32_t ads9_hw_rst_pin_ctrl_ad9986(void *user_data, uint8_t pin_level);
int32_t ads9_user_data_create_ad9986(adi_ad9986_device_t *device, uint8_t spi_cs);
int32_t ads9_user_data_create_hmc7044(adi_hmc7044_device_t *device, uint8_t spi_cs);
int32_t ads9_user_data_create_ad7175(adi_ad7175_device_t *device, uint8_t spi_cs);
int32_t ads9_user_data_free(void **user_data);
int32_t ads9_axi_reg_read32(uint32_t reg_offset, uint32_t *out_data);
int32_t ads9_axi_reg_write32(uint32_t reg_offset, uint32_t data);
int32_t ads9_i2c_board_name_get(char * vendor, char* board_name, char* board_rev);

#ifdef __cplusplus
}
#endif
#endif /*__ADS9_H__*/
