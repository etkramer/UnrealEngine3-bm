/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class CrowdAttractor extends Actor
	placeable
	native(Anim)
	hidecategories(Advanced)
	hidecategories(Collision)
	hidecategories(Display)
	hidecategories(Actor);

/** How desireable this attractor is. Use negative value and ECAM_Repulsor to push agents away from this actor. */
var()	interp float	Attraction;
/** If attractor is currently enabled */
var()	bool			bAttractorEnabled;
/** If attraction should falloff over the radius */
var()	bool			bAttractionFalloff;
/** Indicates if 'target' actions should be performed when near enough to this actor. */
var()	bool			bActionAtThisAttractor;
/** Used to control how close you need to get before 'target' action are performed. 1.0 indicates they happen over the whole radius of the attactor */
var()	float			ActionRadiusScale;
/** Cylinder component  */
var()	CylinderComponent	CylinderComponent;

var		float			AttractionRadius;
var		float			AttractionHeight;

var		native OctreeElementId	OctreeId{FOctreeElementId};

/** If TRUE, kill crowd members when they reach this attractor. */
var()	bool			bKillWhenReached;
/** If bKillWhenReached is TRUE, how far away from attractor agents have to be before killed. */
var()	float			KillDist;

/** Optional other actor that actions should point at, instead of at the actual attractor location. */
var()	actor			ActionTarget;

/** Controls the mode of this attractor (long-range move target or short-range force field) */
var() enum ECrowdAttractorMode
{
	ECAM_MoveTarget,
	ECAM_Repulsor
} Mode;


replication
{
	if (bNoDelete)
		bAttractorEnabled;
}

cpptext
{
	// AActor interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);
	virtual void PreSave();
	virtual void UpdateComponentsInternal(UBOOL bCollisionUpdate=FALSE);
	virtual void ClearComponents();
}

function OnToggle(SeqAct_Toggle action)
{
	if (action.InputLinks[0].bHasImpulse)
	{
		// turn on
		bAttractorEnabled = TRUE;
	}
	else if (action.InputLinks[1].bHasImpulse)
	{
		// turn off
		bAttractorEnabled = FALSE;
	}
	else if (action.InputLinks[2].bHasImpulse)
	{
		// toggle
		bAttractorEnabled = !bAttractorEnabled;
	}

	// Make this actor net relevant, and force replication, even if it now does not differ from class defaults.
	ForceNetRelevant();
	SetForcedInitialReplicatedProperty(Property'Engine.CrowdAttractor.bAttractorEnabled', (bAttractorEnabled == default.bAttractorEnabled));
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	KillDist=256.0

	Attraction=1.0
	bAttractorEnabled=true
	bAttractionFalloff=true
	ActionRadiusScale=1.0
	Mode=ECAM_MoveTarget

	bCollideActors=FALSE
	bNoDelete=true

	Begin Object Class=CylinderComponent NAME=CollisionCylinder
		CollideActors=FALSE
		bDrawNonColliding=TRUE
		CollisionRadius=+0200.000000
		CollisionHeight=+0040.000000
		CylinderColor=(R=0,G=255,B=0)
		bDrawBoundingBox=FALSE
	End Object
	CylinderComponent=CollisionCylinder
	CollisionComponent=CollisionCylinder
	Components.Add(CollisionCylinder)

	Begin Object Class=SpriteComponent Name=Sprite
		HiddenGame=TRUE
		HiddenEditor=FALSE
		AlwaysLoadOnClient=FALSE
		AlwaysLoadOnServer=FALSE
	End Object
	Components.Add(Sprite)
}