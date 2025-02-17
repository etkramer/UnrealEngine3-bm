//------------------------------------------------------------------------------
// An enumeration of the valid phonemes.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPhonemeEnum_H__
#define FxPhonemeEnum_H__

namespace OC3Ent
{

namespace Face
{

/// The new set of phonemes supported by FaceFX.
enum FxPhoneme
{
	PHON_INVALID = -1,
	PHON_FIRST = 0,			// 0
	PHON_SIL = PHON_FIRST,	// 0
	PHON_P,					// 1
	PHON_B,					// 2
	PHON_T,					// 3
	PHON_D,					// 4
	PHON_K,					// 5
	PHON_G,					// 6
	PHON_M,					// 7
	PHON_N,					// 8
	PHON_NG,				// 9
	PHON_RA,				// 10
	PHON_RU,				// 11
	PHON_FLAP,				// 12
	PHON_PH,				// 13
	PHON_F,					// 14
	PHON_V,					// 15
	PHON_TH,				// 16
	PHON_DH,				// 17
	PHON_S,					// 18
	PHON_Z,					// 19
	PHON_SH,				// 20
	PHON_ZH,				// 21
	PHON_CX,				// 22
	PHON_X,					// 23
	PHON_GH,				// 24
	PHON_HH,				// 25
	PHON_R,					// 26
	PHON_Y,					// 27
	PHON_L,					// 28
	PHON_W,					// 29
	PHON_H,					// 30
	PHON_TS,				// 31
	PHON_CH,				// 32
	PHON_JH,				// 33
	PHON_IY,				// 34
	PHON_E,					// 35
	PHON_EN,				// 36
	PHON_EH,				// 37
	PHON_A,					// 38
	PHON_AA,				// 39
	PHON_AAN,				// 40
	PHON_AO,				// 41
	PHON_AON,				// 42
	PHON_O,					// 43
	PHON_ON,				// 44
	PHON_UW,				// 45
	PHON_UY,				// 46
	PHON_EU,				// 47
	PHON_OE,				// 48
	PHON_OEN,				// 49
	PHON_AH,				// 50
	PHON_IH,				// 51
	PHON_UU,				// 52
	PHON_UH,				// 53
	PHON_AX,				// 54
	PHON_UX,				// 55
	PHON_AE,				// 56
	PHON_ER,				// 57
	PHON_AXR,				// 58
	PHON_EXR,				// 59
	PHON_EY,				// 60
	PHON_AW,				// 61
	PHON_AY,				// 62
	PHON_OY,				// 63
	PHON_OW,				// 64
	PHON_LAST = PHON_OW,	// 65
	NUM_PHONS = PHON_LAST - PHON_FIRST + 1 // 66
};

class FxPhonemeUtility
{
public:
	static const FxPhoneme UpdatePhoneme(int oldPhoneme);
	static const FxPhoneme UpdatePhoneme(FxPhoneme oldPhoneme);
};

} // namespace Face

} // namespace OC3Ent

#endif
