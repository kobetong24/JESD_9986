/*!
 * @brief     HSX Standalone App Extras Header File
 *            This file contains all the publicly exposed methods and data
 *            structures to interface with Standalone Application.
 *
 * @copyright copyright(c) 2020 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup ADI_HSX_EXTRAS
 * @{
 */

#ifndef __HSXSTANDALONEEXTRAS_H__
#define __HSXSTANDALONEEXTRAS_H__
#ifdef TEST_BUILD
	#include <unity.h>
#endif
/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/*!
*@brief Structure containing JESD parameters
*/
typedef struct {
	unsigned int l; /*!< No of lanes */
	unsigned int f; /*!< No of octets in a frame */
	unsigned int m; /*!< No of converters */
	unsigned int s; /*!< No of samples */
	unsigned int np; /*!< Bit packing sample */
}adi_hsx_extras_jesd_param_t;

/**
 * @brief  Save vectors from text file
 *
 * @param num_samples_max Maximum number of samples that vector can hold
 * @param vecs Array data structure containing samples
 * @param filename Name of the text file containing samples
 * @param jesdparam Jesdparam struct
 * @param num_samples_read number of samples in text file
 *
 * @return 0                     				Vector successfully stored in data structure from text file
 * @return <0                                   Failed. 
 */
int32_t adi_hsx_extras_vector_from_file(int num_samples_max, int16_t vecs[num_samples_max], char filename[], adi_hsx_extras_jesd_param_t jesdparam, int * num_samples_read); 

/**
 * @brief  Generate single tone sine waves
 *
 * @param f_data_rate the rate samples are output from the FPGA
 * @param f_tone fundamental tone frequency requested
 * @param num_bits number of bits in amplitude
 * @param phase phase in degrees
 * @param backoff amplitude (in db) 0db is full scale
 * @param n_samples number of samples to generate
 * @param vec array data structure to hold generated sine wave
 *
 * @return 0                     				Vector successfully created
 * @return <0                                   Failed. 
 */
int32_t adi_hsx_extras_create_single_tone(double f_data_rate, double f_tone, uint32_t num_bits, double phase, double backoff, uint32_t n_samples, int16_t vec[n_samples]); 

/**
 * @brief  Converts single-link per converter vectors into ADS9 memory format
 *
 * @param num_dacs number of virtual dacs
 * @param vec_len number of samples per converter
 * @param vecs Input array organized by number of virtual dacs and samples per converter
 * @param output_len length of output buffer in ADS9 memory
 * @param output output buffer formatted for ADS9 memory
 * @param jesdparam jesdparam struct
 *
 * @return 0                     				Vector successfully downloaded into ADS9 memory buffer
 * @return <0                                   Failed. 
 */
int32_t adi_hsx_extras_jesd204_download_vectors_single_link(int num_dacs, int vec_len, uint16_t vecs[num_dacs][vec_len], uint32_t output_len, uint8_t output[output_len], adi_hsx_extras_jesd_param_t jesdparam);

/**
 * @brief  Converts dual-link per converter vectors into ADS9 memory format
 *
 * @param num_dacs0 number of virtual dacs in link 0
 * @param vec_len0 number of samples per converter in link 0
 * @param vecs0 Input array organized by number of virtual dacs and samples per converter in link 0
 * @param num_dacs1 number of virtual dacs in link 1
 * @param vec_len1 number of samples per converter in link 1
 * @param vecs1 Input array organized by number of virtual dacs and samples per converter in link 1
 * @param output_len length of output buffer in ADS9 memory
 * @param output output buffer formatted for ADS9 memory
 * @param jesdparam jesdparam struct
 *
 * @return 0                     				Vector successfully downloaded into ADS9 memory buffer
 * @return <0                                   Failed. 
 */
int32_t adi_hsx_extras_jesd204_download_vectors_dual_link(int num_dacs0, int vec_len0, uint16_t vecs0[num_dacs0][vec_len0], int num_dacs1, int vec_len1, uint16_t vecs1[num_dacs1][vec_len1], uint32_t output_len, uint8_t output[output_len], adi_hsx_extras_jesd_param_t jesdparam); 

/**
 * @brief Stores raw capture data in text file
 *
 * @param num_raw_bytes number of bytes captured
 * @param raw_data Array data structure containing captured bytes
 * @param filename Filename of file containing raw bytes
 * @param bytes_per_line 1: Data printed in file as 1 byte per line (8 bits)
 * 						 2: Data printed in file as 2 bytes per line (16 bits)
 * 						 4: Data printed in file as 4 bytes per line (32 bits)
 * 
 * @return 0                     				Raw capture data successfully stored in file
 * @return <0                                   Failed. 
 */
int32_t adi_hsx_raw_data_capture_to_file(int num_raw_bytes, uint8_t raw_data[num_raw_bytes], char filename[], int bytes_per_line); 

/**
 * @brief  Converts capture data to samples per dac per link format
 *
 * @param num_raw_bytes number of bytes captured
 * @param raw_data Array data structure containing captured bytes
 * @param num_links number of links (1 or 2)
 * @param num_dacs_per_link number of dacs per link
 * @param samples_per_converter number of samples per virtual converter
 * @param extracted_data data structure containing formatted capture data
 * @param jesdparam jesdparam struct
 *
 * @return 0                     				Capture data successfully extracted
 * @return <0                                   Failed. 
 */
int32_t adi_hsx_extras_extract_capture(int num_raw_bytes, uint8_t raw_data[num_raw_bytes], int num_links, int num_dacs_per_link, int samples_per_converter, uint16_t extracted_data[num_links][num_dacs_per_link][samples_per_converter],  adi_hsx_extras_jesd_param_t jesdparam); 

/**
 * @brief  Converts capture data to samples per dac per link format
 *
 * @param num_raw_bytes number of bytes captured
 * @param raw_data Array data structure containing captured bytes
 * @param num_links number of links (1 or 2)
 * @param jesdparam jesdparam struct
 * @param filename_prefix Prefix for each formatted captured data file organized by link by dac
 *
 * @return 0                     				Capture data successfully extracted and saved
 * @return <0                                   Failed. 
 */
int32_t adi_hsx_extras_extract_capture_to_file(int num_raw_bytes, uint8_t raw_data[num_raw_bytes], int num_links, adi_hsx_extras_jesd_param_t jesdparam, char filename_prefix[]); 


#endif