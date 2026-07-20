/*!
 * @brief     COMMON Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_COMMON__
 * @{
 */

#ifndef COMMON_H
#define COMMON_H




/*============= I N C L U D E S ============*/
#include "xparameters.h"
#include "xil_io.h"
#include "xil_cache.h"



/*============= D E F I N E S ==============*/
#define reg_wr(x,y) Xil_Out32(x,y)
#define reg_rd(x) Xil_In32(x)
#define delay_us(us) usleep(us)
#define delay_ms(ms) usleep(1000*ms)




/*============= E X P O R T S ==============*/
void cache_flush(void);
void system_init(void);
void system_close(void);

#endif

/*! @} */
