/*!
 * @brief     ADS9 API Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_ADS9_API__
 * @{
 */
#ifndef __ADI_ADS9_H__
#define __ADI_ADS9_H__

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "adi_ad9986.h"

/*============= D E F I N E S ==============*/
typedef enum {
    START = 0,
    NEXT = 1,
    END = 2
} adi_ads9_data_xfer_e;

typedef struct {
    uint32_t address;     /* HMC Memory Address */
    uint32_t filetype;    /* 0x1: data vector, 0x2: FPGA image */
    uint32_t filesize;    /* The size of file for downloading */
    uint32_t filenameLen; /* The length of file name, only used for FPGA image; For data vector, keep it as zero */
    uint8_t filename[64]; /* The file name of FPGA image, only used for FPGA image */
}adi_ads9_xfer_meta_t;

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

int32_t adi_ads9_reg_set(uint32_t reg_addr, uint32_t data);
int32_t adi_ads9_reg_get(uint32_t reg_addr, uint32_t *out_data);

int32_t adi_ads9_rev_get(uint32_t *rev);
int32_t adi_ads9_ver_get(uint32_t *ver);
int32_t adi_ads9_sw_ver_get(uint32_t *sw_ver);

int32_t adi_ads9_config_jesd(adi_cms_jesd_param_t jrx_param[2], adi_cms_jesd_param_t *jtx_param);
int32_t adi_ads9_config_jesd_get(adi_cms_jesd_param_t *jrx_param, adi_cms_jesd_param_t *jtx_param);
int32_t adi_ads9_jesd_tx_lane_driver_config(uint8_t lanes, uint8_t post_cursor, uint8_t pre_cursor, uint32_t diff_ctrl);
int32_t adi_ads9_pattern_len_set(uint64_t len);
int32_t adi_ads9_pattern_addr_set(uint32_t addr);
int32_t adi_ads9_start_capture();
int32_t adi_ads9_stop_capture();
int32_t adi_ads9_capture_size_set(uint32_t size);
int32_t adi_ads9_start_bidir();
int32_t adi_ads9_stop_bidir();
int32_t adi_ads9_start_transmit();
int32_t adi_ads9_stop_transmit();
int32_t adi_ads9_skip_rx_link_init_set(uint8_t value);
int32_t adi_ads9_capture_status_get(uint32_t *capture_complete, uint32_t *capture_state);
int32_t adi_ads9_config_jtx_prbs(adi_cms_jesd_prbs_pattern_e prbs);
int32_t adi_ads9_jtx_err_status_get(uint8_t *error);
int32_t adi_ads9_jrx_err_status_get(uint8_t *error);

int32_t adi_ads9_ad9528_vcxo_select_set(uint8_t value);
int32_t adi_ads9_mgt_ref_clk_select_set(uint8_t value);
int32_t adi_ads9_gbl_clk_select_set(uint8_t value);
int32_t adi_ads9_spi_freq_adjust(uint64_t freq);

/**
 * \brief  Set Sysref select
 *
 * \param[in]  sysref_src   0:Use SYSREF through FMC, 1:Use AD9528-generated or SMA sysref
 *
 * \return API_CMS_ERROR_OK is returned upon success. Otherwise, a failure code.
 */
int32_t adi_ads9_sysref_config(uint8_t sysref_src);

/**
 * \brief Delay Rx Capture Enable
 * 
 * \param enable        Delayed Rx Enable flag
 * \param delay         Rx Delay Length in LEMC/LMFC Cycles
 * 
 * \return API_CMS_ERROR_OK is returned upon success. Otherwise, a failure code. 
 */
int32_t adi_ads9_bidir_sync_set(uint8_t enable, uint32_t delay);

int32_t adi_ads9_transfer_config(adi_ads9_xfer_meta_t *meta);
int32_t adi_ads9_read(uint8_t *data, int32_t data_size, adi_ads9_data_xfer_e *xfer_type);
int32_t adi_ads9_write(uint8_t *data, int32_t data_size, adi_ads9_data_xfer_e *xfer_type);

#ifdef __cplusplus
}
#endif

#endif /*__ADI_ADS9_H__*/
/*! @} */

