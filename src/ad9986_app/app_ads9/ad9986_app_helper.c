/*!
 * @brief     Standalone Linux Application Helper Functions
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

static int32_t app_show_jrx_204b_lane_status(uint16_t ad9986_jrx_link_status, uint16_t num_lanes);
static int32_t app_show_jrx_204c_state_machine_status(uint16_t ad9986_jrx_link_status);


int32_t app_show_jrx_204b_lane_status(uint16_t ad9986_jrx_link_status, uint16_t num_lanes)
{
    int8_t i;
    uint16_t lanes_status;

    lanes_status = (ad9986_jrx_link_status & 0x0ff) ^ ((1 << num_lanes) - 1);
    for (i = 0; i < 8; i++) {
        if (lanes_status & (1 << i)) {
            printf("    Lane %d has cgs/ils/fs errors.\n", i);
        }
    }
    return API_CMS_ERROR_OK;
}

int32_t app_show_jrx_204c_state_machine_status(uint16_t ad9986_jrx_link_status)
{
    uint8_t jrx_state_status;
    jrx_state_status = (ad9986_jrx_link_status & 0xf00) >> 8;

    if (jrx_state_status == 0x0) {
        printf("    AD9986 JESD204C RX state machine in reset.\n");
    } else if (jrx_state_status == 0x1) {
        printf("    AD9986 JESD204C RX state machine is unlocked.\n");
    } else if (jrx_state_status == 0x2) {
        printf("    AD9986 JESD204C RX state machine is block aligned.\n");
    } else if (jrx_state_status == 0x3) {
        printf("    AD9986 JESD204C RX state machine is lane aligned.\n");
    } else if (jrx_state_status == 0x4) {
        printf("    AD9986 JESD204C RX state machine is extended multiblock aligned.\n");
    } else if (jrx_state_status == 0x6) {
        printf("    AD9986 JESD204C RX state machine is locked.\n");
    }
    return API_CMS_ERROR_OK;
}

int32_t app_show_link_status(adi_ad9986_device_t *device)
{
    int32_t  err;
    uint32_t fpga_use_204c;
    uint32_t fpga_jrx_np, fpga_jtx_np;

    /* get link configuration */
    if (err = adi_ads9_reg_get(0x943, &fpga_use_204c), err != API_CMS_ERROR_OK)
        return err;
    printf("APP: Checking JESD link status: %s Mode \n", (fpga_use_204c ? "204C" : "204B"));
    
    if (err = adi_ads9_reg_get(0x121, &fpga_jrx_np), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_reg_get(0x521, &fpga_jtx_np), err != API_CMS_ERROR_OK)
        return err;
 

#if !defined(AD9207_ID) && !defined(AD9209_ID)
    /* get link status of tx */
    uint16_t ad9986_jrx_link_status[2];
    uint32_t fpga_jesd204b_tx_status;
    uint8_t ad9986_jrx_tpl_link_status[2];
    uint32_t fpga_jtx_lscrparam;
    uint8_t linkup = 0;
    uint16_t jrx_dl_204c_state_link_up = 0x600;

    if (err = adi_ads9_reg_get(0x54e, &fpga_jesd204b_tx_status), err != API_CMS_ERROR_OK)
        return err;
    if ((fpga_jtx_np & 0x00ff) > 0) { /* link0 */ 
        if (err = adi_ad9986_jesd_rx_link_status_get(device, AD9986_LINK_0, &ad9986_jrx_link_status[0]), err != API_CMS_ERROR_OK)
            return err;
    }
    if ((fpga_jtx_np & 0xff00) > 0) { /* link1 */
        if (err = adi_ad9986_jesd_rx_link_status_get(device, AD9986_LINK_1, &ad9986_jrx_link_status[1]), err != API_CMS_ERROR_OK)
            return err;
    }
    err = adi_ad9986_jesd_rx_link_select_set(device, AD9986_LINK_0);
    err = adi_ad9986_device_spi_register_get(device, 0x04a0, &ad9986_jrx_tpl_link_status[0]);
    err = adi_ad9986_jesd_rx_link_select_set(device, AD9986_LINK_1);
    err = adi_ad9986_device_spi_register_get(device, 0x04a0, &ad9986_jrx_tpl_link_status[1]);

    if (err = adi_ads9_reg_get(0x520, &fpga_jtx_lscrparam), err != API_CMS_ERROR_OK)
        return err;
    fpga_jtx_lscrparam = (fpga_jtx_lscrparam & 0x1f) + 1;

    if (fpga_use_204c == 0) {
        if ((fpga_jtx_np & 0x00ff) > 0) { /* link0 */    
            linkup = ((ad9986_jrx_link_status[0] & 0x0ff) == ((1 << fpga_jtx_lscrparam) - 1)) ? 1 : 0;
            if (linkup) {
                printf("  AD9986 JRX link0 is up.\n");
            } else {
                printf("  AD9986 JRX link0 isn't up.\n");
                app_show_jrx_204b_lane_status(ad9986_jrx_link_status[0], fpga_jtx_lscrparam);
            }
            printf("  AD9986 JRX Status = 0x%.4x, AD9986 JRX TPL Status =0x%.2x FPGA JTX Status = 0x%.2x\n", ad9986_jrx_link_status[0], ad9986_jrx_tpl_link_status[0], fpga_jesd204b_tx_status);
        }
        if ((fpga_jtx_np & 0xff00) > 0) { /* link1 */
            linkup = ((ad9986_jrx_link_status[1] & 0x0ff) == ((1 << fpga_jtx_lscrparam) - 1)) ? 1 : 0;
            if (linkup) {
                printf("  AD9986 JRX link1 is up.\n");
            } else {
                printf("  AD9986 JRX link1 isn't up.\n");
                app_show_jrx_204b_lane_status(ad9986_jrx_link_status[1], fpga_jtx_lscrparam);
            }
            printf("  AD9986 JRX Status = 0x%.4x, AD9986 TPL Status =0x%.2x, FPGA JTX Status = 0x%.2x\n", ad9986_jrx_link_status[1], ad9986_jrx_tpl_link_status[1],fpga_jesd204b_tx_status);
        }
    } else { /* 204C */
        if ((fpga_jtx_np & 0x00ff) > 0) { /* link0 */   
            linkup = ((ad9986_jrx_link_status[0] & 0xf00) == jrx_dl_204c_state_link_up) ? 1 : 0;
            if (linkup) {
                printf("  AD9986 JRX link0 is up.\n");
            } else {
                printf("  AD9986 JRX link0 isn't up.\n");
                app_show_jrx_204c_state_machine_status(ad9986_jrx_link_status[0]);
            } 
            printf("  AD9986 JRX Status = 0x%.4x, AD9986 TPL Status =0x%.2x, FPGA JTX Status = 0x%.2x\n", ad9986_jrx_link_status[0], ad9986_jrx_tpl_link_status[0], fpga_jesd204b_tx_status);
        }
        if ((fpga_jtx_np & 0xff00) > 0) { /* link1 */
            linkup = ((ad9986_jrx_link_status[1] & 0xf00) == jrx_dl_204c_state_link_up) ? 1 : 0;
            if (linkup) {
                printf("  AD9986 JRX link1 is up.\n");
            } else {
                printf("  AD9986 JRX link1 isn't up.\n");
                app_show_jrx_204c_state_machine_status(ad9986_jrx_link_status[1]);
            }
            printf("  AD9986 JRX Status = 0x%.4x, AD9986 TPL Status =0x%.2x, FPGA JTX Status = 0x%.2x\n", ad9986_jrx_link_status[1], ad9986_jrx_tpl_link_status[1], fpga_jesd204b_tx_status);
        }
    }
#endif

#if !defined(AD9177_ID)
    /* get link status of rx */
    uint16_t ad9986_jtx_link_status[2];
    uint32_t fpga_jesd204b_rx_status, fpga_jesd240c_rx_status, fpga_jesd204c_link_err_cnt, fpga_rx_err_total_cnt;
    if (err = adi_ads9_reg_get(0x14e, &fpga_jesd204b_rx_status), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_reg_get(0x205, &fpga_jesd240c_rx_status), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_reg_get(0x160, &fpga_rx_err_total_cnt),   err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_reg_get(0x220, &fpga_jesd204c_link_err_cnt), err != API_CMS_ERROR_OK)
        return err;
    if ((fpga_jrx_np & 0x00ff) > 0) { /* link0 */ 
        if (err = adi_ad9986_jesd_tx_link_status_get(device, AD9986_LINK_0, &ad9986_jtx_link_status[0]), err != API_CMS_ERROR_OK)
            return err;    
    }
    if ((fpga_jrx_np & 0xff00) > 0) { /* link1 */
        if (err = adi_ad9986_jesd_tx_link_status_get(device, AD9986_LINK_1, &ad9986_jtx_link_status[1]), err != API_CMS_ERROR_OK)
            return err;      
    }
    if (fpga_use_204c == 0) {
        if ((fpga_jrx_np & 0x00ff) > 0) { /* link0 */
            printf((((ad9986_jtx_link_status[0] & 0xff) == 0x7d) && ((fpga_jesd204b_rx_status & 0x10) == 0x00)) ? "  AD9986 JTX link0 is up.\n" : "  AD9986 JTX link0 isn't up.\n");
            printf("  AD9986 JTX Status = 0x%.4x, FPGA JRX Status = (0x%.2x, 0x%.2x), EMB_ERR_CNT = %d, CRC_ERR_CNT = %d, RX_ERR_CNT = %d\n", 
                     ad9986_jtx_link_status[0], fpga_jesd204b_rx_status, fpga_jesd240c_rx_status, fpga_jesd204c_link_err_cnt >> 16, fpga_jesd204c_link_err_cnt & 0xffff, fpga_rx_err_total_cnt);
        }
        if ((fpga_jrx_np & 0xff00) > 0) { /* link1 */
            printf((((ad9986_jtx_link_status[1] & 0xff) == 0x7d) && ((fpga_jesd204b_rx_status & 0x10) == 0x00)) ? "  AD9986 JTX link1 is up.\n" : "  AD9986 JTX link1 isn't up.\n");
            printf("  AD9986 JTX Status = 0x%.4x, FPGA JRX Status = (0x%.2x, 0x%.2x), EMB_ERR_CNT = %d, CRC_ERR_CNT = %d, RX_ERR_CNT = %d\n", 
                     ad9986_jtx_link_status[1], fpga_jesd204b_rx_status, fpga_jesd240c_rx_status, fpga_jesd204c_link_err_cnt >> 16, fpga_jesd204c_link_err_cnt & 0xffff, fpga_rx_err_total_cnt);
        }
    } else { /* 204C */
        if ((fpga_jrx_np & 0x00ff) > 0) { /* link0 */
            printf((((ad9986_jtx_link_status[0] & 0x60) == 0x60) && ((fpga_jesd204b_rx_status & 0x10) == 0x00)) ? "  AD9986 JTX link0 is up.\n" : "  AD9986 JTX link0 isn't up.\n");
            printf("  AD9986 JTX Status = 0x%.4x, FPGA JRX Status = (0x%.2x, 0x%.2x), EMB_ERR_CNT = %d, CRC_ERR_CNT = %d, RX_ERR_CNT = %d\n", 
                     ad9986_jtx_link_status[0], fpga_jesd204b_rx_status, fpga_jesd240c_rx_status, fpga_jesd204c_link_err_cnt >> 16, fpga_jesd204c_link_err_cnt & 0xffff, fpga_rx_err_total_cnt);
        }
        if ((fpga_jrx_np & 0xff00) > 0) { /* link1 */
            printf((((ad9986_jtx_link_status[1] & 0x60) == 0x60) && ((fpga_jesd204b_rx_status & 0x10) == 0x00)) ? "  AD9986 JTX link1 is up.\n" : "  AD9986 JTX link1 isn't up.\n");
            printf("  AD9986 JTX Status = 0x%.4x, FPGA JRX Status = (0x%.2x, 0x%.2x), EMB_ERR_CNT = %d, CRC_ERR_CNT = %d, RX_ERR_CNT = %d\n", 
                     ad9986_jtx_link_status[1], fpga_jesd204b_rx_status, fpga_jesd240c_rx_status, fpga_jesd204c_link_err_cnt >> 16, fpga_jesd204c_link_err_cnt & 0xffff, fpga_rx_err_total_cnt);
        }
    }
#endif

    return API_CMS_ERROR_OK;
}

int32_t app_test_phy_prbs(adi_ad9986_device_t *device, adi_cms_jesd_prbs_pattern_e prbs)
{
#if !defined(AD9207_ID) && !defined(AD9209_ID)
    /* Make sure clock to fpga is lane_rate/64 for 204C, and lane_rate/20 for 204B */
    int32_t err, i;
    adi_ad9986_prbs_test_t result;

    printf("APP: Starting AD9986 JRX PHY PRBS Test: \n");
    if (err = adi_ads9_stop_transmit(), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_config_jtx_prbs(prbs), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_start_transmit(), err != API_CMS_ERROR_OK)
        return err;
    /* Configure link and enable prior to running PRBS */
    /* Available APIs for link operation: adi_ad9986_jesd_rx_link_config_set, adi_ad9986_jesd_rx_link_enable_set */
    if (err = adi_ad9986_jesd_rx_phy_prbs_test(device, prbs, 1), err != API_CMS_ERROR_OK)
        return err;    
    for(i = 0; i < 8; i ++) {
        if (err = adi_ad9986_jesd_rx_phy_prbs_test_result_get(device, i, &result), err != API_CMS_ERROR_OK)
            return err;
        printf("APP: PRBS Error Count on Lane %d: %d \n", i, result.phy_prbs_err_cnt);
    }
#endif

    return API_CMS_ERROR_OK;
}

int32_t app_sysref_clk_src_sel(void *sysref_clk) {
    adi_hmc7044_device_t *hmc7044_device = (adi_hmc7044_device_t*) sysref_clk;
    if (hmc7044_device == NULL)
        return API_CMS_ERROR_ERROR;

    return adi_hmc7044_pulse_gen_set(hmc7044_device); 
}

void app_calc_tx_lane_rate(uint64_t clock_hz[4], adi_cms_jesd_param_t *jesd_rx_param, uint8_t transmit_interp[2], uint64_t *calc_app_jrx_lane_rate)
{
	uint8_t k;
	*calc_app_jrx_lane_rate = 0;

	for (k = 0; k < (jesd_rx_param->jesd_duallink + 1) ; k++) {
		if (jesd_rx_param->jesd_l) {
			*calc_app_jrx_lane_rate = (jesd_rx_param->jesd_m * jesd_rx_param->jesd_np * ((jesd_rx_param->jesd_jesdv == 1) ? 10 : 66) * clock_hz[2]);
			*calc_app_jrx_lane_rate = *calc_app_jrx_lane_rate / (jesd_rx_param->jesd_l * ((jesd_rx_param->jesd_jesdv == 1) ? 8 : 64) * transmit_interp[0] * transmit_interp[1]);
		}
	}
}

void app_calc_rx_lane_rate(uint64_t clock_hz[4], adi_cms_jesd_param_t jesd_tx_param[2], uint8_t jesd_tx_chip_dcm_ratio[2], uint64_t calc_app_jtx_lane_rate[2])
{
	uint8_t k;
    calc_app_jtx_lane_rate[0] = 0;
    calc_app_jtx_lane_rate[1] = 0;

	for (k = 0; k < (jesd_tx_param[0].jesd_duallink + 1) ; k++) {
		if (jesd_tx_param[k].jesd_l) {
			calc_app_jtx_lane_rate[k] = jesd_tx_param[k].jesd_np * jesd_tx_param[k].jesd_m * clock_hz[3] * ((jesd_tx_param[k].jesd_jesdv == 1) ? 10 : 66);
			calc_app_jtx_lane_rate[k] = calc_app_jtx_lane_rate[k] / (jesd_tx_param[k].jesd_l * jesd_tx_chip_dcm_ratio[k] * ((jesd_tx_param[k].jesd_jesdv == 1) ? 8 : 64));
        }
	}
}

int32_t app_cal_ctle_manual_config_load(adi_ad9986_device_t *device, char filename[])
{
    int32_t err;
    int8_t i;
    FILE *fp;
    char lane_data[20];
    printf("APP: Loading CTLE Coefficient values from file\n");

    /* load ctle coeffs values from file to array */
    strcat(filename, ".txt");
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("APP: fopen failed to open the file\n");
        return API_CMS_ERROR_NULL_PARAM;
    }
    for (i = 0; i < 8; i++) {
        if (fgets(lane_data, 20, fp) != NULL) {
            sscanf(lane_data, "%u,%u,%u,%u\n", (unsigned int*)&(device->serdes_info.des_settings.ctle_coeffs[i][0]), (unsigned int*)&(device->serdes_info.des_settings.ctle_coeffs[i][1]), 
            (unsigned int*)&(device->serdes_info.des_settings.ctle_coeffs[i][2]), (unsigned int*)&(device->serdes_info.des_settings.ctle_coeffs[i][3]));
        }    
    }
    fclose(fp);

    /*set array ctle coeffs to regs*/
    for (int i = 0; i < 8; i++) {
        if (err = adi_ad9986_jesd_rx_ctle_manual_config_set(device, i), err != API_CMS_ERROR_OK)
            return err;
    }
    return API_CMS_ERROR_OK;
}

int32_t app_cal_ctle_manual_config_save(adi_ad9986_device_t *device, char filename[])
{
    int32_t err;
    int8_t i;
    FILE *fp;
    
    printf("APP: Saving CTLE Coefficient values to file\n");

    /* get ctle coeffs values from regs */
    for (i = 0; i < 8; i++) {
        if (err = adi_ad9986_jesd_rx_ctle_manual_config_get(device, i), err != API_CMS_ERROR_OK)
            return err;
    }

    /* save values to file */
    strcat(filename, ".txt");
    fp = fopen(filename, "w+");
    if (fp == NULL) {
        printf("APP: fopen failed to open the file\n");
        return API_CMS_ERROR_NULL_PARAM;
    }
    for (i = 0; i < 8; i++) {
        char * lane_data = malloc(sizeof(char)*20);
        snprintf(lane_data, 20, "%u,%u,%u,%u\n", (uint8_t)device->serdes_info.des_settings.ctle_coeffs[i][0], (uint8_t)device->serdes_info.des_settings.ctle_coeffs[i][1], 
        (uint8_t)device->serdes_info.des_settings.ctle_coeffs[i][2], (uint8_t)device->serdes_info.des_settings.ctle_coeffs[i][3]);
        fputs(lane_data, fp);
        free(lane_data);
    }
    fclose(fp);

    return API_CMS_ERROR_OK;
}

int32_t app_calc_nco_freq_get(uint64_t freq, uint64_t ftw, double *nco_shift)
{
    double sign = 1.0;
    double freq_mhz = freq/1e6;
    double nco_freq = 0.00;

    if (ftw > (140737488355328ull)) {
        ftw = (1ull << 48) - ftw;
        sign = -1.0;
    }
    nco_freq = (freq_mhz * ftw)/(1ull << 48);
    *nco_shift = sign * nco_freq;

    printf("APP: NCO freq: %.2f MHz\n",  *nco_shift);

    return API_CMS_ERROR_OK;
}

int32_t app_test_eye_scan_write_to_file(adi_ad9986_device_t *device, adi_ad9986_deser_mode_e mode, uint8_t lane, uint8_t spo_size, int16_t eye_scan_data[])
{
    uint8_t i;
    FILE *fp;
    char filename[50] = "eye_scan_lane_";
    char lane_save[50];
    snprintf(lane_save, 50, "%d", lane);
    strcat(filename, lane_save);
    strcat(filename, ".txt");
    fp = fopen(filename, "w+");
    if (fp == NULL) {
        printf("APP: fopen failed to open the file\n");
        return API_CMS_ERROR_NULL_PARAM;
    }
    for (i = 0; i < spo_size; i++) {
        char * spo = malloc(sizeof(char)*20);
        snprintf(spo, 20, "%d,%d,%d\n", (int16_t)eye_scan_data[i*3], (int16_t)eye_scan_data[(i*3)+1], (int16_t)eye_scan_data[(i*3)+2]);
        fputs(spo, fp);
        free(spo);
    }
    fclose(fp);
    printf("APP: Scan Complete \n");
    
    return API_CMS_ERROR_OK;
}

int32_t app_test_eye_scan(adi_ad9986_device_t *device, adi_cms_jesd_param_t *jesd_rx_param, int32_t uc, adi_ad9986_deser_mode_e mode, adi_cms_jesd_prbs_pattern_e prbs, uint8_t lane)
{
#if !defined(AD9207_ID) && !defined(AD9209_ID)
    int32_t err;
    uint8_t spo_size=(mode == (AD9986_QUART_RATE) ? 32:64) +1;
    int16_t *eye_scan_data = (int16_t*) calloc((spo_size*3), sizeof(int16_t));

    printf("APP: Starting Eye Scan.... Text file output is saved to device when completed.\n");
    printf("APP: WARNING Run all eye scan use cases with direct clock only \n");

    if ((jesd_rx_param->jesd_jesdv == 2 && device->dev_info.jesd_rx_lane_rate/64 != clk_hz[uc][1]) || (jesd_rx_param->jesd_jesdv == 1 && device->dev_info.jesd_rx_lane_rate/20 != clk_hz[uc][1])) {
        printf("APP: WARNING Set FPGA input clock to lane_rate/64 for 204C or lane_rate/20 for 204B to run PRBS test modes.\n");
    }

    /* Setup FPGA to send PRBS test pattern */
    if (err = adi_ads9_stop_transmit(), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_config_jtx_prbs(prbs), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_start_transmit(), err != API_CMS_ERROR_OK)
        return err;
    
    /* Run Eye Scan */
    if (mode == AD9986_QUART_RATE) {
        printf("APP: Running Quarter Rate Eye Scan...\n");
        if (err = adi_ad9986_jesd_cal_bg_cal_pause(device), err != API_CMS_ERROR_OK)
            return err;
        if (err = adi_ad9986_jesd_rx_qr_two_dim_eye_scan(device, lane, eye_scan_data), err != API_CMS_ERROR_OK)
            return err;
        if (err = adi_ad9986_jesd_cal_bg_cal_start(device), err != API_CMS_ERROR_OK)
            return err;
    }
    else if (mode == AD9986_HALF_RATE) {
        printf("APP: Running Half Rate Eye Scan...\n");
        if (err = adi_ad9986_jesd_rx_hr_two_dim_eye_scan(device, lane, prbs, 500, eye_scan_data), err != API_CMS_ERROR_OK)
            return err;
    }
    else {
        printf("APP: Lane rate is not within eye scan capabilities (8 Gbps and above).\n");
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if (err = app_test_eye_scan_write_to_file(device, mode, lane, spo_size, eye_scan_data), err != API_CMS_ERROR_OK)
        return err;
    
    free(eye_scan_data);
#endif

    return API_CMS_ERROR_OK;
}

int32_t app_collect_power_measurements(adi_ad7175_device_t *ad7175_dev)
{
    int32_t err;
    uint32_t data_status[8] = {0};
    float data_only[8] = {0};
    float conversion[8] = {0};
    float power[4] = {0};
    float total_power = 0;
    adi_ad7175_adc_mode_e adc_mode = AD7175_MODE_SINGLE_CONV;    

    for (int i = 0; i < 8; i++){
        /* Trigger conversion */
        if (err = adi_ad7175_device_channel_run(ad7175_dev, i, 1, AD7175_INT_OSC, adc_mode, AD7175_DELAY_0_US, 0, 0), err != API_CMS_ERROR_OK)
            return err;
        /* Collect Data */
        if (err = adi_ad7175_device_run_data_conv_get(ad7175_dev, &data_status[i], i), err != API_CMS_ERROR_OK) {
            if (err == API_CMS_ERROR_DELAY_US) {
                printf("APP: ERROR timed out before AD7175 data conversion.\n");
                return API_CMS_ERROR_DELAY_US;
            }
            if (err == API_CMS_ERROR_ERROR) {
                printf("APP: ERROR 7175 reported error in conversion, check STATUS register in log.\n");
                return API_CMS_ERROR_ERROR;
            }
        }
        data_only[i] = (float) (data_status[i]>>8);
        conversion[i] = (data_only[i]/65535)*2.5*1.058;
    }
    /* Resistors being measured across */
    float r1m = 0.05;
    float r1xa = 0.05;
    float r1 = 0.05;
    float r18 = 1;

    /* Power Calculations */
    power[0] = (conversion[0]/r1m)-0.012; //1V digital domain
    power[1] = (conversion[2]/r1xa)-0.014; //1V mixed domain
    power[2] = (conversion[4]/r1)-0.014; //1V analog domain
    power[3] = 2*((conversion[6]/r18)-0.006); //2V analog domain
    total_power = power[0] + power[1] + power[2] + power[3];
    printf("APP: Power consumption Measurements:\n \
    1V digital domain: %.3fW\n \
    1V mixed domain: %.3fW\n \
    1V analog domain: %.3fW\n \
    2V analog domain: %.3fW\n \
    Total power: %.3fW\n", power[0], power[1], power[2], power[3], total_power);
        
    return API_CMS_ERROR_OK;    
}