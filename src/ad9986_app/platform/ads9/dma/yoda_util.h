/*!
 * @brief     YODA UTIL Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_YODA_UTIL__
 * @{
 */

#ifndef YODA_UTIL_H
#define YODA_UTIL_H

/*============= I N C L U D E S ============*/
#include "xil_io.h"
#include "fpga_regs.h"
#include "hal_fpga.h"

/*============= D E F I N E S ==============*/
#define GET_YODA_FIELD(reg_data, field_name)        (((reg_data) & (BITM_ ## field_name)) >> (BITP_ ## field_name))
#define SET_YODA_FIELD(field_name, field_data)      (((field_data) << (BITP_ ## field_name)) & (BITM_ ## field_name))

#define SET_VAR_YODA_FIELD(var, field_name_p, field_name_m, field_data) (var) |= ((field_data << BITP_ ## field_name_p) & BITM_ ## field_name_m)

#define YODA_ADDR(base, reg)                        ((base) + (4 * (reg)))
#define YODA_WR(base, reg, data)                    (Hal_FpgaOut32(reg, data))
#define YODA_RD(base, reg)                          (Hal_FpgaIn32(reg))
#define YODA_WR_MASK(base, reg, data, mask)         (YODA_WR(base, reg, ((data) & (mask)) | ((YODA_RD(base, reg) & ~(mask)))))
#define YODA_WR_FIELD(base, reg, data, field_name)  (YODA_WR_MASK(base, reg, SET_YODA_FIELD(field_name, data), BITM_ ## field_name))
#define YODA_RD_FIELD(base, reg, field_name)        (GET_YODA_FIELD(YODA_RD(base, reg), field_name))

#endif

/*! @} */
