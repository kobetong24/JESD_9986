/*!
 * @brief     HMC7044 register and bit fields
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __HMC7044_REG_
 * @{
 */

#ifndef __HMC7044_REG_H__
#define __HMC7044_REG_H__

/*============= I N C L U D E S ============*/
#include "adi_utils.h"

/*============= D E F I N E S ==============*/
#define HMC7044_

#define HMC7044_GLOBAL_SW_RESET_CTRL_REG        0x00
#define HMC7044_GLOBAL_REQUEST_MODE_CTRL_REG	0x01
#define HMC7044_GLOBAL_ENABLE_CTRL_REG			0x03
#define HMC7044_GLOBAL_CH_ENABLE_CTRL_REG       0x04
#define HMC7044_GLOBAL_MODE_ENABLE_CTRL_REG     0x05
#define HMC7044_SOFT_RESET                      ADI_UTILS_BIT(0)
#define HMC7044_RESET_DIV_FSM                   ADI_UTILS_BIT(1)
#define HMC7044_MUTE_OP_DRIVERS_EN              ADI_UTILS_BIT(2)
#define HMC7044_CLK_IP_BUFF_BASE_REG            0x000A
#define HMC7044_CLK_IP_BUFF_OFFSET              0x1
#define HMC7044_CLK_IP_PRIORITY_REG             0x014

#define HMC7044_PLL1_LOS_TIMER_CTRL_REG         0x0015
#define HMC7044_PLL1_HOLDOVER_EXIT_CTRL_REG     0x0016
#define HMC7044_PLL1_HOLDOVER_DAC_CTRL_REG      0x0017
#define HMC7044_PLL1_HOLDOVER_ADC_CTRL_REG      0x0018
#define HMC7044_PLL1_CHARGE_PUMP_CRTL_REG       0x001A
#define HMC7044_PLL1_LOS_VALID_TIMER(x)         ((x & 0x7)<<0)
#define HMC7044_PLL1_LOS_MODE_REG               0x0019
#define HMC7044_LOS_INPUT_PRESCALER_BYPASS      ADI_UTILS_BIT(1)
#define HMC7044_LOS_VCXO_PRESCALER_EN           ADI_UTILS_BIT(0)

#define HMC7044_CHIP_ID_0_REG                   0x078
#define HMC7044_CHIP_ID_1_REG                   0x079
#define HMC7044_CHIP_ID_2_REG                   0x07A

#define HMC7044_CLKINX_PRESCALER_BASE_REG       0x001C
#define HMC7044_CLKINX_PRESCALER_OFFSET         0x1

#define HMC7044_OSCIN_PRESCALER_REG             0x0020

#define HMC7044_PLL1_R_DIV_LSB_REG             0x0021
#define HMC7044_PLL1_R_DIV_MSB_REG             0x0022

#define HMC7044_PLL1_N_DIV_LSB_REG             0x0026
#define HMC7044_PLL1_N_DIV_MSB_REG             0x0027

#define HMC7044_PLL1_LOCK_DETECT_CTRL_REG      0x0028
#define HMC7044_PLL1_REF_SWITCH_CTRL_REG       0x0029
#define HMC7044_PLL1_HOLDOFF_TIME_CRTL_REG     0x002A

#define HMC7044_PLL2_FREQ_DOUBLER_REG          0x0032
#define HMC7044_PLL2_FREQ_DOUBLER_EN           ADI_UTILS_BIT(0)
#define HMC7044_PLL2_R_DIV_LSB_REG             0x0033
#define HMC7044_PLL2_R_DIV_MSB_REG             0x0034

#define HMC7044_PLL2_N_DIV_LSB_REG             0x0035
#define HMC7044_PLL2_N_DIV_MSB_REG             0x0036

#define HMC7044_PLL2_CHARGE_PUMP_CRTL_REG      0x0037
#define HMC7044_PLL2_OSCOUT_PATH_CTRL_REG      0x0039
#define HMC7044_OSCOUT1_CTRL_REG               0x003A
#define HMC7044_OSC_OP_DRIVER_MODE(x)          ((x & 0x3)<<4)
#define HMC7044_OSC_OP_DRIVER_IMPEDANCE(x)     ((x & 0x3)<<1)

#define HMC7044_SYSREF_SYNC_CTRL_REG           0x005A
#define HMC7044_SYNC_CTRL_REG                  0x005B
#define HMC7044_SYNC_TIMER_LSB_CTRL_REG        0x005C
#define HMC7044_SYNC_TIMER_MSB_CTRL_REG        0x005D

#define HMC7044_ALARM_MASK_CTRL_1_REG          0x0070
#define HMC7044_ALARM_MASK_CTRL_2_REG          0x0071

#define HMC7044_GPI_CTRL_1_REG				   0x0046
#define HMC7044_GPI_CTRL_2_REG				   0x0047
#define HMC7044_GPI_CTRL_3_REG				   0x0048
#define HMC7044_GPI_CTRL_4_REG				   0x0049
#define HMC7044_GPO_CTRL_1_REG				   0x0050
#define HMC7044_GPO_CTRL_2_REG				   0x0051
#define HMC7044_GPO_CTRL_3_REG				   0x0052
#define HMC7044_GPO_CTRL_4_REG				   0x0053

#define HMC7044_SDATA_CTRL_REG				   0x0054

#define HMC7044_CLK_EXT_VCO_CTRL               0x0064
#define HMC7044_CLK_OP_CTRL_0_REG              0x00C8
#define HMC7044_CLK_OP_HIGH_PERFORM_EN         ADI_UTILS_BIT(7)
#define HMC7044_CLK_OP_SYNC_EN                 ADI_UTILS_BIT(6)
#define HMC7044_CLK_OP_SLIP_EN                 ADI_UTILS_BIT(5)
#define HMC7044_CLK_OP_STARTUP_MODE(x)         ((x & 0x3)<<2)
#define HMC7044_CLK_OP_MULTI_SLIP_EN           ADI_UTILS_BIT(1)
#define HMC7044_CLK_OP_EN                      ADI_UTILS_BIT(0)
#define HMC7044_CLK_OP_CTRL_1_REG              0x00C9
#define HMC7044_CLK_OP_CTRL_2_REG              0x00CA
#define HMC7044_CLK_OP_CTRL_3_REG              0x00CB
#define HMC7044_CLK_OP_FINE_DELAY(x)           ((x & 0xF)<<0)
#define HMC7044_CLK_OP_CTRL_4_REG              0x00CC
#define HMC7044_CLK_OP_COURSE_DELAY(x)         ((x & 0x1F)<<0)
#define HMC7044_CLK_OP_CTRL_5_REG              0x00CD
#define HMC7044_CLK_OP_CTRL_6_REG              0x00CE
#define HMC7044_CLK_OP_CTRL_7_REG              0x00CF
#define HMC7044_CLK_OP_MUX_SEL(x)              ((x & 0x3)<<0)
#define HMC7044_CLK_OP_CTRL_8_REG              0x00D0
#define HMC7044_CLK_OP_FORCE_MUTE(x)           ((x & 0x3)<<6)
#define HMC7044_CLK_OP_DYNAMIC_DR_EN           ADI_UTILS_BIT(5)
#define HMC7044_CLK_OP_DRIVER_MODE(x)          ((x & 0x3)<<3)
#define HMC7044_CLK_OP_DRIVER_IMPEDANCE(x)     ((x & 0x3)<<0)
#define HMC7044_CLK_OP_CTRL_9_REG              0x00D1

#define HMC7044_CLK_OP_CTRL_OFFSET             0x0A
#define HMC7044_OSC_OP_CTRL_OFFSET             0x01

#endif /*__HMC7044_REG_H__*/
/*! @} */
