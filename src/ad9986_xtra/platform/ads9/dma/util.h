/*!
 * @brief     UTIL Header File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_UTIL__
 * @{
 */

#ifndef UTIL_H_
#define UTIL_H_


/*============= I N C L U D E S ============*/
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


/*============= D E F I N E S ==============*/

/*! \brief Wait until test evaluates to true, or until a timeout is reached
  \param [in] timeout max time to wait, in microseconds
  \param [in] loop_delay time to wait between calls to test, in microseconds
  \param [out] timed_out error variable which will be set to non-zero if timeout occured. Must already be declared.
  \param [in] test statement to evaluate, stop if evaluates to true
  
  Example:
  int timed_out;
  while_timeout(100, 10, timed_out, my_function());
*/
#define while_timeout(timeout, loop_delay, timed_out, test) \
  timed_out = 1; \
  for(int _timeout_loop_ = 0; _timeout_loop_ < ((timeout + loop_delay - 1) / loop_delay); _timeout_loop_++) { \
    if(test) { \
      timed_out = 0; \
      break; \
    } \
    usleep(loop_delay); \
  } \
  if(timed_out && (test)) { \
    timed_out = 0; \
  }
  
#ifdef __cplusplus
}
#endif



#endif

/*! @} */
