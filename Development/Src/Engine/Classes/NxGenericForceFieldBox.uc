/**
 * Copyright � 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class NxGenericForceFieldBox extends NxGenericForceField
	native(ForceField)
	placeable;


/** Used to preview the radius of the force. */
var	DrawBoxComponent			RenderComponent;

/** Radius of influence of the force. */
var()	interp vector	BoxExtent;

cpptext
{
	virtual void InitRBPhys();
	virtual void TermRBPhys(FRBPhysScene* Scene);

	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void TickSpecial(FLOAT DeltaSeconds);
	virtual void EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);

	virtual FPointer DefineForceFieldShapeDesc();
}


defaultproperties
{
	Begin Object Class=DrawBoxComponent Name=DrawBox0
		BoxColor=(R=64,G=70,B=255,A=255)
		BoxExtent=(X=200.0, Y=200.0, Z=200.0);
	End Object
	RenderComponent=DrawBox0
	Components.Add(DrawBox0)

	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_RadForce'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	TickGroup=TG_PreAsyncWork

	BoxExtent=(X=200.0, Y=200.0, Z=200.0)

	RemoteRole=ROLE_SimulatedProxy
	bNoDelete=true
	bAlwaysRelevant=true
	bOnlyDirtyReplication=true
}
