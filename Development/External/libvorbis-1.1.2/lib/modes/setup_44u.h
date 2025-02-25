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

 function: toplevel settings for 44.1/48kHz uncoupled modes
 last mod: $Id: setup_44u.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include "modes/residue_44u.h"

static double rate_mapping_44_un[13] =
{
  28000.0, 40000.0, 48000.0, 60000.0, 70000.0, 80000.0, 86000.0,
  96000.0, 110000.0, 120000.0, 140000.0, 160000.0, 240001.0
};

ve_setup_data_template ve_setup_44_uncoupled =
{
	12,
	rate_mapping_44_un,
	quality_mapping_44,
	-1,
	40000,
	50000,

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

	{ _noise_start_short_44, _noise_start_long_44 },
	{ _noise_part_short_44, _noise_part_long_44 },
	_noise_thresh_44,

	_psy_ath_floater,
	_psy_ath_abs,

	_psy_lowpass_44,

	_psy_global_44,
	_global_mapping_44,
	NULL,

	_floor_books,
	_floor,
	_floor_short_mapping_44,
	_floor_long_mapping_44,

	_mapres_template_44_uncoupled
};
