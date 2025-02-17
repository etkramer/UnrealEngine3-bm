/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAnimBlendByPosture extends UTAnimBlendBase
	native(Animation);                                         

cpptext
{
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight  );
}


defaultproperties
{
	Children(0)=(Name="Run",Weight=1.0)
	Children(1)=(Name="Crouch")
	bFixNumChildren=true
}
