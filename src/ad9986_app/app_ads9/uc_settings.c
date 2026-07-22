/*!
 * @brief     Use Case Settings
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_AD9986_APP__
 * @{
 */
/******************************************************
The AD9081/AD9082/AD9988/AD9986 standalone application use-case Settings holds the AD9081/AD9082/AD9988/AD9986 configuration parameters of a number of modes of operation or "usecase" for the ad9986.
Currently the Standalone App is QA'd with 20+ usecases that. These are usecases that the AD9081/AD9082/AD9988/AD9986 ADI evaluation platforms can support and can be used as a reference to�
build a usecase parameters for a custom application.
*/

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <unistd.h>
#include "adi_ad9986.h"

/*============= D A T A ====================*/
/* Usecase Frequency Scheme Settings
*   This array list the desired clocks required for the usecase.
*   For each usecase an array of depth 4 provides the frequecy in Hz for the following clocks
*   As this application targets the Txfe/MxFE evaluation board, that my have various XTAL as reference to the HMC7044.
*   The comments following the usecase entry will indicate if the scheme can be used with the target hardware.
*   The Comments also provide the expected lane rate of the usecase.
*          clk_hz["usecase"][0]  Txfe/MxFE Device reference Clock (dev_ref), if not using on chip pll, this should be equal to the dac_clk
*          clk_hz["usecase"][1]  The value of the reference clock to the ADS9 FPGA JESD Blocks, This is not used by MxFE/TxFE API. (204C lane rate/66) or (204B lane rate/20)
*          clk_hz["usecase"][2]  The MxFE TX data path DAC sampling clock (dac_clk) as per the desired usecase
*          clk_hz["usecase"][3]  The MxFE Rx data path ADC aampling clock (adc_clk) as per the desired usecase
*   Notes:
*         The Example Application will use adi_ad9986_device_clk_config_set to configure the MxFE/TxFE based on th dev_ref, dac_clk, and adc_clk.
*         If the on-chip PLL feature of MxFe/Txfe is not required dev_ref and dac_clk should be set to the same value.
*         If on chip PLL is required, then based on the applied dev_ref clock and the desired dac_clk the API will configure the onchip -pll appropriately
*         The API will also configure the chip to generate the desired adc_clk based on the dac_clk.
*
*/
uint64_t clk_hz[][4] = {
    /*dev_ref,  fpga_ref, dac_clk,    adc_clk */   /* UC,   JESD,   Crystal type,       Lane rate,   Comments                  */
    { 122.88e6, 122.88e6, 5898.24e6,  2949.12e6 }, /* uc0,  nco test                                                           */
    //{ 122.88e6, 737.28e6, 5898.24e6,  2949.12e6 }, /* uc1,  204B,   100MHz/122.88MHz    14.54560Gbps                           */
    { 122.88e6, 122.88e6, 7864.32e6,  3932.16e6 },
    { 122.88e6, 737.28e6, 8847.36e6,  2949.12e6 }, /* uc2,  204B,   100MHz/122.88MHz,   14.54560Gbps                           */
    { 122.88e6, 368.64e6, 5898.24e6,  2949.12e6 }, /* uc3,  204B,   100MHz/122.88MHz,    7.37280Gbps                           */
    { 122.88e6, 368.64e6, 8847.36e6,  2949.12e6 }, /* uc4,  204B,   100MHz/122.88MHz,    7.37280Gbps                           */
    { 384e6,    768e6,    6144e6,     3072e6    }, /* uc5,  204B,   100MHz/122.88MHz,   15.36000Gbps                           */
    { 384e6,    768e6,    6144e6,     3072e6    }, /* uc6,  204B,   100MHz/122.88MHz,   15.36000Gbps                           */
    { 384e6,    768e6,    6144e6,     3072e6    }, /* uc7,  204B,   100MHz/122.88MHz,   15.36000Gbps                           */
    { 384e6,    768e6,    6144e6,     3072e6    }, /* uc8,  204B,   100MHz/122.88MHz,   15.36000Gbps                           */
    { 122.88e6, 737.28e6, 5898.24e6,  2949.12e6 }, /* uc9,  204B,   100MHz/122.88MHz,   14.54560Gbps, 9082 rx FBW              */
    { 122.88e6, 122.88e6, 5898.24e6,  1474.56e6 }, /* uc10, 204C,   100MHz/122.88MHz,    8.11008Gbps                           */
    { 11796.48e6,245.76e6,11796.48e6, 2949.12e6 }, /* uc11, 204C,   Direct Clock,       16.22016Gbps  Eye scan                 */
    { 122.88e6, 368.64e6, 11796.48e6, 2949.12e6 }, /* uc12, 204C,   100MHz/122.88MHz,   24.33024Gbps                           */
    { 125e6,    250e6,    12000e6,    4000e6    }, /* uc13, 204C,   100MHz,             16.50000Gbps, loopback                 */
    { 125e6,    375e6,    6000e6,     3000e6    }, /* uc14, 204C,   100MHz,             24.75000Gbps, 9081 rx FBW              */
    { 122.88e6, 184.32e6, 7864.32e6,  3932.16e6 }, /* uc15, 204C,   100MHz/122.88MHz,   12.16512Gbps, 9082 rx FBW              */
    { 5898.24e6,184.32e6, 5898.24e6,  2949.12e6 }, /* uc16, 204C,   Direct Clock,       12.16512Gbps  Eye scan                 */
    { 384e6,    768e6,    6144e6,     3072e6    }, /* uc17, 204B,   100MHz/122.88MHz,   15.36000Gbps, tx only                  */
    { 384e6,    768e6,    6144e6,     3072e6    }, /* uc18, 204B,   100MHz/122.88MHz,   15.36000Gbps, rx only                  */
    { 122.88e6, 245.76e6, 11796.48e6, 5898.24e6 }, /* uc19, 204C,   100MHz/122.88MHz,   16.22016Gbps, 9082/9986/9207 only      */
    { 122.88e6, 368.64e6, 11796.48e6, 5898.24e6 }, /* uc20, 204C,   100MHz/122.88MHz,   24.33024Gbps  9082/9986/9207 only      */
    { 122.88e6, 184.32e6, 5898.24e6,  2949.12e6 }, /* uc21, 204C,   100MHz/122.88MHz,   12.16512Gbps, subclass1                */
    { 125e6,    375e6,    8000e6,     4000e6    }, /* uc22, 204C,   100MHz,             24.75000Gbps, rx only, 9081 rx FBW     */
    { 125e6,    500e6,    12000e6,    4000e6    }, /* uc23, 204B,   100MHz,             10.00000Gbps                           */
    { 122.88e6, 491.52e6, 5898.24e6,  2949.12e6 }, /* uc24, 204B,   100MHz/122.88MHz,    9.83040Gbps                           */
    { 122.88e6, 368.64e6, 11796.48e6, 2949.12e6 }, /* uc25, 204C,   100MHz/122.88MHz,   24.33024Gbps                           */
    { 125e6,    375e6,    8000e6,     4000e6    }, /* uc26, 204C,   100MHz,             24.75000Gbps, loopback, MCS            */
    { 125e6,    375e6,    8000e6,     4000e6    }, /* uc27, 204C,   100MHz,             24.75000Gbps, loopback, MCS            */
    { 312.5e6,  625e6,    5000e6,     2500e6    }, /* uc28, 204B,   100MHz,             12.5000Gbps,  tx only                  */
    { 312.5e6,  625e6,    5000e6,     2500e6    }, /* uc29, 204B,   100MHz,             12.5000Gbps,  rx only                  */
    { 125e6,    375e6,    12000e6,    3000e6    }, /* uc30, 204C,   100MHz,             24.33024Gbps  TPC MODE                 */
    { 3240e6,   202.5e6,  3240e6,     3240e6    }, /* uc31, 204C,   Direct Clock Only   13.36500Gbps  rx only                  */
    { 250e6,	  250e6,	  4000e6,     2000e6    }, /* uc32, 204C,   100MHz,             16.5,         9081 rx FBW              */
    { 100e6,    400e6,    4800e6,     4800e6    }, /* uc33, 204B    100MHz,             8G/4G         rx0/rx1 & 9082/9207 only */
    { 100e6,    400e6,    4800e6,     4800e6    }, /* uc34, 204B    100MHz,             8G/2G         rx0/rx1 & 9082/9207 only */
    { 100e6,    400e6,    9600e6,     4800e6    }, /* uc35, 204B    100MHz,             8G/8G         tx + rx & 9082/9207 only */
    { 100e6,    200e6,    4800e6,     2400e6    }, /* uc36, 204B    100MHz,             4G/4G         tx + rx                  */
    { 100e6,    400e6,    3200e6,     1600e6    }, /* uc37, 204B    100MHz,             8G/4G         tx + rx                  */
    { 100e6,    600e6,    4800e6,     2400e6    }, /* uc38, 204B    100MHz,             12G/3G,12G/6G tx0/rx0,tx1/rx1          */
    { 125e6,    375e6,    8000e6,     4000e6    }, /* uc39, 204C,   100MHz,             24.75000Gbps, rx only, 9081 rx FBW     */
    { 125e6,    250e6,    12000e6,    4000e6    }, /* uc40, 204C,   100MHz,             16.50000Gbps, txrx cap gate, subclass1 */
    { 125e6,    375e6,    12000e6,     3000e6   }, /* uc41, 204C,   100MHz,             24.75000Gbps, 9081 rx FBW ace uc       */

};

#if !defined(AD9207_ID) && !defined(AD9209_ID)
/* TX DataPath Configuration Settings*/
/*  DAC Crossbar Configuration
*   Use this parameter to define Mapping from Map JRx Samples to DAC Channelizer to MAIN DAC Datapath
*   For Modes that support Use Channel Interpolation > 1
*       JRX Converter Samples are mapped to Channels Datapaths as follows:
*       Note each AD9986_DAC_CH_X is made up of an IQ pair sample
*       DUAL LINK  | M  |  Default Channel Datapth Mapping (0-7)
*        0         | 2  |  AD9986_DAC_CH_0
*        0         | 4  |  AD9986_DAC_CH_0 , AD9986_DAC_CH_1
*        0         | 6  |  AD9986_DAC_CH_0 , AD9986_DAC_CH_1 , AD9986_DAC_CH_2
*        0         | 8  |  AD9986_DAC_CH_0 , AD9986_DAC_CH_1 , AD9986_DAC_CH_2, AD9986_DAC_CH_3 
*        0         | 12 |  AD9986_DAC_CH_0 , AD9986_DAC_CH_1 , AD9986_DAC_CH_2, AD9986_DAC_CH_3, AD9986_DAC_CH_4, AD9986_DAC_CH_5 
*        0         | 16 |  AD9986_DAC_CH_0 , AD9986_DAC_CH_1 , AD9986_DAC_CH_2, AD9986_DAC_CH_3, AD9986_DAC_CH_4, AD9986_DAC_CH_5, AD9986_DAC_CH_6, AD9986_DAC_CH_7
*        1         | 2  |  Link 0: AD9986_DAC_CH_0 , Link1: AD9986_DAC_CH_4 
*        1         | 4  |  Link 0: AD9986_DAC_CH_0,AD9986_DAC_CH_1 , Link1: AD9986_DAC_CH_4, AD9986_DAC_CH_5
*        1         | 6  |  Link 0: AD9986_DAC_CH_0,AD9986_DAC_CH_1, AD9986_DAC_CH_3 , Link1: AD9986_DAC_CH_4, AD9986_DAC_CH_5, AD9986_DAC_CH_6
*        1         | 8  |  Link 0:AD9986_DAC_CH_0 , AD9986_DAC_CH_1 , AD9986_DAC_CH_2, AD9986_DAC_CH_3, Link1: AD9986_DAC_CH_4, AD9986_DAC_CH_5, AD9986_DAC_CH_6, AD9986_DAC_CH_7
*       Each Main DAC Datapath may be a summation of the Channel datapaths. Use tx_dac_chan_xbar as follows to achieve that as described below.

*       uint8_t tx_dac_chan_xbar[usecase][ORed List Channel (0-7) to be summed and mapped to DAC Datapath(0-3)]
*       Each usecase has an Array of depth 4, each member value is the list of Channels to be mapped a Main DAC Datapath,
*       where index 0 = DAC0, index 1 = DAC1 etc
*       OR adi_ad9082_dac_channel_select_e enumerations to create the list
*       For example: ad9986_dac_chan_xbar[0][0] =AD9986_DAC_CH_0 |AD9986_DAC_CH_1
*       Map Channlizer 0 and Channelizer 1 to DAC 0
*       
*  For Modes that bypass Channel ie Channel Interpolation = 1 and Main DAC Interpolation > 1
*      JRX Converter Samples are mapped by default to DAC Main Data apths as follows
*       DUAL LINK  | M  |  Default Main DAC Datapath  Mapping (0-3)                                    | DAC DP to DAC Mapping (0-3)           
*        0         | 2  |  AD9986_DAC_DP_0                                                               | AD9986_DAC_DP_0 ->DAC 0
*        0         | 4  |  AD9986_DAC_DP_0 , AD9986_DAC_DP_1                                               | AD9986_DAC_DP_0 ->DAC 0,  AD9986_DAC_DP_1 ->DAC 1
*        0         | 6  |  AD9986_DAC_DP_0 , AD9986_DAC_DP_1 , AD9986_DAC_DP_2                               | AD9986_DAC_DP_0 ->DAC 0,  AD9986_DAC_DP_1 ->DAC 1, AD9986_DAC_DP_2 ->DAC 2
*        0         | 8  |  AD9986_DAC_DP_0 , AD9986_DAC_DP_1 , AD9986_DAC_DP_2, AD9986_DAC_DP_3                | AD9986_DAC_DP_0 ->DAC 0,  AD9986_DAC_DP_1 ->DAC 1, AD9986_DAC_DP_2 ->DAC 2 , AD9986_DAC_DP_3 ->DAC 3
*        1         | 2  |  Link 0: AD9986_DAC_DP_0 , Link1: AD9986_DAC_DP_2                                | AD9986_DAC_DP_0 ->DAC 0, AD9986_DAC_DP_2 ->DAC 2
*        1         | 4  |  Link 0: AD9986_DAC_DP_0,AD9986_DAC_DP_1 , Link1: AD9986_DAC_DP_2, AD9986_DAC_DP_3   | AD9986_DAC_DP_0 ->DAC 0,  AD9986_DAC_DP_1 ->DAC 1, AD9986_DAC_DP_2 ->DAC 2 , AD9986_DAC_DP_3 ->DAC 3
*      Each converter sample pair may be routed to an alternative Datapath.  Use tx_dac_chan_xbar as follows to achieve that as described below.
*      
*      uint8_t tx_dac_chan_xbar[usecase][DAC Datapath(0-3) to be mapped to DAC (0-3)]
*      Each usecase has an Array of depth 4, each member value is the DAC MAIN DP to be routed
*      where index 0 = DAC0  index 1 = DAC1 etc

*      For example: ad9986_dac_chan_xbar[0][1] =AD9986_DAC_DP _3
*      DAC Datapath 3 is routed to DAC 1
*      
*    
*      
*/
uint8_t tx_dac_chan_xbar[][4] = { /* dac0, dac1, dac2, dac3 */
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_0, AD9986_DAC_CH_0,  AD9986_DAC_CH_0 }, /* uc0 */
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc1 */
    { AD9986_DAC_CH_0 | AD9986_DAC_CH_1, AD9986_DAC_CH_2 | AD9986_DAC_CH_3, AD9986_DAC_CH_4 | AD9986_DAC_CH_5, AD9986_DAC_CH_6 | AD9986_DAC_CH_7 }, /* uc2 */
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc3 */
    { AD9986_DAC_CH_0 | AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_2, AD9986_DAC_CH_3 }, /* uc4 */
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc5 */
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_2, AD9986_DAC_CH_3,  AD9986_DAC_CH_1 }, /* uc6 */
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_3,  AD9986_DAC_CH_2 }, /* uc7 */
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc8 */
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc9 */
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc10*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc11*/
    { AD9986_DAC_CH_0 | AD9986_DAC_CH_1, AD9986_DAC_CH_2 | AD9986_DAC_CH_3, AD9986_DAC_CH_4 | AD9986_DAC_CH_5, AD9986_DAC_CH_6 | AD9986_DAC_CH_7 }, /* uc12*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc13*/
    { AD9986_DAC_CH_0 | AD9986_DAC_CH_1, AD9986_DAC_CH_2 | AD9986_DAC_CH_3, AD9986_DAC_CH_4 | AD9986_DAC_CH_5, AD9986_DAC_CH_6 | AD9986_DAC_CH_7 }, /* uc14*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc15*/
    { AD9986_DAC_CH_0 | AD9986_DAC_CH_1, AD9986_DAC_CH_2 | AD9986_DAC_CH_3, AD9986_DAC_CH_4 | AD9986_DAC_CH_5, AD9986_DAC_CH_6 | AD9986_DAC_CH_7 }, /* uc16*/
    { AD9986_DAC_CH_2,  AD9986_DAC_CH_0, AD9986_DAC_CH_3,  AD9986_DAC_CH_1 }, /* uc17*/
    { 0 }, /* uc18*/
    { AD9986_DAC_CH_0 | AD9986_DAC_CH_1, AD9986_DAC_CH_2 | AD9986_DAC_CH_3, AD9986_DAC_CH_4 | AD9986_DAC_CH_5, AD9986_DAC_CH_6 | AD9986_DAC_CH_7 }, /* uc19*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc20*/
    { AD9986_DAC_CH_0 | AD9986_DAC_CH_1, AD9986_DAC_CH_2 | AD9986_DAC_CH_3, AD9986_DAC_CH_4 | AD9986_DAC_CH_5, AD9986_DAC_CH_6 | AD9986_DAC_CH_7 }, /* uc21*/
    { 0 }, /* uc22*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc23*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc24*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1 }, /* uc25*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc26*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc27*/
    { AD9986_DAC_CH_2,  AD9986_DAC_CH_0, AD9986_DAC_CH_3,  AD9986_DAC_CH_1 }, /* uc28*/
    { 0 }, /* uc29*/
    { AD9986_DAC_CH_0 | AD9986_DAC_CH_1, AD9986_DAC_CH_2 | AD9986_DAC_CH_3, AD9986_DAC_CH_4 | AD9986_DAC_CH_5, AD9986_DAC_CH_6 | AD9986_DAC_CH_7 }, /* uc30*/
    { 0 }, /* uc31*/
    { AD9986_DAC_CH_0 | AD9986_DAC_CH_1, AD9986_DAC_CH_2 | AD9986_DAC_CH_3, AD9986_DAC_CH_4 | AD9986_DAC_CH_5, AD9986_DAC_CH_6 | AD9986_DAC_CH_7 }, /* uc32*/
    { 0 }, /* uc33 */
    { 0 }, /* uc34 */
    { AD9986_DAC_CH_0 , AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc35 */
    { AD9986_DAC_CH_0 , AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc36 */
    { AD9986_DAC_CH_0 , AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc37 */
    { AD9986_DAC_CH_0 , AD9986_DAC_CH_1 }, /* uc38 */
    { 0 }, /* uc39*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc40*/
    { AD9986_DAC_CH_0,  AD9986_DAC_CH_1, AD9986_DAC_CH_2,  AD9986_DAC_CH_3 }, /* uc41*/
};

int64_t tx_main_shift[][4] = { /* dac0, dac1, dac2, dac3 */
    { 1000.00e6, 1000.00e6, 1000.00e6, 1000.00e6 }, /* uc0 */
    { 1842.50e6, 1842.50e6, 1842.50e6, 1842.50e6 }, /* uc1 */
    { 1991.25e6, 1991.25e6, 1991.25e6, 1991.25e6 }, /* uc2 */
    { 1842.50e6, 1842.50e6, 0,         0         }, /* uc3 */
    { 1991.25e6, 0,         0,         0         }, /* uc4 */
    { 672.00e6,  0,         0,         0         }, /* uc5 */
    { 396.00e6,  0,         0,         396.00e6  }, /* uc6 */
    { 396.00e6,  0,         0,         396.00e6  }, /* uc7 */
    { 672.00e6,  0,         0,         0         }, /* uc8 */
    { 1000e6,    1000e6,    0,         0         }, /* uc9 */
    { 0e6,       0e6,       0e6,       0e6       }, /* uc10*/
    { 0e6,       0e6,       0e6,       0e6       }, /* uc11*/
    { 1000e6,    1000e6,    1000e6,    1000e6    }, /* uc12*/
    { 1000e6,    1000e6,    1000e6,    1000e6    }, /* uc13*/
    { 3000e6,    3000e6,    3000e6,    3000e6    }, /* uc14*/
    { 4000e6,    4000e6,    0,         0         }, /* uc15*/
    { 1000e6,    1000e6,    1000e6,    1000e6    }, /* uc16*/
    { 0,         396.00e6,  0,         396.00e6  }, /* uc17*/
    { 0,         0,         0,         0         }, /* uc18*/
    { 1100e6,    1900e6,    1100e6,    1900e6    }, /* uc19*/
    { 900e6,     2100e6,    900e6,     2100e6    }, /* uc20*/
    { 1000e6,    1000e6,    1000e6,    1000e6    }, /* uc21*/
    { 0,         0,         0,         0         }, /* uc22*/
    { 0,         0,         0,         0         }, /* uc23*/
    { 396e6,     396e6,     396e6,     396e6     }, /* uc24*/
    { 1000e6,    1000e6,    1000e6,    1000e6    }, /* uc25*/
    { 1300e6,    0e6,       1300e6,    0e6       }, /* uc26*/
    { 1300e6,    0e6,       1300e6,    0e6       }, /* uc27*/
    { 0,         1000e6,    0,         1000e6    }, /* uc28*/
    { 0,         0,         0,         0         }, /* uc29*/
    { 3552e6,    3552e6,    3552e6,    3552e6    }, /* uc30*/
    { 1000e6,    2000e6,    3000e6,    4000e6    }, /* uc31*/
    { 1000e6,    2000e6,    3000e6,    4000e6    }, /* uc32*/
    { 0,         0,         0,         0         }, /* uc33*/
    { 0,         0,         0,         0         }, /* uc34*/
    { 0,         0,         0,         0         }, /* uc35*/
    { 0,         0,         0,         0         }, /* uc36*/
    { 0,         0,         0,         0         }, /* uc37*/
    { 500e6,     500e6,     500e6,     500e6     }, /* uc38*/
    { 0,         0,         0,         0         }, /* uc39*/
    { 1000e6,    1000e6,    1000e6,    1000e6    }, /* uc40*/
    { 1991.25e6, 1991.25e6, 1991.25e6, 1991.25e6 }, /* uc41*/
};

int64_t tx_chan_shift[][8] = { /* ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7 */
    { 100e6,     0,        0,         0,        0,         0,        0,         0        }, /* uc0 */
    { 0e6,       0e6,      0e6,       0e6,      0,         0,        0,         0        }, /* uc1 */
    { -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6 }, /* uc2 */
    { 0e6,       0e6,      0,         0,        0,         0,        0,         0        }, /* uc3 */
    { -148.75e6, 148.75e6, 0,         0,        0,         0,        0,         0        }, /* uc4 */
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc5 */
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc6 */
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc7 */
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc8 */
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc9 */
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc10*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc11*/
    { -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6 }, /* uc12*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc13*/
    { -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6 }, /* uc14*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc15*/
    { -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6 }, /* uc16*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc17*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc18*/
    { -200e6,    200e6,    -200e6,    200e6,    -200e6,    200e6,    -200e6,    200e6    }, /* uc19*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc20*/
    { -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6, -148.75e6, 148.75e6 }, /* uc21*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc22*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc23*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc24*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc25*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc26*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc27*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc28*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc29*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc30*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc31*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc32*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc33*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc34*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc35*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc36*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc37*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc38*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc39*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc40*/
    { 0,         0,        0,         0,        0,         0,        0,         0        }, /* uc41*/
};
int8_t  tx_chan_gain[][8] = { /* ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7 */
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc0 */
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc1 */
    { -7,   0,   0,   0,   0,   0,   0,   0 }, /* uc2 */
    { -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9 }, /* uc3 */
    {-12, -12, -12, -12, -12, -12, -12, -12 }, /* uc4 */
    { -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7 }, /* uc5 */
    { -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7 }, /* uc6 */
    { -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7 }, /* uc7 */
    { -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7 }, /* uc8 */
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc9 */
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc10*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc11*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc12*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc13*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc14*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc15*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc16*/
    { -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7 }, /* uc17*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc18*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc19*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc20*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc21*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc22*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc23*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc24*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc25*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc26*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc27*/
    { -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7 }, /* uc28*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc29*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc30*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc31*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc32*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc33*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc34*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc35*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc36*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc37*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc38*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc39*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc40*/
    {  0,   0,   0,   0,   0,   0,   0,   0 }, /* uc41*/
};
uint8_t tx_interp[][2] = {
/* {main DUC Interpolation, Channelizer DUC Interpolation} */
    {   8,    2   }, /* uc0 */
    {   4,    4   }, /* uc1 */
    {   6,    4   }, /* uc2 */
    {   4,    4   }, /* uc3 */
    {   6,    4   }, /* uc4 */
    {   4,    1   }, /* uc5 */
    {   4,    1   }, /* uc6 */
    {   4,    1   }, /* uc7 */
    {   4,    1   }, /* uc8 */
    {   4,    1   }, /* uc9 */
    {  12,    1   }, /* uc10*/
    {  12,    1   }, /* uc11*/
    {   8,    2   }, /* uc12*/
    {  12,    1   }, /* uc13*/
    {   4,    2   }, /* uc14*/
    {   4,    1   }, /* uc15*/
    {   8,    2   }, /* uc16*/
    {   4,    1   }, /* uc17*/
    {   0,    0   }, /* uc18*/
    {   8,    3   }, /* uc19*/
    {   8,    1   }, /* uc20*/
    {   8,    2   }, /* uc21*/
    {   0,    0   }, /* uc22*/
    {   8,    6   }, /* uc23*/
    {   6,    2   }, /* uc24*/
    {   8,    1   }, /* uc25*/
    {   4,    1   }, /* uc26*/
    {   4,    1   }, /* uc27*/
    {   4,    1   }, /* uc28*/
    {   0,    0   }, /* uc29*/
    {   8,    1   }, /* uc30*/
    {   0,    0   }, /* uc31*/
    {   2,    1   }, /* uc32*/
    {   0,    0   }, /* uc33*/
    {   0,    0   }, /* uc34*/
    {   8,    3   }, /* uc35*/
    {   8,    3   }, /* uc36*/
    {   4,    2   }, /* uc37*/
    {   4,    2   }, /* uc38*/
    {   0,    0   }, /* uc39*/
    {  12,    1   }, /* uc40*/
    {   8,    1   }, /* uc41*/
};
#endif

/* RX Main Path DDC Enable Configuration */
/* MUX 0 & MUX 1 Settings
 * List the ADC_Coarse DDCs to be mapped to logical ADCs
 * OR�adi_ad9082_adc_coarse_ddc_select_e enumerations to create the list
 * Note all DDC are enabled for all use cases that configure main datapath.
 * Deselect CDDCs and FDDCs to override default Mux3 settings and enable full bandwidth mode
 */
uint8_t rx_cddc_select[] = {
  0, /* uc0 */
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc1 */
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc2 */
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc3 */
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc4 */
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc5 */
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc6 */
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc7 */
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc8 */
  0, /* uc9 */
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc10*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc11*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc12*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc13*/
  0, /* uc14*/
  0, /* uc15*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc16*/
  0, /* uc17*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc18*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc19*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc20*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc21*/
  0, /* uc22*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc23*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc24*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc25*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc26*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc27*/
  0, /* uc28*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc29*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc30*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc31*/
  0, /* uc32*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc33*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc34*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc35*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc36*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc37*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc38*/
  0, /* uc39*/
  AD9986_ADC_CDDC_0 | AD9986_ADC_CDDC_1 | AD9986_ADC_CDDC_2 | AD9986_ADC_CDDC_3, /* uc40*/
  0, /* uc41*/
}; 

/* RX Main Path DDC  NCO Frequency Configuration
 * List the ADC_Coarse DDCs desired Frequency Shift
 * Each usecase has an Array of depth 4, each member value is the desired frequency shift (MHz)of a Coarse DDC Datapath,
 * where index 0 = Frequency Shift for� Coarse DDC 0, index 1 = Frequency Shift for�Coarse DDC 1 etc
 *
 *   For example: rx_cddc_shift[0][0] = 1842.5e6,
 *   For Use case 0, Frequency Shift for Coarse DDC0 is 1842.5e6MHz
 */
int64_t rx_cddc_shift[][4] = {
 /* {cddc0, cddc1, cddc2, cddc3 }*/
    { 0,        0,        0,      0     }, /* uc0 */
    { 1842.5e6, 1842.5e6, 350e6,  350e6 }, /* uc1 */
    { 1842.5e6, 1842.5e6, 350e6,  350e6 }, /* uc2 */
    { 1842.5e6, 350e6,    0,      0     }, /* uc3 */
    { 1842.5e6, 350e6,    0,      0     }, /* uc4 */
    { 396e6,    55e6,     768e6,  0     }, /* uc5 */
    { 396e6,    396e6,    396e6,  396e6 }, /* uc6 */
    { 396e6,    396e6,    396e6,  396e6 }, /* uc7 */
    { 396e6,    55e6,     768e6,  0     }, /* uc8 */
    { 0,        0,        0,      0     }, /* uc9 */
    { 366e6,    376e6,    386e6,  396e6 }, /* uc10*/
    { 366e6,    376e6,    386e6,  396e6 }, /* uc11*/
    { 1000e6,   1000e6,   1000e6, 1000e6}, /* uc12*/
    { 1000e6,   1000e6,   1000e6, 1000e6}, /* uc13*/
    { 0,        0,        0,      0     }, /* uc14*/
    { 0,        0,        0,      0     }, /* uc15*/
    { 396e6,    396e6,    396e6,  396e6 }, /* uc16*/
    { 0,        0,        0,      0     }, /* uc17*/
    { 396e6,    396e6,    396e6,  396e6 }, /* uc18*/
    { 1100e6,   1100e6,   1900e6, 1900e6}, /* uc19*/
    { 900e6,    900e6,    2100e6, 2100e6}, /* uc20*/
    { 396e6,    396e6,    396e6,  396e6 }, /* uc21*/
    { 0,        0,        0,      0     }, /* uc22*/
    { 396e6,    396e6,    396e6,  396e6 }, /* uc23*/
    { 396e6,    396e6,    396e6,  396e6 }, /* uc24*/
    { 1000e6,   1000e6,   1000e6, 1000e6}, /* uc25*/
    {-125e6,   -700e6,   -125e6, -700e6 }, /* uc26*/
    {-125e6,   -700e6,   -125e6, -700e6 }, /* uc27*/
    { 0,        0,        0,      0     }, /* uc28*/
    { 1000e6,   1000e6,   1000e6, 1000e6}, /* uc29*/
    { 1000e6,   1000e6,   0,      0     }, /* uc30*/
    { 1000e6,   1000e6,   1000e6, 1000e6}, /* uc31*/
    { 0,        0,        0,      0     }, /* uc32*/
    { 120e6,    120e6,    120e6,  120e6 }, /* uc33*/
    { 120e6,    120e6,    120e6,  120e6 }, /* uc34*/
    { 0,        0,        0,      0     }, /* uc35*/
    { 120e6,    120e6,    120e6,  120e6 }, /* uc36*/
    { 120e6,    120e6,    120e6,  120e6 }, /* uc37*/
    { 0,        0,        0,      0     }, /* uc38*/
    { 0,        0,        0,      0     }, /* uc39*/
    { 1000e6,   1000e6,   1000e6, 1000e6}, /* uc40*/
    { 0,        0,        0,      0     }, /* uc41*/
};
/* RX Main Path DDC Data Decimation
 * List the ADC_Coarse DDCs desired data decimation
 * Each usecase has an Array of depth 4, each member value is the desired decimation of a Coarse DDC Datapath,
 * where index 0 = Decimation Factor for� Coarse DDC 0, index 1 = decimation Factor for�Coarse DDC 1 etc
 *
 *   For example: rx_cddc_dcm[1][0] =AD9986_CDDC_DCM_2,
 *   For Use case 1, Frequency Factor for Coarse DDC0 is 1
 *   Use adi_ad9082_adc_coarse_ddc_dcm_e to set the decimation setting
 */
uint8_t rx_cddc_dcm[][4] = {
  /*{cddc0, cddc1, cddc2, cddc3} */
    { 0 }, /* uc0 */
    { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 }, /* uc1 */
    { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 }, /* uc2 */
    { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 }, /* uc3 */
    { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 }, /* uc4 */
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_4 }, /* uc5 */
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc6 */
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc7 */
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_1, AD9986_CDDC_DCM_4 }, /* uc8 */
    { 0 }, /* uc9 */
    { AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3 }, /* uc10*/
    { AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3 }, /* uc11*/
    { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 }, /* uc12*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc13*/
    { 0 }, /* uc14*/
    { 0 }, /* uc15*/
    { AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3 }, /* uc16*/
    { 0 }, /* uc17*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc18*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc19*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc20*/
    { AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3 }, /* uc21*/
    { 0 }, /* uc22*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc23*/
    { AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3, AD9986_CDDC_DCM_3 }, /* uc24*/
    { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 }, /* uc25*/
    { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 }, /* uc26*/
    { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 }, /* uc27*/
    { 0 }, /* uc28*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc29*/
    { AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2, AD9986_CDDC_DCM_2 }, /* uc30*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc31*/
    { 0 }, /* uc32*/
    { AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6 }, /* uc33*/
    { AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6 }, /* uc34*/
    { AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6 }, /* uc35*/
    { AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6, AD9986_CDDC_DCM_6 }, /* uc36*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc37*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc38*/
    { 0 }, /* uc39*/
    { AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4, AD9986_CDDC_DCM_4 }, /* uc40*/
    { 0 }, /* uc41*/
};
/* RX Main Path DDC Has Optional Complex to Real Convertor
 * rx_cddc_c2r sets the enable for Complex to Real Converter per Main/Coarse DDC
 * Each usecase has an Array of depth 4, each member value is the enable of a Coarse DDC Datapath Complex to Real Converter
 * where index 0 = Enable Complex to Real Convertor for Coarse DDC 0,
 *       index 1 = Enable Complex to Real Convertor for Coarse DDC 1
 *
 *   For example: rx_cddc_c2r[1][0] = 0,
 *   For Use case 1, Complex to Real Converter for Coarse DDC 0 is Disabled
 *
 */
uint8_t rx_cddc_c2r[][4] = { /* cddc0, cddc1, cddc2, cddc3 */
    { 0, 0, 0, 0 }, /* uc0 */
    { 0, 0, 0, 0 }, /* uc1 */
    { 0, 0, 0, 0 }, /* uc2 */
    { 0, 0, 0, 0 }, /* uc3 */
    { 0, 0, 0, 0 }, /* uc4 */
    { 0, 0, 0, 0 }, /* uc5 */
    { 0, 0, 0, 0 }, /* uc6 */
    { 0, 0, 0, 0 }, /* uc7 */
    { 0, 0, 0, 0 }, /* uc8 */
    { 0, 0, 0, 0 }, /* uc9 */
    { 0, 0, 0, 0 }, /* uc10*/
    { 0, 0, 0, 0 }, /* uc11*/
    { 0, 0, 0, 0 }, /* uc12*/
    { 0, 0, 0, 0 }, /* uc13*/
    { 0, 0, 0, 0 }, /* uc14*/
    { 0, 0, 0, 0 }, /* uc15*/
    { 0, 0, 0, 0 }, /* uc16*/
    { 0, 0, 0, 0 }, /* uc17*/
    { 0, 0, 0, 0 }, /* uc18*/
    { 0, 0, 0, 0 }, /* uc19*/
    { 0, 0, 0, 0 }, /* uc20*/
    { 0, 0, 0, 0 }, /* uc21*/
    { 0, 0, 0, 0 }, /* uc22*/
    { 0, 0, 0, 0 }, /* uc23*/
    { 0, 0, 0, 0 }, /* uc24*/
    { 0, 0, 0, 0 }, /* uc25*/
    { 0, 0, 0, 0 }, /* uc26*/
    { 0, 0, 0, 0 }, /* uc27*/
    { 0, 0, 0, 0 }, /* uc28*/
    { 0, 0, 0, 0 }, /* uc29*/
    { 0, 0, 0, 0 }, /* uc30*/
    { 0, 0, 0, 0 }, /* uc31*/
    { 0, 0, 0, 0 }, /* uc32*/
    { 0, 0, 0, 0 }, /* uc33*/
    { 0, 0, 0, 0 }, /* uc34*/
    { 0, 0, 0, 0 }, /* uc35*/
    { 0, 0, 0, 0 }, /* uc36*/
    { 0, 0, 0, 0 }, /* uc37*/
    { 0, 0, 0, 0 }, /* uc38*/
    { 0, 0, 0, 0 }, /* uc39*/
    { 0, 0, 0, 0 }, /* uc40*/
    { 0, 0, 0, 0 }, /* uc41*/
};
/* RX Channelizer/Fine DDC Datapath Selection
 * List the ADC Fine DDCs data path for routing Data from Main/ Coarse DDC Datapth to the JESD Tx
 * Each usecase has an Array of depth 1, each member value is the masked list of Fine DDCs to be selected
 * Note Coarse DDC0 & Coarse DDC1 can be routed to Fine DDC0- Fine DDC3 datapaths and
 *      Coarse DDC0 & Coarse DDC1 can be routed to Fine DDC4- Fine DDC6 datapaths
 * For example: rx_fddc_select[3] = AD9986_ADC_FDDC_0 |AD9986_ADC_FDDC_1,
 *   For Use case 3,  Fine DDC 0 and Fine DDC 1 are enabled
 * Use adi_ad9082_adc_fine_ddc_select_e to create the list of fine DDCs to be enabled
 * Note: Based on the input from the user, will apply a recommended muxing from CDDC to FDDC,
 *         An Error will be returned if the muxing is not supported for that devices,
 * Note: If Fine DDC Decimation Factor is set to 1, Fine DDC in the Channel will be disabled
 *
 */
uint8_t rx_fddc_select[] = {
    0, /* uc0 */
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc1 */
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc2 */
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1, /* uc3 */
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1, /* uc4 */
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc5 */
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc6 */
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc7 */
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc8 */
    0, /* uc9 */
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc10*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc11*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_2 | AD9986_ADC_FDDC_3 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5 | AD9986_ADC_FDDC_6 | AD9986_ADC_FDDC_7, /* uc12*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc13*/
    0, /* uc14*/
    0, /* uc15*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc16*/
    0, /* uc17*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc18*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_2 | AD9986_ADC_FDDC_3 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5 | AD9986_ADC_FDDC_6 | AD9986_ADC_FDDC_7, /* uc19*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc20*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc21*/
    0, /* uc22*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc23*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1, /* uc24*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1, /* uc25*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc26*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc27*/
    0, /* uc28*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc29*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc30*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc31*/
    0, /* uc32*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc33*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc34*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1, /* uc35*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1, /* uc36*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_4, /* uc37*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc38*/
    0, /* uc39*/
    AD9986_ADC_FDDC_0 | AD9986_ADC_FDDC_1 | AD9986_ADC_FDDC_4 | AD9986_ADC_FDDC_5, /* uc40*/
    0, /* uc41*/
};
/* RX Channelizer Path DDC  NCO Frequency Configuration
 * List the ADC_Fine DDCs desired Frequency Shift
 * Each usecase has an Array of depth 8, each member value is the desired frequency shift (MHz)of a Fine DDC Datapath,
 * where index 0 = Frequency Shift for� Fine DDC 0, index 1 = Frequency Shift for�Fine DDC 1 etc
 *
 *   For example: rx_fddc_shift[0][0] = 1842.5e6,
 *   For Use case 0, Frequency Shift for Fine DDC0 is 1842.5e6MHz
 */
int64_t rx_fddc_shift[][8] = {
/* fddc0, fddc1, fddc2, fddc3, fddc4, fddc5, fdddc6, fddc7 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc0 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc1 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc2 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc3 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc4 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc5 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc6 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc7 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc8 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc9 */
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc10*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc11*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc12*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc13*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc14*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc15*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc16*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc17*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc18*/
    {-200e6, 200e6, -200e6, 200e6, -200e6, 200e6, -200e6, 200e6 }, /* uc19*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc20*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc21*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc22*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc23*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc24*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc25*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc26*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc27*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc28*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc29*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc30*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc31*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc32*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc33*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc34*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc35*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc36*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc37*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc38*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc39*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc40*/
    { 0, 0, 0, 0, 0, 0, 0, 0 }, /* uc41*/
};
/* RX Channelizer Data Decimation
 * List the ADC Fine DDCs desired data decimation
 * Each usecase has an Array of depth 8, each member value is the desired decimation of a Fine DDC Datapath,
 * where index 0 = Decimation factor for� Fine DDC 0, index 1 = decimation factor for�Fine DDC 1 etc
 * Note where No Fine DDC Decimation is required (bypassed), Decimation factor is set to 1,AD9986_FDDC_DCM_1
 *
 *   For example: rx_fddc_dcm[1][0] =AD9986_FDDC_DCM_4,
 *   For Use case 1, Decimation Factor for FineDDC0 is 4
 *   Use adi_ad9082_adc_fine_ddc_dcm_e to set the decimation setting
 */
uint8_t rx_fddc_dcm[][8] = { /* fddc0, fddc1, fddc2, fddc3, fddc4, fddc5, fdddc6, fddc7 */
    { 0 }, /* uc0 */
    { AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4, 0, 0, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4 }, /* uc1 */
    { AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4, 0, 0, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4 }, /* uc2 */
    { AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4 }, /* uc3 */
    { AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4 }, /* uc4 */
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc5 */
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc6 */
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc7 */
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc8 */
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc9 */
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc10*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc11*/
    { AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2 }, /* uc12*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc13*/
    { 0 }, /* uc14*/
    { 0 }, /* uc15*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc16*/
    { 0 }, /* uc17*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc18*/
    { AD9986_FDDC_DCM_3, AD9986_FDDC_DCM_3, AD9986_FDDC_DCM_3, AD9986_FDDC_DCM_3, AD9986_FDDC_DCM_3, AD9986_FDDC_DCM_3, AD9986_FDDC_DCM_3, AD9986_FDDC_DCM_3 }, /* uc19*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc20*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc21*/
    { 0 }, /* uc22*/
    { AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4, 0, 0, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4 }, /* uc23*/
    { AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2 }, /* uc24*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc25*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc26*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc27*/
    { 0 }, /* uc28*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc29*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc30*/
    { AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4 }, /* uc31*/
    { 0 }, /* uc32*/
    { AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2, 0, 0, AD9986_FDDC_DCM_4, AD9986_FDDC_DCM_4 },   /* uc33*/
    { AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2, 0, 0, AD9986_FDDC_DCM_8, AD9986_FDDC_DCM_8 },   /* uc34*/ 
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc35*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc36*/
    { AD9986_FDDC_DCM_1, 0, 0, 0, AD9986_FDDC_DCM_1 }, /* uc37*/
    { AD9986_FDDC_DCM_2, 0, 0, 0, AD9986_FDDC_DCM_2, AD9986_FDDC_DCM_2 }, /* uc38*/
    { 0 }, /* uc39*/
    { AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1, 0, 0, AD9986_FDDC_DCM_1, AD9986_FDDC_DCM_1 }, /* uc40*/
    { 0 }, /* uc41*/
};
uint8_t rx_fddc_c2r[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; /* uc */

/* Link Settings */
/* Rx Channelizer Virtual Converter Mapping
*  Maps the output of a Fine DDC Data Format O/P to a JESD TX virtual converter
*  Refer to Mux 4 in the Rx Digital Datapath section of the System Developer Userguide
*
*  adi_ad9082_adc_fine_ddc_converter_e enerates the Fine DCC I & Q Outputs
*  Each virtual converter in the JESD TX link must be assigned a source data,  Fine DDC X I or Q
*  Note this should be set carefully and reflect the Fine DDC selected in rx_fddc_select and also JESD TX Configuration
*
*  Each usecase has an Array of depth 2, each member a list of the source of data for the virtual converter for the JESD TX Link
*  where index 0 = Sources of Data for JESD TX Link 0
*   where index 1 = Sources of Data for JESD TX Link 1
*
*   For example: For Use Case 29 with is a Dual link JESD Mode with M =4, we specify the I & Q source for each virtual Converter
*    jtx_conv_sel[28][0]={AD9986_FDDC_0_I,AD9986_FDDC_0_Q,AD9986_FDDC_1_I,AD9986_FDDC_1_Q }
*    jtx_conv_sel[28][1]={AD9986_FDDC_4_I,AD9986_FDDC_4_Q,AD9986_FDDC_5_I,AD9986_FDDC_5_Q }
*/
adi_ad9986_jtx_conv_sel_t jtx_conv_sel[][2] = {
    { { 0 } }, /* uc0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc1.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc2.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q },   /* uc3.link0 */
      { AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc3.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q },   /* uc4.link0 */
      { AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc4.link1 */
    { { AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q }, /* uc5.link0 */
      { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc5.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q },   /* uc6.link0 */
      { AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc6.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q },   /* uc7.link0 */
      { AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc7.link1 */
    { { AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q }, /* uc8.link0 */
      { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc8.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q  } }, /* uc9.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc10.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc11.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_2_I, AD9986_FDDC_2_Q, AD9986_FDDC_3_I, AD9986_FDDC_3_Q,     /* uc12.link0 */
        AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q, AD9986_FDDC_6_I, AD9986_FDDC_6_Q, AD9986_FDDC_7_I, AD9986_FDDC_7_Q } }, /* uc12.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc13.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc14.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q } }, /* uc15.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc16.link0 */
    { { 0 } }, /* uc17.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q },   /* uc18.link0 */
      { AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc18.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_2_I, AD9986_FDDC_2_Q, AD9986_FDDC_3_I, AD9986_FDDC_3_Q,     /* uc19.link0 */
        AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q, AD9986_FDDC_6_I, AD9986_FDDC_6_Q, AD9986_FDDC_7_I, AD9986_FDDC_7_Q } }, /* uc19.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc20.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc21.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc22.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc23.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q },   /* uc24.link0 */
      { AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc24.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc25.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc26.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc27.link0 */
    { { 0 } }, /* uc28.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q },   /* uc29.link0 */
      { AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc29.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc30.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc31.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc32.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q },   /* uc33.link0 */
      { AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc33.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q },   /* uc34.link0 */
      { AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc34.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc35.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc36.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q } }, /* uc37.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q },   /* uc38.link0 */ 
      { AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } },   /* uc38.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q }, /* uc39.link0 */
      { AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc39.link1 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q, AD9986_FDDC_4_I, AD9986_FDDC_4_Q, AD9986_FDDC_5_I, AD9986_FDDC_5_Q } }, /* uc40.link0 */
    { { AD9986_FDDC_0_I, AD9986_FDDC_0_Q, AD9986_FDDC_1_I, AD9986_FDDC_1_Q } }, /* uc41.link0 */

};

/* Total Decimation Settings */
/* The total decimation is used to calculate lane rate for each link which in
*  turn is used to determine FPGA clock sources and line rates for JESD204B
*
*  Each usecase has an Array of depth 2,
*   where index 0 = Total decimation for link 0
*   where index 1 = Total decimation for link 1
*
*  For example: For Use Case 3 with is a Dual link JESD Mode, total decimation for each link
*   jtx_chip_dcm[3][0]= 8
*   jtx_chip_dcm[3][1]= 8
*/
uint8_t jtx_chip_dcm[][2] = {
    { 0 }, /* uc0.link0 */
    { 8 }, /* uc1.link0 */
    { 8 }, /* uc2.link0 */
    { 8 ,  /* uc3.link0 */
      8 }, /* uc3.link1 */
    { 8 ,  /* uc4.link0 */
      8 }, /* uc4.link1 */
    { 2 ,  /* uc5.link0 */
      4 }, /* uc5.link1 */
    { 4 ,  /* uc6.link0 */
      4 }, /* uc6.link1 */
    { 4 ,  /* uc7.link0 */
      4 }, /* uc7.link1 */
    { 1 ,  /* uc8.link0 */
      4 }, /* uc8.link1 */
    { 1 }, /* uc9.link0 */
    { 3 }, /* uc10.link0 */
    { 3 }, /* uc11.link0 */
    { 4 }, /* uc12.link0 */
    { 4 }, /* uc13.link0 */
    { 1 }, /* uc14.link0 */
    { 1 }, /* uc15.link0 */
    { 3 }, /* uc16.link0 */
    { 0 }, /* uc17.link0 */
    { 4 ,  /* uc18.link0 */
      4 }, /* uc18.link1 */
    { 12}, /* uc19.link0 */
    { 4 }, /* uc20.link0 */
    { 3 }, /* uc21.link0 */
    { 1 }, /* uc22.link0 */
    { 16}, /* uc23.link0 */
    { 6 ,  /* uc24.link0 */
      6 }, /* uc24.link1 */
    { 2 }, /* uc25.link0 */
    { 2 }, /* uc26.link0 */
    { 2 }, /* uc27.link0 */
    { 0 }, /* uc28.link0 */
    { 4 ,  /* uc29.link0 */
      4 }, /* uc29.link1 */
    { 2 }, /* uc30.link0 */
    { 16}, /* uc31.link0 */
    { 1 }, /* uc32.link0 */
    { 12 , /* uc33.link0 */ 
      24}, /* uc33.link1 */
    { 12 , /* uc34.link0 */ 
      48}, /* uc34.link1 */
    { 6 }, /* uc35.link0 */
    { 6 }, /* uc36.link0 */
    { 4 }, /* uc37.link0 */
    { 8 ,  /* uc38.link0 */  
      8 }, /* uc38.link1 */
    { 1 ,  /* uc39.link0 */
      1 }, /* uc39.link1 */
    { 4 }, /* uc40.link0 */
    { 1 }, /* uc41.link0 */
};

uint8_t jtx_logiclane_mapping_pe_brd[2][8] = { { 0, 1, 2, 3, 4, 5, 6, 7 }, { 4, 5, 6, 7, 0, 1, 2, 3 } }; 
uint8_t jtx_logiclane_mapping_ce_brd[2][8] = { { 6, 4, 3, 2, 1, 0, 7, 5 }, { 2, 0, 7, 7, 7, 7, 3, 1 } }; 
adi_cms_jesd_param_t jrx_param[] = {
      /*L   F   M   S   HD  K    N    N'   CF  CS DID BID LID  SC SCR Dual V  Mode */
      { 8,  4,  16, 1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  16 }, /* uc0 : nco test */
      { 4,  4,  8,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  9, }, /* uc1 : txmode = 194 */
      { 8,  4,  16, 1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  16 }, /* uc2 : txmode = 214 */
      { 4,  2,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  10 }, /* uc3 : txmode = 437 */
      { 4,  2,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  10 }, /* uc4 : txmode = 440 */
      { 4,  1,  2,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  62 }, /* uc5 : txmode = 263 */
      { 8,  1,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  17 }, /* uc6 : txmode = 258 */
      { 4,  1,  2,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  62 }, /* uc7 : txmode = 266 */
      { 4,  1,  2,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  62 }, /* uc8 : txmode = 263 */
      { 8,  1,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  17 }, /* uc9 : txmode = 258 */
      { 8,  2,  8,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  15 }, /* uc10: txmode = 378 */
      { 8,  2,  8,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  15 }, /* uc11: txmode = 378 */
      { 8,  4,  16, 1,  0,  64,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  16 }, /* uc12: txmode = 91  */
      { 8,  2,  8,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  15 }, /* uc13: txmode = 378 */
      { 8,  4,  16, 1,  0,  64,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  16 }, /* uc14: txmode = 88  */
      { 8,  3,  4,  4,  0,  256, 12,  12,  0,  0,  0,  0,  0,  0,  1,  0,  2,  35 }, /* uc15: txmode = 573 */
      { 8,  4,  16, 1,  0,  64,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  16 }, /* uc16: txmode = 91  */
      { 8,  1,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  17 }, /* uc17: txmode = 258 */
      { 0 }, /* uc18: rx only */
      { 8,  4,  16, 1,  0,  64,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  16 }, /* uc19: txmode = 371 */
      { 8,  2,  8,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  15 }, /* uc20: txmode = 99  */
      { 8,  4,  16, 1,  0,  64,  16,  16,  0,  0,  0,  0,  0,  1,  1,  0,  2,  16 }, /* uc21: txmode = 91  */
      { 0 }, /* uc22: rx only */
      { 4,  4,  8,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  9  }, /* uc23: txmode = 417 */
      { 8,  2,  8,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  15 }, /* uc24: txmode = 248 */
      { 4,  2,  4,  1,  1,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  10 }, /* uc25: txmode = 97  */
      { 8,  3,  8,  2,  0,  256, 12,  12,  0,  0,  0,  0,  0,  1,  1,  0,  2,  24 }, /* uc26: txmode = 140 */
      { 8,  3,  8,  2,  0,  256, 12,  12,  0,  0,  0,  0,  0,  1,  1,  0,  2,  24 }, /* uc27: txmode = 140 */
      { 8,  1,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  17 }, /* uc28: txmode = 258 */
      { 0 }, /* uc29: rx only */
      { 8,  2,  8,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  15 }, /* uc30: txmode = 99  */
      { 0 }, /* uc31: rx only */
      { 8,  1,  4,  1,  1,  256, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  17 }, /* uc32: txmode = ??? */
      { 0 }, /* uc33: rx only */
      { 0 }, /* uc34: rx only */
      { 8,  2,  8,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  15 }, /* uc35: */
      { 8,  2,  8,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  15 }, /* uc36: */
      { 8,  2,  8,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  15 }, /* uc37: */
      { 4,  2,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  10 }, /* uc38 */
      { 0 }, /* uc39: rx only */
      { 8,  2,  8,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  1,  1,  0,  2,  15 }, /* uc40: txmode = 378 */
      { 8,  2,  8,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  15 }, /* uc41: txmode = 378 */
      /*L   F   M   S   HD  K    N    N'   CF  CS DID BID LID  SC SCR Dual V  Mode */
};
adi_cms_jesd_param_t jtx_param[][2] = {
      /*L   F   M   S   HD  K    N    N'   CF  CS DID BID LID  SC SCR Dual V  Mode C2R ModeS */
    { { 0 } }, /* uc0 : nco test */
    { { 4,  4,  8,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  10,  0,  0 } }, /* uc1 : rxmode = 392, link0 */
    { { 4,  4,  8,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  10,  0,  0 } }, /* uc2 : rxmode = 392, link0 */
    { { 2,  2,  2,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  7,   0,  0 },   /* uc3 : rxmode = 360, link0 */
      { 2,  2,  2,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  7,   0,  0 } }, /* uc3 : rxmode = 360, link1 */
    { { 2,  2,  2,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  7,   0,  0 },   /* uc4 : rxmode = 360, link0 */
      { 2,  2,  2,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  7,   0,  0 } }, /* uc4 : rxmode = 360, link1 */
    { { 4,  1,  1,  2,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  13,  0,  0 },   /* uc5 : rxmode = 1088,link0 */
      { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 } }, /* uc5 : rxmode = 411, link1 */
    { { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 },   /* uc6 : rxmode = 411, link0 */
      { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 } }, /* uc6 : rxmode = 411, link1 */
    { { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 },   /* uc7 : rxmode = 411, link0 */
      { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 } }, /* uc7 : rxmode = 411, link1 */
    { { 4,  1,  1,  2,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  13,  0,  0 },   /* uc8 : rxmode = 1087,link0 */
      { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 } }, /* uc8 : rxmode = 411, link1 */
    { { 8,  1,  2,  2,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  19,  0,  0 } }, /* uc9 : rxmode = 502, link0 */
    { { 8,  2,  8,  1,  0,  128, 12,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  16,  0,  0 } }, /* uc10: rxmode = 226, link0 */
    { { 8,  2,  8,  1,  0,  128, 12,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  16,  0,  0 } }, /* uc11: rxmode = 226, link0 */
    { { 8,  4,  16, 1,  0,  64,  12,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  17,  0,  0 } }, /* uc12: rxmode = 234, link0 */
    { { 8,  2,  8,  1,  0,  128, 12,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  16,  0,  0 } }, /* uc13: rxmode = 227, link0 */
    { { 8,  1,  4,  1,  0,  256, 12,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  18,  0,  0 } }, /* uc14: rxmode = 252, link0 */
    { { 8,  3,  2,  8,  0,  256, 12,  12,  0,  0,  0,  0,  0,  0,  1,  0,  2,  28,  0,  0 } }, /* uc15: rxmode = 1263,link0 */
    { { 8,  6,  8,  4,  0,  128, 12,  12,  0,  0,  0,  0,  0,  0,  1,  0,  2,  26,  0,  1 } }, /* uc16: rxmode = 1260,link0 */
    { { 0 } }, /* uc17: tx only */
    { { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 },   /* uc18: rxmode = 411, link0 */
      { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 } }, /* uc18: rxmode = 411, link1 */
    { { 8,  4,  16, 1,  0,  64,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  17,  0,  0 } }, /* uc19: rxmode = 239, link0 */
    { { 8,  2,  8,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  16,  0,  0 } }, /* uc20: rxmode = 227, link0 */
    { { 8,  6,  8,  4,  0,  128, 12,  12,  0,  0,  0,  0,  0,  1,  1,  0,  2,  26,  0,  1 } }, /* uc21: rxmode = 1260,link0 */
    { { 8,  3,  4,  4,  0,  256, 12,  12,  0,  0,  0,  0,  0,  0,  1,  0,  2,  27,  0,  0 } }, /* uc22: rxmode = 1261,link0 */
    { { 4,  4,  8,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  10,  0,  0 } }, /* uc23: rxmode = 397, link0 */
    { { 2,  2,  2,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  7,   0,  0 },   /* uc24: rxmode = 358, link0 */
      { 2,  2,  2,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  7,   0,  0 } }, /* uc24: rxmode = 358, link1 */
    { { 4,  2,  4,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  11,  0,  0 } }, /* uc25: rxmode = 1452,link0 */
    { { 8,  3,  8,  2,  0,  256, 12,  12,  0,  0,  0,  0,  0,  1,  1,  0,  2,  26,  0,  0 } }, /* uc26: rxmode = 1565,link0 */
    { { 8,  3,  8,  2,  0,  256, 12,  12,  0,  0,  0,  0,  0,  1,  1,  0,  2,  26,  0,  0 } }, /* uc27: rxmode = 1565,link0 */
    { { 0 } }, /* uc28: tx only */
    { { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 },   /* uc29: rxmode = 411, link0 */
      { 4,  2,  4,  1,  1,  32,  12,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 } }, /* uc29: rxmode = 411, link1 */
    { { 8,  2,  8,  1,  0,  128, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  16,  0,  0 } }, /* uc30: rxmode = 225,link0 */
    { { 2,  8,  8,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  4,   0,  0 } }, /* uc31: rxmode = 411, link0 */
    { { 8,  1,  4,  1,  1,  256, 16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  18,  0,  0 } }, /* uc32: rxmode = ???, link0 */
    { { 4,  2,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 },   /* uc33: rxmode = , link0 */
      { 4,  2,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 } }, /* uc33: rxmode = , link1 */
    { { 4,  2,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 },   /* uc34: rxmode = , link0 */
      { 4,  2,  4,  1,  0,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 } }, /* uc34: rxmode = , link1 */
    { { 8,  1,  4,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  18,  0,  0 } }, /* uc35: rxmode */
    { { 8,  1,  4,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  18,  0,  0 } }, /* uc36: rxmode */
    { { 8,  1,  4,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  0,  1,  18,  0,  0 } }, /* uc37: rxmode */
    { { 4,  1,  2,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  13,  0,  0 },   /* uc38: rxmode = , link0 */
      { 4,  2,  4,  1,  1,  32,  16,  16,  0,  0,  0,  0,  0,  0,  1,  1,  1,  11,  0,  0 } }, /* uc38: rxmode = , link1 */
    { { 4,  3,  2,  4,  0,  256, 12,  12,  0,  0,  0,  0,  0,  0,  1,  1,  2,  14,  0,  0 },   /* uc39: rxmode = 1261,link0 */
      { 4,  3,  2,  4,  0,  256, 12,  12,  0,  0,  0,  0,  0,  0,  1,  1,  2,  14,  0,  0 } }, /* uc39: rxmode = 1261,link1 */
    { { 8,  2,  8,  1,  0,  128, 12,  16,  0,  0,  0,  0,  0,  1,  1,  0,  2,  16,  0,  0 } }, /* uc40: rxmode = 227, link0 */
    { { 8,  1,  4,  1,  0,  256, 12,  16,  0,  0,  0,  0,  0,  0,  1,  0,  2,  18,  0,  0 } }, /* uc41: rxmode = 252, link0 */
      /*L   F   M   S   HD  K    N    N'   CF  CS DID BID LID  SC SCR Dual V  Mode C2R ModeS */
};

