/*!
 * @brief     API header file
 *            This file contains all the publicly exposed methods and data 
 *            structures to interface with API.
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup ADI_AD7175
 * @{
 */
#ifndef __ADI_AD7175_H__
#define __ADI_AD7175_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"

/*============= D E F I N E S ==============*/

typedef enum {
    AD7175_MODE_CONT_CONV  = 0x0000, /* Continous conversion mode */
    AD7175_MODE_SINGLE_CONV  = 0x0001, /* Single conversion mode*/
    AD7175_MODE_STANDBY  = 0x0002, /* Standby mode */
    AD7175_MODE_PD  = 0x0003, /* Power-down mode */
    AD7175_MODE_INT_OFFSET_CAL  = 0x0004, /* Internal offset calibration */
    AD7175_MODE_SYS_OFFSET_CAL  = 0x0006, /* System offset calibration */
    AD7175_MODE_SYS_GAIN_CAL  = 0x0007, /* System gain calibration */
}adi_ad7175_adc_mode_e;

typedef enum {
    AD7175_INTF_WL16 = 0x01, /* Changes ADC data register to 16 bits */
    AD7175_INTF_CRC_EN = 0x0D, /* Enables CRC protection of register r/w */
    AD7175_INTF_REG_CHECK = 0x20, /* Enables register integrity checker */
    AD7175_INTF_DATA_STAT = 0x40, /* Enables status register appending to data register */
    AD7175_INTF_CONTREAD = 0x80, /* Enables continuous read mode of ADC data*/
    AD7175_INTF_DOUT_RESET = 0x100, /* Resets DOUT pin */
    AD7175_INTF_IOSTRENGTH = 0x200, /* Enables drive strength of DOUT pin */
    AD7175_INTF_ALT_SYNC = 0x400, /* Enables SYNC pin as conversion control */
}adi_ad7175_interface_mode_e;

typedef enum {
    AD7175_INT_OSC  = 0x0, /* Internal oscillator */
    AD7175_INT_OSC_XTAL2  = 0x1, /* Internal oscillator output on XTAL2/CLKIO pin */
    AD7175_EXT_CLKIO  = 0x2, /* External clock input on XTAL2/CLKIO pin */
    AD7175_XTAL2  = 0x3, /* External crystal on XTAL1 and XTAL2/CLKIO pins */
}adi_ad7175_clk_sel_e;

typedef enum {
    AD7175_DELAY_0_US = 0x0,
    AD7175_DELAY_4_US = 0x1,
    AD7175_DELAY_16_US = 0x2,
    AD7175_DELAY_40_US = 0x3,
    AD7175_DELAY_100_US = 0x4,
    AD7175_DELAY_200_US = 0x5,
    AD7175_DELAY_500_US = 0x6,
    AD7175_DELAY_1000_US = 0x7,
}adi_ad7175_prog_delay_e;

typedef enum {
    AD7175_DISABLE  = 0x0000, /* Disabled */
    AD7175_XOR_R_CHECKSUM  = 0x0001, /* XOR checksum for read, CRC for write */
    AD7175_CRC_WR_CHECKSUM  = 0x0002, /* CRC checksum for r/w */
}adi_ad7175_crc_en_e;

typedef enum {      
    AD7175_SETUP0  = 0x0000,
    AD7175_SETUP1  = 0x0001,
    AD7175_SETUP2  = 0x0002,
    AD7175_SETUP3  = 0x0003,
    AD7175_SETUP4  = 0x0004,
    AD7175_SETUP5  = 0x0005,
    AD7175_SETUP6  = 0x0006,
    AD7175_SETUP7  = 0x0007,
}adi_ad7175_setup_sel_e;

typedef enum {
    AD7175_SETUP_REF_EXT = 0x00, /* External reference */
    AD7175_SETUP_REF_EXT2 = 0x01, /* External reference 2 supplied to AIN1/REF2+ AND AIN0/REF2- pins */
    AD7175_SETUP_REF_INT = 0x02, /* Internal 2.5V reference */
    AD7175_SETUP_REF_AVDD1_AVSS = 0x03, /* AVDDS1 - AVSS Diagonstic to validate other refs */
}adi_ad7175_setup_ref_sel_e;

typedef enum {
    AD7175_SETUP_BURNOUT_EN = 0x80, /* Enables burnout currents */
    AD7175_SETUP_AINBUF_N = 0x100, /* Enables AIN- input buffer */
    AD7175_SETUP_AINBUF_P = 0x200, /* Enables AIN+ input buffer */
    AD7175_SETUP_REFBUF_N = 0x400, /* Enables REF- input buffer */
    AD7175_SETUP_REFBUF_P = 0x800, /* Enables REF+ input buffer */
    AD7175_SETUP_BI_UNIPOLAR = 0x1000, /* Output coding */
}adi_ad7175_setup_config_e;

typedef enum {      
    AD7175_AIN0 = 0x0000,
    AD7175_AIN1 = 0x0001,  
    AD7175_AIN2 = 0x0002,
    AD7175_AIN3 = 0x0003,
    AD7175_AIN4 = 0x0004,
    AD7175_AIN5 = 0x0005,
    AD7175_AIN6 = 0x0006,
    AD7175_AIN7 = 0x0007,
    AD7175_AIN8 = 0x0008,
    AD7175_AIN9 = 0x0009,
    AD7175_AIN10 = 0x000A,
    AD7175_AIN11 = 0x000B,
    AD7175_AIN12 = 0x000C,
    AD7175_AIN13 = 0x000D,
    AD7175_AIN14 = 0x000E,
    AD7175_AIN15 = 0x000F,
    AD7175_AIN16 = 0x0010,
}adi_ad7175_ain_e;

typedef struct {
    void *                user_data;
    adi_spi_xfer_t        spi_xfer;             /*!< Function Pointer to HAL SPI access function */
    adi_delay_us_t        delay_us;             /*!< Function Pointer to HAL delay function */
    adi_hw_open_t         hw_open;              /*!< OPTIONAL Function Pointer to HAL initialization function */
    adi_hw_close_t        hw_close;             /*!< OPTIONAL Function Pointer to HAL De-initialization function */
}adi_ad7175_hal_t;

typedef struct {
    uint8_t  dev_prod_id;                       /*!< Product ID */
}adi_ad7175_info_t;

typedef struct {
    adi_ad7175_hal_t  hal_info;                /*!< HAL information */
    adi_ad7175_info_t dev_info;                /*!< DEV information */
}adi_ad7175_device_t;


/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Open device
 *
 * @param  device Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_INVALID_SPI_XFER_PTR   Invalid HAL SPI XFER FUNCTION
 * @return API_CMS_ERROR_INVALID_DELAYUS_PTR    Invalid HAL SPI XFER FUNCTION
 * @return API_CMS_ERROR_DELAY_US               Invalid HAL SPI XFER FUNCTION
 */
int32_t adi_ad7175_device_hw_open(adi_ad7175_device_t *device);

 /**
 * @brief  Close device
 *
 * @param  device Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_INVALID_DELAYUS_PTR    Invalid HAL SPI XFER FUNCTION
 * @return API_CMS_ERROR_DELAY_US               Invalid HAL SPI XFER FUNCTION
 */
int32_t adi_ad7175_device_hw_close(adi_ad7175_device_t *device);

/**
 * @brief Perform SPI register write access to device
 *
 * @param device   Pointer to the device structure
 * @param addr     SPI address to which the value of data parameter shall be written
 * @param val      8-bit value to be written to SPI register defined
 *                 by the address parameter.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_ad7175_device_spi_register_set(adi_ad7175_device_t *device, uint16_t addr, uint32_t val);

/**
 * @brief Perform SPI register read access to device.
 *
 *
 * @param device        Pointer to the device structure
 * @param addr          SPI address from which the value of val parameter shall be read,
 * @param val           Pointer to an 8-bit variable to which the value of the
 *                      SPI register at the address defined by address parameter
 *                      shall be stored.
 * @param size_bytes    Number of bytes being read
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_ad7175_device_spi_register_get(adi_ad7175_device_t *device, uint16_t addr, uint32_t *val, uint32_t size_bytes);

/**
 * @brief  Get API Revision Data
 *
 * @param  device    Pointer to the device structure.
 * @param  rev_major Pointer to variable to store the Major Revision Number
 * @param  rev_minor Pointer to variable to store the Minor Revision Number
 * @param  rev_rc    Pointer to variable to store the RC Revision Number
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_ad7175_device_api_revision_get(adi_ad7175_device_t *device, uint8_t *rev_major, uint8_t *rev_minor, uint8_t *rev_rc);


/**
 * @brief Get chip identification data
 * 
 * @param device                    Pointer to the device structure
 * @param id                        Pointer to chip id
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_ad7175_device_id_get(adi_ad7175_device_t *device, uint32_t *id);

/**
 * @brief Configure channel
 * 
 * @param device                Pointer to the device structure
 * @param chan                  Channel Index
 * @param ch_enable             Enable: 1, Disable: 0
 * @param setup                 Selected configuration for channel
 * @param ainpos                Positive analog input to channel
 * @param ainneg                Negative analog input to channel
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_ad7175_device_channel_config_set(adi_ad7175_device_t *device, uint8_t chan, uint8_t ch_enable, adi_ad7175_setup_sel_e setup, 
adi_ad7175_ain_e ainpos, adi_ad7175_ain_e ainneg);

/**
 * @brief Run selected channel
 * 
 * @param device                Pointer to the device structure
 * @param chan                  Channel Index
 * @param ref_en                Enable internal reference and output to REFOUT pin
 *                              Enable:1, Disable:0
 * @param clk_sel               Select ADC clock source @see adi_ad7175_clk_sel_e for details
 * @param adc_mode              Desired conversion mode of the adc, @see adi_ad7175_adc_mode_e for details
 * @param delay                 Desired programmable delay after channel switch @see adi_ad7175_prog_delay_e for details
 * @param hide_delay            Hides delay by absorbing it into conversion time for selected data rates
 *                              Enable:0, Disable:1
 * @param sing_cyc              Set ADC to only output at settled filter data rate when using single channel only
 *                              Enable:1, Disable:0
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_ad7175_device_channel_run(adi_ad7175_device_t *device, uint8_t chan, uint8_t ref_en, adi_ad7175_clk_sel_e clk_sel, 
adi_ad7175_adc_mode_e adc_mode, adi_ad7175_prog_delay_e delay, uint8_t hide_delay, uint8_t sing_cyc);

/**
 * @brief Configures the reference selection, input buffers, and output coding
 * 
 * @param device                    Pointer to the device structure
 * @param setup                     Selected configuration for channel, @see adi_ad7175_setup_sel_e for options
 * @param ref_sel                   Select reference source for ADC conversion, @see adi_ad7175_setup_ref_sel_e for details
 * @param setup_config              Selection configuration of input buffers and output coding, @see adi_ad7175_setup_config_e for details
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_ad7175_device_setup_config_set(adi_ad7175_device_t *device, adi_ad7175_setup_sel_e setup, adi_ad7175_setup_ref_sel_e ref_sel, uint32_t setup_config);

/**
 * @brief Set conversion mode of ADC to continuous or single
 * 
 * @param device                    Pointer to the device structure
 * @param ref_en                    Enable internal reference and output to REFOUT pin
 *                                  Enable:1, Disable:0
 * @param clk_sel                   Select ADC clock source @see adi_ad7175_clk_sel_e for details
 * @param adc_mode                  Desired conversion mode of the adc, @see adi_ad7175_adc_mode_e for details
 * @param delay                     Desired programmable delay after channel switch @see adi_ad7175_prog_delay_e for details
 * @param hide_delay                Hides delay by absorbing it into conversion time for selected data rates
 *                                  Enable:0, Disable:1
 * @param sing_cyc                  Set ADC to only output at settled filter data rate when using single channel only
 *                                  Enable:1, Disable:0
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_ad7175_device_adc_mode_config_set(adi_ad7175_device_t *device, uint8_t ref_en, adi_ad7175_clk_sel_e clk_sel, 
adi_ad7175_adc_mode_e adc_mode, adi_ad7175_prog_delay_e delay, uint8_t hide_delay, uint8_t sing_cyc);

/**
 * @brief Configure serial interface options
 * 
 * @param device                    Pointer to the device structure
 * @param intf_mode                 Desired digital interface operation, @see adi_ad7175_interface_mode_e for details
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_ad7175_device_intf_config_set(adi_ad7175_device_t *device, uint32_t intf_mode);

/**
 * @brief Get ADC conversion result
 * 
 * @param device                    Pointer to the device structure
 * @param data_status               Pointer to ADC conversion result
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_ad7175_device_data_get(adi_ad7175_device_t *device, uint32_t *data_status);

/**
 * @brief Checks status bit and triggers data conversion
 * 
 * @param device                    Pointer to the device structure
 * @param data_status               Pointer to status and data register data
 * @param channel                   Active channel index
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_ad7175_device_run_data_conv_get(adi_ad7175_device_t *device, uint32_t *data_status, uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_AD7175_H__ */
/*! @} */