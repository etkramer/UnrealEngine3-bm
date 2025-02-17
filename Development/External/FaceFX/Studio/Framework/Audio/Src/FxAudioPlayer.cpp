//------------------------------------------------------------------------------
// An abstract audio player.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxAudioPlayer.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxAudioPlayerVersion 0

FX_IMPLEMENT_CLASS(FxAudioPlayer,kCurrentFxAudioPlayerVersion,FxObject);

FxAudioPlayer::FxAudioPlayer()
	: _window(NULL)
	, _source(NULL)
	, _rangeStart(0.0f)
	, _rangeEnd(0.0f)
	, _volume(0.0f)
	, _pitch(1.0f)
	, _loop(FxFalse)
	, _paused(FxFalse)
{
}

FxAudioPlayer::~FxAudioPlayer()
{
}

} // namespace Face

} // namespace OC3Ent
