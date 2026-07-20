/*!
 * @brief     FPGA REG UTIL Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_FPGA_REG_UTIL__
 * @{
 */
#ifndef FPGA_REG_UTIL_H
#define FPGA_REG_UTIL_H

/*============= I N C L U D E S ============*/
#include "xparameters.h"
#include "xparameters_ads9v2.h"
#include "yoda_util.h"

/*============= D E F I N E S ==============*/
#define GET_FPGA_REG_FIELD(reg_data, field_name)   GET_YODA_FIELD(reg_data, FPGA_REGS_ ## field_name)
#define SET_FPGA_REG_FIELD(field_name, field_data) SET_YODA_FIELD(FPGA_REGS_ ## field_name, field_data)

#define SET_VAR_FPGA_REG_FIELD(var, field_name, field_data) SET_VAR_YODA_FIELD(var, FPGA_REGS_ ## field_name, field_data)

#define FPGA_REG_ADDR(reg)                          YODA_ADDR(XPAR_M_AXI_REGBUS_BASEADDR, reg)
#define FPGA_REG_WR(reg, data)                      YODA_WR(XPAR_M_AXI_REGBUS_BASEADDR, reg, data)
#define FPGA_REG_RD(reg)                            YODA_RD(XPAR_M_AXI_REGBUS_BASEADDR, reg)
#define FPGA_REG_WR_MASK(reg, data, mask)           YODA_WR_MASK(XPAR_M_AXI_REGBUS_BASEADDR, reg, data, mask)
#define FPGA_REG_WR_FIELD(reg, data, field_name)    YODA_WR_FIELD(XPAR_M_AXI_REGBUS_BASEADDR, reg, data, FPGA_REGS_ ## field_name)
#define FPGA_REG_RD_FIELD(reg, field_name)          YODA_RD_FIELD(XPAR_M_AXI_REGBUS_BASEADDR, reg, FPGA_REGS_ ## field_name)

#define MZ_REG_WR(reg, data)                        YODA_WR(XPAR_M_AXI_REGBUS_MZ_BASEADDR, reg, data)
#define MZ_REG_RD(reg)                              YODA_RD(XPAR_M_AXI_REGBUS_MZ_BASEADDR, reg)

#endif

/*! @} */
