/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
 
class SpeedTreeActor extends Actor
	native(SpeedTree)
	placeable;
	
var() const editconst SpeedTreeComponent SpeedTreeComponent;

cpptext
{
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();
}

defaultproperties
{	
	Begin Object Class=SpeedTreeComponent Name=SpeedTreeComponent0
		bAllowApproximateOcclusion=TRUE
		bCastDynamicShadow=FALSE
		bForceDirectLightMap=TRUE
	End Object
	SpeedTreeComponent = SpeedTreeComponent0;
	CollisionComponent = SpeedTreeComponent0;
	Components.Add(SpeedTreeComponent0);
	
	bEdShouldSnap	= FALSE

	bStatic			= TRUE 
	bMovable		= FALSE
	bNoDelete		= TRUE

	bCollideActors	= TRUE
	bBlockActors	= TRUE
	bWorldGeometry	= TRUE
}
