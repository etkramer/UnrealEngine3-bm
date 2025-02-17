//------------------------------------------------------------------------------
// An enumeration of the valid phonemes.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxPhonemeEnum.h"

namespace OC3Ent
{

namespace Face
{

const int EnumUpdateMapVersion0To1[] = {0x22,0x33,0x25,0x3c,0x38,0x27,0x3d,0x3e,0x32,0x29,0x3f,0x40,0x35,0x2d,0x39,0x36,0x12,0x14,0x13,0x15,0xe,0x10,017,0x11,0x7,010,0x9,0x1c,0x1a,0x1d,0x1b,0x19,0x2,0x4,0x21,0x6,0x1,0x3,0x5,0x20,0x0,0x0,0xc};

const FxPhoneme FxPhonemeUtility::UpdatePhoneme(int oldPhoneme)
{
	return static_cast<FxPhoneme>(EnumUpdateMapVersion0To1[oldPhoneme]);
}

const FxPhoneme FxPhonemeUtility::UpdatePhoneme(FxPhoneme oldPhoneme)
{
	return UpdatePhoneme(static_cast<int>(oldPhoneme));
}

} // namespace Face

} // namespace OC3Ent

