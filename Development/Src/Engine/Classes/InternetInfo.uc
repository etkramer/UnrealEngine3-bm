//=============================================================================
// InternetInfo: Parent class for Internet connection classes
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class InternetInfo extends Info
	native
	transient;

function string GetBeaconAddress( int i );
function string GetBeaconText( int i );

defaultproperties
{
}
