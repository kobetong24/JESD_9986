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
 * @addtogroup ADI_HMC7044
 * @{
 */
#ifndef __ADI_HMC7044_H__
#define __ADI_HMC7044_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"

/*============= D E F I N E S ==============*/
#define HMC7044_NOF_OP_CH					14
#define HMC7044_CH_DIV_MAX					4095
#define HMC7044_SLIP_DELAY_MAX				4095
#define HMC7044_SPI_RESET_PERIOD_US         10
#define HMC7044_DIV_RESET_PERIOD_US         5000
#define HMC7044_NOF_CLK_IN					4
#define HMC7044_NOF_OSC_IN                  1
#define HMC7044_NOF_GPIO_MAX				4
#define HMC7044_PLL2_R_DIV_MAX				4095
#define HMC7044_HW_RESET_PERIOD_US			10
#define HMC7044_REF_CLK_FREQ_HZ_MIN			150
#define HMC7044_REF_CLK_FREQ_HZ_MAX 		800000000ull 
#define HMC7044_EXT_VCO_CLK_FREQ_HZ_MIN		400000000ull 
#define HMC7044_EXT_VCO_CLK_FREQ_HZ_MAX		6000000000ull 
#define HMC7044_VCXO_CLK_FREQ_HZ_MIN		10000000ull 
#define HMC7044_VCXO_CLK_FREQ_HZ_MAX		500000000ull 
#define HMC7044_PD1_CLK_FREQ_HZ_MIN			150
#define HMC7044_PD1_CLK_FREQ_HZ_MAX			50000000ull 
#define HMC7044_PD2_CLK_FREQ_HZ_MIN			150
#define HMC7044_PD2_CLK_FREQ_HZ_MAX			250000000ull 
#define HMC7044_LCM_CLK_FREQ_HZ_MIN			150
#define HMC7044_LCM_CLK_FREQ_HZ_MAX			123000000ull 
#define HMC7044_VCO_CLK_FREQ_HZ_MIN			2650000000ull
#define HMC7044_VCO_CLK_FREQ_HZ_MAX			3550000000ull
#define HMC7044_PLL1REF_CLK_FREQ_HZ_MIN		30000000ull
#define HMC7044_PLL1REF_CLK_FREQ_HZ_MAX		40000000ull
#define HMC7044_PLL2REF_CLK_DB_FREQ_HZ_MIN	10000000ull
#define HMC7044_PLL2REF_CLK_DB_FREQ_HZ_MAX	125000000ull

typedef enum {
    HMC7044_OP_CH_0  = 0x0001,
    HMC7044_OP_CH_1  = 0x0002,
    HMC7044_OP_CH_2  = 0x0004,
    HMC7044_OP_CH_3  = 0x0008,
    HMC7044_OP_CH_4  = 0x0010,
    HMC7044_OP_CH_5  = 0x0020,
    HMC7044_OP_CH_6  = 0x0040,
    HMC7044_OP_CH_7  = 0x0080,
    HMC7044_OP_CH_8  = 0x0100,
    HMC7044_OP_CH_9  = 0x0200,
    HMC7044_OP_CH_10 = 0x0400,
    HMC7044_OP_CH_11 = 0x0800,
    HMC7044_OP_CH_12 = 0x1000,
    HMC7044_OP_CH_13 = 0x2000,
    HMC7044_OP_CH_ALL = 0x3FFF
}adi_hmc7044_op_ch_e;

typedef enum
{
    HMC7044_CH0_CH1_EN   = 0x01,
    HMC7044_CH2_CH3_EN   = 0x02,
    HMC7044_CH4_CH5_EN   = 0x04,
    HMC7044_CH6_CH7_EN   = 0x08,
    HMC7044_CH8_CH9_EN   = 0x10,
    HMC7044_CH10_CH11_EN = 0x20,
    HMC7044_CH12_CH13_EN = 0x40,
    HMC7044_CH_ALL_EN    = 0x7F
}adi_hmc7044_ch_out_e;

typedef enum {
    HMC7044_CLK_IN_NONE = 0x0,
    HMC7044_CLK_IN_0    = 0x1,
    HMC7044_CLK_IN_1    = 0x2,
    HMC7044_CLK_IN_2    = 0x4,
    HMC7044_CLK_IN_3    = 0x8,
    HMC7044_CLK_IN_ALL  = 0XF
}adi_hmc7044_clk_in_e;

typedef enum
{
    HMC7044_LOS_TIMER_NONE       = 0,
    HMC7044_LOS_TIMER_CYCLES_2   = 1,
    HMC7044_LOS_TIMER_CYCLES_4   = 2,
    HMC7044_LOS_TIMER_CYCLES_8   = 3,
    HMC7044_LOS_TIMER_CYCLES_16  = 4,
    HMC7044_LOS_TIMER_CYCLES_32  = 5,
    HMC7044_LOS_TIMER_CYCLES_64  = 6,
    HMC7044_LOS_TIMER_CYCLES_128 = 7
}adi_hmc7044_los_timer_e;

typedef enum
{
    HMC7044_MANUAL_MODE_CLK_0 = 0,
    HMC7044_MANUAL_MODE_CLK_1 = 1,
    HMC7044_MANUAL_MODE_CLK_2 = 2,
    HMC7044_MANUAL_MODE_CLK_3 = 3
}adi_hmc7044_manual_clk_switching_e;

typedef enum
{
    HMC7044_PLL1_LOCK_ST = 1,          /*BIT 0 Indicates PLL1 Lock Status*/
    HMC7044_PLL2_LOCK_ST = 2,          /*BIT 1 Indicates PLL1 Lock Status*/
    HMC7044_PLL1_AND_PLL2_LOCK_ST = 4, /*BIT 2 Indicates PLL1 AND PLL2 Lock Status*/
}adi_hmc7044_pll_lock_status_mask_e;

typedef enum
{
    HMC7004_HO_EA_RESET_DIVIDERS = 0,
    HMC7044_HO_EA_DO_NOTHING_1   = 1,
    HMC7044_HO_EA_DO_NOTHING_2   = 2,
    HMC7044_HO_EA_DAC_ASSIT      = 3,
    HMC7044_HO_EC_LOS_GONE_0     = 0,
    HMC7044_HO_EC_ZERO_PHASE_ERR = 1,
    HMC7044_HO_EC_LOS_GONE_2     = 2,
    HMC7044_HO_EC_IMMEDIATE      = 3
}adi_hmc7044_holdover_exit_e;

typedef enum {
    HMC7044_OSCOUT_DIV_1 = 0,
    HMC7044_OSCOUT_DIV_2 = 1,
    HMC7044_OSCOUT_DIV_4 = 2,
    HMC7044_OSCOUT_DIV_8 = 3,
}adi_hmc7044_oscout_div_e;

typedef enum {
    HMC7044_OP_SIG_CH_DIV = 0x0,
    HMC7044_OP_SIG_ANALOG_DELAY = 0x1,
    HMC7044_OP_SIG_CH_PAIR = 0x2,
    HMC7044_OP_SIG_IP_VC0 = 0x3,
    HMC7044_OP_SIG_INVALID = 0x4
}adi_hmc7044_op_source_e;

typedef enum
{
    HMC7044_VCO_DISABLED         = 0,
    HMC7044_VCO_INTERNAL_3GHZ    = 1,
    HMC7044_VCO_INTERNAL_2GHZ    = 2,
    HMC7044_VCO_EXTERNAL         = 3,
    HMC7044_VCO_SEL_INVALID      =4
}adi_hmc7044_vco_sel_e;

typedef enum
{
    HMC7044_GPI_PLL1_HOLDOVER             = 1,
    HMC7044_GPI_PLL1_REFERENCE_BIT1       = 2,
    HMC7044_GPI_PLL1_REFERENCE_BIT0       = 3,
    HMC7044_GPI_CHIP_SLEEP_MODE           = 4,
    HMC7044_GPI_ISSUE_MUTE                = 5,
    HMC7044_GPI_SELECT_VCO_TYPE           = 6,
    HMC7044_GPI_PLL2_HIGH_PERFORM_AND_VCO = 7,
    HMC7044_GPI_PLUSE_GENERATE_REQ        = 8,
    HMC7044_GPI_RESEED_REQ                = 9,
    HMC7044_GPI_RESTART_REQ               = 10,
    HMC7044_GPI_CHIP_FANOUT_MODE          = 11,
    HMC7044_GPI_SLIP_REQ                  = 13 
}adi_hmc7044_gpi_op_config_e;

typedef enum
{
    HMC7044_GPO_ALARM_SIG                      = 0,
    HMC7044_GPO_SPI_SDATA                      = 1,
    HMC7044_GPO_LOS_CLKIN3_IN                  = 2,
    HMC7044_GPO_LOS_CLKIN2_IN                  = 3,
    HMC7044_GPO_LOS_CLKIN1_IN                  = 4,
    HMC7044_GPO_LOS_CLKIN0_IN                  = 5,
    HMC7044_GPO_PLL1_HOLDOVER_SIG_EN           = 6,
    HMC7044_GPO_PLL1_LOCK_DETECT_SIG           = 7,
    HMC7044_GPO_ACQ_PLL1_LOCK_SIG              = 8,
    HMC7044_GPO_PLL1_NEAR_LOCK_ACQ_STATUS      = 9,
    HMC7044_GPO_PLL2_LOCK_DETECT_SIG           = 10,
    HMC7044_GPO_SYSREF_NO_SYNC_ON_RESET        = 11,
    HMC7044_GPO_CLK_OUT_PHASE_STATUS           = 12,
    HMC7044_GPO_PLL1_PLL2_LOCK_DETECT_LOCKED   = 13,
    HMC7044_GPO_SYNC_REQ_STATUS_SIG            = 14,
    HMC7044_GPO_PLL1_ACTIVE_CLKIN_0            = 15,
    HMC7044_GPO_PLL1_ACTIVE_CLKIN_1            = 16,
    HMC7044_GPO_PLL1_HOLDOVER_ADC_RANGE_STATUS = 17,
    HMC7044_GPO_PLL1_HOLDOVER_ADC_STATUS       = 18,
    HMC7044_GPO_PLL1_VCXO_STATUS               = 19,
    HMC7044_GPO_PLL1_ACTIVE_CLKINx_STATUS      = 20,
    HMC7044_GPO_PLL1_FSM_BIT0_STATUS           = 21,
    HMC7044_GPO_PLL1_FSM_BIT1_STATUS           = 22,
    HMC7044_GPO_PLL1_FSM_BIT2_STATUS           = 23,
    HMC7044_GPO_HOLDOVER_EXIT_BIT0_PHASE       = 24,
    HMC7044_GPO_HOLDOVER_EXIT_BIT1_PHASE       = 25,
    HMC7044_GPO_CH_OUTS_FSM_BUSY               = 26,
    HMC7044_GPO_SYSREF_FSM_BIT0_STATUS         = 27,
    HMC7044_GPO_SYSREF_FSM_BIT1_STATUS         = 28,
    HMC7044_GPO_SYSREF_FSM_BIT2_STATUS         = 29,
    HMC7044_GPO_SYSREF_FSM_BIT3_STATUS         = 30,
    HMC7044_GPO_FORCE_LOGIC_1                  = 31,
    HMC7044_GPO_FORCE_LOGIC_0                  = 32,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_AVG_BIT0_VAL = 39,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_AVG_BIT1_VAL = 40,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_AVG_BIT2_VAL = 41,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_AVG_BIT3_VAL = 42,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_CUR_BIT0_VAL = 43,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_CUR_BIT1_VAL = 44,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_CUR_BIT2_VAL = 45,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_CUR_BIT3_VAL = 46,
    HMC7044_GPO_HOLDOVER_COM_STATUS            = 61,
    HMC7044_GPO_PULSE_GEN_REQ_STATUS           = 62
}adi_hmc7044_gpo_op_config_e;

typedef enum
{
    HMC7044_SYSREF_LEVEl_SENS      = 0,
    HMC7044_SYSREF_1_PULSE         = 1,
    HMC7044_SYSREF_2_PULSE         = 2,
    HMC7044_SYSREF_4_PULSE         = 3,
    HMC7044_SYSREF_8_PULSE         = 4,
    HMC7044_SYSREF_16_PULSE        = 5,
    HMC7044_SYSREF_ALSO_16_PULSE   = 6,
    HMC7044_SYSREF_CONTINUOUS_MODE = 7	
}adi_hmc7044_sysref_mode_config_e;

typedef struct {
    adi_cms_signal_type_e mode;                 /*!< Output Driver Mode */
    adi_cms_signal_impedance_type_e impedance;  /*!< Output Driver Impedance */
    uint8_t dynamic_driver_en;                  /*!< Dynamic Driver Mode Enable */
    uint8_t force_mute_en;                      /*!< Force Mute Enable */
}adi_hmc7044_op_driver_config_t;

typedef struct
{
    uint8_t pll1_lock;
    uint8_t pll1_lock_aquisition;
    uint8_t pll1_lock_detect;
    uint8_t pll1_holdover_status;
    uint8_t pll1_clkInx_status;	
    uint8_t sync_request;
    uint8_t pll1_pll2_lock_detect;
    uint8_t clkoutputs_phase_status;
    uint8_t sysref_sync_status;
    uint8_t pll2_lock_detect;
}adi_hmc7044_alarm_mask_config_t;

typedef struct
{
    uint8_t exit_action;
    uint8_t exit_criteria;
    uint8_t holdover_dac;
    uint8_t adc_tracking;
    uint8_t quick_mode;
    uint8_t holdover_bw;	
}adi_hmc_pll1_holdover_config_t;

typedef enum {
    IPBUFFER_INTERNAL_100_OHM_EN = 0x1,
    IPBUFFER_AC_COUPLED_MODE_EN  = 0x2,
    IPBUFFER_LVPECL_MODE_EN = 0x4,
    IPBUFFER_HIGH_Z_MODE_EN = 0x8,
    IPBUFFER_CONFIG_MAX = 0xF
}adi_hmc7044_ip_buffer_settings_e;

typedef struct {
    void *                user_data;
    adi_spi_xfer_t        spi_xfer;             /*!< Function Pointer to HAL SPI access function */
    adi_delay_us_t        delay_us;             /*!< Function Pointer to HAL delay function */
    adi_hw_open_t         hw_open;              /*!< Function Pointer to HAL initialization function */
    adi_hw_close_t        hw_close;             /*!< Function Pointer to HAL De-initialization function */
    adi_reset_pin_ctrl_t  reset_pin_ctrl;       /*!< Function Pointer to HAL RESETB Pin Control Function */
}adi_hmc7044_hal_t;

typedef struct {
    uint8_t  dev_prod_id;                       /*!< Product ID */
}adi_hmc7044_info_t;

typedef struct {
    adi_hmc7044_hal_t  hal_info;                /*!< HAL information */
    adi_hmc7044_info_t dev_info;                /*!< DEV information */
}adi_hmc7044_device_t;

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
 int32_t adi_hmc7044_device_hw_open(adi_hmc7044_device_t *device);

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
 int32_t adi_hmc7044_device_hw_close(adi_hmc7044_device_t *device);

/**
 * @brief  Initialize device
 *         This API must be called first before any other API calls.
 *         It performs internal API initialization of the memory and API states.
 *         If device member hw_open is not NULL it shall call the function
 *         to which it points. This feature may be used to get and initialize the
 *         hardware resources required by the API and the device.
 *         For example GPIO, SPI etc. 
 *         Its is recommended to call the Reset API after this API to ensure all
 *         SPI registers are reset to ADI recommended defaults.
 *
 * @param  device Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 */
int32_t adi_hmc7044_device_init(adi_hmc7044_device_t *device);
	
/**
 * @brief  put device to sleep mode
 *
 * @param  device								Pointer to the device structure
 * @param  sleepmode							indicate whether hmc7044 is used
 *												1 - put hmc7044 to sleep
 *												0 - enable hmc7044
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 */
int32_t adi_hmc7044_device_sleep_set(adi_hmc7044_device_t *device, uint8_t sleepmode);

/**
 * @brief  De-initialize device.
 *         This API must be called last. No other API should be called after this call.
 *         It performs internal API De-initialization of the memory and API states.
 *         If device member hw_close is not NULL it shall call the function
 *         to which it points. This feature may be used to De-initialize and release
 *         any hardware resources required by the API and the device.
 *         For example GPIO, SPI etc.
 *
 * @param  device Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 */
int32_t adi_hmc7044_device_deinit(adi_hmc7044_device_t *device);

/**
 * @brief  Reset device
 *         Issues a hard reset or soft reset of the device.
 *         Performs a full reset of device via the hardware pin (hard) or
 *         via the SPI register (soft).
 *         Resetting all SPI registers to default and triggering the required
 *         initialization sequence.
 *
 * @param  device   Pointer to the device structure.
 * @param  hw_reset A parameter to indicate if the reset issues is to be via the
 *                  hardware pin or SPI register.
 *                  A value of 1 indicates a hardware reset is required.
 *                  A value of 0 indicates a software reset is required.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_reset(adi_hmc7044_device_t *device, uint8_t hw_reset);

/**
 * @brief  Get Device Chip ID
 *
 * @param  device   Pointer to the device structure.
 * @param  chip_id  A pointer to a variable of type adi_chip_id_t
 *                  to return the details of the device id.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_chip_id_get(adi_hmc7044_device_t *device, adi_cms_chip_id_t *chip_id);

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
int32_t adi_hmc7044_device_api_revision_get(adi_hmc7044_device_t *device, uint8_t *rev_major,
    uint8_t *rev_minor, uint8_t *rev_rc);

/**
 * @brief Perform SPI register write access to device
 *
 * @param device   Pointer to the device structure
 * @param address  SPI address to which the value of data parameter shall be written
 * @param data     8-bit value to be written to SPI register defined
 *                 by the address parameter.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_spi_register_set(adi_hmc7044_device_t *device, uint16_t addr, uint8_t val);

/**
 * @brief Perform SPI register read access to device.
 *
 *
 * @param device   Pointer to the device structure
 * @param address  SPI address from which the value of data parameter shall be read,
 * @param data     Pointer to an 8-bit variable to which the value of the
 *                 SPI register at the address defined by address parameter
 *                 shall be stored.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_spi_register_get(adi_hmc7044_device_t *device, uint16_t addr, uint8_t *val);

/**
 * @brief Trigger Internal Dividers and FSM restart via SPI
 *
 *
 * @param device   Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_trigger_restart(adi_hmc7044_device_t *device);
	
/**
 * @brief Configure and enable GPI
 *
 *
 * @param device       Pointer to the device structure
 * @param gpi_index    Reference GPI index. Range 0 to 3
 *                
 * @param gpi_config   Config setting for GPI defined by the enum adi_hmc7044_gpi_op_config_e
 * @param enable       Enable GPI Driver. 
 * 		       0 Disable driver 
 * 		       1 Enable driver
 *              
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_gpi_config_set(adi_hmc7044_device_t *device, uint8_t gpi_index, uint8_t gpi_config, uint8_t enable);
	
/**
 * @brief Configure and enable GPO
 *
 *
 * @param device       Pointer to the device structure
 * @param gpo_index    Reference GPO index. Range 0 to 3
 *                
 * @param gpo_config   Config setting for GPO defined by the enum adi_hmc7044_gpo_op_config_e
 * @param mode         GPO driver mode
 * @param enable       Enable GPO Driver. 
 *                     0 Disable driver 
 *                     1 Enable driver
 *               
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_gpo_config_set(adi_hmc7044_device_t *device, uint8_t gpo_index, uint8_t gpo_config, uint8_t mode, uint8_t enable);

/**
 * @brief Configure and enable SDATA
 *
 *
 * @param device   Pointer to the device structure
 * @param mode     SDATA driver mode 
 * @param enable   Enable SDATA Driver. 
 *                 0 Disable driver 
 *                 1 Enable driver
 *               
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */	
int32_t adi_hmc7044_sdata_config_set(adi_hmc7044_device_t *device, uint8_t mode, uint8_t enable);

/**
 * @brief Configure and enable Reference Clock Inputs
 *
 *
 * @param device   Pointer to the device structure
 * @param clk_in   Reference clock Input index. Range 0 to 3 and Oscillator input index as 4
 * @param config   Input Buffer Configuration for Reference Clock Input.
 *                 Refer to adi_hmc7044_ip_buffer_settings_e for configuration mask settings.
 *                 For example for ac coupled inputs with 100 Ohm internal termination.
 *                 Set config to IPBUFFER_INTERNAL_100_OHM_EN | PBUFFER_AC_COUPLED_MODE_EN
 *                 and Input Buffer Configuration for Reference Oscillator Input
 * @param enable   Enable setting for Reference clock.
 *                 0 Disable Input
 *                 1 Enable Input
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_input_reference_set(adi_hmc7044_device_t *device, uint8_t clk_in, uint8_t config, uint8_t enable);

/**
 * @brief Configure and enable Reference Clock Inputs
 *
 * @pre-requisite  adi_hmc7044_input_reference_set should be called with required CLKIN and OSCIN configs
 *
 * @param device   Pointer to the device structure
 * @param clk_in   Reference clock Input index. Range 0 to 3 and Oscillator input index as 4
 * @param enable   Enable setting for Reference clock.
 *                 0 Disable Input
 *                 1 Enable Input
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_enable_input_reference_set(adi_hmc7044_device_t *device, uint8_t clk_in, uint8_t enable);

/**
 * @brief readback Reference Clock Inputs
 *
 *
 * @param device   Pointer to the device structure
 * @param status   Readback the setting for Reference clock.
 *                 0 using onboard crystal
 *                 1 using external reference
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_input_reference_get(adi_hmc7044_device_t *device,  uint8_t *status);

/**
 * @brief Set the input reference priority order
 *
 *
 * @param device     Pointer to the device structure
 * @param priority   Pointer to clock reference priority list.
 *                   where priority[0]
 *                   value represents the clock input index with the highest priority.
 * @param nof_ref    Size of/ Number of clock reference priority list. Range 1 to 4
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_input_reference_priority_set(adi_hmc7044_device_t *device, uint8_t priority[4], uint8_t nof_ref);

/**
 * @brief  Configure the operation during Loss of Signal(LOS)
 *
 * @param device         Pointer to the device structure
 * @param timer_cycles   Setting to configure the number of timer cycle to wait
 *                       before exiting LOS state after reference becomes valid.
 *                       Where the set value x, translatest 2^x timer cycles.
 *                       Valid Range 0 to 7
 *                       For example
 *                       0 - no wait
 *                       7 - 128 timer cycles.
 * @param ip_prescaler   LOS operation bypasses input prescaler.
 *                       1 - bypass input prescaler
 *                       0 - do not bypass input prescaler
 * @param vcxo_prescaler LOS operation uses VCXO prescaler.
 *                       1 - Use VCXO prescaler
 *                       0 - do not Use VCXO prescaler
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_input_reference_los_config_set(adi_hmc7044_device_t *device,
        uint8_t timer_cycles, uint8_t prescaler_bypass, uint8_t vcxo_prescaler_en);

/**
 * @brief  Configure the Input Reference Input Prescaler LCM Dividers
 *
 * @param device    Pointer to the device structure
 * @param clk_in    Reference Clock input index. Range 0 to 3
 * @param lcm_div   Value of prescaler divider setpoint 1 to 255
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_input_reference_prescaler_config_set(adi_hmc7044_device_t *device,
        uint8_t clk_in, uint8_t lcm_div);

/**
 * @brief  Configure the Oscillator Input Prescaler LCM Divider
 *
 * @param device    Pointer to the device structure
 * @param lcm_div   Value of prescaler divider setpoint 1 to 255
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_input_reference_oscin_prescaler_config_set(adi_hmc7044_device_t *device, uint8_t lcm_div);

/**
 * @brief  Configure the PLL1 PFD
 *
 * @param device   Pointer to the device structure
 * @param r_div    RefA input divider. 16-bit Value
 * @param n_div    Feedback divider to PLL1 16-bit Value
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll1_config_set(adi_hmc7044_device_t *device, uint16_t r_div, uint16_t n_div);

/**
 * @brief  Configure the PLL2 PFD
 *
 * @param device       Pointer to the device structure
 * @param freq_dbl_en  Enable Pre-divider Frequency Doubler
 * @param r_div        PLL2 R divider. 12 bit vlaue Range 1 to 4095
 * @param n_div        16 Bit Feedback divider to PLL2
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll2_config_set(adi_hmc7044_device_t *device, uint8_t freq_dbl_en, uint16_t r_div, uint16_t n_div);

/**
 * @brief  Get pll1 and pll2  lock status detected by device 
 *
 * @param  device   Pointer to the device structure.
 * @param  status   Pointer to return bitwise representation of pll1 and pll2 lock status
 *                  Bit value 0f 0-> unlocked, bit value of 1->locked
 *                  BIT[0] Represents PLL1 LOCK status
 *                  BIT[1] Represents PLL2 LOCK status 
 *                  BIT[2] Represents PLL1 AND PLL2 Lock status
 *                  Ref also di_hmc7044_pll_status_mask_e 
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter 
 */
int32_t adi_hmc7044_device_pll_lock_status_get(adi_hmc7044_device_t *device, uint8_t *status);

/**
 * @brief  Configure the Output Driver Controls
 *
 * @param device       Pointer to the device structure
 * @param output_ch    Ouput Channel Index. Range 0-13
 * @param output_sel   Output Signal Selection
 * @param ch_div       Ouput Channel Divider setting Range 1-4049
 * @param mode         Performace mode Enable
 *                     1- High Performance Mode Enabled
 *                     0- Normal Output Mode Enabled
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_output_config_set(adi_hmc7044_device_t *device, uint8_t output_ch,
            adi_hmc7044_op_source_e output_sel, uint16_t ch_div, uint8_t mode, uint8_t enable);

/**
 * @brief  Configure the Output Driver Controls
 *
 * @param device       Pointer to the device structure
 * @param output_ch    Ouput Channel Index.
 * @param config       A pointer to a structure defining the desired Output Driver Configuration.
 *        mode         Output Signal Type. Valid options CML, LVPECL, LVDS or CMOS.
 *        impedance    Output Signal Impedendence.
 *                     Valid Options: Internal Resistor Disabled, 100 Ohm, 50 Ohm
 *        dynamic_driver_en   Dynamic Driver Control for Pulse Generator Mode Only
 *                            0 - Driver Enable Controlled by Channel Enable
 *                            1 - Driver Dynamically Enabled with Pulse Gen Events.
 *        force_mute_en       Idle at Logic 0 Mode Enable for Pulse Generator Mode Only
 *                            0 - Normal Mode
 *                            1 - Force to Logic 0
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_output_driver_config_set(adi_hmc7044_device_t *device,
        uint8_t output_ch, adi_hmc7044_op_driver_config_t *config);

/**
 * @brief  Configure the Output Analog and Digital Delay Adjustments
 *
 * @param device       Pointer to the device structure
 * @param output_ch    Ouput Channel Index.
 * @param course_adj   Digital Course Delay Adjustment in 1/2 VCO Clk Step sizesd.Range 0-17
 * @param fine_adj     Analog Fine Delay Adjustment in 25ps. Range 0 to 24.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_output_delay_set(adi_hmc7044_device_t *device,
                uint8_t output_ch, uint8_t coarse_adj, uint8_t fine_adj);

/**
 * @brief  Configure the Output Analog and Digital Delay Adjustments
 *
 * @param device        Pointer to the device structure
 * @param output_ch     Ouput Channel Index.
 * @param startup_mode  Set the channel start mode. Range 0-1
 *                      0 - Asynchronous
 *                      1 - Dynamic / Pulse Generator Mode
 * @param slip_mode_en  Slip Event Enable Setting. Channel processes Slip events. Range 0-1
 * @param sync_mode_en  Sync Event Enable Setting. Channel Process Sync Events. Range 0-1.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_output_sync_config_set(adi_hmc7044_device_t *device, uint8_t output_ch,
                uint8_t startup_mode, uint8_t slip_mode_en, uint8_t sync_mode_en);

/**
 * @brief Get startup mode for channel
 * 
 * @param device                Pointer to the device structure
 * @param ch                    Ouput Channel Index
 * @param startup_mode          Get the channel start mode. Range 0-1
 *                              0 - Asynchronous
 *                              1 - Dynamic / Pulse Generator Mode
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_hmc7044_channel_startup_mode_get(adi_hmc7044_device_t *device, uint8_t ch, uint8_t *startup_mode);

/**
 * @brief  Configure the Channel For Multislip Configuration
 *
 * @param device        Pointer to the device structure
 * @param output_ch     Ouput Channel Index.
 * @param multi_slip_en Multi Slip Enable Setting.Range 0-1.
 * @param slip_delay    Multi Slip Delay Setting in VCO cycles. Range 0-4095.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_output_multi_slip_config_set(adi_hmc7044_device_t *device,
            uint8_t output_ch, uint8_t multi_slip_en, uint16_t slip_delay);
            
/**
 * @brief  Configure the Output Enable
 * Note: All configuration setting should be completed before enabling.
 *
 * @param device   Pointer to the device structure
 * @param enable   Output Signal Enable Range 0-1
 *                 0 - Disable
 *                 1 - Enable.
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_output_enable_set(adi_hmc7044_device_t *device, uint8_t output_ch, uint8_t en);
	            
/**
 * @brief  Configure the Output performance setting
 * Note: All configuration setting should be completed before enabling.
 *
 * @param device   Pointer to the device structure
 * @param enable   optimum performance enable
 *		   0 - Disable
 *		   1 - Enable
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_output_performance_set(adi_hmc7044_device_t *device, uint8_t enable);
	      
/**
 * @brief  HMC register update
 *
 * @param device   					Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_reg_update(adi_hmc7044_device_t *device);

/**
 * @brief Toggle reseed request
 * 
 * @param device                    Pointer to the device structure
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_reseed_request_set(adi_hmc7044_device_t *device);


/**
 * @brief  HMC set to High performace
 *
 * @param device   Pointer to the device structure
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_high_performance_set(adi_hmc7044_device_t *device);
	
/**
 * @brief  Set VCO to be used. External VCO or one of the two Internal VCOs
           External VCO mod has an option to divide input by 2.
 *
 * @param device    Pointer to the device structure
 * @param vco_sel   VCO selection as defined by the enum adi_hmc7044_vco_sel_e
 * @param ext_vco_div_en     Parameter to enable external vco signal by two
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_vco_sel_set(adi_hmc7044_device_t *device, uint8_t vco_sel , uint8_t ext_vco_div_en);

/**
 * @brief  HMC Enable output Channel
 *
 * @param device   Pointer to the device structure
 * @param ch_en    Enable 0-6 to enable 14 output channels
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */	
int32_t adi_hmc7044_channel_out_en(adi_hmc7044_device_t *device, uint8_t ch_en);

/**
 * @brief  Configure  input Reference path configuration
 * 
 * @param device           Pointer to the device structure
 * @param sync_mode        Sync Pin configuration with respect PLL2
 * @param vco_input_mode   CLKIN1 used for external VCO
 *						   0 not used
 *						   1 used
 * @param sync_input_mode  CLKIN0 used for external RF sync
 *						   0  not used
 *						   1 used
 * @param clk_in           CLKIN selected for PLL1 Reference, range 0-3
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_input_reference_path_en(adi_hmc7044_device_t *device, uint8_t sync_mode, uint8_t vco_input_mode, uint8_t sync_input_mode, uint8_t clk_in);
	
/**
 * @brief Configure PLL1 HOldover Exit set
 * 
 * @param device   Pointer to the device structure
 * @param config   Holdover configuration object as defined by adi_hmc_pll1_holdover_config_t
 *        exit_action    PLL1 exit action mode HMC7004_HO_EA_RESET_DIVIDERS, HMC7044_HO_EA_DO_NOTHING_1, HMC7044_HO_EA_DO_NOTHING_2, HMC7044_HO_EA_DAC_ASSIT
 *        exit_criteria  PLL1 exit criteria mode HMC7044_HO_EC_LOS_GONE_0, HMC7044_HO_EC_ZERO_PHASE_ERR, HMC7044_HO_EC_LOS_GONE_2, HMC7044_HO_EC_IMMEDIATE
 *        holdover_dac   Holdover DAC value
 *        adc_tracking   ADC tracking
 *                       1 - Disable ADC tracking
 *                       0 - Use DAC Hold value
 *        quick_mode     Force DAC to holdover
 *        		 1 - Force DAC control value to DAC holdover value immediately
 *        		 0 -  Force DAC control value to DAC holdover value gradually
 *        holdover_bw    Tracking BW reduction value
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */	
int32_t adi_hmc7044_pll1_holdover_exit_ctrl_set(adi_hmc7044_device_t *device, adi_hmc_pll1_holdover_config_t *config);

/**
 * @brief set charge pump for PLL1
 * 
 * @param device        Pointer to the device structure
 * @param charge_pump   Charge pump value
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll1_charge_pump_ctrl_set(adi_hmc7044_device_t *device, uint8_t charge_pump);

/**
 * @brief set PLL1 dectect control
 * 
 * @param device            Pointer to the device structure
 * @param slip_use          Use slip indicator
 * @param lock_detect_timer PLL1 lock detect center depth value
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll1_lock_detect_set(adi_hmc7044_device_t *device, uint8_t slip_use, uint8_t lock_detect_timer);

/**
 * @brief set PLL1 detect control
 * 
 * @param device             Pointer to the device structure
 * @param bypass_debounce    bypass debouncer in manual mode
 * @param manual_switch      manual reference mode
 * @param holdover_dac_use   Holdover uses
 *			     0 - Tristate hcarge pump
 *			     1 - Holdover DAC
 * @param autorevert_switch  Revert to PLL2 best option if available again
 * @param automode_switch    Automatic swtiching mode using the reference clk priority
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll1_reference_switch_ctrl_set(adi_hmc7044_device_t *device, uint8_t bypass_debounce, uint8_t manual_switch, uint8_t holdover_dac_use, uint8_t autorevert_switch, uint8_t automode_switch);

/**
 * @brief set PLL1 hold offo time control set
 * 
 * @param device          Pointer to the device structure
 * @param hold_off_time   Holdoff timer value
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll1_holdoff_time_ctrl_set(adi_hmc7044_device_t *device, uint8_t hold_off_time);
	
/**
 * @brief set charge pump for PLL2
 * 
 * @param device          Pointer to the device structure
 * @param charge_pump     Charge pump value
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll2_charge_pump_ctrl_set(adi_hmc7044_device_t *device, uint8_t charge_pump);

/**
 * @brief set OSC out path 
 * 
 * @param device     Pointer to the device structure
 * @param divider    Oscillator output divider ratio
 * @param enable     Enable Oscillator output path
 *		     0 - Disable path 
 *		     1 - Enable path
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_oscout_path_config_set(adi_hmc7044_device_t *device, uint8_t divider, uint8_t enable);
	
/**
 * @brief  Configure the Output Driver Controls for Oscillators
 *
 * @param device      Pointer to the device structure
 * @param output_ch   Ouput Channel Index.
 * @param config      A pointer to a structure defining the desired Output Driver Configuration.
 *        mode        Output Signal Type. Valid options CML, LVPECL, LVDS or CMOS.
 *        impedance   Output Signal Impedendence.
 *                    Valid Options: Internal Resistor Disabled, 100 Ohm, 50 Ohm
 *        dynamic_driver_en   Enable Oscillator driver 
 *                            0 - Driver Disable 
 *                            1 - Driver Enable
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_oscout_driver_config_set(adi_hmc7044_device_t *device, uint8_t oscout_ch, adi_hmc7044_op_driver_config_t *config);

/**
 * @brief Configure global enable control
 * 
 * @param device                Pointer to the device structure
 * @param reseed_en             Enable RF reseed for SYSREF
 * @param sysref_timer_en       Enable internal SYSREF time reference
 * 
 * @return API_CMS_ERROR_OK      API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_device_sysref_enable_control_set(adi_hmc7044_device_t *device, uint8_t reseed_en, uint8_t sysref_timer_en); 

/**
 * @brief  Configure the SYNC
 *
 * @param device           Pointer to the device structure
 * @param pulse_gen_mode   Pulse Generation mode selection, as defined by the enum adi_hmc7044_sysref_mode_config_e
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_sysref_config_set(adi_hmc7044_device_t *device, uint8_t pulse_gen_mode);
	
/**
 * @brief  Configure the SYNC
 *
 * @param device        Pointer to the device structure
 * @param sync_retime   Retime the Sync
 *		        0 Bypass the retime
 *		        1 Retime the external SYNC from Ref 0
 *					     
 * @param sync_pll2     Reseed event thorugh PLL2
 *			0 Do not allow Reseed event through PLL2
 *			1 allow Reseed event through PLL2
 *						 
 * @param sync_polarity SYNC Polarity
 *			0 Positive
 *			1 Negative
 *						 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_sync_config_set(adi_hmc7044_device_t *device, uint8_t sync_retime, uint8_t sync_pll2, uint8_t sync_polarity);

/**
 * @brief  Configure the SYREF Timer
 *
 * @param device          Pointer to the device structure
 * @param sysref_timer    12-bit SYSref Timer set point value    
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_sysref_timer_config_set(adi_hmc7044_device_t *device, uint16_t sysref_timer);

/**
 * @brief Toggle pulse generator request
 * 
 * @param device    Pointer to the device structure
 * 
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return <0                                   Failed. @see adi_cms_error_e for details. 
 */
int32_t adi_hmc7044_pulse_gen_set(adi_hmc7044_device_t *device);

/**
* @brief  Configure the Alarm Mask
*
* @param device    Pointer to the device structure
* @param config    Configuguration of the alarm mask as defined in adi_hmc7044_alarm_mask_config_t
*        pll1_lock                 PLL1 near lock signal to generate alarm signal            
*                                  0 - Do not allow
*                                  1 - Allow
*        pll1_lock_aquisition      PLL1 lock acquisition signal to generate alarm signal
*                                  0 - Do not allow
*                                  1 - Allow
*        pll1_lock_detect          PLL1 lock detect signal to generate alarm signal
*                                  0 - Do not allow
*                                  1 - Allow
*        pll1_holdover_status      PLL1 holdover status signal to generate alarm signal
*        pll1_clkInx_status        CLKINx los to generate alarm signal, as defined by adi_hmc7044_clk_in_e enum
*                                  0 - Do not allow
*                                  1 - Allow
*        sync_request              sync request signals to generate alarm signal
*                                  0 - Do not allow
*                                  1 - Allow
*        pll1_pll2_lock_detect     PLL1 and PLL2 lock detect signals to generate alarm signal
*                                  0 - Do not allow
*                                  1 - Allow
*        clkoutputs_phase_status   Clock outputs phase status signal to generate alarm signal
*                                  0 - Do not allow
*                                  1 - Allow
*        sysref_sync_status        SYSREF sync status signal to generate alarm signal
*                                  0 - Do not allow
*                                  1 - Allow
*        pll2_lock_detect          PLL2 lock detect signal to generate alarm signal
*                                  0 - Do not allow
*                                  1 - Allow
*
* @return API_CMS_ERROR_OK                     API Completed Successfully
* @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
* @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
* @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
*/	
int32_t adi_hmc7044_alarm_mask_config_set(adi_hmc7044_device_t *device, adi_hmc7044_alarm_mask_config_t *config);

/**
 * @brief  Config PLLs (R1 N1 R2 N2) based on  pfd frequcencies 
 *         for desired up distribution Frequency
 *
 * @param device       		   Pointer to the device structure
 * @param ref_clk_freq_hz	   on board osc ref clk
 * @param pfd1_freq_hz		   phase frequency detector for pll1
 * @param pfd2_freq_hz		   phase frequency detector for pll2
 * @param dist_freq_hz		   Distribution frequency
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_distribution_clk_config(adi_hmc7044_device_t *device, uint64_t ref_clk_freq_hz, uint64_t pfd1_freq_hz, uint64_t dist_freq_hz, uint64_t pfd2_freq_hz);
			

/**
 * @brief  HMC7044 clock configuration top level API,  need to call adi_hmc7044_reg_update() API after this API call
 *
 * @param device                    Pointer to the device structure
 * @param ref_ch				    Channel mask of Clock Input reference source
 * @param ref_clk_freq_hz           Ref clk from clock input channel		
 * @param ref_priority				Ref clk priority 
 * @param fvcxo_clk_freq_hz         Desired fvcxo clk frequency
 * @param output_ch                 Channel mask of desired output channel 								
 * @param output_freq_hz[14]        Desired generated clocks on desired output channel
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_clk_config(adi_hmc7044_device_t *device, adi_hmc7044_clk_in_e ref_ch, uint8_t ref_priority[4], uint64_t ref_clk_freq_hz, uint64_t fvcxo_clk_freq_hz, uint16_t output_ch, uint64_t output_freq_hz[14]);

/**
 * @brief  HMC7044 pll configuration 
 *
 * @param device                    Pointer to the device structure
 * @param ref_ch                    Channel mask of Clock Input reference source
 * @param ref_clk_freq_hz           Ref clk from clock input channel										
 * @param fvcxo_clk_freq_hz         Desired fvcxo clk frequency
 * @param fpfd1_freq_hz             Phase frequency detector for pll1
 * @param fvco_freq_hz              Desired fvco clk frequency 
 *
 * @return API_CMS_ERROR_OK                     API Completed Successfully
 * @return API_CMS_ERROR_INVALID_HANDLE_PARAM   Invalid Device Handle
 * @return API_CMS_ERROR_SPI_XFER               SPI Access Failed
 * @return API_CMS_ERROR_INVALID_PARAM          Invalid Parameter
 */
int32_t adi_hmc7044_pll_config(adi_hmc7044_device_t *device, adi_hmc7044_clk_in_e ref_ch, uint64_t ref_clk_freq_hz, uint64_t fvcxo_clk_freq_hz, uint64_t fpfd1_freq_hz, uint64_t fvco_freq_hz);	

#ifdef __cplusplus
}
#endif

#endif /* __ADI_HMC7044_H__ */
/*! @} */