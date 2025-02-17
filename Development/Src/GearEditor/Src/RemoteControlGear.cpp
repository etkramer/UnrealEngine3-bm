//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

#include "UnrealEd.h"

#include "RemoteControlGear.h"




static UBOOL bXRCsHaveBeenLoaded = FALSE;

void LoadGearRemoteControlXRCs()
{
	if( bXRCsHaveBeenLoaded == FALSE )
	{
		verify( wxXmlResource::Get()->Load( TEXT("wxRC/GearRemoteControl.xrc") ) );


		bXRCsHaveBeenLoaded = TRUE;
	}
}




