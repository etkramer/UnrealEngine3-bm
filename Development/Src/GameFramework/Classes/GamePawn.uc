/**
 * GamePawn
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GamePawn extends Pawn
	config(Game)
	native
	abstract
	notplaceable;




/**
 * Returns cylinder information used for target friction.
 * @todo call the native version of this directly
 **/
simulated function GetTargetFrictionCylinder( out float CylinderRadius, out float CylinderHeight )
{
	GetBoundingCylinder( CylinderRadius, CylinderHeight );	
}


defaultproperties
{
	bCanBeAdheredTo=TRUE
	bCanBeFrictionedTo=TRUE
}
