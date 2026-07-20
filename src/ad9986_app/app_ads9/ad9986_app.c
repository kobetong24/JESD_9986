/*!
 * @brief     Standalone Linux Application
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
#include "ad9986_app_helper.h"

/*============= M A C R O S====================*/

/*============= D A T A ====================*/
/*Eval Application Usecase Datapath Configuration*/
/*Refer to uc_settings.c for list of uc definitions*/
/*Clock Scheme Sources and Frequencies*/
extern uint64_t clk_hz[][4];
/* Transmit Datapath Configuration*/ 
extern uint8_t  tx_dac_chan_xbar[][4];  /*Transmit Channel and Main DAC Datapaths*/
extern int8_t   tx_chan_gain[][8];      /*Transmit Channel Gain DAC Datapaths*/
extern int64_t  tx_main_shift[][4];     /*Transmit Main/Coarse DUC NCO Frequency Setting*/
extern int64_t  tx_chan_shift[][8];     /*Transmit Channel/Fine DUC NCO Frequency Setting*/
extern uint8_t  tx_interp[][2];         /*Trnacmit Data Interpolation for Coarse/Fine DUCs */
extern uint8_t  rx_cddc_select[];
extern uint8_t  rx_fddc_select[];
extern int64_t  rx_cddc_shift[][4];
extern int64_t  rx_fddc_shift[][8];
extern uint8_t  rx_cddc_dcm[][4];
extern uint8_t  rx_fddc_dcm[][8];
extern uint8_t  rx_cddc_c2r[][4];
extern uint8_t  rx_fddc_c2r[8];

extern uint8_t  jtx_logiclane_mapping_pe_brd[2][8];
extern uint8_t  jtx_logiclane_mapping_ce_brd[2][8];

extern adi_ad9986_jtx_conv_sel_t jtx_conv_sel[][2];
extern adi_cms_jesd_param_t    jrx_param[];
extern adi_cms_jesd_param_t    jtx_param[][2];
extern uint8_t jtx_chip_dcm[][2];

/*============= C O D E ====================*/

int32_t main(int argc, char *argv[])
{
    int32_t  err, i, uc = -1, use_7044 = 0, use_ce_brd = 1;
    uint64_t hmc7044_crystal_input = 122.88e6;
    uint64_t app_jrx_lane_rate = 0, app_jtx_lane_rate[2] = {0};
    uint8_t k, j, en_ad7175 = 0;
    float line_rate_refclk_ratio = 0.0;

    /* determine use case */
    if (argc > 1) {
        uc = atoi(argv[1]);
    }
    /* determine if using 7044 or not */
    if (argc > 2) {
        use_7044 = atoi(argv[2]);
    }
    /* determine if using ce board or pe board */
    if (argc > 3) {
        use_ce_brd = atoi(argv[3]);
        if (use_ce_brd <= 0 && use_7044 > 0){
            printf("PE Board is not compatible with HMC7044 Clock.");
            return API_CMS_ERROR_OK;
        }
    }
    /* print use case info */
    if (uc >= 0) {
        printf("Running uc%d ", uc);
        printf(use_ce_brd > 0 ? "on CE board " : "on PE board ");
        printf(use_7044   > 0 ? "using hmc7044.\n" : "using direct clk.\n");
    }
    else if (uc < -1) {
        printf("Usage: ad9986_app [uc] [use_7044] [use_ce_brd]:\n");
        printf("   -1: show link status \n");
        printf("    0: uc0 (nco test) \n");
        printf("    1: uc1 (txmode = 194, rxmode = 392) \n");
        printf("    2: uc2 (txmode = 214, rxmode = 392) \n");
        printf("    ...");
        return API_CMS_ERROR_OK;
    }

    /* connect to platform */
    adi_ad9986_device_t ad9986_dev = {
        .hal_info = {
            .sdo = SPI_SDO,
            .msb = SPI_MSB_FIRST,
            .addr_inc = SPI_ADDR_INC_AUTO,
            .log_write = ads9_log_write,
            .delay_us = ads9_wait_us,
            .spi_xfer = ads9_spi_xfer_ad9986,
            .reset_pin_ctrl = ads9_hw_rst_pin_ctrl_ad9986,
        },
        .serdes_info = {
            .ser_settings = { /* ad9986 jtx */
                .lane_settings = {
                    {.swing_setting = AD9986_SER_SWING_850,.pre_emp_setting = AD9986_SER_PRE_EMP_0DB,.post_emp_setting = AD9986_SER_POST_EMP_0DB},
                    {.swing_setting = AD9986_SER_SWING_850,.pre_emp_setting = AD9986_SER_PRE_EMP_0DB,.post_emp_setting = AD9986_SER_POST_EMP_0DB},
                    {.swing_setting = AD9986_SER_SWING_850,.pre_emp_setting = AD9986_SER_PRE_EMP_0DB,.post_emp_setting = AD9986_SER_POST_EMP_0DB},
                    {.swing_setting = AD9986_SER_SWING_850,.pre_emp_setting = AD9986_SER_PRE_EMP_0DB,.post_emp_setting = AD9986_SER_POST_EMP_0DB},
                    {.swing_setting = AD9986_SER_SWING_850,.pre_emp_setting = AD9986_SER_PRE_EMP_0DB,.post_emp_setting = AD9986_SER_POST_EMP_0DB},
                    {.swing_setting = AD9986_SER_SWING_850,.pre_emp_setting = AD9986_SER_PRE_EMP_0DB,.post_emp_setting = AD9986_SER_POST_EMP_0DB},
                    {.swing_setting = AD9986_SER_SWING_850,.pre_emp_setting = AD9986_SER_PRE_EMP_0DB,.post_emp_setting = AD9986_SER_POST_EMP_0DB},
                    {.swing_setting = AD9986_SER_SWING_850,.pre_emp_setting = AD9986_SER_PRE_EMP_0DB,.post_emp_setting = AD9986_SER_POST_EMP_0DB},
                },
                .invert_mask = 0x00,
                .lane_mapping = { { 6, 4, 3, 2, 1, 0, 7, 5 }, { 2, 0, 7, 7, 7, 7, 3, 1 } }, /* link0, link1 */
            },
            .des_settings = { /* ad9986 jrx */
                .boost_mask = 0xff,
                .invert_mask = 0x00,
                .ctle_filter = { 2, 2, 2, 2, 2, 2, 2, 2 },
                .cal_mode = AD9986_CAL_MODE_RUN,
                .ctle_coeffs = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, /* CTLE 1-4 for lanes 0-7 */
                .lane_mapping = { { 0, 1, 2, 3, 4, 5, 6, 7 }, { 4, 5, 6, 7, 0, 1, 2, 3 } }, /* link0, link1 */
            }
        },
        .clk_info = {
            .sysref_mode = SYSREF_NONE,
        }
    };

    /* Change Lane Mapping for specified modes */
    {
        if (use_ce_brd <= 0){
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 8; j++){
                    ad9986_dev.serdes_info.ser_settings.lane_mapping[i][j] = jtx_logiclane_mapping_pe_brd[i][j];
                }
            }
        }
        uint8_t lb_lane_mapping[2][8] = { { 0, 1, 2, 3, 4, 5, 6, 7 }, { 4, 5, 6, 7, 0, 1, 2, 3 } };
        switch (uc) {
        case 13:
        case 26:
        case 27:
            /*Internal Loopback Modes*/
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 8; j++) {
                    ad9986_dev.serdes_info.ser_settings.lane_mapping[i][j] = lb_lane_mapping[i][j];
                }
            }
        default:
            break;

        }
    }

    adi_hmc7044_device_t hmc7044_dev = {
        .hal_info = {
            .spi_xfer = ads9_spi_xfer_hmc7044,
            .delay_us = ads9_wait_us
        }
    };

    adi_ad7175_device_t ad7175_dev = {
        .hal_info = {
            .spi_xfer = ads9_spi_xfer_ad7175,
            .delay_us = ads9_wait_us
        }
    };

    { /* setup platform */
        /* open platform */
        if (err = ads9_hw_open("ad9986_app.log"), err != API_CMS_ERROR_OK)
            return err;

        /* create user data */
        if (err = ads9_user_data_create_ad9986(&ad9986_dev, 0), err != API_CMS_ERROR_OK)
            return err;
        if (err = ads9_user_data_create_hmc7044(&hmc7044_dev, 1), err != API_CMS_ERROR_OK)
            return err;
        if (err = ads9_user_data_create_ad7175(&ad7175_dev, 0), err != API_CMS_ERROR_OK)
            return err;

        /* show link status only */
        if (uc == -1) {
            if (err = app_show_link_status(&ad9986_dev), err != API_CMS_ERROR_OK)
                return err;
            printf("\n");
            return API_CMS_ERROR_OK;
        }

        /* show connected device board info */
        char vendor[16], board[64], board_rev[16];
        if (err = ads9_i2c_board_name_get(vendor, board, board_rev), err == API_CMS_ERROR_OK) {
            if (strcmp(vendor, "Analog Devices") == 0) {
                printf("Found connected device board: %s Rev%s\n", board, board_rev);
                if ((strncmp(board, "AD9081-FMCA-EBZ-A", 17) == 0) ||
                    (strncmp(board, "AD9082-FMCA-EBZ-A", 17) == 0) ||
                    (strncmp(board, "AD9177-FMCA-EBZ", 15) == 0) ||
                    (strncmp(board, "AD9207-FMCA-EBZ", 15) == 0) ||
                    (strncmp(board, "AD9209-FMCA-EBZ", 15) == 0))
                {
                    hmc7044_crystal_input = 100e6;
                }
                if ((strncmp(board_rev, ":D", 2) == 0) &&
                    ((strncmp(board, "AD9986-FMCB-EBZ", 15) == 0) || (strncmp(board, "AD9988-FMCB-EBZ", 15) == 0)))
                {
                    en_ad7175 = 1;
                    printf("APP: AD7175 Power Monitoring ADC Connected\n");
                }
            }
        }
        printf("On board crystal select: %6.2fMHz\n", (hmc7044_crystal_input / 1.0e6));

        /* show api version */
        uint8_t ad9986_rev[3];
        if (err = adi_ad9986_device_api_revision_get(&ad9986_dev, &ad9986_rev[0], &ad9986_rev[1], &ad9986_rev[2]), err != API_CMS_ERROR_OK)
            return err;
        printf("AD9986 API v%d.%d.%d\n", ad9986_rev[0], ad9986_rev[1], ad9986_rev[2]);

        /* show fpga version */
        uint32_t fpga_ver, fpga_sw_ver;
        if (err = adi_ads9_ver_get(&fpga_ver), err != API_CMS_ERROR_OK)
            return err;
        if (err = adi_ads9_sw_ver_get(&fpga_sw_ver), err != API_CMS_ERROR_OK)
            return err;
        printf("FPGA Image v%x sv%x\n", fpga_ver, fpga_sw_ver);
    }
    printf("APP: Configure Usecase:%d, Tx Path: DAC Clk: %lld, JESD Rx Mode: %d & Rx Path: ADC CLK: %lld, Jesdmode Tx Mode: %d \r\n",
        uc, clk_hz[uc][2], jrx_param[uc].jesd_mode_id, clk_hz[uc][3], jtx_param[uc][0].jesd_mode_id);
    {/* Do Hard Rest of Devices*/
        adi_ad9986_device_reset(&ad9986_dev, AD9986_HARD_RESET); 
    }
    {
#if !defined(AD9207_ID) && !defined(AD9209_ID)	
        app_calc_tx_lane_rate(clk_hz[uc], &jrx_param[uc], tx_interp[uc], &app_jrx_lane_rate);
#endif
#if !defined(AD9177_ID)
        app_calc_rx_lane_rate(clk_hz[uc], jtx_param[uc], jtx_chip_dcm[uc], app_jtx_lane_rate);
#endif
        printf("APP: Tx Lane Rate : %llu Rx0 Lane Rate : %llu Rx1 Lane Rate : %llu \r\n", app_jrx_lane_rate, app_jtx_lane_rate[0], app_jtx_lane_rate[1]); 
        
        /* setup reference clock */
        printf("APP: Configure Platform Reference Clocks\n");
        if (use_7044 == 0) { /* set clock path to use external reference */
            if (err = adi_ads9_ad9528_vcxo_select_set(0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ads9_mgt_ref_clk_select_set(0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ads9_gbl_clk_select_set(0), err != API_CMS_ERROR_OK)
                return err;
        }
        else { /* configure 7044 to generate clock */
            uint64_t ad9986_clk = clk_hz[uc][0], fpga_clk = clk_hz[uc][1];
            uint64_t sysref_clk_204b = fpga_clk/16, sysref_clk_204c = fpga_clk/32;
            /*Configure FPGA to Use FMC CLKs*/
            if (err = adi_ads9_mgt_ref_clk_select_set(1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ads9_gbl_clk_select_set(0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ads9_sysref_config(0), err != API_CMS_ERROR_OK)
                return err; 

#if !defined(AD9207_ID) && !defined(AD9209_ID) 
            /* Configure HMC7044 SYSREF frequency output channels for SC1 Use Case */
            switch (uc){
            case 21:
            case 26:
            case 27:
            case 40:
                ad9986_dev.clk_info.sysref_mode = SYSREF_CONT;
                ad9986_dev.clk_info.sysref_clk = &hmc7044_dev;
                ad9986_dev.clk_info.sysref_ctrl = app_sysref_clk_src_sel;
                /* Calculate SYSREF Freq for SC1 Use cases*/
                if ((jrx_param[uc].jesd_jesdv == 2) || (jtx_param[uc][0].jesd_jesdv == 2)) {
                    if (err = adi_ad9986_sync_sysref_frequency_set(&ad9986_dev, &sysref_clk_204c, ad9986_clk, clk_hz[uc][2], clk_hz[uc][3], tx_interp[uc][0], tx_interp[uc][1], rx_cddc_dcm[uc], rx_fddc_dcm[uc], jtx_param[uc][0].jesd_duallink, &jrx_param[uc], jtx_param[uc]), err != API_CMS_ERROR_OK)
                        return err;
                    printf("APP: SYSREF Clk: %lld\n", sysref_clk_204c);
                }
                else {
                    if (err = adi_ad9986_sync_sysref_frequency_set(&ad9986_dev, &sysref_clk_204b, ad9986_clk, clk_hz[uc][2], clk_hz[uc][3], tx_interp[uc][0], tx_interp[uc][1], rx_cddc_dcm[uc], rx_fddc_dcm[uc], jtx_param[uc][0].jesd_duallink, &jrx_param[uc], jtx_param[uc]), err != API_CMS_ERROR_OK)
                        return err;
                    printf("APP: SYSREF Clk: %lld\n", sysref_clk_204b);
                }
                break;
            default:
                ad9986_dev.clk_info.sysref_clk = &hmc7044_dev;
                ad9986_dev.clk_info.sysref_ctrl = app_sysref_clk_src_sel;
                break;
            }
#endif
            
            /*Configure HMC7044 to Provide Clocks For Txfe & FPGA*/
            uint8_t  hmc_priority[] = { 1, 0, 2, 3 };
            uint16_t hmc_out_ch = HMC7044_OP_CH_0 | HMC7044_OP_CH_2 | HMC7044_OP_CH_3 | HMC7044_OP_CH_6 | HMC7044_OP_CH_8 | HMC7044_OP_CH_10 | HMC7044_OP_CH_12 | HMC7044_OP_CH_13;
            uint64_t hmc_out_204b[14] = { (app_jtx_lane_rate[0]) ? (app_jtx_lane_rate[0]/40) : (fpga_clk / 2) , 0, ad9986_clk, sysref_clk_204b, 0, 0, fpga_clk / 2,
                                          0, fpga_clk, 0, (app_jtx_lane_rate[1]) ? (app_jtx_lane_rate[1]/40) : (fpga_clk / 2), 0, fpga_clk, sysref_clk_204b };
            uint64_t hmc_out_204c[14] = { fpga_clk,   0, ad9986_clk, sysref_clk_204c, 0, 0, fpga_clk,   0, fpga_clk, 0, fpga_clk,   0, fpga_clk,  sysref_clk_204c };

            /* Disable SYSREF signal channels for Subclass 0 */
            if (((jrx_param[uc].jesd_subclass == 0) && (jtx_param[uc][0].jesd_subclass == 0))) {
                hmc_out_ch &= ~(HMC7044_OP_CH_3 | HMC7044_OP_CH_13);
                if (((jrx_param[uc].jesd_jesdv == 2) || (jtx_param[uc][0].jesd_jesdv == 2))) {
                    hmc_out_204c[3] = 0;
                    hmc_out_204c[13] = 0;
                }
                else {
                    hmc_out_204b[3] = 0;
                    hmc_out_204b[13] = 0;
                }
            }
            if (((jrx_param[uc].jesd_jesdv == 2) || (jtx_param[uc][0].jesd_jesdv == 2)) && (fpga_clk * 66 < 8.3e9)) { /* double fpga refclk for 204C lane rate < 8.3Gbps */
                hmc_out_204c[8] *= 2;
                hmc_out_204c[12] *= 2;
            }
            if (err = adi_hmc7044_device_init(&hmc7044_dev), err != API_CMS_ERROR_OK)
               return err;
           if (err = adi_hmc7044_device_reset(&hmc7044_dev, 0), err != API_CMS_ERROR_OK)
                return err;
           /*Disable Configure but Output Drivers Prior to Clocking Scheme Configuration*/

           for (i = 0; i< HMC7044_NOF_OP_CH; i++) {
              if (err = adi_hmc7044_output_config_set(&hmc7044_dev, i, HMC7044_OP_SIG_CH_DIV, 0, 0, 0), err != API_CMS_ERROR_OK)
                  return err;
            }
            if (err = adi_hmc7044_input_reference_set(&hmc7044_dev, 0, IPBUFFER_INTERNAL_100_OHM_EN | IPBUFFER_AC_COUPLED_MODE_EN, 1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_hmc7044_input_reference_set(&hmc7044_dev, 1, IPBUFFER_INTERNAL_100_OHM_EN | IPBUFFER_AC_COUPLED_MODE_EN, 1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_hmc7044_input_reference_los_config_set(&hmc7044_dev, 7, 0, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_hmc7044_vco_sel_set(&hmc7044_dev, HMC7044_VCO_INTERNAL_3GHZ, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_hmc7044_output_sync_config_set(&hmc7044_dev, 3, ((ad9986_dev.clk_info.sysref_mode == SYSREF_ONESHOT) ? 1 : 0), 1, 1), err != API_CMS_ERROR_OK) /* clkout3 as async mode or pulse gen mode*/
                return err;
            if (err = adi_hmc7044_output_sync_config_set(&hmc7044_dev, 13, ((ad9986_dev.clk_info.sysref_mode == SYSREF_ONESHOT) ? 1 : 0), 1, 1), err != API_CMS_ERROR_OK) /* clkout13 as async mode or pulse gen mode*/
                return err;
            if (err = adi_hmc7044_output_multi_slip_config_set(&hmc7044_dev, 3, 0, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_hmc7044_output_multi_slip_config_set(&hmc7044_dev, 13, 0, 0), err != API_CMS_ERROR_OK)
                return err;
        
            /* Configure number of pulses generated from HMC7044 SYSREF - default set to continuous pulses */
            if (err = adi_hmc7044_sysref_config_set(&hmc7044_dev, HMC7044_SYSREF_CONTINUOUS_MODE), err != API_CMS_ERROR_OK)
                return err;   
        
            if (err = adi_hmc7044_clk_config(&hmc7044_dev, HMC7044_CLK_IN_0, hmc_priority, hmc7044_crystal_input, hmc7044_crystal_input, hmc_out_ch,
                ((jrx_param[uc].jesd_jesdv == 2) || (jtx_param[uc][0].jesd_jesdv == 2)) ? (hmc_out_204c) : (hmc_out_204b)),
                err != API_CMS_ERROR_OK) {
                if (err == API_CMS_ERROR_INVALID_PARAM)
                    printf("APP: HMC7044: Invalid param passed.\n");
                return err;
            }
            
            if (err = adi_hmc7044_device_sysref_enable_control_set(&hmc7044_dev, 1, 1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_hmc7044_high_performance_set(&hmc7044_dev), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_hmc7044_reg_update(&hmc7044_dev), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_hmc7044_reseed_request_set(&hmc7044_dev), err != API_CMS_ERROR_OK)
                return err;
            if (err = ads9_wait_us(NULL, 100000), err != API_CMS_ERROR_OK)
                return err;
            uint8_t  hmc_pll_locked = 0;
            if (err = adi_hmc7044_device_pll_lock_status_get(&hmc7044_dev, &hmc_pll_locked), err != API_CMS_ERROR_OK) {
                printf("APP: HMC7044: PLL is not locked.\n");
                return err;
            }
        }
        printf("APP: Reference Clocks Configured\n");
    }
    /* ADS9 FPGA  JESD Interface and Data & Playback Configuration Sequenece
    *  Do FPGA JESD Rx and Tx Interface Parameter Configuration
    *       adi_ads9_config_jesd
    *  DO FPGA RAM Assignment for Playback of Vector  Data and Captured Data
    *       adi_ads9_capture_size_set
    *       adi_ads9_pattern_addr_set
    *       adi_ads9_pattern_len_set
    * DO  FPGA Clock Configuration
    *
    *
    */
    if (uc > 0) { /* setup fpga jesd params */
        printf("APP: Configure FPGA JESD Interfaces\n");
        if (err = adi_ads9_config_jesd(jtx_param[uc], &jrx_param[uc]), err != API_CMS_ERROR_OK)
            return err;
        uint32_t sysref_always_align = (((jrx_param[uc].jesd_subclass == 1) || (jtx_param[uc][0].jesd_subclass == 1)) && (ad9986_dev.clk_info.sysref_mode == SYSREF_CONT)) ? 0x0001 : 0x0000;
        if (adi_ads9_reg_set(0x602, sysref_always_align), err != API_CMS_ERROR_OK) /* jesd204c_tx_sysref_always */
            return err;
        if (adi_ads9_reg_set(0x202, sysref_always_align), err != API_CMS_ERROR_OK) /* jesd204c_rx_sysref_always */
            return err;
        if (err = adi_ads9_capture_size_set(0x20), err != API_CMS_ERROR_OK)
            return err;
        if (err = adi_ads9_pattern_addr_set(0x80000000), err != API_CMS_ERROR_OK)
            return err;
        if (err = adi_ads9_pattern_len_set(0x8000), err != API_CMS_ERROR_OK)
            return err;
        uint32_t ads9_link_clk_div = (((jrx_param[uc].jesd_jesdv == 2) || (jtx_param[uc][0].jesd_jesdv == 2)) && (clk_hz[uc][1] * 66 < 8.3e9)) ? 0x101 : 0x000;
#if !defined(AD9177_ID)
        if (jtx_param[uc][0].jesd_jesdv == 1) {
            for (k = 0, j = 0; k < (jtx_param[uc][0].jesd_duallink + 1) ; k++, j+=8) {
                line_rate_refclk_ratio = app_jtx_lane_rate[k] / clk_hz[uc][1] ;
                if (line_rate_refclk_ratio == 20) {
                    ads9_link_clk_div &= ~(1 << j);   
                }
                else if (line_rate_refclk_ratio == 10) {
                    ads9_link_clk_div |= (1 << j); 
                }
                else if (line_rate_refclk_ratio == 5) {
                    ads9_link_clk_div |= (2 << j); 
                }
                else if (line_rate_refclk_ratio == 2.5) {
                    ads9_link_clk_div |= (3 << j);
                } 
            }
        }
#endif
        if (err = adi_ads9_reg_set(0x10c, ads9_link_clk_div), err != API_CMS_ERROR_OK) /* line_rate_rx_ctrl */
            return err;
#if !defined(AD9207_ID) && !defined(AD9209_ID)
        if (jrx_param[uc].jesd_jesdv == 1) {
            ads9_link_clk_div = 0;
            for (k = 0, j = 0; k < (jrx_param[uc].jesd_duallink + 1) ; k++, j+=8 ) {
                line_rate_refclk_ratio = app_jrx_lane_rate / clk_hz[uc][1] ;
                if (line_rate_refclk_ratio == 20) {
                    ads9_link_clk_div &= ~(1 << j);   
                }
                else if (line_rate_refclk_ratio == 10) {
                    ads9_link_clk_div |= (1 << j); 
                }
                else if (line_rate_refclk_ratio == 5) {
                    ads9_link_clk_div |= (2 << j); 
                }
                else if (line_rate_refclk_ratio == 2.5) {
                    ads9_link_clk_div |= (3 << j);
                } 
            }
        }
#endif
        if (err = adi_ads9_reg_set(0x50c, ads9_link_clk_div), err != API_CMS_ERROR_OK) /* line_rate_tx_ctrl */
            return err;
        if (err = adi_ads9_reg_set(0x540, 1), err != API_CMS_ERROR_OK) /* transmit_skip_data = 1 */
            return err;
        if (err = adi_ads9_reg_set(0x106, 0x000), err != API_CMS_ERROR_OK) /* skip_rx_link_init = 0 */
            return err;
        if (err = adi_ads9_reg_set(0x947, 2), err != API_CMS_ERROR_OK) /* bidir_start = 1 */
            return err;
        if (ad9986_dev.clk_info.sysref_mode == SYSREF_ONESHOT) {
            if (err = adi_hmc7044_pulse_gen_set(&hmc7044_dev), err != API_CMS_ERROR_OK)
                return err;
        }
        if (err = adi_ads9_reg_set(0x106, 0x400), err != API_CMS_ERROR_OK) /* skip_rx_link_init = 1 */
            return err;
        switch(uc) {
            case 40: /* Set capture gating sync feature to only run Capture when Transmit is playing */
                if (adi_ads9_bidir_sync_set(1, 0), err != API_CMS_ERROR_OK)
                    return err;
                break;
            default:
                break;
        }
        if ((jrx_param[uc].jesd_l > 0) && (jrx_param[uc].jesd_jesdv == 2) && ((clk_hz[uc][1] * 66) > AD9986_JESDRX_204C_CAL_THRESH)) {
            if (adi_ads9_jesd_tx_lane_driver_config(0xFF, 0x0, 0x0, 0x14), err != API_CMS_ERROR_OK) {
                return err;
            }
        }
        else {
            if (adi_ads9_jesd_tx_lane_driver_config(0xFF, 0x4, 0x4, 0x1C), err != API_CMS_ERROR_OK) {
                return err;
            }
        }
        printf("APP: FPGA JESD Interfaces Configured\n");
    }

    /* AD7175 Power ADC Configuration Sequence
    *  Do Channel Configuration for Analog Inputs
    *  Choose Setup Configuration
    * 
    */
    if (en_ad7175) {
        printf("APP: Configure AD7175 Power Monitoring ADC\n");
        uint32_t id = 0;
        if (err = adi_ad7175_device_id_get(&ad7175_dev, &id), err != API_CMS_ERROR_OK)
            return err;
        
        if (id>>4 != 0x3CD){
            en_ad7175 = 0;
            printf("APP: AD7175 SPI ERROR: ID register should be 0x3CDx, value is 0x%.4x\n", id);
        }
        /* Configure all the channels with appropriate analog inputs */
        adi_ad7175_ain_e ad7175_ains[16] = {AD7175_AIN0, AD7175_AIN1, AD7175_AIN2, AD7175_AIN3, AD7175_AIN4, AD7175_AIN5, AD7175_AIN6, AD7175_AIN7, AD7175_AIN8, 
                                            AD7175_AIN9, AD7175_AIN10, AD7175_AIN11, AD7175_AIN12, AD7175_AIN13, AD7175_AIN14, AD7175_AIN15};
        for (i=0; i<8; i++){
            if (err = adi_ad7175_device_channel_config_set(&ad7175_dev, i, 1, AD7175_SETUP0, ad7175_ains[i*2], ad7175_ains[i*2+1]), err != API_CMS_ERROR_OK)
                return err;
        }
        /* Choose setup configuration, default is SETUP0*/
        if (err = adi_ad7175_device_setup_config_set(&ad7175_dev, AD7175_SETUP0, AD7175_SETUP_REF_INT, (AD7175_SETUP_AINBUF_P | AD7175_SETUP_AINBUF_N)), err != API_CMS_ERROR_OK)
            return err;
        if (err = adi_ad7175_device_intf_config_set(&ad7175_dev, (AD7175_INTF_DATA_STAT | AD7175_INTF_WL16)), err != API_CMS_ERROR_OK)
            return err;
        printf("APP: AD7175 Configured\n");
    }

    /* AD9986 Device Data Path Configuration Sequenece
    *  Do AD9986 Device RESET, adi_ad9986_device_reset
    *  Do AD9986 Device Initialization: adi_ad9986_device_init
    *  Do AD9986 Device Clocks Config (REF CLK, DAC CLK, ADC Clock):adi_ad9986_device_clk_config_set
    *  Do AD9986 Device TX Datapath (JESD RX to DAC) Primary Configuration: adi_ad9986_device_startup_tx
    *  Do AD9986 Device Tx Datapath Secondary Configuration as per specific UC requirements
    *       eg  TX Gain APIs
    *                     - adi_ad9986_dac_duc_nco_gains_set
    *           TX Datapath Customization API:
    *                     -adi_ad9986_dac_modulation_mux_mode_set
    *                     -adi_ad9986_dac_xbar_set
    *           TX Test Modes APIs:
    *                     -adi_ad9986_device_startup_nco_test_mode
    * Do AD9986 Device RX Datapath (ADC to JESD TX) Primary Configuration: adi_ad9986_device_startup_rx
    * Do AD9986 Device Rx Datapath Secondary Configuration as per specific UC requirements
    *            eg: RX Gain APIs:
    *                   - adi_ad9986_adc_ddc_fine_gain_set
    *              RX Datapath Customization APIs:
    *                   - adi_ad9986_adc_nyquist_zone_set
    *                   - adi_ad9986_adc_xbar_set
    *              RX Test Mode APIs:
    *                   - adi_ad9986_jesd_loopback_mode_set
    *                   - adi_ad9986_adc_ddc_coarse_nco_mode_set
    *
    */
    {
        uint8_t adc_cddc_xbar, cddc_fddc_xbar;
        uint16_t phase = 0;
        printf("APP: Configure ad9986 Device\n");

        /* reset ad9986 */
        if (err = adi_ad9986_device_reset(&ad9986_dev, AD9986_SOFT_RESET), err != API_CMS_ERROR_OK) {
            printf("APP: ad9986 Initialisation Error\n");
            return err;
        }

        /* init ad9986 */
        if (err = adi_ad9986_device_init(&ad9986_dev), err != API_CMS_ERROR_OK) {
            printf("APP: ad9986 Initialisation Error\n");
            return err;
        }

        /* setup ad9986 clock */
        err = adi_ad9986_device_clk_config_set(&ad9986_dev, clk_hz[uc][2], clk_hz[uc][3], clk_hz[uc][0]);
        uint8_t ad9986_pll_locked = 0;
        adi_ad9986_device_clk_pll_lock_status_get(&ad9986_dev, &ad9986_pll_locked);
        if (ad9986_pll_locked == 0x3) {
            printf("APP: ad9986 PLL LOCKED\n");
        }
        if (err != API_CMS_ERROR_OK) {
           printf("APP: Clock Configuration error\n");
           return err;
        }   


#if !defined(AD9207_ID) && !defined(AD9209_ID) 
        /* nco test case */
        if (uc == 0) {
            if (err = adi_ad9986_device_startup_nco_test_mode(&ad9986_dev, tx_interp[uc][0], tx_interp[uc][1], tx_dac_chan_xbar[uc],
                tx_main_shift[uc], tx_chan_shift[uc], &jrx_param[uc], (uint16_t)pow(10, ((0 + 20 * log10(0x5a82)) / 20))), err != API_CMS_ERROR_OK)
                return err;
        }
#endif



#if !defined(AD9207_ID) && !defined(AD9209_ID) 
        switch (uc) {
        case 0: /* NCO Only mode */
            break;
        case 18:
        case 22:
        case 29:
        case 31:
        case 33:
        case 34:
        case 39:
            /* Power Down DAC blocks */
            if (err = adi_ad9986_dac_power_up_set(&ad9986_dev, AD9986_DAC_ALL, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_dac_dll_power_down(&ad9986_dev, AD9986_DAC_ALL), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_jesd_rx_power_down_des(&ad9986_dev), err != API_CMS_ERROR_OK)
                return err;
            break;  /* Rx Only Cases */
        default:
            /* start ad9986 tx */
            if (err = adi_ad9986_device_startup_tx(&ad9986_dev, tx_interp[uc][0], tx_interp[uc][1], tx_dac_chan_xbar[uc],
                tx_main_shift[uc], tx_chan_shift[uc], &jrx_param[uc]), err != API_CMS_ERROR_OK) {
                printf("APP: ad9986 Tx  Path Configuration Error \n");
                if (err == API_CMS_ERROR_JESD_PLL_NOT_LOCKED) {
                    printf("APP: ad9986 Tx  Path Configuration JESD PLL Not Locked \n");
                }
                return err;
            }
            /* Setup ad9986 tx channel gain */
            uint16_t tx_chan_gains[8];
            for (i = 0; i < 8; i++)
                tx_chan_gains[i] = (uint16_t)(pow(2, 11) * pow(10, (tx_chan_gain[uc][i]) / 20.0));
            if (err = adi_ad9986_dac_duc_nco_gains_set(&ad9986_dev, tx_chan_gains), err != API_CMS_ERROR_OK)
                return err;
        }


        /* Application/Usecase Specific Customization if Required*/
        switch (uc) {
        case 19:
        case 20:
            /*Select Main CDUC0/1->DAC0, CDUC2/3->DAC2 */
            if (err = adi_ad9986_dac_modulation_mux_mode_set(&ad9986_dev, AD9986_DAC_PAIR_ALL, AD9986_DAC_MUX_MODE_3), err != API_CMS_ERROR_OK)
                return err;
            break;
        case 32:
            /*Route second IQ pair from virtual converters to DAC2
            * Select Modulation Mux Mode to DAC0 => I0, DAC1 =>Q0
            */
            if (err = adi_ad9986_dac_xbar_set(&ad9986_dev, AD9986_DAC_2, AD9986_DAC_1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_dac_modulation_mux_mode_set(&ad9986_dev, AD9986_DAC_PAIR_ALL, AD9986_DAC_MUX_MODE_2), err != API_CMS_ERROR_OK)
                return err;
            break;
        default:
            /*Use Default Muxing as per SDUG*/
            break;
        }

#endif

        /*Configure Primary Rx Datapath Settings*/
        switch (uc) {
        case 0:
        case 17:
        case 28:
            /*DO NOT Configure as these are TX Only Modes*/
            break;
        default:
            if (err = adi_ad9986_device_startup_rx(&ad9986_dev, rx_cddc_select[uc], rx_fddc_select[uc], rx_cddc_shift[uc], rx_fddc_shift[uc],
                rx_cddc_dcm[uc], rx_fddc_dcm[uc], rx_cddc_c2r[uc], rx_fddc_c2r, jtx_param[uc], jtx_conv_sel[uc]), err != API_CMS_ERROR_OK) {
                printf("APP: ad9986 Rx Path Configuration Error \n");
                return err;
            }
        }

        /*Enable Internal Loopback Mode*/
        switch (uc) {
        case 13:
        case 26:
        case 27:
            printf("APP: This is being configured as a loopback usecase \n");
            if (err = adi_ad9986_jesd_loopback_mode_set(&ad9986_dev, 1), err != API_CMS_ERROR_OK)
                return err;
            /* power down unused PHY */
            if (err = adi_ad9986_jesd_tx_power_down_ser(&ad9986_dev), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_jesd_rx_power_down_des(&ad9986_dev), err != API_CMS_ERROR_OK)
                return err;
        default:
            break;

        }
        /* Set gain settings as per Application Usecase */
        switch (uc) {
        case 26:
        case 27:
            if (err = adi_ad9986_adc_ddc_fine_gain_set(&ad9986_dev, rx_fddc_select[uc], 1), err != API_CMS_ERROR_OK)
                return err;
            break;
        default:
            break;
        }
        /*Override NCO Mode*/
        switch (uc) {
        case 15:
            if (err = adi_ad9986_adc_ddc_coarse_nco_mode_set(&ad9986_dev, rx_cddc_select[uc], AD9986_ADC_NCO_ZIF), err != API_CMS_ERROR_OK)
                return err;
            break;
        case 26:
        case 27:
            /*Enable NCO TEST Mode*/
            if (err = adi_ad9986_adc_ddc_coarse_nco_mode_set(&ad9986_dev, rx_cddc_select[uc], AD9986_ADC_NCO_TEST), err != API_CMS_ERROR_OK)
                return err;
            break;
        default:
            break;
        }

        /* Override/Customize Default Xbar Settings*/
        switch (uc) {
        case 19:
        case 20:
            adc_cddc_xbar = AD9986_ADC_2_ADC_REAL_MODE;   /* ADC0->CDDC0/2, ADC1-> CDDC1/3 */
            cddc_fddc_xbar = AD9986_ADC_CDDC0_TO_FDDC0 | AD9986_ADC_CDDC0_TO_FDDC1 | AD9986_ADC_CDDC1_TO_FDDC2 | AD9986_ADC_CDDC1_TO_FDDC3;
            cddc_fddc_xbar |= AD9986_ADC_CDDC2_TO_FDDC4 | AD9986_ADC_CDDC2_TO_FDDC5 | AD9986_ADC_CDDC3_TO_FDDC6 | AD9986_ADC_CDDC3_TO_FDDC7;
            if (err = adi_ad9986_adc_xbar_set(&ad9986_dev, adc_cddc_xbar, cddc_fddc_xbar), err != API_CMS_ERROR_OK)
                return err;
            break;
        default:
            break;
        }
    
        /* Set ADC Nyquist Zone as per Application Usecase for ADC optimal Background Cal Operation*/
        switch (uc) {
        case 26:
        case 27:/*ADC Test Test Mode*/
            if (err = adi_ad9986_adc_nyquist_zone_set(&ad9986_dev, AD9986_ADC_ALL, AD9986_ADC_NYQUIST_ZONE_EVEN), err != API_CMS_ERROR_OK)
                return err;
            break;
        default:
            break;
        }
#if !defined(AD9177_ID) 
        /* Configure PFILT */
        int16_t pfir_coeffs[192] = {-32,-32,-7,31,31,31,31,31,-32,-32,-32,31,31,31,-12,31,31,-32,-32,-32,31,31,31,31,
            1770,1297,-2047,-2048,-1503,1814,1522,-116,1647,2047,1789,-2048,-7400,-1458,3267,676,-639,7720,14648,2089,-23120,-29208,-1139,32767,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            32767,-1139,-29208,-23120,2089,14648,7720,-639,676,3267,-1458,-7400,-2048,1789,2047,1647,-116,1522,1814,-1503,
            -2048,-2047,1297,1770,31,31,31,31,-32,-32,-32,31,31,-12,31,31,31,-32,-32,-32,31,31,31,31,31,-7,-32,-32,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        adi_ad9986_adc_pfir_gain_t pfir_gain = {
            .ix_gain = AD9986_ADC_PFIR_GAIN_0DB, 
            .iy_gain = AD9986_ADC_PFIR_GAIN_0DB, 
            .qx_gain = AD9986_ADC_PFIR_GAIN_0DB,
            .qy_gain = AD9986_ADC_PFIR_GAIN_0DB}; 
        switch(uc) {
        case 41:
            /* 2949.12 sampling rate, BPF for 24-44% of NZ1*/
            if (err = adi_ad9986_adc_pfir_coeff_table_load_set(&ad9986_dev, AD9986_ADC_PFIR_ADC_PAIR_ALL, AD9986_ADC_PFIR_COEFF_PAGE0, AD9986_ADC_PFIR_I_MODE_REAL_N2, AD9986_ADC_PFIR_Q_MODE_REAL_N2,
                &pfir_gain, pfir_coeffs, 1), err != API_CMS_ERROR_OK)
                return err;
            break;
        default:
            break;
        }
#endif
        /* Configure Synchronization Options as per Application Use-case*/
        /* By Default Application uses Subclass 0 and Internal Sysref Synchronization*/
        /* Note 21/26/27 Are examples of Subclass 1*/
        switch (uc) {
        case 0: /*NCO Test Mode- No JESD*/
            if (err = adi_ad9986_jesd_oneshot_sync(&ad9986_dev,  0), err != API_CMS_ERROR_OK){
                if (err == API_CMS_ERROR_JESD_SYNC_NOT_DONE) {
                    printf("APP: JESD Oneshot Synchronization Not Completed");
                }
                return err;
            }
            break;
        case 21: /* Subclass 1 Use case - default set to continuous pulse */
        case 40:
            printf("APP: JESD RX Synchronization Mode: %s\n", ((jrx_param[uc].jesd_subclass == JESD_SUBCLASS_1) ? "JESD_SUBCLASS_1" : "JESD_SUBCLASS_0"));
            /* Configure Sysref Receiver and Input mode */
            if (err = adi_ad9986_sync_sysref_input_config_set(&ad9986_dev, COUPLING_AC, SIGNAL_CML, 0, 0), err != API_CMS_ERROR_OK) 
                return err;
            /* Configure cddc nco sync */
            if (err = adi_ad9986_adc_ddc_coarse_sync_enable_set(&ad9986_dev, AD9986_ADC_CDDC_ALL, 1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_ddc_coarse_sync_next_set(&ad9986_dev, AD9986_ADC_CDDC_ALL, 1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_ddc_coarse_trig_nco_reset_enable_set(&ad9986_dev, AD9986_ADC_CDDC_ALL, 0), err != API_CMS_ERROR_OK)
                return err;
            /* Perform oneshot sync */
            if (err = adi_ad9986_jesd_oneshot_sync(&ad9986_dev,  1), err != API_CMS_ERROR_OK){
                if (err == API_CMS_ERROR_JESD_SYNC_NOT_DONE) {
                    printf("APP: JESD Oneshot Synchronization Not Completed\n");
                }
                return err;
            }
            if (err = adi_ad9986_jesd_sysref_monitor_phase_get(&ad9986_dev, &phase), err != API_CMS_ERROR_OK)
                return err;
            printf("APP: Phase offset between incoming SYSREF and internal LMFC/LEMC: %d DAC clock units\n", phase);

            break;
        case 26: /*ADC Test Test Mode*/
            printf("APP: JESD RX Synchronization Mode: %s\n", ((jrx_param[uc].jesd_subclass == JESD_SUBCLASS_1) ? "JESD_SUBCLASS_1" : "JESD_SUBCLASS_0"));
            /* Configure Sysref Receiver and Input mode */
            if (err = adi_ad9986_sync_sysref_input_config_set(&ad9986_dev, COUPLING_AC, SIGNAL_CML, 0, 0), err != API_CMS_ERROR_OK) 
                return err;
            /* Perform oneshot sync */
            if (err = adi_ad9986_jesd_oneshot_sync(&ad9986_dev,  1), err != API_CMS_ERROR_OK){
                if (err == API_CMS_ERROR_JESD_SYNC_NOT_DONE) {
                    printf("APP: JESD Oneshot Synchronization Not Completed\n");
                }
                return err;
            }
            if (err = adi_ad9986_adc_nco_master_slave_sync(&ad9986_dev, 1, 2, 0, 1), err != API_CMS_ERROR_OK) /*mcs: main*/
                return err;
            break;
        case 27: /*ADC Test Test Mode*/
            printf("APP: JESD RX Synchronization Mode: %s\n", ((jrx_param[uc].jesd_subclass == JESD_SUBCLASS_1) ? "JESD_SUBCLASS_1" : "JESD_SUBCLASS_0"));
            /* Configure Sysref Receiver and Input mode */
            if (err = adi_ad9986_sync_sysref_input_config_set(&ad9986_dev, COUPLING_AC, SIGNAL_CML, 0, 0), err != API_CMS_ERROR_OK) 
                return err;
            /* Perform oneshot sync */
            if (err = adi_ad9986_jesd_oneshot_sync(&ad9986_dev,  1), err != API_CMS_ERROR_OK){
                if (err == API_CMS_ERROR_JESD_SYNC_NOT_DONE) {
                    printf("APP: JESD Oneshot Synchronization Not Completed\n");
                }
                return err;
            }
            if (err = adi_ad9986_adc_nco_master_slave_sync(&ad9986_dev, 0, 2, 0, 1), err != API_CMS_ERROR_OK) /*mcs: subordinate*/
                return err;
            break;
        default:
            printf("APP: JESD RX Synchronization Mode: %s\n", ((jrx_param[uc].jesd_subclass == JESD_SUBCLASS_1) ? "JESD_SUBCLASS_1" : "JESD_SUBCLASS_0"));
            /* Power down Sysref Receiver circuitry*/ 
            if (err = adi_ad9986_jesd_sysref_input_mode_set(&ad9986_dev, jrx_param[uc].jesd_subclass > 0 ? 1 : 0, jrx_param[uc].jesd_subclass > 0 ? 1 : 0, COUPLING_AC), err != API_CMS_ERROR_OK)     
                return err;
            /* Perform oneshot sync */
            if (err = adi_ad9986_jesd_oneshot_sync(&ad9986_dev,  0), err != API_CMS_ERROR_OK){
                if (err == API_CMS_ERROR_JESD_SYNC_NOT_DONE) {
                    printf("APP: JESD Oneshot Synchronization Not Completed");
                }
                return err;
            }
            break;
        }
    }
    /* SYSTEM Link Bring Up Sequenece
    *  Check AD9986 JESD PLL Lock Status
    *  Enable AD9986 JESD Rx/ JESD TX Links
    *  Ensure FPGA JESD RX/ JESD TX Links are configured
    *  Ensure FPGA is transmitting Data
    *  Toggle AD9986 JESD RX Links Enable
    *  Run AD9986 JESD RX 204C Calibration if Lane Rate is above Threshold AD9986_JESDRX_204C_CAL_THRESH
    *  Toggle AD9986 JESD RX Links Enable
    *  Check Link Status after short period time
    */
    if (uc > 0) {
        /* Check JESD PLL LOCK Status */
        uint8_t uc_jesd_pll_status = 0x00;
        err = adi_ad9986_jesd_pll_lock_status_get(&ad9986_dev, &uc_jesd_pll_status);
        printf("APP: ad9986 JESD PLL lock Status: %s : %s\n", (uc_jesd_pll_status ? "LOCKED" : "NOT LOCKED"), (uc_jesd_pll_status ? "Enabling Links" : "Exiting"));
        if (err != API_CMS_ERROR_OK) {
            return err;
        }

#if !defined(AD9177_ID)
        /* enable ad9986 Rx Path: JESD Tx Link links */
        switch (uc) {
        case 17:
        case 28:
            /*TX ONLY MODES*/
            break;
        default:
            if (err = adi_ad9986_jesd_tx_link_enable_set(&ad9986_dev, (jtx_param[uc][0].jesd_duallink > 0) ? AD9986_LINK_ALL : AD9986_LINK_0, 1), err != API_CMS_ERROR_OK)
                return err;
            break;
        }
#endif
#if !defined(AD9207_ID) && !defined(AD9209_ID) 
        /* enable ad9986 Tx Path: JESD Rx Link links */
        switch (uc) {
        case 18:
        case 22:
        case 29:
        case 31:
        case 33:
        case 34:
        case 39:
            /*RX ONLY MODES*/
            break;
        default:
            if (err = adi_ad9986_jesd_rx_link_enable_set(&ad9986_dev, (jrx_param[uc].jesd_duallink > 0) ? AD9986_LINK_ALL : AD9986_LINK_0, 1), err != API_CMS_ERROR_OK)
                return err;
            break;
        }
#endif

        /*Enable FPGA JESD RxTx/ Initiate Playback */
        if (err = adi_ads9_reg_set(0x106, 0x000), err != API_CMS_ERROR_OK) /* skip_rx_link_init = 0 */
            return err;
        if (err = adi_ads9_reg_set(0x947, 2), err != API_CMS_ERROR_OK) /* bidir_start = 1 */
            return err;
        if (err = ads9_wait_us(NULL, 100000), err != API_CMS_ERROR_OK)
            return err;
        if (ad9986_dev.clk_info.sysref_mode == SYSREF_ONESHOT) {
            if (err = adi_hmc7044_pulse_gen_set(&hmc7044_dev), err != API_CMS_ERROR_OK)
                return err;
        }
        if (err = adi_ads9_reg_set(0x106, 0x400), err != API_CMS_ERROR_OK) /* skip_rx_link_init = 1 */
            return err;
        if (err = adi_ads9_reg_set(0x537, 4), err != API_CMS_ERROR_OK) /* gt_tx_ptn_play_stop = 1 */
            return err;

#if !defined(AD9207_ID) && !defined(AD9209_ID) 
        if (err = adi_ad9986_jesd_rx_link_enable_set(&ad9986_dev, AD9986_LINK_ALL, 0), err != API_CMS_ERROR_OK)
            return err;
#endif
        if (err = adi_ads9_reg_set(0x537, 1), err != API_CMS_ERROR_OK) /* gt_tx_ptn_play_start = 1 */
            return err;
#if !defined(AD9207_ID) && !defined(AD9209_ID) 
        if (err = adi_ad9986_jesd_rx_link_enable_set(&ad9986_dev, (jrx_param[uc].jesd_duallink > 0) ? AD9986_LINK_ALL : AD9986_LINK_0, 1), err != API_CMS_ERROR_OK)
            return err;
#endif

        /* calibrate jrx when lane rate is high for 204c */
        printf("APP: Run JESD RX 204C Calibration & Enable TX Path Links\n");
#if !defined(AD9207_ID) && !defined(AD9209_ID) 
        char ctle_file[] = "ctle_coeffs";
        if ((jrx_param[uc].jesd_l > 0) && (jrx_param[uc].jesd_jesdv == 2) && ((clk_hz[uc][1] * 66) > AD9986_JESDRX_204C_CAL_THRESH)) {
            if (ad9986_dev.serdes_info.des_settings.cal_mode == AD9986_CAL_MODE_BYPASS) {
                app_cal_ctle_manual_config_load(&ad9986_dev, ctle_file);
            }

            if (err = adi_ad9986_jesd_rx_calibrate_204c(&ad9986_dev, 1, 0x00, (ad9986_dev.serdes_info.des_settings.cal_mode == AD9986_CAL_MODE_RUN_AND_SAVE) ? 0 : 1), err != API_CMS_ERROR_OK) {
                printf("APP: ad9986 JESD RX Calibration Error\n");
                return err;
            }

            if (ad9986_dev.serdes_info.des_settings.cal_mode == AD9986_CAL_MODE_RUN_AND_SAVE) {
                app_cal_ctle_manual_config_save(&ad9986_dev, ctle_file);
            }

            if (err = adi_ad9986_jesd_rx_link_enable_set(&ad9986_dev, AD9986_LINK_ALL, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_jesd_rx_link_enable_set(&ad9986_dev, (jrx_param[uc].jesd_duallink > 0) ? AD9986_LINK_ALL : AD9986_LINK_0, 1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_jesd_rx_link_enable_set(&ad9986_dev, AD9986_LINK_ALL, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_jesd_rx_link_enable_set(&ad9986_dev, (jrx_param[uc].jesd_duallink > 0) ? AD9986_LINK_ALL : AD9986_LINK_0, 1), err != API_CMS_ERROR_OK)
                return err;
        }
        /* run 2D eye scan */
        switch (uc)
        {
        case 16:
            /* half rate mode */
            if (err = app_test_eye_scan(&ad9986_dev, &jrx_param[uc], uc, AD9986_HALF_RATE, PRBS7, 1), err != API_CMS_ERROR_OK)
                return err;
            break;
        case 11:
            /* quarter rate mode */
            if (err = app_test_eye_scan(&ad9986_dev, &jrx_param[uc], uc, AD9986_QUART_RATE, PRBS7, 1), err != API_CMS_ERROR_OK)
                return err;
            break;
        default:
            break;
        }
#endif
#if !defined(AD9177_ID)
    /* TDD Power Savings*/
        switch (uc)
        {
        case 25:
            if (err = adi_ad9986_adc_adc0_rxen_pwdn_ctrl_set(&ad9986_dev, 0, 1, 0, 1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_adc1_rxen_pwdn_ctrl_set(&ad9986_dev, 1, 0, 1, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_adc2_rxen_pwdn_ctrl_set(&ad9986_dev, 1, 0, 1, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_adc3_rxen_pwdn_ctrl_set(&ad9986_dev, 0, 1, 0, 1), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_rxen0_sel_set(&ad9986_dev, 7, 255, 0, 0, 255), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_rxen0_ctrl_set(&ad9986_dev, 0, 0, 0, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_rxen1_sel_set(&ad9986_dev, 0, 0, 0, 0, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_rxen1_ctrl_set(&ad9986_dev, 0, 0, 0, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_rxengp0_sel_set(&ad9986_dev, 0, 0, 0, 0, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_rxengp0_ctrl_set(&ad9986_dev, 0, 0, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_rxengp1_sel_set(&ad9986_dev, 0, 0, 0, 0, 0), err != API_CMS_ERROR_OK)
                return err;
            if (err = adi_ad9986_adc_rxengp1_ctrl_set(&ad9986_dev, 0, 0, 0), err != API_CMS_ERROR_OK)
                return err;
            break;
        }
#endif

        /* Collect Power Data from attached AD7175 */
        if (en_ad7175) {
            if (err = app_collect_power_measurements(&ad7175_dev), err != API_CMS_ERROR_OK)
                return err;
        }


        /* delay some time for link to be stable */
        if (err = ads9_wait_us(NULL, 10000000), err != API_CMS_ERROR_OK)
            return err;
        /* check link status */
        if (err = app_show_link_status(&ad9986_dev), err != API_CMS_ERROR_OK)
            return err;
    }
    
    { /* free resource and close platform */
        /* free user data */
        if (err = ads9_user_data_free(&ad9986_dev.hal_info.user_data), err != API_CMS_ERROR_OK)
            return err;
        if (err = ads9_user_data_free(&hmc7044_dev.hal_info.user_data), err != API_CMS_ERROR_OK)
            return err;
        if (err = ads9_user_data_free(&ad7175_dev.hal_info.user_data), err != API_CMS_ERROR_OK)
            return err;

        /* close platform */
        if (err = ads9_hw_close(), err != API_CMS_ERROR_OK)
            return err;
    }

    printf("\n");
    return API_CMS_ERROR_OK;
}





