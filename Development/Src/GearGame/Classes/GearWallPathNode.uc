/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearWallPathNode extends PathNode
	placeable
	native;

cpptext
{
	virtual void AddForcedSpecs( AScout *Scout );
	virtual void FindBase();
	virtual UBOOL CanConnectTo(ANavigationPoint* Dest, UBOOL bCheckDistance);
	virtual UBOOL GetUpDir( FVector &V )
	{
		V = -(Rotation.Vector());
		return 1;
	}

	UBOOL IsUsableAnchorFor(APawn* P);
}

var() const float MaxJumpDist;

simulated event string GetDebugAbbrev()
{
	return "WPN";
}

defaultproperties
{
	MaxJumpDist=512.f

	bSpecialMove=TRUE
	bBuildLongPaths=FALSE
}
