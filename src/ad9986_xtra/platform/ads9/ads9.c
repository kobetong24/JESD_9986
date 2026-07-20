/*!
 * @brief     ADS9 platform configuration and control
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

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>    
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "ads9.h"

/*============= D E F I N E S ==============*/
/* log file limit */
#define MAX_FILE_NAME_LENGTH    64
#define MAX_LOG_LINE_LENGTH     1000
#define MAX_LOG_NUM_LINES       250000

/* sysbuf length for spi init */
#define SYSFS_DATALEN           32

/* definition of spi devices' path */
#define SPI_CONTROLLERDEV       "/dev/uio1"
#define SPI_CHIPSELECTDEV       "/dev/uio2"
#define AXI_JESD204DEV          "/dev/uio3"
#define I2C_CONTROLLERDEV       "/dev/uio6"
#define SPI_CONTROLLER_SYSNAME  "/sys/class/uio/uio1/name"
#define SPI_CHIPSELECT_SYSNAME  "/sys/class/uio/uio2/name"
#define AXI_JESD204DEV_SYSNAME  "/sys/class/uio/uio3/name"
#define I2C_CONTROLLER_SYSNAME  "/sys/class/uio/uio6/name"

/* definition of spi device name in Linux */
#define SPI_CONTROLLERNAME      "uio_spi_controller"
#define SPI_CHIPSELECTNAME      "uio_spi_gpio"
#define AXI_JESD204NAME         "uio_axi_fpga"
#define I2C_CONTROLLERNAME      "uio_i2c_controller"

/* definition of memory size */
#define SPI_CONTROLLERSIZE       0x01000
#define SPI_CHIPSELECTSIZE       0x01000
#define AXI_JESD204SIZE          0x10000
#define I2C_CONTROLLERSIZE       0x01000

/* definition of register offset for spi controller */
#define SPIM_REVID               0x000
#define SPIM_RBUF_RDDATA         0x004
#define SPIM_BUF_STATUS          0x008
#define SPIM_XFER_STATUS         0x00C
#define SPIM_WBUF_WADDR          0x040
#define SPIM_WBUF_RADDR          0x044
#define SPIM_RBUF_WADDR          0x048
#define SPIM_RBUF_RADDR          0x04C
#define SPIM_CTL                 0x080
#define SPIM_CS_ENABLE           0x084
#define SPIM_CS_POLARITY         0x088
#define SPIM_XFER_COUNT          0x08C
#define SPIM_XFER_DELAY          0x090
#define SPIM_CS_COUNT            0x094
#define SPIM_WR_DELAY_COUNT      0x098
#define SPIM_WR_BYTE_COUNT       0x09C
#define SPIM_RD_DELAY_COUNT      0x0A0
#define SPIM_RD_BYTE_COUNT       0x0A4
#define SPIM_WBUF_WRDATA         0x0A8
#define SPIM_XFER_START          0x0AC
#define SPIM_SCRATCH             0x3FC

/* definition of AXI JESD204 register offsets */
#define AXI_FPGA_MISC_1_REG      0x10B
#define AXI_FPGA_DUT_RSTB        0x002
#define AXI_FPGA_DUT_TX_EN       0x00C
#define AXI_FPGA_DUT_RX_EN       0x030

/* definition of register offset for i2c controller */
#define I2CM_RX_FIFO             0x001
#define I2CM_TRANS_RESET         0x002
#define I2CM_STATUS              0x003
#define I2CM_TRANS_SETTINGS      0x010
#define I2CM_SLAVE_ADDR          0x011
#define I2CM_REG_ADDR            0x012
#define I2CM_READ_LEN            0x013
#define I2CM_TX_FIFO             0x014
#define I2CM_TRANS_CTRL          0x015
#define I2CM_ERRORS              0x020

/**
 * @brief data structure to hold customer user data content
 */
typedef struct adi_hal_cfg {   
    uint8_t   spi_cs;
    uint8_t   spi_wire;
    uint8_t   spi_msb;
} adi_hal_cfg_t;

/*============= D A T A ====================*/
/* definition of device handler */
static int      g_spi_ctrl_fd;
static int      g_spi_cs_fd;
static int      g_axi_jesd204_fd;
static int      g_i2c_ctrl_fd;

/* definition of device memory for register access */
static void*    g_spi_ctrl_mem;
static void*    g_spi_cs_mem;
static void*    g_axi_jesd204_mem;
static void*    g_i2c_ctrl_mem;

/* spi access counters */
static uint32_t g_spi_rd_cnt, g_spi_wr_cnt;

/* log file */
static FILE    *g_log_fd = NULL;
static char     g_log_file_name[MAX_FILE_NAME_LENGTH];
static uint32_t g_log_line_num;

/* time base */
static struct   timespec ts_start;

/*============= C O D E ====================*/
static int32_t ads9_spi_reg_read32(uint32_t reg_offset, uint32_t *out_data);
static int32_t ads9_spi_reg_write32(uint32_t reg_offset, uint32_t data);
static int32_t ads9_spi_periph_sel(uint32_t ad9986_o_hmc);
static int32_t ads9_spi_wait_idle(void);
static int32_t ads9_spi_bridge_detect(void);
static int32_t ads9_spi_init(void);
static int32_t ads9_spi_open(void);
static int32_t ads9_spi_configure(uint32_t wire3, uint32_t msb);
static int32_t ads9_spi_read(uint32_t slave_addr, uint32_t *data, uint32_t data_bits);
static int32_t ads9_spi_read_7175(uint32_t slave_addr, uint32_t *data, uint32_t data_bits);
static int32_t ads9_spi_write(uint32_t slave_addr, uint32_t data, uint32_t data_bits);
static int32_t ads9_spi_write_7175(uint32_t slave_addr, uint32_t data);
static int32_t ads9_spi_close(void);
static int32_t ads9_log_open();
static int32_t ads9_log_close();

static int32_t ads9_spi_reg_read32(uint32_t reg_offset, uint32_t *out_data)
{
    *out_data = *((uint32_t *)((uint32_t)g_spi_ctrl_mem + reg_offset));
    return API_CMS_ERROR_OK;
}

static int32_t ads9_spi_reg_write32(uint32_t reg_offset, uint32_t data)
{
   *((uint32_t *)((uint32_t)g_spi_ctrl_mem + reg_offset)) = data;
    return API_CMS_ERROR_OK;
}

int32_t ads9_axi_reg_read32(uint32_t reg_offset, uint32_t *out_data)
{
    *out_data = *((uint32_t *)((uint32_t)g_axi_jesd204_mem + 4 * reg_offset));
    return API_CMS_ERROR_OK;
}

int32_t ads9_axi_reg_write32(uint32_t reg_offset, uint32_t data)
{
   *((uint32_t *)((uint32_t)g_axi_jesd204_mem + 4 * reg_offset)) = data;
    return API_CMS_ERROR_OK;
}

static int32_t ads9_spi_periph_sel(uint32_t ad9986_o_hmc)
{
    /* ad9986_o_hmc: 1 - HMC7044, 0 - ad9986 */
    return ads9_axi_reg_write32(0x901, ad9986_o_hmc);
}

int32_t ads9_i2c_reg_read32(uint32_t reg_offset, uint32_t *out_data)
{
    *out_data = *((uint32_t *)((uint32_t)g_i2c_ctrl_mem + 4 * reg_offset));
    return API_CMS_ERROR_OK;
}

int32_t ads9_i2c_reg_write32(uint32_t reg_offset, uint32_t data)
{
   *((uint32_t *)((uint32_t)g_i2c_ctrl_mem + 4 * reg_offset)) = data;
    return API_CMS_ERROR_OK;
}

static int32_t ads9_spi_wait_idle(void)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t cnt, start, status;
    cnt = 0;
    do {
        ads9_spi_reg_read32(SPIM_XFER_START,  &start);
        ads9_spi_reg_read32(SPIM_XFER_STATUS, &status);
        if (((start & 0x1) == 0) && ((status & 0x1) == 0)) {
          err = API_CMS_ERROR_OK;
        }
    } while ((++ cnt < 1000) && (err != 0)); /* 5~6 for 5MHz SPI on MicroZed */
    return err;
}

static int32_t ads9_spi_bridge_detect(void)
{
    int32_t err = API_CMS_ERROR_ERROR;
    int32_t gpio_fd, bdg0_fd, bdg1_fd;
    char buf[2];

    /* configure GPIO */
    if (access("/sys/class/gpio/gpio970/value", F_OK) != 0) {
        system("echo 970 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo out > /sys/class/gpio/gpio970/direction");
        system("echo 0 > /sys/class/gpio/gpio970/value");
    }
    if (access("/sys/class/gpio/gpio969/value", F_OK) != 0) {
        system("echo 969 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo out > /sys/class/gpio/gpio969/direction");
        system("echo 1 > /sys/class/gpio/gpio969/value");
    }
    if (access("/sys/class/gpio/gpio968/value", F_OK) != 0) {
        system("echo 968 > /sys/class/gpio/export");
        usleep(10 * 1000);
    }
    usleep(50 * 1000);
    
    /* configure GPIO for chip-to-chip Bridge detection
     * GPIO961 is EMIO[1] 
     * GPIO964 is EMIO[4]
     * GPIO965 is EMIO[5]
     */
    if (access("/sys/class/gpio/gpio961/value", F_OK) != 0) {
        system("echo 961 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo in > /sys/class/gpio/gpio961/direction");
    }
    if (access("/sys/class/gpio/gpio964/value", F_OK) != 0) {
        system("echo 964 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo in > /sys/class/gpio/gpio964/direction");
    }
    if (access("/sys/class/gpio/gpio965/value", F_OK) != 0) {
        system("echo 965 > /sys/class/gpio/export");
        usleep(10 * 1000);
        system("echo in > /sys/class/gpio/gpio965/direction");
    }

    gpio_fd = open("/sys/class/gpio/gpio961/value", O_RDONLY);
    if (gpio_fd > 0) {
        read(gpio_fd, buf, 1);
        if (buf[0] == '1') {
            memset(buf, 0, sizeof(buf));
            bdg0_fd = open("/sys/class/gpio/gpio964/value", O_RDONLY);
            bdg1_fd = open("/sys/class/gpio/gpio965/value", O_RDONLY);
            if ((bdg0_fd > 0) && (bdg1_fd > 0)) {
                read(bdg0_fd, &buf[0], 1);
                read(bdg1_fd, &buf[1], 1);
                if ((buf[0] == '1') && (buf[1] == '1')) {
                    err = API_CMS_ERROR_OK;
                    printf("Chip-to-chip bridge: Ready detected.\r\n");
                }
                else {
                    printf("Chip-to-chip bridge: Not ready, %c, %c.\r\n", buf[0], buf[1]);
                }
            }
            else {
                printf("Chip-to-chip bridge: Cannot find GPIO964 or GPIO965.\r\n");
            }
        }
        else {
            printf("Chip-to-chip bridge: ADS9_DONE is not ready.\r\n");
        }
        close(gpio_fd);
    }
    else {
        printf("Chip-to-chip bridge: Cannot find GPIO961.\r\n");
    }
    return err;
}

static int32_t ads9_spi_init(void)
{
    int32_t sysfd0, sysfd1, sysfd2, sysfd3;
    char    sysbuf0[SYSFS_DATALEN], sysbuf1[SYSFS_DATALEN], sysbuf2[SYSFS_DATALEN], sysbuf3[SYSFS_DATALEN];
    int32_t err = API_CMS_ERROR_ERROR;

    g_spi_ctrl_mem    = NULL;
    g_spi_cs_mem      = NULL;
    g_axi_jesd204_mem = NULL;
    g_i2c_ctrl_mem    = NULL;
    memset(sysbuf0, 0, sizeof(sysbuf0));
    memset(sysbuf1, 0, sizeof(sysbuf1));
    memset(sysbuf2, 0, sizeof(sysbuf2));
    memset(sysbuf3, 0, sizeof(sysbuf3));
    if ((access(SPI_CONTROLLERDEV, F_OK) == 0) && 
        (access(SPI_CHIPSELECTDEV, F_OK) == 0) && 
        (access(AXI_JESD204DEV,    F_OK) == 0) &&
        (access(I2C_CONTROLLERDEV, F_OK) == 0)) {
        sysfd0 = open(SPI_CONTROLLER_SYSNAME, O_RDONLY);
        sysfd1 = open(SPI_CHIPSELECT_SYSNAME, O_RDONLY);
        sysfd2 = open(AXI_JESD204DEV_SYSNAME, O_RDONLY);
        sysfd3 = open(I2C_CONTROLLER_SYSNAME, O_RDONLY);
        if ((sysfd0 > 0) && (sysfd1 > 0) && (sysfd2 > 0) && (sysfd3 > 0)) {
            read(sysfd0, sysbuf0, strlen(SPI_CONTROLLERNAME));
            read(sysfd1, sysbuf1, strlen(SPI_CHIPSELECTNAME));
            read(sysfd2, sysbuf2, strlen(AXI_JESD204NAME));
            read(sysfd3, sysbuf3, strlen(I2C_CONTROLLERNAME));
            if ((strcmp(sysbuf0, SPI_CONTROLLERNAME) == 0) && 
                (strcmp(sysbuf1, SPI_CHIPSELECTNAME) == 0) && 
                (strcmp(sysbuf2, AXI_JESD204NAME)    == 0) &&
                (strcmp(sysbuf3, I2C_CONTROLLERNAME) == 0)) {
                printf("UIO devices found: %s, %s, %s, %s.\r\n", sysbuf0, sysbuf1, sysbuf2, sysbuf3);
                err = API_CMS_ERROR_OK;
            }
            else {
                printf("UIO devices name is wrong: %s, %s, %s, %s.\r\n", sysbuf0, sysbuf1, sysbuf2, sysbuf3);
            }
            close(sysfd0);
            close(sysfd1);
            close(sysfd2);
            close(sysfd3);
        }
        else {
            printf("UIO SysFS doesn't work.\r\n");
        }
    }
    else {
        printf("UIO device doesn't exist.\r\n");
    }
    return err;
}

static int32_t ads9_spi_open(void)
{
    int32_t err = API_CMS_ERROR_ERROR;
    g_spi_ctrl_fd    = open(SPI_CONTROLLERDEV, O_RDWR);
    g_spi_cs_fd      = open(SPI_CHIPSELECTDEV, O_RDWR);
    g_axi_jesd204_fd = open(AXI_JESD204DEV, O_RDWR);
    g_i2c_ctrl_fd    = open(I2C_CONTROLLERDEV, O_RDWR);
    if ((g_spi_ctrl_fd > 0) && (g_spi_cs_fd > 0) && (g_axi_jesd204_fd > 0) && (g_i2c_ctrl_fd > 0)) {
        g_spi_ctrl_mem    = mmap(NULL, SPI_CONTROLLERSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_spi_ctrl_fd, 0);
        g_spi_cs_mem      = mmap(NULL, SPI_CHIPSELECTSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_spi_cs_fd,   0);
        g_axi_jesd204_mem = mmap(NULL, AXI_JESD204SIZE,    PROT_READ | PROT_WRITE, MAP_SHARED, g_axi_jesd204_fd, 0);
        g_i2c_ctrl_mem    = mmap(NULL, I2C_CONTROLLERSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_i2c_ctrl_fd, 0);
        if ((g_spi_ctrl_mem == MAP_FAILED) || (g_spi_cs_mem == MAP_FAILED) || (g_axi_jesd204_mem == MAP_FAILED) || (g_i2c_ctrl_mem == MAP_FAILED)) {
            printf("UIO mmap error.\r\n");
        }
        else {
            printf("UIO mapping address: 0x%x, 0x%x, 0x%x, 0x%x.\r\n", (int)g_spi_ctrl_mem, (int)g_spi_cs_mem, (int)g_axi_jesd204_mem, (int)g_i2c_ctrl_mem);
            err = API_CMS_ERROR_OK;
        }
    }
    else {
        printf("UIO device cannot be opened.\r\n");
    }
    return err;
}

static int32_t ads9_spi_configure(uint32_t wire3, uint32_t msb)
{
    ads9_spi_reg_write32(SPIM_CTL, (0x140 | (wire3 == 1 ? 0x80 : 0x00) | (msb == 1 ? 0x00 : 0x05)));
    ads9_spi_reg_write32(SPIM_XFER_START, 0);    
    ads9_spi_reg_write32(SPIM_CS_COUNT, 24);                     /* assert cs# for 24 cycles (default) */
    ads9_spi_reg_write32(SPIM_CS_POLARITY, 0);                   /* cs# active low */
    ads9_spi_reg_write32(SPIM_RD_BYTE_COUNT, 1);                 /* read 1 byte (default) */
    ads9_spi_reg_write32(SPIM_RD_DELAY_COUNT, 16);               /* rd: 16 cycles delay */
    ads9_spi_reg_write32(SPIM_WR_BYTE_COUNT, 3);                 /* write 2 byte address + 1 byte data (default) */
    ads9_spi_reg_write32(SPIM_WR_DELAY_COUNT, 0);                /* wr: no delay */
    ads9_spi_reg_write32(SPIM_XFER_COUNT, 1);                    /* # of xfers per transaction */
    ads9_spi_reg_write32(SPIM_XFER_DELAY, 1);                    /* # of idle clocks between xfers */
    return API_CMS_ERROR_OK;
}

static int32_t ads9_spi_read(uint32_t slave_addr, uint32_t *data, uint32_t data_bits)
{
    int32_t err = API_CMS_ERROR_ERROR;
    g_spi_rd_cnt ++;
    if (ads9_spi_wait_idle() == 0) {
        ads9_spi_reg_write32(SPIM_CS_ENABLE, 1);
        ads9_spi_reg_write32(SPIM_CS_COUNT, 16 + data_bits);
        ads9_spi_reg_write32(SPIM_WR_BYTE_COUNT, 2);
        ads9_spi_reg_write32(SPIM_RD_BYTE_COUNT, data_bits / 8);
        ads9_spi_reg_write32(SPIM_WBUF_WRDATA, slave_addr);
        ads9_spi_reg_write32(SPIM_XFER_START, 1);
        if (ads9_spi_wait_idle() == 0) {
            if (data_bits == 8) {
                ads9_spi_reg_read32(SPIM_RBUF_RDDATA, data);
                *data = (*data) >> 8;
            }
            if (data_bits == 32) {
                uint32_t fifo_data;
                ads9_spi_reg_read32(SPIM_RBUF_RDDATA, &fifo_data);                
                *data = ((fifo_data & 0xff00) >> 8) + ((fifo_data & 0x00ff) << 8);
                ads9_spi_reg_read32(SPIM_RBUF_RDDATA, &fifo_data);
                *data += ((fifo_data & 0xff00) << 8) + ((fifo_data & 0x00ff) << 24);
            }
            err = API_CMS_ERROR_OK;
        }
    }
    return err;
}

static int32_t ads9_spi_read_7175(uint32_t slave_addr, uint32_t *data, uint32_t data_bits)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t fifo_data;
    g_spi_rd_cnt ++;
    if (ads9_spi_wait_idle() == 0) {
        ads9_spi_reg_write32(SPIM_CS_ENABLE, 2);
        ads9_spi_reg_write32(SPIM_CS_COUNT, 16 + data_bits);
        ads9_spi_reg_write32(SPIM_WR_BYTE_COUNT, 2);
        ads9_spi_reg_write32(SPIM_RD_BYTE_COUNT, data_bits / 8);
        ads9_spi_reg_write32(SPIM_WBUF_WRDATA, slave_addr);
        ads9_spi_reg_write32(SPIM_XFER_START, 1);
        if (ads9_spi_wait_idle() == 0) {
            if (data_bits == 8) {
                ads9_spi_reg_read32(SPIM_RBUF_RDDATA, &fifo_data);
                *data = fifo_data >> 8;
            }
            if (data_bits == 16) {
                ads9_spi_reg_read32(SPIM_RBUF_RDDATA, &fifo_data);  
                *data = ((fifo_data & 0xff00) >> 8) + ((fifo_data & 0x00ff) << 8);
            }
            if (data_bits == 24) {
                ads9_spi_reg_read32(SPIM_RBUF_RDDATA, &fifo_data);
                *data = ((fifo_data & 0xff00) >> 8) + ((fifo_data & 0x00ff) << 8);
                ads9_spi_reg_read32(SPIM_RBUF_RDDATA, &fifo_data);
                *data += ((fifo_data & 0xff00) << 8);
            }
            err = API_CMS_ERROR_OK;
        }
    }
    return err;
}

static int32_t ads9_spi_write(uint32_t slave_addr, uint32_t data, uint32_t data_bits)
{
    int32_t err = API_CMS_ERROR_ERROR;
    g_spi_wr_cnt ++;
    if (ads9_spi_wait_idle() == 0) {
        ads9_spi_reg_write32(SPIM_CS_ENABLE, 1);
        ads9_spi_reg_write32(SPIM_CS_COUNT, 16 + data_bits);
        ads9_spi_reg_write32(SPIM_WR_BYTE_COUNT, 2 + (data_bits/8));
        ads9_spi_reg_write32(SPIM_RD_BYTE_COUNT, 0);
        ads9_spi_reg_write32(SPIM_WBUF_WRDATA, slave_addr);
        if (data_bits == 8) {
            ads9_spi_reg_write32(SPIM_WBUF_WRDATA, ((data & 0x000000ff) << 8));
        }
        if (data_bits == 32) {            
            ads9_spi_reg_write32(SPIM_WBUF_WRDATA, ((data & 0x000000ff) << 8) + ((data & 0x0000ff00) >> 8));
            ads9_spi_reg_write32(SPIM_WBUF_WRDATA, ((data & 0x00ff0000) >> 8) + ((data & 0xff000000) >> 24));
        }
        ads9_spi_reg_write32(SPIM_XFER_START, 1);
        if (ads9_spi_wait_idle() == 0) {
            err = API_CMS_ERROR_OK;
        }
    }
    return err;
}

static int32_t ads9_spi_write_7175(uint32_t slave_addr, uint32_t data)
{
    int32_t err = API_CMS_ERROR_ERROR;
    g_spi_wr_cnt ++;
    if (ads9_spi_wait_idle() == 0) {
        ads9_spi_reg_write32(SPIM_CS_ENABLE, 2);
        ads9_spi_reg_write32(SPIM_CS_COUNT, 32);
        ads9_spi_reg_write32(SPIM_WR_BYTE_COUNT, 4);
        ads9_spi_reg_write32(SPIM_RD_BYTE_COUNT, 0);
        ads9_spi_reg_write32(SPIM_WBUF_WRDATA, slave_addr);
        ads9_spi_reg_write32(SPIM_WBUF_WRDATA, ((data & 0x000000ff) << 8) + ((data & 0x0000ff00) >> 8));
        ads9_spi_reg_write32(SPIM_XFER_START, 1);
        if (ads9_spi_wait_idle() == 0) {
            err = API_CMS_ERROR_OK;
        }
    }
    return err;
}

static int32_t ads9_spi_write_burst6(uint32_t slave_addr, uint8_t *data)
{
    int32_t err = API_CMS_ERROR_ERROR;
    g_spi_wr_cnt ++;
    if (ads9_spi_wait_idle() == 0) {
        ads9_spi_reg_write32(SPIM_CS_ENABLE, 1);
        ads9_spi_reg_write32(SPIM_CS_COUNT, 16 + 6 * 8);
        ads9_spi_reg_write32(SPIM_WR_BYTE_COUNT, 2 + 6);
        ads9_spi_reg_write32(SPIM_RD_BYTE_COUNT, 0);
        ads9_spi_reg_write32(SPIM_WBUF_WRDATA, slave_addr);
        ads9_spi_reg_write32(SPIM_WBUF_WRDATA, (data[0] << 8) + data[1]);
        ads9_spi_reg_write32(SPIM_WBUF_WRDATA, (data[2] << 8) + data[3]);
        ads9_spi_reg_write32(SPIM_WBUF_WRDATA, (data[4] << 8) + data[5]);
        ads9_spi_reg_write32(SPIM_XFER_START, 1);
        if (ads9_spi_wait_idle() == 0) {
            err = API_CMS_ERROR_OK;
        }
    }
    return err;
}

static int32_t ads9_i2c_wait_idle(void)
{
    int32_t  err = API_CMS_ERROR_ERROR;
    uint32_t i, start, status;
    for(i = 0; i < 5; i ++) {
        ads9_i2c_reg_read32(I2CM_TRANS_CTRL, &start);
        ads9_i2c_reg_read32(I2CM_STATUS, &status);
        if (((start & 0x1) == 0) && ((status & 0x3) == 0)) {
          err = API_CMS_ERROR_OK;
          break;
        }
        ads9_wait_us(NULL, 1000);
    };
    return err;
}

static int32_t ads9_i2c_read(uint32_t slave_addr, uint8_t reg_addr, uint8_t *data)
{
    int32_t err = API_CMS_ERROR_ERROR;
    if (ads9_i2c_wait_idle() == 0) {
        ads9_i2c_reg_write32(I2CM_TRANS_SETTINGS, 0x4); /* 0x04 for read, 0x0 for write */
        ads9_i2c_reg_write32(I2CM_SLAVE_ADDR, slave_addr);
        ads9_i2c_reg_write32(I2CM_REG_ADDR, reg_addr);
        ads9_i2c_reg_write32(I2CM_READ_LEN, 1);
        ads9_i2c_reg_write32(I2CM_TRANS_CTRL, 1); /* start xfer */
        if (ads9_i2c_wait_idle() == 0) {
            uint32_t fifo_data;
            ads9_i2c_reg_read32(I2CM_RX_FIFO, &fifo_data);
            *data = fifo_data & 0xff;
            err = API_CMS_ERROR_OK;
        }
    }
    return err; 
}

int32_t ads9_i2c_board_name_get(char * vendor, char* board_name, char* board_rev)
{
    uint8_t i;
    
    uint8_t pca_config;
    ads9_i2c_read(0x70, 0x04, &pca_config); /* select fmc channel */

    uint8_t common_header;
    ads9_i2c_read(0x50, 0x00, &common_header);
    if (common_header != 0x01)
        return API_CMS_ERROR_ERROR;
    
    uint8_t board_manufacture_type_len;
    ads9_i2c_read(0x50, 0x0e, &board_manufacture_type_len);
    
    /* read vendor */
    for (i = 0; i < (board_manufacture_type_len & 0x0f); i ++) {
        ads9_i2c_read(0x50, i + 0x0f, (uint8_t *)&vendor[i]);
    }
    vendor[i] = 0;

    uint8_t board_product_name_type_len;
    ads9_i2c_read(0x50, 0x0f + (board_manufacture_type_len & 0x0f), &board_product_name_type_len);
    
    uint8_t board_part_number_type_len;
    ads9_i2c_read(0x50, 0x0f + (board_manufacture_type_len & 0x0f) + (board_product_name_type_len & 0x0f) + 2, &board_part_number_type_len);

    /* read board name */
    for (i = 0x0; i < (board_part_number_type_len & 0x3f); i ++) {
        ads9_i2c_read(0x50, i + 0x0f + (board_manufacture_type_len & 0x0f) + (board_product_name_type_len & 0x0f) + 3, (uint8_t *)&board_name[i]);
    }
    board_name[i] = 0;
    
    uint8_t pcb_rev_len;
    ads9_i2c_read(0x50, 0x0f + (board_manufacture_type_len & 0x0f) + (board_product_name_type_len & 0x0f) + (board_part_number_type_len & 0x3f) + 4, &pcb_rev_len);

    /* read board rev */
    for (i = 0x0; i < (pcb_rev_len & 0x0f); i ++) {
        ads9_i2c_read(0x50, i + 0x0f + (board_manufacture_type_len & 0x0f) + (board_product_name_type_len & 0x0f) + (board_part_number_type_len & 0x3f) + 5, (uint8_t *)&board_rev[i]);
    }
    board_rev[0] = ':';
    board_rev[i] = 0;
    
    return API_CMS_ERROR_OK;
}

static int32_t ads9_spi_close(void)
{    
    if (g_spi_ctrl_mem != NULL) {
        munmap(g_spi_ctrl_mem, SPI_CONTROLLERSIZE);
        g_spi_ctrl_mem = NULL;
    }
    if (g_spi_cs_mem != NULL) {
        munmap(g_spi_cs_mem, SPI_CHIPSELECTSIZE);
        g_spi_cs_mem = NULL;
    }
    if (g_axi_jesd204_mem != NULL) {
        munmap(g_axi_jesd204_mem, AXI_JESD204SIZE);
        g_axi_jesd204_mem = NULL;
    }
    if (g_i2c_ctrl_mem != NULL) {
        munmap(g_i2c_ctrl_mem, I2C_CONTROLLERSIZE);
        g_i2c_ctrl_mem = NULL;
    }
    close(g_spi_ctrl_fd);
    close(g_spi_cs_fd);
    close(g_axi_jesd204_fd);
    close(g_i2c_ctrl_fd);
    return API_CMS_ERROR_OK;
}

static int32_t ads9_log_open()
{
    int32_t err;
    struct tm tm = { 0 };
    time_t t = time(NULL);
    
    if ((g_log_fd != NULL) || (g_log_file_name[0] == '\0')) {
        return API_CMS_ERROR_LOG_OPEN;
    }    
    g_log_fd = fopen(g_log_file_name, "w+");
    if (g_log_fd == NULL) {
        return API_CMS_ERROR_LOG_OPEN;
    }
    tm = *localtime(&t);
    err = fprintf(g_log_fd, "000.000: API log file[%04d-%02d-%02d %02d:%02d:%02d]\n\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    if (err < 0) {
        return API_CMS_ERROR_LOG_OPEN;
    }
    if (fflush(g_log_fd) < 0) {
        return API_CMS_ERROR_LOG_OPEN;
    }
    clock_gettime(CLOCK_REALTIME, &ts_start);

    return API_CMS_ERROR_OK;
}

static int32_t ads9_log_close()
{
    if (g_log_fd != NULL) {
        if (fflush(g_log_fd) < 0) {
           return API_CMS_ERROR_LOG_CLOSE;
        }
        if (fclose(g_log_fd) < 0) {
           return API_CMS_ERROR_LOG_CLOSE;
        }
        g_log_fd = NULL;
    }
    return API_CMS_ERROR_OK;
}

int32_t ads9_spi_xfer_ad9986(void *user_data, uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes)
{      
    uint32_t value;
    char msg[100];
    va_list argp = {0};

    adi_hal_cfg_t *hal_cfg = (adi_hal_cfg_t *)user_data;
    ads9_spi_periph_sel(hal_cfg->spi_cs);     /* select ad9986 */
    ads9_spi_configure(hal_cfg->spi_wire, hal_cfg->spi_msb);  /* 3/4wire, msb/lsb */

    uint32_t addr = (hal_cfg->spi_msb == 1) ? ((in_data[0] << 8) | in_data[1]) : ((in_data[1] << 8) | in_data[0]);
    
    if (in_data[0] & 0x80) { /* spi read */        
        if ((size_bytes & 0x30000000) == 0x00000000) { /* 8-bit data */
            ads9_spi_read(addr, &value, 8);
            out_data[2] = (uint8_t)value;
        }
        if ((size_bytes & 0x30000000) == 0x20000000) { /* 32-bit data */
            ads9_spi_read(addr, &value, 32);
            out_data[2] = (value >>  0) & 0xff;
            out_data[3] = (value >>  8) & 0xff;
            out_data[4] = (value >> 16) & 0xff;
            out_data[5] = (value >> 24) & 0xff;
        }
    }
    else { /* spi write */
        if ((size_bytes & 0x300000ff) == 0x00000003) { /* 1 reg byte */
            ads9_spi_write(addr, in_data[2], 8);
        }
        if ((size_bytes & 0x30000000) == 0x20000000) { /* 4 reg bytes */
            value = (in_data[2]) + (in_data[3] << 8) + (in_data[4] << 16) + (in_data[5] << 24);
            ads9_spi_write(addr, value, 32);
        }
        if ((size_bytes & 0x300000ff) == 0x00000008) { /* 6 reg bytes */
            ads9_spi_write_burst6(addr, &in_data[2]);
            sprintf(msg, "ad9986: w@%.4x = %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, ", addr, in_data[2], in_data[3], in_data[4], in_data[5], in_data[6], in_data[7]);
            ads9_log_write(user_data, ADI_CMS_LOG_MSG, msg, argp);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t ads9_spi_xfer_hmc7044(void *user_data, uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes)
{
    uint32_t value;    
    char msg[100];
    va_list argp = {0}; 

    if (user_data == NULL || in_data == NULL || out_data == NULL) {
        return API_CMS_ERROR_NULL_PARAM;
    }
    adi_hal_cfg_t *hal_cfg = (adi_hal_cfg_t *)user_data;
    ads9_spi_periph_sel(hal_cfg->spi_cs);  /* select hmc7044 */
    ads9_spi_configure(1, 1);              /* 3wire, msb */
    uint32_t addr = (in_data[0] << 8) | in_data[1];
    if (in_data[0] & 0x80) {        
        ads9_spi_read(addr, &value, 8);
        out_data[2] = (uint8_t)value;
        sprintf(msg, "  7044: r@%.4x = %.2x", addr, out_data[2]);
        ads9_log_write(user_data, ADI_CMS_LOG_MSG, msg, argp);
    }
    else {
        ads9_spi_write(addr, in_data[2], 8);
        sprintf(msg, "  7044: w@%.4x = %.2x", addr, in_data[2]);
        ads9_log_write(user_data, ADI_CMS_LOG_MSG, msg, argp);
    }

    return API_CMS_ERROR_OK;
}

int32_t ads9_spi_xfer_ad7175(void *user_data, uint8_t *in_data, uint8_t *out_data, uint32_t size_bytes)
{
    uint32_t value;
    char msg[100];
    va_list argp = {0};

    if (user_data == NULL || in_data == NULL || out_data == NULL) {
        return API_CMS_ERROR_NULL_PARAM;
    }
    adi_hal_cfg_t *hal_cfg = (adi_hal_cfg_t *)user_data;
    ads9_spi_periph_sel(hal_cfg->spi_cs); /* select ad7175 shares spi0 bus with ad9986*/
    ads9_spi_configure(0, 1); /* 3wire, msb */
    uint32_t addr = (in_data[0] << 8) | in_data[1];
    
    if (in_data[1] & 0x40) {
        if (size_bytes == 0x01) { /* 8-bit data */
            ads9_spi_read_7175(addr, &value, 8);
            out_data[2] = (value >> 0) & 0xff;
            sprintf(msg, "  7175: r@%.4x = %.2x", (((addr & 0x40) << 8) | (addr & 0x3f)), out_data[2]);
            ads9_log_write(user_data, ADI_CMS_LOG_MSG, msg, argp);
        } 
        if (size_bytes == 0x02) { /* 16-bit data */
            ads9_spi_read_7175(addr, &value, 16);
            out_data[2] = (value >>  0) & 0xff;
            out_data[3] = (value >>  8) & 0xff;
            sprintf(msg, "  7175: r@%.4x = %.2x, %.2x", (((addr & 0x40) << 8) | (addr & 0x3f)), out_data[2], out_data[3]);
            ads9_log_write(user_data, ADI_CMS_LOG_MSG, msg, argp);
        }
        if (size_bytes == 0x03) { /* 24-bit data */
            ads9_spi_read_7175(addr, &value, 24);
            out_data[2] = (value >>  0) & 0xff;
            out_data[3] = (value >>  8) & 0xff;
            out_data[4] = (value >> 16) & 0xff;
            sprintf(msg, "  7175: r@%.4x = %.2x, %.2x, %.2x", (((addr & 0x40) << 8) | (addr & 0x3f)), out_data[2], out_data[3], out_data[4]);
            ads9_log_write(user_data, ADI_CMS_LOG_MSG, msg, argp);
        } 
    }
    else { /* spi write, 16 bits */
        value = (in_data[2]) + (in_data[3] << 8);
        ads9_spi_write_7175(addr, value);
        sprintf(msg, "  7175: w@%.4x = %.4x", (addr & 0x3f), ((in_data[2] << 8) + in_data[3]));
        ads9_log_write(user_data, ADI_CMS_LOG_MSG, msg, argp);
    }
    return API_CMS_ERROR_OK;
}

int32_t ads9_log_write(void *user_data, int32_t log_type, const char *comment, va_list argp)
{
    int32_t err;
    struct timespec ts_now;
    char log_msg[MAX_LOG_LINE_LENGTH] = {0};
    const char *log_type_str;

    if ((g_log_fd == NULL) || (g_log_file_name[0] == '\0'))
        return API_CMS_ERROR_LOG_WRITE;
    if (g_log_line_num >= MAX_LOG_NUM_LINES) {
        rewind(g_log_fd);
        g_log_line_num = 0;
    }

    log_type_str = "MESSAGE:";
    if ((log_type & ADI_CMS_LOG_WARN) > 0)
        log_type_str = "WARNING:";
    if ((log_type & ADI_CMS_LOG_ERR)  > 0)
        log_type_str = "ERROR  :";

    clock_gettime(CLOCK_REALTIME, &ts_now);
    ts_now.tv_nsec = ts_now.tv_nsec - ts_start.tv_nsec;
    ts_now.tv_sec  = ts_now.tv_sec  - ts_start.tv_sec;
    if (ts_now.tv_sec > 0 && ts_now.tv_nsec < 0) {
        ts_now.tv_nsec += 1e9;
        ts_now.tv_sec--;
    }

    err = snprintf(log_msg + strlen(log_msg), MAX_LOG_LINE_LENGTH, "%03d.%06d: %s ", (int)ts_now.tv_sec, (int)(ts_now.tv_nsec/1e3), log_type_str);
    if (err < 0)
       return API_CMS_ERROR_LOG_WRITE;
    if (vsprintf(log_msg + strlen(log_msg), comment, argp) < 0)
        return API_CMS_ERROR_LOG_WRITE;
    if (fprintf(g_log_fd, "%s\n", log_msg) < 0)
        return API_CMS_ERROR_LOG_WRITE;
    if (fflush(g_log_fd) < 0)
        return API_CMS_ERROR_LOG_WRITE;
    g_log_line_num += 1;
    
    return API_CMS_ERROR_OK;
}

int32_t ads9_hw_open(const char *log_file)
{
    int32_t err;

    /* reset spi rd/wr counter */
    g_spi_rd_cnt = 0;
    g_spi_wr_cnt = 0;

    /* save log file name */
    strncpy(g_log_file_name, log_file, MAX_FILE_NAME_LENGTH);

    /* open log */
    if (err = ads9_log_open(), err != API_CMS_ERROR_OK)
        return err;
    
    /* open spi */
    if (err = ads9_spi_bridge_detect(), err != API_CMS_ERROR_OK) {
        ads9_wait_us(NULL, 100000);
        if (err = ads9_spi_bridge_detect(), err != API_CMS_ERROR_OK)
            return err;
    }
    if (err = ads9_spi_init(), err != API_CMS_ERROR_OK)
        return err;
    if (err = ads9_spi_open(), err != API_CMS_ERROR_OK)
        return err;
    if (err = ads9_spi_configure(0, 1), err != API_CMS_ERROR_OK)
        return err;
 
    return API_CMS_ERROR_OK;
}

int32_t ads9_hw_close()
{
    int32_t err;
    char    msg[100];
    va_list argp = {0};
    struct timespec ts_now;

    clock_gettime(CLOCK_REALTIME, &ts_now);
    ts_now.tv_nsec = ts_now.tv_nsec - ts_start.tv_nsec;
    ts_now.tv_sec  = ts_now.tv_sec  - ts_start.tv_sec;
    if (ts_now.tv_sec > 0 && ts_now.tv_nsec < 0) {
        ts_now.tv_nsec += 1e9;
        ts_now.tv_sec--;
    }
    printf("%03d.%03d: ads9_hw_close(), spi_rd = %d, spi_wr = %d, total = %d \r\n", (int)ts_now.tv_sec, (int)(ts_now.tv_nsec/1e6), g_spi_rd_cnt, g_spi_wr_cnt, g_spi_rd_cnt + g_spi_wr_cnt);

    sprintf(msg, "ads9_hw_close(), spi_rd = %d, spi_wr = %d, total = %d ", g_spi_rd_cnt, g_spi_wr_cnt, g_spi_rd_cnt + g_spi_wr_cnt);
    ads9_log_write(NULL, ADI_CMS_LOG_MSG, msg, argp);

    /* close log file */
    if (err = ads9_log_close(), err != API_CMS_ERROR_OK)
        return err;

    /* close spi device */
    if (err = ads9_spi_close(), err != API_CMS_ERROR_OK)
        return err;

    return API_CMS_ERROR_OK;
}

int32_t ads9_hw_rst_pin_ctrl_ad9986(void *user_data, uint8_t pin_level)
{
    if (user_data == NULL)
        return API_CMS_ERROR_NULL_PARAM;

    uint32_t tmp_val;
    ads9_axi_reg_read32(AXI_FPGA_MISC_1_REG, &tmp_val);
    if (pin_level) {
        tmp_val |= AXI_FPGA_DUT_RSTB;
    } else {
        tmp_val &= (~AXI_FPGA_DUT_RSTB);
    }
    ads9_axi_reg_write32(AXI_FPGA_MISC_1_REG,tmp_val);
    return API_CMS_ERROR_OK;
}

int32_t ads9_user_data_create_ad9986(adi_ad9986_device_t *device, uint8_t spi_cs)
{
    adi_hal_cfg_t *hal_cfg = (adi_hal_cfg_t *)calloc(1, sizeof(adi_hal_cfg_t));    
    if (hal_cfg == NULL)
        return API_CMS_ERROR_ERROR;

    hal_cfg->spi_cs = spi_cs;
    hal_cfg->spi_wire = device->hal_info.sdo == SPI_SDIO ? 1 : 0;
    hal_cfg->spi_msb = device->hal_info.msb == SPI_MSB_FIRST ? 1 : 0;
    device->hal_info.user_data = hal_cfg;
    
    return API_CMS_ERROR_OK;
}

int32_t ads9_user_data_create_hmc7044(adi_hmc7044_device_t *device, uint8_t spi_cs)
{
    adi_hal_cfg_t *hal_cfg = (adi_hal_cfg_t *)calloc(1, sizeof(adi_hal_cfg_t));    
    if (hal_cfg == NULL)
        return API_CMS_ERROR_ERROR;

    hal_cfg->spi_cs = spi_cs;
    hal_cfg->spi_wire = 1;
    device->hal_info.user_data = hal_cfg;       

    return API_CMS_ERROR_OK;
}

int32_t ads9_user_data_create_ad7175(adi_ad7175_device_t *device, uint8_t spi_cs)
{
    adi_hal_cfg_t *hal_cfg = (adi_hal_cfg_t *)calloc(1, sizeof(adi_hal_cfg_t));    
    if (hal_cfg == NULL)
        return API_CMS_ERROR_ERROR;
        
    hal_cfg->spi_cs = spi_cs;
    hal_cfg->spi_wire = 0;  /* 4 wire */
    device->hal_info.user_data = hal_cfg;       

    return API_CMS_ERROR_OK;
}

int32_t ads9_user_data_free(void **user_data)
{    
    free(*user_data);
    *user_data = NULL;
    return API_CMS_ERROR_OK;
}

int32_t ads9_wait_us(void *user_data, uint32_t time_us)
{
    if (usleep(time_us) < 0)
        return API_CMS_ERROR_DELAY_US;
    return API_CMS_ERROR_OK;
}
