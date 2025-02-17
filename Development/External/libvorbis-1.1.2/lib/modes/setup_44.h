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

 function: toplevel settings for 44.1/48kHz
 last mod: $Id: setup_44.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#include "modes/floor_all.h"
#include "modes/residue_44.h"
#include "modes/psych_44.h"

static double rate_mapping_44_stereo[13] =
{
	16000.0, 24000.0, 32000.0, 40000.0, 48000.0, 56000.0, 64000.0,
	80000.0, 96000.0, 112000.0, 128000.0, 160000.0, 250001.0
};

static double quality_mapping_44[13] =
{
	-0.2, -0.1, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0
};

static int blocksize_short_44[12] =
{
	512, 512, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256
};
static int blocksize_long_44[12] =
{
	4096, 4096, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048
};

static double _psy_compand_short_mapping[13] =
{
	1.0, 1.0, 1.0, 1.0, 1.3, 1.6, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0
};

static double _psy_compand_long_mapping[13] =
{
	4.0, 4.0, 4.0, 4.0, 4.3, 4.6, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0
};

static double _global_mapping_44[13] =
{
	0.0, 1.0, 2.0, 2.0, 2.5, 3.0, 3.0, 3.5, 3.7, 4.0, 4.0, 5.0, 5.0 // low
};

static int _floor_short_mapping_44[12] =
{
	1, 1, 0, 0, 2, 2, 4, 5, 5, 5, 5, 5
};
static int _floor_long_mapping_44[12] =
{
	8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

ve_setup_data_template ve_setup_44_stereo =
{
//  11,
	12,
	rate_mapping_44_stereo,
	quality_mapping_44,
	2,
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
	_psy_stereo_modes_44,

	_floor_books,
	_floor,
	_floor_short_mapping_44,
	_floor_long_mapping_44,

	_mapres_template_44_stereo
};

