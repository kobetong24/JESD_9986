/**************************************************************************//**
*   @file   AD7175_regs.h
*   @brief  AD7175 Registers Definitions.
*   @author acozma (andrei.cozma@analog.com)
*
*******************************************************************************
* Copyright 2011(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*  - Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  - Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
*  - Neither the name of Analog Devices, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*  - The use of this software may or may not infringe the patent rights
*    of one or more patent holders.  This license does not release you
*    from the requirement that you obtain separate licenses from these
*    patent holders to use this software.
*  - Use of the software either in source or binary form, must be run
*    on or directly connected to an Analog Devices Inc. component.
*
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

/*!
 * @addtogroup __AD7175_REG_
 * @{
 */

#ifndef __AD7175_REGS_H__
#define __AD7175_REGS_H__

/*============= I N C L U D E S ============*/
#include "adi_utils.h"

/*============= D E F I N E S ==============*/
#define AD7175_COMMS_REG	0x00
#define AD7175_STATUS_REG	0x00
#define AD7175_ADC_MODE_REG	0x01
#define AD7175_IF_MODE_REG	0x02
#define AD7175_DATA_REG		0x04
#define AD7175_IOCON_REG	0x06
#define AD7175_ID_REG		0x07
#define AD7175_CH0_REG		0x10
#define AD7175_CH1_REG		0x11
#define AD7175_CH2_REG		0x12
#define AD7175_CH3_REG		0x13
#define AD7175_CH4_REG		0x14
#define AD7175_CH5_REG		0x15
#define AD7175_CH6_REG		0x16
#define AD7175_CH7_REG		0x17
#define AD7175_CH8_REG		0x18
#define AD7175_CH9_REG		0x19
#define AD7175_CH10_REG		0x1A
#define AD7175_CH11_REG		0x1B
#define AD7175_CH12_REG		0x1C
#define AD7175_CH13_REG		0x1D
#define AD7175_CH14_REG		0x1E
#define AD7175_CH15_REG		0x1F
#define AD7175_SETUPCON0_REG	0x20

#define AD7175_FILTCON0_REG	0x28




#define AD7175_SLAVE_ID    1	

/* Communication Register bits */
#define COMM_REG_WEN    ADI_UTILS_BIT(7)
#define COMM_REG_WR     (0 << 6)
#define COMM_REG_RD     ADI_UTILS_BIT(6)

/* Status Register bits */
#define STATUS_REG_RDY      ADI_UTILS_BIT(7)
#define STATUS_REG_ADC_ERR  ADI_UTILS_BIT(6)
#define STATUS_REG_CRC_ERR  ADI_UTILS_BIT(5)
#define STATUS_REG_REG_ERR  ADI_UTILS_BIT(4)
#define STATUS_REG_CH(x)    ((x) & 0x0F)

/* ADC Mode Register */
#define ADC_MODE_REG_REF_EN         ADI_UTILS_BIT(15)
#define ADC_MODE_REG_DELAY(x)       (((x) & 0x7) << 8)
#define ADC_MODE_REG_MODE(x)        (((x) & 0x7) << 4)
#define ADC_MODE_REG_CLKSEL(x)      (((x) & 0x3) << 2)

/* Interface Mode Register bits */
#define INTF_MODE_REG_DOUT_RESET    ADI_UTILS_BIT(8)
#define INTF_MODE_REG_CONT_READ     ADI_UTILS_BIT(7)
#define INTF_MODE_REG_DATA_STAT     ADI_UTILS_BIT(6)
#define INTF_MODE_REG_CRC_EN        (0x02 << 2)
#define INTF_MODE_REG_CRC_STAT(x)   (((x) & INTF_MODE_REG_CRC_EN) == INTF_MODE_REG_CRC_EN)
#define INTF_MODE_REG_WL16          ADI_UTILS_BIT(0)

/* GPIO Configuration Register */
#define GPIO_CONF_REG_MUX_IO        ADI_UTILS_BIT(12)
#define GPIO_CONF_REG_SYNC_EN       ADI_UTILS_BIT(11)
#define GPIO_CONF_REG_ERR_EN(x)     (((x) & 0x3) << 9)
#define GPIO_CONF_REG_ERR_DAT       ADI_UTILS_BIT(8)
#define GPIO_CONF_REG_IP_EN1        ADI_UTILS_BIT(5)
#define GPIO_CONF_REG_IP_EN0        ADI_UTILS_BIT(4)
#define GPIO_CONF_REG_OP_EN1        ADI_UTILS_BIT(3)
#define GPIO_CONF_REG_OP_EN0        ADI_UTILS_BIT(2)
#define GPIO_CONF_REG_DATA1         ADI_UTILS_BIT(1)
#define GPIO_CONF_REG_DATA0         ADI_UTILS_BIT(0)

/* ID Register */
#define ID_REG_PRODUCT_ID(x)        (((x) & 0xFF) << 8)

/* Channel Map Register 1-4 */
#define CH_MAP_REG_CHEN         ADI_UTILS_BIT(15)
#define CH_MAP_REG_SETUP(x)     (((x) & 0x7) << 12)
#define CH_MAP_REG_AINPOS(x)    (((x) & 0x1F) << 5)    
#define CH_MAP_REG_AINNEG(x)    (((x) & 0x1F) << 0)

/* Setup Configuration Register 1-4 */
#define SETUP_CONF_REG_CHOP_MD(x)       (((x) & 0x3) << 14)
#define SETUP_CONF_REG_BI_UNIPOLAR      ADI_UTILS_BIT(12)
#define SETUP_CONF_REG_REF_BUF_P        ADI_UTILS_BIT(11)
#define SETUP_CONF_REG_REF_BUF_N        ADI_UTILS_BIT(10)
#define SETUP_CONF_REG_AIN_BUF_P        ADI_UTILS_BIT(9)
#define SETUP_CONF_REG_AIN_BUF_N        ADI_UTILS_BIT(8)
#define SETUP_CONF_REG_BRNOUT_EN        ADI_UTILS_BIT(7)
#define SETUP_CONF_REG_REF_SEL(x)       (((x) & 0x3) << 4)

/* Filter Configuration Register 1-4 */
#define FILT_CONF_REG_EXTCLKFREQ(x)     (((x) & 0x3) << 13)
#define FILT_CONF_REG_ENHFILTEN         ADI_UTILS_BIT(11)
#define FILT_CONF_REG_ENHFILTSEL(x)     (((x) & 0x7) << 8)
#define FILT_CONF_REG_ORDER(x)          (((x) & 0x7) << 5)
#define FILT_CONF_REG_ODR(x)            (((x) & 0x1F) << 0)


#endif //__AD7175_REGS_H__
/*! @} */