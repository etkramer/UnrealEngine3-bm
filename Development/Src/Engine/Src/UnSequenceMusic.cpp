/*=============================================================================
   UnSequenceMusic.cpp: Gameplay Music Sequence native code
   Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"
#include "EngineAudioDeviceClasses.h"

IMPLEMENT_CLASS(UMusicTrackDataStructures);
IMPLEMENT_CLASS(USeqAct_PlayMusicTrack);

void USeqAct_PlayMusicTrack::Activated()
{
	AWorldInfo *WorldInfo = GWorld->GetWorldInfo();
	if (WorldInfo != NULL)
	{
		WorldInfo->UpdateMusicTrack(MusicTrack);
	}
}



