/**
 * Copyright � 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class NxForceFieldRadial extends NxForceField
	native(ForceField)
	placeable;

var() editinline ForceFieldShape Shape;

var native ActorComponent DrawComponent;

/** Strength of the force applied by this actor. Positive forces are applied outwards. */
var()	interp float	ForceStrength;

/** Radius of influence of the force. */
var()	interp float	ForceRadius;

/** */
var()	interp float	SelfRotationStrength;

/** Way in which the force falls off as objects are further away from the location. */
var()	PrimitiveComponent.ERadialImpulseFalloff	ForceFalloff;

/** custom force field kernel */
var const native transient pointer		Kernel{class NxForceFieldKernelRadial};

cpptext
{
	virtual void InitRBPhys();
	virtual void TermRBPhys(FRBPhysScene* Scene);
	virtual void DefineForceFunction(FPointer ForceFieldDesc);
	virtual FPointer DefineForceFieldShapeDesc();

	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);
	virtual void PostLoad();
}


defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_RadForce'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	ForceRadius=200.0
}
