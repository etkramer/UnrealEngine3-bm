//=============================================================================
// Simplified version of ambient sound used to enhance workflow.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class AmbientSoundMovable extends AmbientSound
	native( Sound );

defaultproperties
{
	Begin Object Name=DrawSoundRadius0
		SphereColor=(R=50,G=240,B=50)
	End Object

	TickGroup=TG_DuringAsyncWork
	Physics=PHYS_Interpolating
	bMovable=TRUE
	bStatic=FALSE
}
