/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: 16kHz settings 
 last mod: $Id: setup_16.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include "psych_16.h"
#include "residue_16.h"

static int blocksize_16_short[4] =
{
	1024, 1024, 512, 512
};

static int blocksize_16_long[4] =
{
	1024, 1024, 1024, 1024
};

static int _floor_mapping_16_short[4] =
{
	9, 9, 3, 3
};

static int _floor_mapping_16[4] =
{
	9, 9, 9, 9
};

static double rate_mapping_16[5] =
{
	10000.0, 12000.0, 20000.0, 44000.0, 86000.0
};

static double rate_mapping_16_uncoupled[5] =
{
	16000.0, 18000.0, 28000.0, 64000.0, 100000.0
};

static double _global_mapping_16[5] =
{
	2.0, 2.0, 3.0, 4.0, 5.0
};

static double quality_mapping_16[5] =
{
	-0.2, -0.1, 0.05, 0.5, 1.0 
};

static double _psy_compand_16_mapping[5] =
{
	0.0, 1.0, 1.8, 2.0, 2.0
};

ve_setup_data_template ve_setup_16_stereo =
{
//  3,
	4,
	rate_mapping_16,
	quality_mapping_16,
	2,
	15000,
	19000,

	blocksize_16_short,
	blocksize_16_long,

	_psy_tone_masteratt_16,
	_psy_tone_0dB,
	_psy_tone_suppress,

	_vp_tonemask_adj_16,
	_vp_tonemask_adj_16,
	_vp_tonemask_adj_16,

	_psy_noiseguards_8,
	_psy_noisebias_16_impulse,
	_psy_noisebias_16_short,
	_psy_noisebias_16_short,
	_psy_noisebias_16,
	_psy_noise_suppress,

	_psy_compand_8,
	_psy_compand_16_mapping,
	_psy_compand_16_mapping,

	{ _noise_start_16, _noise_start_16 },
	{ _noise_part_16, _noise_part_16 },
	_noise_thresh_16,

	_psy_ath_floater_16,
	_psy_ath_abs_16,

	_psy_lowpass_16,

	_psy_global_44,
	_global_mapping_16,
	_psy_stereo_modes_16,

	_floor_books,
	_floor,
	_floor_mapping_16_short,
	_floor_mapping_16,

	_mapres_template_16_stereo
};

ve_setup_data_template ve_setup_16_uncoupled =
{
//  3,
	4,
	rate_mapping_16_uncoupled,
	quality_mapping_16,
	-1,
	15000,
	19000,

	blocksize_16_short,
	blocksize_16_long,

	_psy_tone_masteratt_16,
	_psy_tone_0dB,
	_psy_tone_suppress,

	_vp_tonemask_adj_16,
	_vp_tonemask_adj_16,
	_vp_tonemask_adj_16,

	_psy_noiseguards_8,
	_psy_noisebias_16_impulse,
	_psy_noisebias_16_short,
	_psy_noisebias_16_short,
	_psy_noisebias_16,
	_psy_noise_suppress,

	_psy_compand_8,
	_psy_compand_16_mapping,
	_psy_compand_16_mapping,

	{ _noise_start_16, _noise_start_16 },
	{ _noise_part_16, _noise_part_16 },
	_noise_thresh_16,

	_psy_ath_floater_16,
	_psy_ath_abs_16,

	_psy_lowpass_16,

	_psy_global_44,
	_global_mapping_16,
	_psy_stereo_modes_16,

	_floor_books,
	_floor,
	_floor_mapping_16_short,
	_floor_mapping_16,

	_mapres_template_16_uncoupled
};
