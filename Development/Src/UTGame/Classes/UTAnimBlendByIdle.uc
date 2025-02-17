/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAnimBlendByIdle extends UTAnimBlendBase
	native(Animation);

cpptext
{
	// AnimNode interface
	virtual	void TickAnim( float DeltaSeconds, FLOAT TotalWeight  );
}

defaultproperties
{
	Children(0)=(Name="Idle",Weight=1.0)
	Children(1)=(Name="Moving")
	bFixNumChildren=true
}
