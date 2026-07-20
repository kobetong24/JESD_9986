/*!
 * @brief     HAL FPGA Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_HAL_FPGA__
 * @{
 */


#ifndef HAL_FPGA_H           /* prevent circular inclusions */
#define HAL_FPGA_H           /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/*============= I N C L U D E S ============*/
#include "xil_types.h"




/*============= D E F I N E S ==============*/





/*============= E X P O R T S ==============*/
unsigned int Hal_FpgaInit(unsigned int baseaddr, unsigned int size);
unsigned int Hal_FpgaAddr(unsigned int reg);
unsigned int Hal_FpgaIn32(unsigned int reg);
unsigned int Hal_FpgaOut32(unsigned int reg, unsigned int Value);
unsigned int Hal_FpgaDeinit(void);





#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

/*! @} */
