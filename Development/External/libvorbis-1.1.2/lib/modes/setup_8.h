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

 function: 8kHz settings 
 last mod: $Id: setup_8.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include "psych_8.h"
#include "residue_8.h"

static int blocksize_8[3] =
{
	512, 512, 512
};

static int _floor_mapping_8[3] =
{
	6, 6, 6
};

static double rate_mapping_8[4] =
{
	5000.0, 6000.0, 9000.0, 32000.0,
};

static double rate_mapping_8_uncoupled[4] =
{
	8000.0, 10000.0, 14000.0, 42000.0,
};

static double quality_mapping_8[4] =
{
	-0.2, -0.1, 0.0, 1.0
};

static double _psy_compand_8_mapping[4] =
{
	0.0, 1.0, 2.0, 2.0
};

static double _global_mapping_8[4] =
{
	2.0, 2.0, 3.0, 4.0
};

ve_setup_data_template ve_setup_8_stereo =
{
	3,
	rate_mapping_8,
	quality_mapping_8,
	2,
	8000,
	9000,

	blocksize_8,
	blocksize_8,

	_psy_tone_masteratt_8,
	_psy_tone_0dB,
	_psy_tone_suppress,

	_vp_tonemask_adj_8,
	NULL,
	_vp_tonemask_adj_8,

	_psy_noiseguards_8,
	_psy_noisebias_8,
	_psy_noisebias_8,
	NULL,
	NULL,
	_psy_noise_suppress,

	_psy_compand_8,
	_psy_compand_8_mapping,
	NULL,

	{ _noise_start_8,_noise_start_8 },
	{ _noise_part_8,_noise_part_8 },
	_noise_thresh_5only,

	_psy_ath_floater_8,
	_psy_ath_abs_8,

	_psy_lowpass_8,

	_psy_global_44,
	_global_mapping_8,
	_psy_stereo_modes_8,

	_floor_books,
	_floor,
	_floor_mapping_8,
	NULL,

	_mapres_template_8_stereo
};

ve_setup_data_template ve_setup_8_uncoupled =
{
	3,
	rate_mapping_8_uncoupled,
	quality_mapping_8,
	-1,
	8000,
	9000,

	blocksize_8,
	blocksize_8,

	_psy_tone_masteratt_8,
	_psy_tone_0dB,
	_psy_tone_suppress,

	_vp_tonemask_adj_8,
	NULL,
	_vp_tonemask_adj_8,

	_psy_noiseguards_8,
	_psy_noisebias_8,
	_psy_noisebias_8,
	NULL,
	NULL,
	_psy_noise_suppress,

	_psy_compand_8,
	_psy_compand_8_mapping,
	NULL,

	{ _noise_start_8,_noise_start_8 },
	{ _noise_part_8,_noise_part_8 },
	_noise_thresh_5only,

	_psy_ath_floater_8,
	_psy_ath_abs_8,

	_psy_lowpass_8,

	_psy_global_44,
	_global_mapping_8,
	_psy_stereo_modes_8,

	_floor_books,
	_floor,
	_floor_mapping_8,
	NULL,

	_mapres_template_8_uncoupled
};

