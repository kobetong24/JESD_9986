/*!
 * @brief     ADS9 API Source File
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

/*============= I N C L U D E S ============*/
#include "adi_ads9.h"
#include "ads9.h"
#include "cdmaapi.h"

/*============= D E F I N E S ==============*/
#define SIZE_1MB          (1024 * 1024)

static uint32_t g_address;
static uint32_t g_filetype;
static uint32_t g_filesize;
static char     g_filename[64];
static FILE    *g_fp;

/*============= C O D E ====================*/
int32_t adi_ads9_reg_set(uint32_t reg_addr, uint32_t data)
{
    return ads9_axi_reg_write32(reg_addr, data);
}

int32_t adi_ads9_reg_get(uint32_t reg_addr, uint32_t *out_data)
{
    return ads9_axi_reg_read32(reg_addr, out_data);
}

int32_t adi_ads9_rev_get(uint32_t *rev)
{
    ads9_axi_reg_read32(0x002, rev);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_ver_get(uint32_t *ver)
{
    uint32_t ver1, ver2;
    ads9_axi_reg_read32(0x100, &ver1);
    ads9_axi_reg_read32(0x101, &ver2);
    *ver = (ver1 << 16) + ver2;
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_sw_ver_get(uint32_t *sw_ver)
{
    ads9_axi_reg_read32(0x102, sw_ver);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_config_jesd(adi_cms_jesd_param_t jrx_param[2], adi_cms_jesd_param_t *jtx_param)
{
    uint8_t  i, jrx_e0 = 1, jrx_e1 = 1, jtx_e = 1;
    uint32_t jrx_lscrparam0 = (jrx_param[0].jesd_scr << 7) | (jrx_param[0].jesd_l - 1);
    uint32_t jrx_lscrparam1 = (jrx_param[1].jesd_scr << 7) | (jrx_param[1].jesd_l - 1);
    uint32_t jtx_lscrparam  = (jtx_param->jesd_scr   << 7) | (jtx_param->jesd_l   - 1);
    uint32_t jrx_np_param0  = (((jrx_param[0].jesd_np - 1) & 0x1F) << 3);
    uint32_t jrx_np_param1  = (((jrx_param[1].jesd_np - 1) & 0x1F) << 3);
    uint32_t jtx_np_param   = (((jtx_param->jesd_np   - 1) & 0x1F) << 3);
    uint32_t jtx_link_param = (((jtx_param->jesd_s    - 1) & 0x1F) << 19) | (((jtx_param->jesd_n - 1) & 0x1F) << 9);

    /* disable fpga capture */
    adi_ads9_stop_capture();

    /* disable fpga pattern transmit */
    adi_ads9_stop_transmit();

    /* disable bidir */
    adi_ads9_stop_bidir();

    /* no skip rx link init */
    adi_ads9_skip_rx_link_init_set(0);
    
    /* clear prbs test config in fpga */
    for(i = 0; i < 8; i++)
        ads9_axi_reg_write32(0x560 + i, 0);     

    /* set fpga jesd rx link param */
    if (jrx_param[0].jesd_l > 0) {
        ads9_axi_reg_write32(0x11D, jrx_param[0].jesd_subclass);
        if (jrx_param[0].jesd_duallink == 0) {
            ads9_axi_reg_write32(0x11F, 0x0E);
            ads9_axi_reg_write32(0x120, jrx_lscrparam0);
            ads9_axi_reg_write32(0x121, jrx_np_param0);
            ads9_axi_reg_write32(0x122, jrx_param[0].jesd_m - 1);
            ads9_axi_reg_write32(0x123, jrx_param[0].jesd_f - 1);
            ads9_axi_reg_write32(0x124, jrx_param[0].jesd_k - 1);
            jrx_e0 = ((jrx_param[0].jesd_k * jrx_param[0].jesd_f) / (32 * 8));
            ads9_axi_reg_write32(0x203, jrx_e0 - 1);
        } else {
            ads9_axi_reg_write32(0x11F, 0x00);
            ads9_axi_reg_write32(0x120, jrx_lscrparam0 | (jrx_lscrparam1 << 8));
            ads9_axi_reg_write32(0x121, jrx_np_param0  | (jrx_np_param1  << 8));
            ads9_axi_reg_write32(0x122, (jrx_param[0].jesd_m - 1) | ((jrx_param[1].jesd_m - 1) << 8));
            ads9_axi_reg_write32(0x123, (jrx_param[0].jesd_f - 1) | ((jrx_param[1].jesd_f - 1) << 8));
            ads9_axi_reg_write32(0x124, (jrx_param[0].jesd_k - 1) | ((jrx_param[1].jesd_k - 1) << 8));
            jrx_e0 = ((jrx_param[0].jesd_k * jrx_param[0].jesd_f) / (32 * 8));
            jrx_e1 = ((jrx_param[1].jesd_k * jrx_param[1].jesd_f) / (32 * 8));
            ads9_axi_reg_write32(0x203, (jrx_e0 - 1) | ((jrx_e1 - 1) << 8));
        }
        ads9_axi_reg_write32(0x943, (jrx_param[0].jesd_jesdv < 2) ? 0: 1);
    } else {
        ads9_axi_reg_write32(0x11D, 0);
        ads9_axi_reg_write32(0x11F, 0x0E);
        ads9_axi_reg_write32(0x120, 0);
        ads9_axi_reg_write32(0x121, 0);
        ads9_axi_reg_write32(0x122, 0);
        ads9_axi_reg_write32(0x123, 0);
        ads9_axi_reg_write32(0x124, 0);
        ads9_axi_reg_write32(0x203, 0);
    }

    /* set fpga jesd tx link param */
    if (jtx_param->jesd_l > 0 ) {
        ads9_axi_reg_write32(0x51D, jtx_param->jesd_subclass);
        jtx_e = ((jtx_param->jesd_k * jtx_param->jesd_f) / (32 * 8));
        if (jtx_param->jesd_duallink == 0) {
            ads9_axi_reg_write32(0x51F, 0x0E);
            ads9_axi_reg_write32(0x520, jtx_lscrparam);
            ads9_axi_reg_write32(0x521, jtx_np_param);
            ads9_axi_reg_write32(0x522, jtx_param->jesd_m - 1);
            ads9_axi_reg_write32(0x523, jtx_param->jesd_f - 1);
            ads9_axi_reg_write32(0x524, jtx_param->jesd_k - 1);
            ads9_axi_reg_write32(0x525, jtx_link_param);
            ads9_axi_reg_write32(0x603, jtx_e - 1);
        } else {
            ads9_axi_reg_write32(0x51F, 0x0C);
            ads9_axi_reg_write32(0x520, jtx_lscrparam | (jtx_lscrparam << 8));
            ads9_axi_reg_write32(0x521, jtx_np_param | (jtx_np_param << 8));
            ads9_axi_reg_write32(0x522, (jtx_param->jesd_m - 1) | ((jtx_param->jesd_m - 1) << 8));
            ads9_axi_reg_write32(0x523, (jtx_param->jesd_f - 1) | ((jtx_param->jesd_f - 1) << 8));
            ads9_axi_reg_write32(0x524, (jtx_param->jesd_k - 1) | ((jtx_param->jesd_k - 1) << 8));
            ads9_axi_reg_write32(0x525, jtx_link_param);
            ads9_axi_reg_write32(0x526, jtx_link_param);
            ads9_axi_reg_write32(0x603, (jtx_e - 1) | ((jtx_e - 1) << 8));
        }
        ads9_axi_reg_write32(0x943, (jtx_param->jesd_jesdv < 2) ? 0: 1);
    } else {
        ads9_axi_reg_write32(0x51D, 0);
        ads9_axi_reg_write32(0x51F, 0x0E);
        ads9_axi_reg_write32(0x520, 0);
        ads9_axi_reg_write32(0x521, 0);
        ads9_axi_reg_write32(0x522, 0);
        ads9_axi_reg_write32(0x523, 0);
        ads9_axi_reg_write32(0x524, 0);
        ads9_axi_reg_write32(0x525, 0);
        ads9_axi_reg_write32(0x526, 0);
        ads9_axi_reg_write32(0x603, 0);
    }

    return API_CMS_ERROR_OK;
}
int32_t adi_ads9_jesd_tx_lane_driver_config(uint8_t lanes, uint8_t post_cursor, uint8_t pre_cursor, uint32_t diff_ctrl)
{
 
    /* set fpga jtx driver control */
    uint32_t driver_config = 0x0;
    driver_config |= ((post_cursor & 0x1F) << 16); 
    driver_config |= ((post_cursor & 0x1F) << 8);
    driver_config |= ((diff_ctrl & 0x1F) << 0);    /*0x1C == 1V*/
    for (int i = 0; i < 8; i++) {
        if (0x1 << i & lanes) {
            ads9_axi_reg_write32(0x550 + i, driver_config);
        }
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_config_jesd_get(adi_cms_jesd_param_t *jrx_param, adi_cms_jesd_param_t *jtx_param)
{
    uint32_t reg32;

    if (jrx_param != NULL) {
        /* get fpga jesd rx link params (ADC) */
        ads9_axi_reg_read32(0x11D, &reg32);
        jrx_param[0].jesd_subclass = ((reg32 >> 0) & 0x01);
        jrx_param[1].jesd_subclass = ((reg32 >> 0) & 0x01);
        ads9_axi_reg_read32(0x11F, &reg32);
        jrx_param[0].jesd_duallink = ((reg32 & 0x0F) == 0x00) ? 1 : 0;
        jrx_param[1].jesd_duallink = ((reg32 & 0x0F) == 0x00) ? 1 : 0;
        ads9_axi_reg_read32(0x120, &reg32);
        jrx_param[0].jesd_l = ((reg32 >> 0) & 0x01F) + 1;
        jrx_param[0].jesd_scr = (reg32 >> 7) & 0x01;
        jrx_param[1].jesd_l = ((reg32 >> 8) & 0x01F) + 1;
        jrx_param[1].jesd_scr = (reg32 >> 15) & 0x01;
        ads9_axi_reg_read32(0x121, &reg32);
        jrx_param[0].jesd_np = ((reg32 >> 3) & 0x1F) + 1;
        jrx_param[1].jesd_np = ((reg32 >> 11) & 0x1F) + 1;
        ads9_axi_reg_read32(0x122, &reg32);
        jrx_param[0].jesd_m = ((reg32 >> 0) & 0xFF) + 1;
        jrx_param[1].jesd_m = ((reg32 >> 8) & 0xFF) + 1;
        ads9_axi_reg_read32(0x123, &reg32);
        jrx_param[0].jesd_f = ((reg32 >> 0) & 0xFF) + 1;
        jrx_param[1].jesd_f = ((reg32 >> 8) & 0xFF) + 1;
        ads9_axi_reg_read32(0x124, &reg32);
        jrx_param[0].jesd_k = ((reg32 >> 0) & 0x1F) + 1;
        jrx_param[1].jesd_k = ((reg32 >> 8) & 0x1F) + 1;
        ads9_axi_reg_read32(0x943, &reg32);
        jrx_param[0].jesd_jesdv = ((reg32 >> 0) & 0xFFFF) < 1 ? 1 : 2;
    }

    if (jtx_param != NULL) {
        /* get fpga jesd tx link params (DAC) */
        ads9_axi_reg_read32(0x51D, &reg32);
        jtx_param->jesd_subclass = ((reg32 >> 0) & 0x01);
        ads9_axi_reg_read32(0x51F, &reg32);
        jtx_param->jesd_duallink = ((reg32 & 0x0F) == 0x0C) ? 1 : 0;
        ads9_axi_reg_read32(0x520, &reg32);
        jtx_param->jesd_l = ((reg32 >> 0) & 0x01F) + 1;
        jtx_param->jesd_scr = (reg32 >> 7) & 0x01;
        ads9_axi_reg_read32(0x521, &reg32);
        jtx_param->jesd_np = ((reg32 >> 3) & 0x1F) + 1;
        ads9_axi_reg_read32(0x522, &reg32);
        jtx_param->jesd_m = ((reg32 >> 0) & 0xFF) + 1;
        ads9_axi_reg_read32(0x523, &reg32);
        jtx_param->jesd_f = ((reg32 >> 0) & 0xFF) + 1;
        ads9_axi_reg_read32(0x524, &reg32);
        jtx_param->jesd_k = ((reg32 >> 0) & 0x1F) + 1;
        ads9_axi_reg_read32(0x525, &reg32);
        jtx_param->jesd_n = ((reg32 >> 9) & 0x1F) + 1;
        jtx_param->jesd_s = ((reg32 >> 19) & 0x1F) + 1;
        ads9_axi_reg_read32(0x943, &reg32);
        jtx_param->jesd_jesdv = ((reg32 >> 0) & 0xFFFF) < 1 ? 1 : 2;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_start_capture()
{
    ads9_axi_reg_write32(0x140, 0x02);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_stop_capture()
{
    ads9_axi_reg_write32(0x140, 0x08);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_capture_size_set(uint32_t size)
{
    ads9_axi_reg_write32(0x143, size); /* result in size * 64Kbytes */
    ads9_axi_reg_write32(0x144, 0);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_start_bidir()
{
    ads9_axi_reg_write32(0x947, 0x02);
    return API_CMS_ERROR_OK;    
}

int32_t adi_ads9_stop_bidir()
{
    ads9_axi_reg_write32(0x947, 0x08);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_start_transmit()
{    
    ads9_axi_reg_write32(0x540, 0x03);
    sleep(1);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_stop_transmit()
{
    ads9_axi_reg_write32(0x540, 0x08);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_skip_rx_link_init_set(uint8_t value)
{
    uint32_t reg_val;
    ads9_axi_reg_read32(0x106, &reg_val);    
    reg_val = reg_val & (~(0x400));
    reg_val = reg_val | (value << 10);
    ads9_axi_reg_write32(0x106, reg_val);
    return API_CMS_ERROR_OK;    
}

int32_t adi_ads9_capture_status_get(uint32_t *capture_complete, uint32_t *capture_state)
{
    uint32_t status;
    ads9_axi_reg_read32(0x1040, &status); /* bit0: capture complete, bit1: running capture, bit2~9: data capture state */
    *capture_complete = status & 0x1;
    *capture_state = (status >> 2) & 0xff;
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_config_jtx_prbs(adi_cms_jesd_prbs_pattern_e prbs)
{
    uint32_t bf_val = 0, i = 0;
    uint32_t reg_val;
    if (prbs == PRBS_NONE)
        bf_val = 0;
    else if (prbs == PRBS7)
        bf_val = 1;
    else if (prbs == PRBS9)
        bf_val = 2;
    else if (prbs == PRBS15)
        bf_val = 3;
    else if (prbs == PRBS31)
        bf_val = 5;
    else
        bf_val = 0;

    for (i = 0; i < 0x10; i ++) {    
        ads9_axi_reg_read32(0x560 + i, &reg_val);
        reg_val = reg_val & (~(0xf));
        reg_val = reg_val | bf_val;
        ads9_axi_reg_write32(0x560 + i, reg_val);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_pattern_len_set(uint64_t len)
{
    ads9_axi_reg_write32(0x535, (uint32_t)((len) & 0xffffffff));
    ads9_axi_reg_write32(0x536, (uint32_t)((len) >> 32));    
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_pattern_addr_set(uint32_t addr)
{
    ads9_axi_reg_write32(0x531, 0x00); /* repeated pattern */
    ads9_axi_reg_write32(0x533, addr);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_jtx_err_status_get(uint8_t *error)
{
    uint32_t status;
    ads9_axi_reg_read32(0x54E, &status);
    *error = (uint8_t)(status & 0x4F);
    return API_CMS_ERROR_OK;   
}

int32_t adi_ads9_jrx_err_status_get(uint8_t *error)
{
    uint32_t status;
    ads9_axi_reg_read32(0x14E, &status);
    *error  = (uint8_t)(status & 0x1F);
    ads9_axi_reg_read32(0x205, &status);
    *error |= (uint8_t)((status & 0x30) << 1);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_ad9528_vcxo_select_set(uint8_t value)
{
    /* value: 0 - external clk, 1 - fmc+ clk */
    uint32_t reg_val;
    ads9_axi_reg_read32(0x10a, &reg_val);   
    reg_val = reg_val & (~(0x4));
    reg_val = reg_val | (value << 2);
    ads9_axi_reg_write32(0x10a, reg_val);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_mgt_ref_clk_select_set(uint8_t value)
{
    /* value: 0 - external clk, 1 - fmc clk */
    uint32_t reg_val;
    ads9_axi_reg_read32(0x942, &reg_val);    
    reg_val = reg_val & (~(0x1f));
    reg_val = reg_val | (value);
    ads9_axi_reg_write32(0x942, reg_val);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_gbl_clk_select_set(uint8_t value)
{
    /* value: 0 - external clk, 1 - fmc clk */
    uint32_t reg_val;
    ads9_axi_reg_read32(0x942, &reg_val);    
    reg_val = reg_val & (~(0xf00));
    reg_val = reg_val | (value << 8);
    ads9_axi_reg_write32(0x942, reg_val);
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_spi_freq_adjust(uint64_t freq)
{
    uint64_t mindiff = 50000;
	uint64_t freq_in = freq*2;
    int clkoutbuf_mult = 0;
    int clkoutbuf_frac = 0;
    int divclk_divide = 0;
    int clkout4_divide = 0;
    int clkout6_divide = 0;
	for(int m = 16; m <= 512; m++){
		for(int d = 1; d < 106; d++){
			uint64_t vcoFreq = (125e5*(uint64_t)m)/((uint64_t)d);
			if(vcoFreq >= 600e6 && vcoFreq <= 1600e6){
				for(int d4 = 1; d4 < 128; d4++){
					for(int d6 = 1; d6 < 128; d6++){
						uint64_t opFreq = vcoFreq/((uint64_t)(d4)*(uint64_t)(d6));
						uint64_t diff = (freq_in - opFreq >= 0) ? freq_in - opFreq : opFreq - freq_in;
						if(diff < mindiff){
							mindiff = diff;
							clkoutbuf_mult = m/8;
							clkoutbuf_frac = ((m%8)*125) | (1<<10);
							divclk_divide = d;
							clkout4_divide = d4;
							clkout6_divide = d6;
						}
					}
				}
			}
		}
	}
    ads9_axi_reg_write32(0x903, clkoutbuf_mult);
    ads9_axi_reg_write32(0x904, clkoutbuf_frac);
    ads9_axi_reg_write32(0x905, divclk_divide);
    ads9_axi_reg_write32(0x906, clkout4_divide);
    ads9_axi_reg_write32(0x907, clkout6_divide);
    ads9_axi_reg_write32(0x902, 0x001);
    return API_CMS_ERROR_OK;    
}

int32_t adi_ads9_transfer_config(adi_ads9_xfer_meta_t *meta)
{
    // printf("adi_ads9_transfer_config(...): file type is %d, file size is %dBytes, address is 0x%x.\r\n", meta->filetype, meta->filesize, meta->address);
    g_address = meta->address;
    g_filetype = meta->filetype;
    g_filesize = meta->filesize;
    if (meta->filetype == 0x02) {
        memset(g_filename, 0, sizeof(g_filename));
        memcpy(g_filename, meta->filename, meta->filenameLen);
    }
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_read(uint8_t *data, int32_t data_size, adi_ads9_data_xfer_e *xfer_type)
{
    uint32_t loop, off;
    uint8_t  padding[SIZE_1MB];
    uint8_t *ptr = data;

    switch (*xfer_type) {
        case START:
            // printf("adi_ads9_read: START, size is %dBytes.\r\n", data_size);
            if (g_filetype == 0x01) { /* data vector */
                if (cdma_start(g_address, g_filesize, HMCTODDR) != 0) {
                    printf("DMA Start error.\r\n");
                }
            } else { /* image file */                
                g_fp = fopen(g_filename, "rb+");
            }
            break;
        case NEXT:
            // printf("adi_ads9_read: NEXT, size is %dBytes\r\n", data_size);
            if (g_filetype == 0x01) { /* data vector */
                off = data_size % SIZE_1MB;
                loop = off ? (data_size / SIZE_1MB + 1) : (data_size / SIZE_1MB);
                do {
                    if ((loop == 1) && (off != 0)) {
                        if (cdma_inprogress(HMCTODDR, padding, SIZE_1MB) != 0) {
                            printf("DMA in progress error.\r\n");
                        }
                        memcpy(ptr, padding, off);
                    }
                    else {
                        if (cdma_inprogress(HMCTODDR, ptr, SIZE_1MB) != 0) {
                            printf("DMA in progress error.\r\n");
                        }
                    }
                    ptr += SIZE_1MB;
                } while (-- loop > 0);
            } else { /* image file */               
               fread(data, sizeof(char), data_size, g_fp);
            }
            break;
        case END:
            // printf("adi_ads9_read: END, size is %dBytes.\r\n", data_size);
            if (g_filetype == 0x01) { /* data vector */
                if (cdma_end() != 0) {
                    printf("DMA End error.\r\n");
                }
            }
            else { /* image file */
                fclose(g_fp);
            }
            break;
        default:
            printf("adi_ads9_read: unknown xfer_type.\r\n");
            break;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_write(uint8_t *data, int32_t data_size, adi_ads9_data_xfer_e *xfer_type)
{
    uint32_t loop, off;
    uint8_t  padding[SIZE_1MB];
    uint8_t *ptr = data;

    switch (*xfer_type) {
        case START:
            // printf("adi_ads9_write: START, size is %dBytes.\r\n", data_size);
            if (g_filetype == 0x01) { /* data vector */                
                if (cdma_start(g_address, g_filesize, DDRTOHMC) != 0) {
                    printf("DMA Start error.\r\n");
                }
            } else { /* image file */
                if (access(g_filename, F_OK) != -1) {
                    remove(g_filename);
                }
                g_fp = fopen(g_filename, "wb+");
            }
            break;
        case NEXT:
            // printf("adi_ads9_write: NEXT, size is %dBytes. 0x%x. 0x%x\r\n", data_size, data[0], data[1]);
            if (g_filetype == 0x01) { /* data vector */
                off = data_size % SIZE_1MB;
                loop = off ? (data_size / SIZE_1MB + 1) : (data_size / SIZE_1MB);
                do {
                    if ((loop == 1) && (off != 0)) {
                        memcpy(padding, ptr, off);
                        if (cdma_inprogress(DDRTOHMC, padding, SIZE_1MB) != 0) {
                            printf("DMA Inprogress error.\r\n");
                        }
                    }
                    else {
                        if (cdma_inprogress(DDRTOHMC, ptr, SIZE_1MB) != 0) {
                            printf("DMA Inprogress error.\r\n");
                        }
                    }
                    ptr += SIZE_1MB;
                } while (-- loop > 0);
            } else { /* image file */
               fwrite(data, sizeof(char), data_size, g_fp);
            }
            break;
        case END:
            // printf("adi_ads9_write: END, size is %dBytes.\r\n", data_size);
            if (g_filetype == 0x01) { /* data vector */
                if (cdma_end() != 0) {
                    printf("DMA End error.\r\n");
                }
            } else { /* image file */
                fclose(g_fp);
            }
            break;
        default:
            printf("adi_ads9_write: unknown xfer_type.\r\n");
            break;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_sysref_config(uint8_t sysref_src)
{
    uint32_t reg_val;
    if (sysref_src > 2) {
        return API_CMS_ERROR_INVALID_PARAM;
        /* 0:Use SYSREF through FMC, 1:Use AD9528-generated or SMA sysref */
    }
    ads9_axi_reg_read32(0x942, &reg_val);
    reg_val = reg_val & (~(0x30000));
    reg_val |= (sysref_src << 16);
    ads9_axi_reg_write32(0x942, reg_val); 
    return API_CMS_ERROR_OK;
}

int32_t adi_ads9_bidir_sync_set(uint8_t enable, uint32_t delay)
{
    uint32_t reg_val;
    if (enable > 1) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    if (delay > 0xffffff) {
        return API_CMS_ERROR_INVALID_PARAM;
    }
    reg_val = (enable << 24) | (delay & 0xffffff); 
    ads9_axi_reg_write32(0x147, reg_val);
    return API_CMS_ERROR_OK;
}

/*! @} */
