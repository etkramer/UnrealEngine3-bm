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

 function: toplevel settings for 32kHz
 last mod: $Id: setup_32.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

static double rate_mapping_32[13] = 
{
	14000.0, 21000.0, 28000.0, 35000.0, 45000.0, 56000.0, 60000.0,
	75000.0, 90000.0, 100000.0, 115000.0, 150000.0, 190000.0,
};

static double rate_mapping_32_un[13] =
{
	26000.0, 32000.0, 42000.0, 52000.0, 64000.0, 72000.0, 78000.0,
	86000.0, 92000.0, 110000.0, 120000.0, 140000.0, 190000.0,
};

static double _psy_lowpass_32[13] =
{
	12.1, 12.6, 13.0, 13.0, 14.0, 15.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0, 99.0
};

ve_setup_data_template ve_setup_32_stereo =
{
//  11,
	12,
	rate_mapping_32,
	quality_mapping_44,
	2,
	26000,
	40000,

	blocksize_short_44,
	blocksize_long_44,

	_psy_tone_masteratt_44,
	_psy_tone_0dB,
	_psy_tone_suppress,

	_vp_tonemask_adj_otherblock,
	_vp_tonemask_adj_longblock,
	_vp_tonemask_adj_otherblock,

	_psy_noiseguards_44,
	_psy_noisebias_impulse,
	_psy_noisebias_padding,
	_psy_noisebias_trans,
	_psy_noisebias_long,
	_psy_noise_suppress,

	_psy_compand_44,
	_psy_compand_short_mapping,
	_psy_compand_long_mapping,

	{ _noise_start_short_32, _noise_start_long_32 },
	{ _noise_part_short_44, _noise_part_long_44 },
	_noise_thresh_44,

	_psy_ath_floater,
	_psy_ath_abs,

	_psy_lowpass_32,

	_psy_global_44,
	_global_mapping_44,
	_psy_stereo_modes_44,

	_floor_books,
	_floor,
	_floor_short_mapping_44,
	_floor_long_mapping_44,

	_mapres_template_44_stereo
};

ve_setup_data_template ve_setup_32_uncoupled =
{
//  11,
	12,
	rate_mapping_32_un,
	quality_mapping_44,
	-1,
	26000,
	40000,

	blocksize_short_44,
	blocksize_long_44,

	_psy_tone_masteratt_44,
	_psy_tone_0dB,
	_psy_tone_suppress,

	_vp_tonemask_adj_otherblock,
	_vp_tonemask_adj_longblock,
	_vp_tonemask_adj_otherblock,

	_psy_noiseguards_44,
	_psy_noisebias_impulse,
	_psy_noisebias_padding,
	_psy_noisebias_trans,
	_psy_noisebias_long,
	_psy_noise_suppress,

	_psy_compand_44,
	_psy_compand_short_mapping,
	_psy_compand_long_mapping,

	{ _noise_start_short_32, _noise_start_long_32 },
	{ _noise_part_short_44, _noise_part_long_44 },
	_noise_thresh_44,

	_psy_ath_floater,
	_psy_ath_abs,

	_psy_lowpass_32,

	_psy_global_44,
	_global_mapping_44,
	NULL,

	_floor_books,
	_floor,
	_floor_short_mapping_44,
	_floor_long_mapping_44,

	_mapres_template_44_uncoupled
};
