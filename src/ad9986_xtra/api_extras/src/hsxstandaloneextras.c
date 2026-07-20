/*!
 * @brief     HSX Standalone App Extras Source File
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

/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "hsxstandaloneextras.h"

/*============= C O D E ====================*/
int32_t adi_hsx_extras_jesd204_vecs_to_lanes(int num_dacs, int vec_len, uint16_t vecs[num_dacs][vec_len], int num_lanes, int octets_per_lane, uint8_t lanes_output[num_lanes][octets_per_lane], adi_hsx_extras_jesd_param_t jesdparam);
int32_t adi_hsx_extras_jesd204_linear_mapping(int num_dacs, int vec_len, uint16_t vecs[num_dacs][vec_len], int output_len, uint8_t stepone_output[output_len], adi_hsx_extras_jesd_param_t jesdparam);
int32_t adi_hsx_extras_jesd204_map_to_frames(int map_len, uint8_t linearmap[map_len], int num_frames, int octets_per_frame_per_lane, uint8_t steptwo_output[num_frames][octets_per_frame_per_lane], int num_dacs, int vec_len, adi_hsx_extras_jesd_param_t jesdparam);
int32_t adi_hsx_extras_jesd204_frames_to_lanes(int num_frames, int octets_per_frame_per_lane, uint8_t framesmap[num_frames][octets_per_frame_per_lane], int num_lanes, int octets_per_lane, uint8_t stepthree_output[num_lanes][octets_per_lane], int num_dacs, int vec_len, adi_hsx_extras_jesd_param_t jesdparam);
int32_t adi_hsx_extras_jesd204_lanes_to_mem(int num_lanes, int octets_per_lane, uint8_t lanes_link0[num_lanes][octets_per_lane], int num_lanes1, int octets_per_lane1, uint8_t lanes_link1[num_lanes1][octets_per_lane1], uint32_t buffer_len, uint8_t buffer[buffer_len], int vec_len, int num_links, adi_hsx_extras_jesd_param_t jesdparam);

int32_t adi_hsx_extras_vector_from_file(int num_samples_max, int16_t vecs[num_samples_max], char filename[], adi_hsx_extras_jesd_param_t jesdparam, int * num_samples_read)
{
	FILE *fp = fopen(filename, "r");
	size_t len = jesdparam.np;
	char num[len];
	if (fp == NULL) {
		printf("fopen failed to open the file\n");
		return -1;
	}

	int j = 0;
	while (fgets(num, len, fp) != NULL) {
		vecs[j] = atoi(num);
		j++;
		if (j >= num_samples_max) {
			break;
		}
	}
	*num_samples_read = j;
	fclose(fp);    
	return 0;
}

int32_t adi_hsx_extras_create_single_tone(double f_data_rate, double f_tone, uint32_t num_bits, double phase, double backoff, uint32_t n_samples, int16_t vec[n_samples])
{
	double phase_rads = phase * M_PI / 180.0;
	uint32_t m_cycles = (f_tone * n_samples) / f_data_rate;
	if (m_cycles % 2 == 0) {
		m_cycles++;
	}
	f_tone = (m_cycles * f_data_rate) / n_samples;

	double ratio = (2 * M_PI * f_tone) / f_data_rate;
	double amplitude = (1 << ((uint16_t)num_bits - 1)) - 1;
	double backoff_db = pow(10, (backoff / 20.0));
	for (int i = 0; i < n_samples; i++) {
		vec[i] = (int) amplitude * backoff_db * sin((ratio * i) + phase_rads); 
	}
	return 0;
}

int32_t adi_hsx_extras_jesd204_download_vectors_single_link(int num_dacs, int vec_len, uint16_t vecs[num_dacs][vec_len], uint32_t output_len, uint8_t output[output_len], adi_hsx_extras_jesd_param_t jesdparam) 
{
	int num_links = 1;
	int num_lanes = jesdparam.l;
	int num_frames = (jesdparam.m * vec_len * (jesdparam.np / 8.0)) / jesdparam.f;  
	int octets_per_lane = (num_frames / num_lanes) * jesdparam.f;
	uint8_t (*lanes_output)[octets_per_lane] = malloc(num_lanes * octets_per_lane * sizeof(uint8_t));

	adi_hsx_extras_jesd204_vecs_to_lanes(num_dacs, vec_len, vecs, num_lanes, octets_per_lane, lanes_output, jesdparam);
	
	adi_hsx_extras_jesd204_lanes_to_mem(num_lanes, octets_per_lane, lanes_output, num_lanes, octets_per_lane, NULL, output_len, output, vec_len, num_links, jesdparam);
    
	free(lanes_output);
	return 0;
}

int32_t adi_hsx_extras_jesd204_download_vectors_dual_link(int num_dacs0, int vec_len0, uint16_t vecs0[num_dacs0][vec_len0], int num_dacs1, int vec_len1, uint16_t vecs1[num_dacs1][vec_len1], uint32_t output_len, uint8_t output[output_len], adi_hsx_extras_jesd_param_t jesdparam) 
{
	int num_links = 2;
	if (vec_len0 != vec_len1) {
		fprintf(stderr, "Number of samples must be the same in dual link model.");
		return -1;
	}
	int num_dacs = jesdparam.m;
	int num_lanes = jesdparam.l;
	int num_frames = (jesdparam.m * vec_len0 * (jesdparam.np / 8.0)) / jesdparam.f;  
	int octets_per_lane = (num_frames / num_lanes) * jesdparam.f;
	uint8_t (*lanes0_output)[octets_per_lane] = malloc(num_lanes * octets_per_lane * sizeof(uint8_t));
	uint8_t (*lanes1_output)[octets_per_lane] = malloc(num_lanes * octets_per_lane * sizeof(uint8_t));

	adi_hsx_extras_jesd204_vecs_to_lanes(num_dacs, vec_len0, vecs0, num_lanes, octets_per_lane, lanes0_output, jesdparam);
	adi_hsx_extras_jesd204_vecs_to_lanes(num_dacs, vec_len1, vecs1, num_lanes, octets_per_lane, lanes1_output, jesdparam);

	adi_hsx_extras_jesd204_lanes_to_mem(num_lanes, octets_per_lane, lanes0_output, num_lanes, octets_per_lane, lanes1_output, output_len, output, vec_len0, num_links, jesdparam);
    
	free(lanes0_output);
	free(lanes1_output);
	return 0;
}

int32_t adi_hsx_raw_data_capture_to_file(int num_raw_bytes, uint8_t raw_data[num_raw_bytes], char filename[], int bytes_per_line) 
{ 
	FILE * fp = fopen(filename, "w");   
	if (fp == NULL) {
		printf("fopen failed to open the file\n");
		return -1;
	}

	if (bytes_per_line == 4){
		uint32_t * raw_data_32 = (uint32_t *) raw_data; 
		uint32_t num_32bit_words = num_raw_bytes / sizeof(uint32_t);
		for (int i = 0; i < num_32bit_words; i++) {
			char temp[32];
			snprintf(temp, 32, "%08X\n", raw_data_32[i]);
			fputs(temp, fp);
		}
	}
	else if (bytes_per_line == 2){
		uint16_t * raw_data_16 = (uint16_t *) raw_data; 
		uint16_t num_16bit_words = num_raw_bytes / sizeof(uint16_t);
		for (int i = 0; i < num_16bit_words; i++) {
			char temp[32];
			snprintf(temp, 32, "%04X\n", raw_data_16[i]);
			fputs(temp, fp);
		}
	}
    else{
		for (int i = 0; i < num_raw_bytes; i++) {
			char temp[32];
			snprintf(temp, 32, "%02X\n", raw_data[i]);
			fputs(temp, fp);
		}
	}
	
	
	fclose(fp);
	return 0;
	
}

int32_t adi_hsx_extras_extract_capture(int num_raw_bytes, uint8_t raw_data[num_raw_bytes], int num_links, int num_dacs_per_link, int samples_per_converter, uint16_t extracted_data[num_links][num_dacs_per_link][samples_per_converter], adi_hsx_extras_jesd_param_t jesdparam) 
{   
	uint32_t *raw_data_32int = (uint32_t *) raw_data; 
	
	uint32_t num_32int_words = num_raw_bytes / sizeof(uint32_t);
	
	if(jesdparam.np > 16) {
		printf("jesd204 parameter np has to be less than or equal to 16 bits.\n");
		return -1;
	}

	if ((samples_per_converter * 2) % num_dacs_per_link) {
		printf("Samples cannot be formatted with given virtual converters.");
		return -1;
	}
        
	if (num_links > 1) {
		int dac = 0;
		int samp = 0;
		for (int i = 0; i < num_32int_words; i++) {
			uint32_t temp_word = raw_data_32int[i];
			uint16_t msb_converter = ((int16_t)(temp_word >> 16));
			uint16_t lsb_converter = ((int16_t)(temp_word & 0x0000ffff));
			dac = i % num_dacs_per_link;
			samp = i / num_dacs_per_link;
			if (samp == samples_per_converter) {
        		break;
    		}
			extracted_data[0][dac][samp] = lsb_converter;
            extracted_data[1][dac][samp] = msb_converter;
		}
	} else {
		int dac = 0;
		int samp = 0;
		for (int i = 0; i < num_32int_words; i++) {
			uint32_t temp_word = raw_data_32int[i];
			uint16_t msb_converter = ((int16_t)(temp_word >> 16));
			uint16_t lsb_converter = ((int16_t)(temp_word & 0x0000ffff));
			extracted_data[0][dac][samp] = lsb_converter;
			dac++;
			if (dac >= num_dacs_per_link) {
				dac = 0; 
				samp++;
			}
			extracted_data[0][dac][samp] = msb_converter;
			dac++;
			if (dac >= num_dacs_per_link) {
				dac = 0;
				samp++;
			}
			if (samp > samples_per_converter) {
				break;
			}
		}
	}
	return 0;
}

int32_t adi_hsx_extras_extract_capture_to_file(int num_raw_bytes, uint8_t raw_data[num_raw_bytes], int num_links, adi_hsx_extras_jesd_param_t jesdparam, char filename_prefix[]) 
{
	int samples_per_word = 2;
	int num_dacs_per_link = jesdparam.m;
	int samples_per_converter = (int)(num_raw_bytes) / (num_dacs_per_link * num_links * samples_per_word);
	uint16_t(*extracted_data)[num_dacs_per_link][samples_per_converter] = malloc(sizeof(uint16_t[num_links][num_dacs_per_link][samples_per_converter]));

	adi_hsx_extras_extract_capture(num_raw_bytes, raw_data, num_links, num_dacs_per_link, samples_per_converter, extracted_data, jesdparam);   
	for (int i = 0; i < num_links; i++) {
		for (int j = 0; j < num_dacs_per_link; j++) {
			FILE * fp;
			char filenametemp[100];
			strcpy(filenametemp, filename_prefix);
			char temp[100];
			strcat(filenametemp, "_L");
			snprintf(temp, 100, "%d", i);
			strcat(filenametemp, temp);
			strcat(filenametemp, "C");
			snprintf(temp, 100, "%d", j);
			strcat(filenametemp, temp);
			strcat(filenametemp, ".txt");
			fp = fopen(filenametemp, "w");
			if (fp == NULL) {
				printf("fopen failed to open the file\n");
				return -1;
			}
			for (int k = 0; k < samples_per_converter; k++) {
				char * temp2 = malloc(sizeof(char) * 8);
				snprintf(temp2, 8, "%d\n", (int16_t)extracted_data[i][j][k]);
				fputs(temp2, fp);
				free(temp2);
			}
			fclose(fp);      
		}
	}
	free(extracted_data);
	return 0;
}

int32_t adi_hsx_extras_jesd204_vecs_to_lanes(int num_dacs, int vec_len, uint16_t vecs[num_dacs][vec_len], int num_lanes, int octets_per_lane, uint8_t lanes_output[num_lanes][octets_per_lane], adi_hsx_extras_jesd_param_t jesdparam) 
{
	if (jesdparam.l != 0) {
		if (jesdparam.f != (jesdparam.m * jesdparam.s * jesdparam.np) / (8*jesdparam.l)) {
			fprintf(stderr, "jesd204 parameters not valid.\n");
			return -1;
		}
	}
   
   	double bytes_per_sample = jesdparam.np / 8.0;
	int map_len = jesdparam.m * vec_len * bytes_per_sample;
	uint8_t (*linearoutput) = calloc(map_len, sizeof(uint8_t));
	adi_hsx_extras_jesd204_linear_mapping(num_dacs, vec_len, vecs, map_len, linearoutput, jesdparam);


	int num_frames = (jesdparam.m * vec_len * (jesdparam.np / 8.0)) / jesdparam.f;  
	int octets_per_frame_per_lane = jesdparam.f;
	uint8_t (*frameoutput)[octets_per_frame_per_lane] = malloc(num_frames * octets_per_frame_per_lane * sizeof(uint8_t)); 
	adi_hsx_extras_jesd204_map_to_frames(map_len, linearoutput, num_frames, octets_per_frame_per_lane, frameoutput, jesdparam.m, vec_len, jesdparam);


	adi_hsx_extras_jesd204_frames_to_lanes(num_frames, octets_per_frame_per_lane, frameoutput, num_lanes, octets_per_lane, lanes_output, jesdparam.m, vec_len, jesdparam);

	free(linearoutput);
	free(frameoutput);

	return 0;
}

int32_t adi_hsx_extras_jesd204_linear_mapping(int num_dacs, int vec_len, uint16_t vecs[num_dacs][vec_len], int output_len, uint8_t stepone_output[output_len], adi_hsx_extras_jesd_param_t jesdparam) 
{
	double bytes_per_sample = jesdparam.np / 8.0;
	int nibbles_per_sample = (int) 2*bytes_per_sample;
	int num_samples_per_frame = jesdparam.s;

	if (num_dacs != jesdparam.m) {
		fprintf(stderr, "number of dacs don't align with jesdparam\n");
		return -1;
	}  
	int samples_per_dac = vec_len;
    
	int byte_counter = 0;
	int nibble_counter = 0;

	uint16_t sample = 0;

	for (unsigned int i = 0; i < samples_per_dac; i += num_samples_per_frame) {
		for (unsigned int m = 0; m < num_dacs; m++) {
			for (unsigned int s = 0; s < num_samples_per_frame; s++) {
				if ((i + s) > vec_len) {
					break;
				}
				sample = (uint16_t)vecs[m][i + s];
				for (int n = 0; n < nibbles_per_sample; n++) {
					int shift = 4 * (nibbles_per_sample - n - 1);
					stepone_output[byte_counter] |= (uint8_t)((sample >> shift) & 0x000f) << (4 - ((nibble_counter % 2) * 4));
					nibble_counter++;
					if ((nibble_counter % 2) == 0) {
						byte_counter++;
					}
				}
			}
		}
	}
	return 0;
}

int32_t adi_hsx_extras_jesd204_map_to_frames(int map_len, uint8_t linearmap[map_len], int num_frames, int octets_per_frame_per_lane, uint8_t steptwo_output[num_frames][octets_per_frame_per_lane], int num_dacs, int vec_len, adi_hsx_extras_jesd_param_t jesdparam) 
{    
	if(((int)(num_dacs * vec_len * (jesdparam.np / 8.0)) % octets_per_frame_per_lane) != 0) {
		fprintf(stderr, "Octet array length must be a multiple of F.\n");
		return -1;
	}

	int byte_counter = 0;
	for (int i = 0; i < num_frames; i++) {
		for (int j = 0; j < octets_per_frame_per_lane; j++) {
			steptwo_output[i][j] = linearmap[byte_counter];
			byte_counter++;

		}
	}
	return 0;
}

int32_t adi_hsx_extras_jesd204_frames_to_lanes(int num_frames, int octets_per_frame_per_lane, uint8_t framesmap[num_frames][octets_per_frame_per_lane], int num_lanes, int octets_per_lane, uint8_t stepthree_output[num_lanes][octets_per_lane], int num_dacs, int vec_len, adi_hsx_extras_jesd_param_t jesdparam) 
{    
	if(num_frames % num_lanes != 0) {
		fprintf(stderr, "Framed array length must be a multiple of jesdparam L\n");
		return -1;
	}
	for (int l = 0; l < num_lanes; l++) {
		int octet = 0;
		for (int f = l; f < num_frames; f += num_lanes) {
			for (int byteofframe = 0; byteofframe < octets_per_frame_per_lane; byteofframe++) {
				if (octet > (octets_per_lane - 1)) {
					break;
				}
				stepthree_output[l][octet] = framesmap[f][byteofframe];
				octet++;
			}
		}
	}
	return 0;
}

int32_t adi_hsx_extras_jesd204_lanes_to_mem(int num_lanes, int octets_per_lane, uint8_t lanes_link0[num_lanes][octets_per_lane], int num_lanes1, int octets_per_lane1, uint8_t lanes_link1[num_lanes1][octets_per_lane1], uint32_t buffer_len, uint8_t buffer[buffer_len], int vec_len, int num_links, adi_hsx_extras_jesd_param_t jesdparam)
{
	int ads9 = 256;   

	int offset = 0;
	int count = 0;
	int total = octets_per_lane / ads9;
	if (octets_per_lane % ads9 != 0) {
		total += 1;
	}

	for(int set = 0 ; set < total ; set++) {
		offset = set*ads9;
		for (int link = 0; link < num_links; link++) {
			for (int l = 0; l < jesdparam.l; l++) {
				for (int i = 0; i < ads9; i++) {
					if (i + offset < octets_per_lane) {
						if (link == 0) {
							buffer[count] = lanes_link0[l][i + offset];
						}
						else {
							buffer[count] = lanes_link1[l][i + offset];
						}
					}
					count++;
				}
			}
		}
	}
        
	return 0;
}


/*! @} */