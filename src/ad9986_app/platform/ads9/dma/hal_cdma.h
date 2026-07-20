/*!
 * @brief     HAL CDMA Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_HAL_CDMA__
 * @{
 */


#ifndef HAL_CDMA_H           /* prevent circular inclusions */
#define HAL_CDMA_H           /* by using protection macros */


#ifdef __cplusplus
extern "C" {
#endif


/*============= I N C L U D E S ============*/
#include "xil_types.h"




/*============= D E F I N E S ==============*/



/*============= E X P O R T S ==============*/
unsigned int Hal_CdmaInit(unsigned int baseaddr, unsigned int size);
unsigned int Hal_CdmaIn32(unsigned int offset);
unsigned int Hal_CdmaOut32(unsigned int offset, unsigned int Value);
unsigned int Hal_CdmaDeinit(void);



#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

/*! @} */
