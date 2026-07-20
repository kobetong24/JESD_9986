/*!
 * @brief     MxFE Xtras Linux Application
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup ADI_AD9986_XTRAS_APP
 * @{
 */

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include "adi_ad9986.h"
#include "adi_ads9.h"
#include "adi_hmc7044.h"
#include "ads9.h"
#include "hsxstandaloneextras.h"

/*============= D A T A ====================*/


/*============= C O D E ====================*/
static int32_t app_show_link_status(adi_ad9986_device_t *device, uint8_t tx_link_up[2], uint8_t rx_link_up[2]);
static int32_t write_ads9_transmit_to_memory(uint32_t mem_addr, uint32_t num_bytes, uint8_t *raw_bytes);
static int32_t init_ads9_capture(uint32_t num_samples_per_conv, int num_virt_converters, int num_links, uint32_t *num_cap_bytes);
static int32_t read_ads9_capture_memory(uint32_t mem_addr, uint32_t num_cap_bytes, uint8_t *cap_buff);
static int link_state();
static void start_links();
static void print_help();
static void print_usage();
static int32_t atof_check(char *arg, double *value);
static int32_t atoi_check(char *arg, int32_t *value);
static int32_t process_tx (int argc, char *argv[], int arg_idx, adi_hsx_extras_jesd_param_t jesd_jrx_params, int num_links);
static int32_t load_stone_tx (double f_data_rate, double f_tone, uint32_t vec_len, double backoff, uint32_t nbits, adi_hsx_extras_jesd_param_t jesd_jrx_params, int num_links);
static int32_t load_file_tx(int num_files, int num_dacs, int argc, char *argv[], int arg_idx, adi_hsx_extras_jesd_param_t jesd_jrx_params, int num_links);
static int32_t process_rx (int argc, char *argv[], int arg_idx, adi_hsx_extras_jesd_param_t jesd_params_cap, int num_links_cap);
static int32_t load_cap_rx(uint32_t num_samples, uint32_t num_cap_bytes, char* filename, adi_hsx_extras_jesd_param_t jesd_params_cap, int num_links_cap);
static int32_t load_rcap_rx(uint32_t num_cap_bytes, char* filename, adi_hsx_extras_jesd_param_t jesd_params_cap, int num_links_cap);
static int32_t vec_len_from_files(int argc, char *argv[], int arg_idx);
static int32_t calc_nsamples_file(char *filename);

static double calc_rms(int16_t *x, int num_samples);

int32_t main(int argc, char *argv[])
{
    int32_t  err;

    //--help tag
    if (argc < 2) {
        printf("Please fill out all requested command line inputs\n");
        print_help();
        return API_CMS_ERROR_OK;
    }

    /* connect to platform */
    adi_ad9986_device_t ad9986_dev = {
        .hal_info.sdo = SPI_SDO,
        .hal_info.msb = SPI_MSB_FIRST,
        .hal_info.addr_inc = SPI_ADDR_INC_AUTO,
        .hal_info.log_write = ads9_log_write,
        .hal_info.delay_us = ads9_wait_us,
        .hal_info.spi_xfer = ads9_spi_xfer_ad9986,
        .hal_info.reset_pin_ctrl = ads9_hw_rst_pin_ctrl_ad9986
    };
    adi_hmc7044_device_t hmc7044_dev = {
        .hal_info.spi_xfer = ads9_spi_xfer_hmc7044,
        .hal_info.delay_us = ads9_wait_us
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

        /* show link status only */
        uint8_t tx_link_up[2];
        uint8_t rx_link_up[2];
        if (err = app_show_link_status(&ad9986_dev, tx_link_up, rx_link_up), err != API_CMS_ERROR_OK)
            return err;
        printf("\n");

        // Make sure at least one rx link is up to avoid capture hangs
        if (((rx_link_up[0] || rx_link_up[1]) == 0) && strcmp(argv[1], "rx") == 0)
        {
            printf("*** Rx links are down. Won't capture. Exiting...\n");
            exit(-1);
        }
        if (((tx_link_up[0] || tx_link_up[1]) == 0) && strcmp(argv[1], "tx") == 0)
        {
            printf("*** Tx links are down. Won't transmit. Exiting...\n");
            exit(-1);
        }
    }

    /* Get current fpga jesd link configuration */
    adi_cms_jesd_param_t fpga_jtx_params;
    adi_cms_jesd_param_t fpga_jrx_params[2];
    adi_ads9_config_jesd_get(fpga_jrx_params, &fpga_jtx_params);
    
    if (strcmp(argv[1], "tx") == 0) {
        printf("Vector Generation\n");

        /* Gather jrx jesd params (DAC) for the use case */
        adi_hsx_extras_jesd_param_t jesd_jrx_params;
        jesd_jrx_params.f = fpga_jtx_params.jesd_f;
        jesd_jrx_params.l = fpga_jtx_params.jesd_l;
        jesd_jrx_params.s = fpga_jtx_params.jesd_s;
        jesd_jrx_params.m = fpga_jtx_params.jesd_m;
        jesd_jrx_params.np = fpga_jtx_params.jesd_np;

        /* Get number of links and virtual converters for the use case */
        uint8_t num_links = fpga_jtx_params.jesd_duallink == 0 ? 1 : 2;

        err = process_tx(argc, argv, 2, jesd_jrx_params, num_links);
    } else if (strcmp(argv[1], "rx") == 0) {
        printf("Read Capture Data and Format\n");

        /* Gather jtx jesd params (ADC) for the use case */
        adi_hsx_extras_jesd_param_t jesd_params_cap;
        jesd_params_cap.f = fpga_jrx_params[0].jesd_f;
        jesd_params_cap.l = fpga_jrx_params[0].jesd_l;
        jesd_params_cap.m = fpga_jrx_params[0].jesd_m;
        jesd_params_cap.np = fpga_jrx_params[0].jesd_np;

        int num_links_cap = fpga_jrx_params[0].jesd_duallink == 0 ? 1 : 2;
       
        err = process_rx(argc, argv, 2, jesd_params_cap, num_links_cap);
    } else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0){
        print_help();
        exit(-1);
    } else {
        printf("\nUnknown command or arguments.\n");
        err = API_CMS_ERROR_ERROR;
    }

    if (err != API_CMS_ERROR_OK) {
        print_usage();
        exit(-1);
    }
    
    { /* free resource and close platform */
        /* free user data */
        if (err = ads9_user_data_free(&ad9986_dev.hal_info.user_data), err != API_CMS_ERROR_OK)
            return err;
        if (err = ads9_user_data_free(&hmc7044_dev.hal_info.user_data), err != API_CMS_ERROR_OK)
            return err;

        /* close platform */
        if (err = ads9_hw_close(), err != API_CMS_ERROR_OK)
            return err;
    }

    printf("\n");
    return API_CMS_ERROR_OK;
}

int32_t app_show_link_status(adi_ad9986_device_t *device, uint8_t tx_link_up[2], uint8_t rx_link_up[2])
{
    int32_t  err;
    bool link_up = false;
    uint32_t fpga_use_204c;
    uint32_t fpga_jrx_np, fpga_jtx_np;

    tx_link_up[0] = tx_link_up[1] = 0;
    rx_link_up[0] = rx_link_up[1] = 0;
    
    /* get link configuration */
    if (err = adi_ads9_reg_get(0x943, &fpga_use_204c), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_reg_get(0x121, &fpga_jrx_np), err != API_CMS_ERROR_OK)
        return err;
    if (err = adi_ads9_reg_get(0x521, &fpga_jtx_np), err != API_CMS_ERROR_OK)
        return err;
    printf("Checking JESD link status: \n");

#if !defined(AD9207_ID) && !defined(AD9209_ID)
    /* get link status of tx */
    uint16_t ad9986_jrx_link_status[2];
    uint32_t fpga_jesd204b_tx_status;
    uint8_t ad9986_jrx_tpl_link_status[2];
    uint32_t fpga_jtx_lscrparam;

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
            tx_link_up[0] = link_up = ((ad9986_jrx_link_status[0] & 0x0ff) == ((1 << fpga_jtx_lscrparam) - 1));
            printf(link_up ? "  AD9986 JRX link0 is up." : "  AD9986 JRX link0 isn't up.");
            printf("AD9986 JRX Status = 0x%.4x, AD9986 JRX TPL Status =0x%.2x FPGA JTX Status = 0x%.2x\n", ad9986_jrx_link_status[0], ad9986_jrx_tpl_link_status[0], fpga_jesd204b_tx_status);
        }
        if ((fpga_jtx_np & 0xff00) > 0) { /* link1 */
            tx_link_up[1] = link_up = ((ad9986_jrx_link_status[1] & 0x0ff) == ((1 << fpga_jtx_lscrparam) - 1));
            printf(link_up ? "  AD9986 JRX link1 is up." : "  AD9986 JRX link1 isn't up.");
            printf("AD9986 JRX Status = 0x%.4x, AD9986 TPL Status =0x%.2x, FPGA JTX Status = 0x%.2x\n", ad9986_jrx_link_status[1], ad9986_jrx_tpl_link_status[1],fpga_jesd204b_tx_status);
        }
    } else { /* 204C */
        if ((fpga_jtx_np & 0x00ff) > 0) { /* link0 */   
            tx_link_up[0] = link_up = ((ad9986_jrx_link_status[0] & 0xf00) == 0x600);
            printf(link_up ? "  AD9986 JRX link0 is up." : "  AD9986 JRX link0 isn't up.");
            printf("AD9986 JRX Status = 0x%.4x, AD9986 TPL Status =0x%.2x, FPGA JTX Status = 0x%.2x\n", ad9986_jrx_link_status[0], ad9986_jrx_tpl_link_status[0], fpga_jesd204b_tx_status);
        }
        if ((fpga_jtx_np & 0xff00) > 0) { /* link1 */
            tx_link_up[1] = link_up = ((ad9986_jrx_link_status[1] & 0xf00) == 0x600);
            printf(link_up ? "  AD9986 JRX link1 is up." : "  AD9986 JRX link1 isn't up.");
            printf("AD9986 JRX Status = 0x%.4x, AD9986 TPL Status =0x%.2x, FPGA JTX Status = 0x%.2x\n", ad9986_jrx_link_status[1], ad9986_jrx_tpl_link_status[1], fpga_jesd204b_tx_status);
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
            rx_link_up[0] = link_up = (((ad9986_jtx_link_status[0] & 0xff) == 0x7d) && ((fpga_jesd204b_rx_status & 0x10) == 0x00));
            printf(link_up ? "  AD9986 JTX link0 is up." : "  AD9986 JTX link0 isn't up.");
            printf(" AD9986 JTX Status = 0x%.4x, FPGA JRX Status = (0x%.2x, 0x%.2x), EMB_ERR_CNT = %d, CRC_ERR_CNT = %d, RX_ERR_CNT = %d\n", 
                     ad9986_jtx_link_status[0], fpga_jesd204b_rx_status, fpga_jesd240c_rx_status, fpga_jesd204c_link_err_cnt >> 16, fpga_jesd204c_link_err_cnt & 0xffff, fpga_rx_err_total_cnt);
        }
        if ((fpga_jrx_np & 0xff00) > 0) { /* link1 */
            rx_link_up[1] = link_up = (((ad9986_jtx_link_status[1] & 0xff) == 0x7d) && ((fpga_jesd204b_rx_status & 0x10) == 0x00));
            printf(link_up ? "  AD9986 JTX link1 is up." : "  AD9986 JTX link1 isn't up.");
            printf(" AD9986 JTX Status = 0x%.4x, FPGA JRX Status = (0x%.2x, 0x%.2x), EMB_ERR_CNT = %d, CRC_ERR_CNT = %d, RX_ERR_CNT = %d\n", 
                     ad9986_jtx_link_status[1], fpga_jesd204b_rx_status, fpga_jesd240c_rx_status, fpga_jesd204c_link_err_cnt >> 16, fpga_jesd204c_link_err_cnt & 0xffff, fpga_rx_err_total_cnt);
        }
    } else { /* 204C */
        if ((fpga_jrx_np & 0x00ff) > 0) { /* link0 */
            rx_link_up[0] = link_up = (((ad9986_jtx_link_status[0] & 0x60) == 0x60) && ((fpga_jesd204b_rx_status & 0x10) == 0x00));
            printf(link_up ? "  AD9986 JTX link0 is up." : "  AD9986 JTX link0 isn't up.");
            printf(" AD9986 JTX Status = 0x%.4x, FPGA JRX Status = (0x%.2x, 0x%.2x), EMB_ERR_CNT = %d, CRC_ERR_CNT = %d, RX_ERR_CNT = %d\n", 
                     ad9986_jtx_link_status[0], fpga_jesd204b_rx_status, fpga_jesd240c_rx_status, fpga_jesd204c_link_err_cnt >> 16, fpga_jesd204c_link_err_cnt & 0xffff, fpga_rx_err_total_cnt);
        }
        if ((fpga_jrx_np & 0xff00) > 0) { /* link1 */
            rx_link_up[1] = link_up = (((ad9986_jtx_link_status[1] & 0x60) == 0x60) && ((fpga_jesd204b_rx_status & 0x10) == 0x00));
            printf(link_up ? "  AD9986 JTX link1 is up." : "  AD9986 JTX link1 isn't up.");
            printf(" AD9986 JTX Status = 0x%.4x, FPGA JRX Status = (0x%.2x, 0x%.2x), EMB_ERR_CNT = %d, CRC_ERR_CNT = %d, RX_ERR_CNT = %d\n", 
                     ad9986_jtx_link_status[1], fpga_jesd204b_rx_status, fpga_jesd240c_rx_status, fpga_jesd204c_link_err_cnt >> 16, fpga_jesd204c_link_err_cnt & 0xffff, fpga_rx_err_total_cnt);
        }
    }
#endif
    return API_CMS_ERROR_OK;
}

// Write ads memory with vector data for Tx
static int32_t write_ads9_transmit_to_memory(uint32_t mem_addr, uint32_t num_bytes, uint8_t *raw_bytes)
{
    adi_ads9_xfer_meta_t xfer_meta;
    xfer_meta.address =  mem_addr;
    xfer_meta.filesize = num_bytes;
    xfer_meta.filetype = 0x01;
    xfer_meta.filenameLen = 0;

    adi_ads9_transfer_config(&xfer_meta);
        
    adi_ads9_data_xfer_e dl_state = START;
    adi_ads9_write(raw_bytes, 0, &dl_state);
        
    dl_state = NEXT;
    adi_ads9_write(raw_bytes, num_bytes, &dl_state);

    dl_state = END;
    adi_ads9_write(raw_bytes, 0, &dl_state);
                
    adi_ads9_pattern_len_set(num_bytes);
    adi_ads9_pattern_addr_set(xfer_meta.address);
    return API_CMS_ERROR_OK;
}


// Calc required num of capture bytes and set the ads9 capture size based on hw constraints (i.e. 1M min)
static int32_t init_ads9_capture(uint32_t num_samples_per_conv, int num_virt_converters, int num_links, uint32_t *num_cap_bytes)
{
    *num_cap_bytes = num_samples_per_conv * num_virt_converters*num_links * 2;

    // Set the ads9 capture size. Must be >= 1M and divisible by 64k
    // Capture reads will hang if not configured properly
    uint32_t cap_size = (*num_cap_bytes < 0x100000) ? 0x100000 : *num_cap_bytes;          // MIN 1MBytes for MZ capture
    uint32_t num_64k_blks = (cap_size / 65536) + ((cap_size % 65536) > 0 ? 1 : 0);
    adi_ads9_capture_size_set(num_64k_blks);       // specify # of 64k byte blocks

    return API_CMS_ERROR_OK;
}

// Read ads9 capture memory. Must have called init_ads9_capture prior to calling this function.
static int32_t read_ads9_capture_memory(uint32_t mem_addr, uint32_t num_cap_bytes, uint8_t *cap_buff)
{
    int32_t  err;

    adi_ads9_xfer_meta_t xfer_meta;
    xfer_meta.address = 0x00000000;
    xfer_meta.filesize = num_cap_bytes;
    xfer_meta.filetype = 0x01;
    xfer_meta.filenameLen = 0;

    if (err = adi_ads9_transfer_config(&xfer_meta), err != API_CMS_ERROR_OK)
        return err;

    adi_ads9_data_xfer_e dl_state = START;
    if (err = adi_ads9_read(cap_buff, 0, &dl_state), err != API_CMS_ERROR_OK)
        return err;

    dl_state = NEXT;
    if (err = adi_ads9_read(cap_buff, num_cap_bytes, &dl_state), err != API_CMS_ERROR_OK)
        return err;


    dl_state = END;
    if (err = adi_ads9_read(cap_buff, 0, &dl_state), err != API_CMS_ERROR_OK)
        return err;

    return API_CMS_ERROR_OK;
}


static void start_links()
{
    adi_ads9_reg_set(0x537, 0x001); // gt_tx_ptn_play_start
    return;

    for (int x = 0; x < 100; x++) {
        // retry loop if link not up
        adi_ads9_reg_set(0x106, 0x000);             // skip_rx_link_init = 0
        adi_ads9_reg_set(0x947, 0x008);             // bdir stop
        adi_ads9_reg_set(0x947, 0x002);             // bdir start
        adi_ads9_reg_set(0x537, 0x004);     
        printf("Delay...\n");
        ads9_wait_us(NULL, 300000);                 // delay 
        adi_ads9_reg_set(0x537, 0x001);
        adi_ads9_reg_set(0x106, 0x400);             // skip_rx_link_init = 1
            
        bool link_up = (link_state() == 1);
        printf("Link_up: %d\n", link_up); 
        if (link_up) {
            break;
        }
        printf("Delay should be running...\n");
        ads9_wait_us(NULL, 3000000);               // delay 
    }
}

static int link_state()
{
    uint32_t r546;
    
    adi_ads9_reg_get(0x546, &r546);
    adi_ads9_reg_set(0x546, r546 | 2);      // reset sync edge counter
        
    adi_ads9_reg_get(0x546, &r546);
    uint32_t edge_count = r546 >> 16;
    if (edge_count != 0 && ((r546 & 0x01) != 0x1))
    {
        return 2;       // toggle
    }
    
    adi_ads9_reg_get(0x546, &r546);
    return (r546 & 0x01) == 0 ? 0 : 1;  // 0=link down, 1=link up
}

int32_t process_tx(int argc, char *argv[], int arg_idx, adi_hsx_extras_jesd_param_t jesd_jrx_params, int num_links) {
    if (strcmp(argv[arg_idx], "--stone") == 0) {
        arg_idx++;
        if (argc < 4) {
            printf("\nPlease fill out all requested command line inputs\n");
            return API_CMS_ERROR_ERROR;
        }
        double f_data_rate = 0.0;
        if (atof_check(argv[arg_idx++], &f_data_rate) == -1 || f_data_rate < 0){
            printf("\nInvalid data rate value %s\n", argv[arg_idx-1]);
            return API_CMS_ERROR_ERROR;
        }
        double f_tone = 0.0;
        if (atof_check(argv[arg_idx++], &f_tone) == -1){
            printf("\nInvalid tone freq %s\n", argv[arg_idx-1]);
            return API_CMS_ERROR_ERROR;
        }
        
        const int max_vec_len = 1048576;
        const double min_back_off = -100.0;
        const int max_bits = 16;
        const int min_bits = 8;
        int32_t vec_len = 8192;
        double backoff = 0.0;
        int32_t nbits = 16;
        if (argc > 5) {
            if (atoi_check(argv[arg_idx++], &vec_len) == -1 || vec_len < 0 || vec_len > max_vec_len){
                printf("\nInvalid vector len value %s\n", argv[arg_idx-1]);
                if (vec_len > max_vec_len) {
                    printf("Max vec len is %d.\n", max_vec_len);
                }
                return API_CMS_ERROR_ERROR;
            }
        } if (argc > 6) { 
            if (atof_check(argv[arg_idx++], &backoff) == -1 || backoff > 0.0 || backoff < min_back_off){
                printf("\nInvalid backoff value %s.\n", argv[arg_idx-1]);
                printf("Backoff must be between 0.0 and %f.\n", min_back_off);
                return API_CMS_ERROR_ERROR;
            }
        } if (argc > 7) {
            if (atoi_check(argv[arg_idx++], &nbits) == -1 || nbits < 8 || nbits > max_bits){
                printf("\nInvalid num bits value %s.\n", argv[arg_idx-1]);
                printf("Num bits must be between %d and %d.\n", min_bits, max_bits);
                return API_CMS_ERROR_ERROR;
            }
        }
        load_stone_tx(f_data_rate, f_tone, (uint32_t)vec_len, backoff, (uint32_t)nbits, jesd_jrx_params, num_links);
    } else if (strcmp(argv[arg_idx], "--file") == 0) {
        arg_idx++;
        if (argc < 4){
            printf("\nPlease provide filename to import.\n");
            return API_CMS_ERROR_ERROR;
        } else {
            int num_files = argc - arg_idx;
            int num_dacs = jesd_jrx_params.m;
                  
            if (load_file_tx(num_files, num_dacs, argc, argv, arg_idx, jesd_jrx_params, num_links) != API_CMS_ERROR_OK)
            {
                printf("\nError occurred loading TX files.\n");
                return API_CMS_ERROR_ERROR;
            }
        }
    } else {
        printf("\nUnknown tx option %s\n", argv[arg_idx]);
        return API_CMS_ERROR_ERROR;
    }
    return API_CMS_ERROR_OK;
}

int32_t load_stone_tx (double f_data_rate, double f_tone, uint32_t vec_len, double backoff, uint32_t nbits, adi_hsx_extras_jesd_param_t jesd_jrx_params, int num_links) {
    int num_dacs = jesd_jrx_params.m;
    
    /* Allocate block of mem to store vectors */
    int16_t(*vecs)[vec_len] = malloc(sizeof(int16_t[num_dacs][vec_len]));

    for (int i = 0; i < num_dacs; i++) {
        if (f_tone > 0) {
            adi_hsx_extras_create_single_tone(f_data_rate, f_tone, nbits, ((i % 2) == 0) ? 90.0 : 0.0, backoff, vec_len, vecs[i]); /* pos freq */
        } else {
            adi_hsx_extras_create_single_tone(f_data_rate, -1.0*f_tone, nbits, ((i % 2) == 0) ? 0.0 : 90.0, backoff, vec_len, vecs[i]); /* neg freq */
        }
    }

    /* Allocate memory for vector processing (e.g JESD204 transport) and download */
    /* ADS9 memory layout requires 256*L boundaries */
    uint32_t vec_buffer_size = (jesd_jrx_params.m * ((jesd_jrx_params.np *10) / 8) * vec_len)/10;
    uint32_t vec_blks = (vec_buffer_size / (jesd_jrx_params.l * 256));
    uint32_t vec_blk_residual = (vec_buffer_size % (jesd_jrx_params.l * 256));
    vec_blks += (vec_blk_residual == 0) ? 0 : 1;
    vec_buffer_size = vec_blks * (jesd_jrx_params.l * 256) * num_links;

    uint8_t(*vec_buffer) = malloc(vec_buffer_size * sizeof(uint8_t));
    adi_hsx_extras_jesd204_download_vectors_single_link(num_dacs, vec_len, (uint16_t(*)[vec_len])vecs, vec_buffer_size, vec_buffer, jesd_jrx_params);

    /* transmit vector in bytes to memory */
    write_ads9_transmit_to_memory(0x80000000, vec_buffer_size, vec_buffer);

    /* start vector playback */
    start_links(); //add error checking

    free(vec_buffer);
    free(vecs);
    return API_CMS_ERROR_OK;
}

int32_t load_file_tx(int num_files, int num_dacs, int argc, char *argv[], int arg_idx, adi_hsx_extras_jesd_param_t jesd_jrx_params, int num_links) {
    /* Allocate block of mem to store vectors */
    int32_t vec_len = vec_len_from_files(argc, argv, arg_idx);
    if (vec_len < 1) {
        return API_CMS_ERROR_ERROR;
    }

    int16_t(*vecs)[vec_len] = malloc(sizeof(int16_t[num_dacs][vec_len]));

    int v_idx = 0;
    for (int i = 0; i < num_dacs; i++) {
        int num_samples_read = 0;
        v_idx = arg_idx + (i % num_files);
        adi_hsx_extras_vector_from_file(vec_len, vecs[i], argv[v_idx], jesd_jrx_params, &num_samples_read);	
    }

    /* Allocate memory for vector processing (e.g JESD204 transport) and download */
    /* ADS9 memory layout requires 256*L boundaries */
    uint32_t vec_buffer_size = jesd_jrx_params.m * (jesd_jrx_params.np / 8) * vec_len;
    uint32_t vec_blks = (vec_buffer_size / (jesd_jrx_params.l * 256));
    uint32_t vec_blk_residual = (vec_buffer_size % (jesd_jrx_params.l * 256));
    vec_blks += (vec_blk_residual == 0) ? 0 : 1;
    vec_buffer_size = vec_blks * (jesd_jrx_params.l * 256) * num_links;

    uint8_t(*vec_buffer) = malloc(vec_buffer_size * sizeof(uint8_t));
    adi_hsx_extras_jesd204_download_vectors_single_link(num_dacs, vec_len, (uint16_t(*)[vec_len])vecs, vec_buffer_size, vec_buffer, jesd_jrx_params);

    /* transmit vector in bytes to memory */
    write_ads9_transmit_to_memory(0x80000000, vec_buffer_size, vec_buffer);

    /* start vector playback */
    start_links(); //add error checking

    free(vec_buffer);
    free(vecs);

    return API_CMS_ERROR_OK;
}

int32_t process_rx (int argc, char *argv[], int arg_idx, adi_hsx_extras_jesd_param_t jesd_params_cap, int num_links_cap) {
    /* Setup ads9 for a capture. Calc number of bytes required based on num samples and link configuration */
    if (argc < 3) {
        printf("\nPlease fill out all requested command line inputs\n");
        return API_CMS_ERROR_ERROR;
    }

    int32_t max_nsamples = 1048576;
    int32_t nsamples = 8192;
    if (strcmp(argv[arg_idx], "--cap") == 0) {
        arg_idx++;
        char *filename_prefix = argv[arg_idx++];
        if (argc > 4) {
            if (atoi_check(argv[arg_idx++], &nsamples) == -1 || nsamples < 0 || nsamples > max_nsamples){
                printf("\nInvalid nsamples value %s.\n", argv[arg_idx-1]);
                printf("Max capture samples per converter is %d\n", max_nsamples);
                return API_CMS_ERROR_ERROR;
            }
        }
        uint32_t num_cap_bytes;
        init_ads9_capture((uint32_t)nsamples, jesd_params_cap.m, num_links_cap, &num_cap_bytes);
        load_cap_rx((uint32_t)nsamples, num_cap_bytes, filename_prefix, jesd_params_cap, num_links_cap);
    } else if (strcmp(argv[arg_idx], "--rcap") == 0) {
        arg_idx++;
        char *filename = argv[arg_idx++];
        if (argc > 4) {
            if (atoi_check(argv[arg_idx++], &nsamples) == -1 || nsamples < 0 || nsamples > max_nsamples){
                printf("\nInvalid nsamples value %s\n", argv[arg_idx-1]);
                printf("Max capture samples per converter is %d\n", max_nsamples);
                return API_CMS_ERROR_ERROR;
            }
        }            
        uint32_t num_cap_bytes;
        init_ads9_capture((uint32_t)nsamples, jesd_params_cap.m, num_links_cap, &num_cap_bytes);
        load_rcap_rx(num_cap_bytes, filename, jesd_params_cap, num_links_cap);
    } else {
        printf("\nUnknown rx option %s\n", argv[arg_idx]);
        return API_CMS_ERROR_ERROR;
    }
    return API_CMS_ERROR_OK;
}

int32_t load_cap_rx(uint32_t num_samples, uint32_t num_cap_bytes, char* filename, adi_hsx_extras_jesd_param_t jesd_params_cap, int num_links_cap) {
    /* Allocate capture buffer */
    uint8_t *cap_buff = (uint8_t *)malloc(num_cap_bytes);

    /* Start a capture */
    adi_ads9_start_capture();
    ads9_wait_us(NULL, 500000);   // Give time for cap to complete

    /* Get cap status */
    uint32_t cap_complete, cap_status;
    adi_ads9_capture_status_get(&cap_complete, &cap_status);
    printf("cap_complete: %d\n", cap_complete);

    /* Read raw ads9 capture memory */
    read_ads9_capture_memory(0, num_cap_bytes, cap_buff);

    /* Print current working dir - this is where capture files are located on MicroZed */
    char cwd[1024];
    getcwd(cwd, 1024);
    printf("Capture file location: %s\n", cwd);

    /* Extract per virtual converter capture samples and save to files */
    adi_hsx_extras_extract_capture_to_file(num_cap_bytes, cap_buff, num_links_cap, jesd_params_cap, filename);

    // Allocate mem for per-converter capture samples
    uint16_t (*cap_samples)[jesd_params_cap.m][num_samples] = malloc(sizeof(uint16_t) * jesd_params_cap.m * num_links_cap * num_samples);
    adi_hsx_extras_extract_capture(num_cap_bytes, cap_buff, num_links_cap, jesd_params_cap.m, num_samples, cap_samples, jesd_params_cap);
    /* Print rms value of tone */
    double rms = 0.0;
    for (int l=0; l<num_links_cap; l++) {
        printf("Link %d: \n", l);
        for (int m=0; m<jesd_params_cap.m; m++) {
            rms = calc_rms((int16_t *) cap_samples[l][m], num_samples);
            printf("%7.1f  ", rms);
        }
        printf("\n");
    } 
    free(cap_samples);

    free(cap_buff);
    return API_CMS_ERROR_OK;
}

int32_t load_rcap_rx(uint32_t num_cap_bytes, char* filename, adi_hsx_extras_jesd_param_t jesd_params_cap, int num_links_cap) {
    /* Allocate capture buffer */
    uint8_t *cap_buff = (uint8_t *)malloc(num_cap_bytes);

    /* Start a capture */
    adi_ads9_start_capture();
    ads9_wait_us(NULL, 500000);   // Give time for cap to complete

    /* Get cap status */
    uint32_t cap_complete, cap_status;
    adi_ads9_capture_status_get(&cap_complete, &cap_status);
    printf("cap_complete: %d\n", cap_complete);

    /* Read raw ads9 capture memorY */
    read_ads9_capture_memory(0, num_cap_bytes, cap_buff);

    /* Print current working dir - this is where capture files are located on MicroZed */
    char cwd[1024];
    getcwd(cwd, 1024);
    printf("Capture file location: %s\n", cwd);

    /* Save the raw ads9 memory for file */
    adi_hsx_raw_data_capture_to_file(num_cap_bytes, cap_buff, filename, 4);

    /* Print first few raw capture values from ads9 */
    for (int i = 0; i < 8; i++) {
        printf("0x%08X\n", (*(uint32_t *)&cap_buff[i * 4]));
    }
    free(cap_buff);
    return API_CMS_ERROR_OK;
}

int32_t atof_check(char *arg, double *value) {
    char *input = arg;
    while (*input != '\0') {
        if (!isdigit(*input) && (*input != '.') && (*input != '-') && (*input != '+') && (*input != 'e' && *input != 'E')) {
            return -1;
        }
        input++;
    }
    *value = atof(arg);
    return API_CMS_ERROR_OK;
}

int32_t atoi_check(char *arg, int32_t *value) {
    char *input = arg;
    while (*input != '\0') {
        if (!isdigit(*input) && *input != '-' && *input != '+') {
            return -1;
        }
        input++;
    }
    *value = atoi(arg);
    return API_CMS_ERROR_OK;
}

int32_t vec_len_from_files(int argc, char *argv[], int arg_idx) {

    int nfiles = argc - arg_idx;
    int n, nsamples;
    char *filename;

    if (nfiles < 1) {
        printf("No files specified\n");
        return -1;
    }
    
    for (int i=0; i<nfiles; i++)
    {
        filename = argv[arg_idx + i];
        if ( (n = calc_nsamples_file(filename)) < 0 ) {
            return -1;
        }

        if (i == 0) {
            nsamples = n;
        } else if (nsamples != n) {
            printf("All TX files must have same vector length. Last file read: %s. (%d != %d)\n", filename, nsamples, n);
            return -1;
        }
    }
    
    return nsamples;
}

int32_t calc_nsamples_file(char *filename)
{
    int ch;
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        printf("fopen failed to open the file %s\n", filename);
        return -1;
    }

    int32_t j = 0;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            j++;
        }
    }
    fclose(fp);    
    return j;
}

static void print_usage() {
    printf("Usage:");
    printf("\t ad9986_xtra tx --stone <fd> <ft> [veclen = 8192] [backoff = 0.0] [nbits = 16]\n");
    printf("\t ad9986_xtra tx --file <file0> [<file1> <file2> . . .]\n");
    printf("\t ad9986_xtra rx --cap <file> [nsamples = 8192]\n");
    printf("\t ad9986_xtra rx --rcap <file> [nsamples = 8192]\n");  
    printf("\t ad9986_xtra -h | --help\n");
}

static void print_help() {
    printf("Usage:");
    printf("\t ad9986_xtra tx --stone <fd> <ft> [veclen = 8192] [backoff = 0.0] [nbits = 16]\n");
    printf("\t ad9986_xtra tx --file <file0> [<file1> <file2> . . .]\n");
    printf("\t ad9986_xtra rx --cap <file> [nsamples = 8192]\n");
    printf("\t ad9986_xtra rx --rcap <file> [nsamples = 8192]\n");
    printf("\t ad9986_xtra -h | --help\n");

    printf("Options:\n");
    printf("\t -h --help             show this screen\n");
    printf("\t tx                    transfer\n");
    printf("\t --stone               generate single tone\n");
    printf("\t <fd>                  data rate\n");
    printf("\t <ft>                  tone frequency\n");
    printf("\t veclen                vector length per virtual converter [default: 8192]\n");
    printf("\t backoff               vector backoff in db [default: 0.0]\n");
    printf("\t nbits                 number of bits in sample [default: 16]\n");
    printf("\t --file                generate from file\n");
    printf("\t <file0>               required text file\n");
    printf("\t <file1> <file2> . . . optional text files\n");
    printf("\t rx                    receive\n");
    printf("\t --cap                 capture to separate files\n");
    printf("\t --rcap                capture raw data to file\n");
    printf("\t <file>                output file prefix\n");
    printf("\t nsamples              number of samples to capture [default: 8192]\n");
}

static double calc_rms(int16_t *x, int num_samples)
{
    double n = 0;
    double sum = 0.0;
    double sum_squares = 0.0;
    // double mean = 0.0;
    double sd = 0.0;

    for (int i=0; i<num_samples; i++) {
        n += 1.0;
        sum += x[i];
        sum_squares += x[i]*x[i];

        // mean = sum/n;
        sd = (n == 1) ? 0.0 : sqrt((sum_squares - (sum*sum/n))/ (n - 1.0));

        // printf("mean=%f    sd=%f\n", mean, sd);
    }

    return sd;
}