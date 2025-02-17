/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Placeable actor that will allow aim friction/adhesion, just like on enemy pawns.
 */
class GearAimAssistActor extends Actor
	placeable
	native;

/** Assuming a sphere shape. */
var() const protected float		Radius;

/** True if this repulsor is currently active, false otherwise. */
var() bool						bEnabled;

/** Affects the intensity of the friction (slower turn rate when crosshair is over the actor).  Set to 0 to disable friction. */
var() const float				FrictionScale;
/** Affects the intensity of the adhesion (if actor moves while crosshair is on it, aim will "pull" towards the actor). */
var() const float				AdhesionScale;

/** Scalar for how large the "bullet magnet" is relative to the Radius.  Set to 0 to disable the bullet magnet. */
var() const float				BulletMagnetScale;

var	private DrawSphereComponent	SphereRenderComponent;

cpptext
{
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);
}

simulated function PostBeginPlay()
{
	if (WorldInfo.GRI != None)
	{
		GearGRI(WorldInfo.GRI).RegisterAimAssistActor(self);
	}

	// seems redundant, but ensures flags are set properly initially
	SetEnabled(bEnabled);

	super.PostBeginPlay();
}

/** Handle kismet Toggle action */
simulated function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if( Action.InputLinks[0].bHasImpulse )
	{
		SetEnabled( TRUE );
	}
	// Turn OFF
	else if( Action.InputLinks[1].bHasImpulse )
	{
		SetEnabled( FALSE );
	}
	// Toggle
	else if( Action.InputLinks[2].bHasImpulse )
	{
		SetEnabled( !bEnabled );
	}
}

simulated function SetEnabled( bool bNewEnabled )
{
	bEnabled = bNewEnabled;

	if (bEnabled)
	{
		bCanBeFrictionedTo = FrictionScale > 0.f;
		bCanBeFrictionedTo= AdhesionScale > 0.f;
	}
	else
	{
		bCanBeFrictionedTo = FALSE;
		bCanBeAdheredTo = FALSE;
	}
}

/**
 * Returns aim-friction zone extents for this actor.
 * Extents are in world units centered around Actor's location, and assumed to be 
 * oriented to face the viewer (like a billboard sprite).
 */
simulated function GetAimFrictionExtent(out float Width, out float Height, out vector Center)
{
	if (FrictionScale > 0.f)
	{
		Width = Radius;
		Height = Radius;
	}
	else
	{
		Width = 0.f;
		Height = 0.f;
	}
	Center = Location;
}

/**
 * Returns aim-adhesion zone extents for this actor.
 * Extents are in world units centered around Actor's location, and assumed to be 
 * oriented to face the viewer (like a billboard sprite).
 */
simulated function GetAimAdhesionExtent(out float Width, out float Height, out vector Center)
{
	if (AdhesionScale > 0.f)
	{
		Width = Radius;
		Height = Radius;
	}
	else
	{
		Width = 0.f;
		Height = 0.f;
	}
	Center = Location;
}

simulated function GetAimAttractorRadii(out float InnerRad, out float OuterRad)
{
	OuterRad = Radius * BulletMagnetScale;
	InnerRad = OuterRad * 0.5f;
}

defaultproperties
{
	bEnabled=TRUE
	Radius=64

	FrictionScale=1.f
	AdhesionScale=1.f
	BulletMagnetScale=1.f
	// set these to true and respect the editable bAllow* fields instead
	bCanBeAdheredTo=TRUE
	bCanBeFrictionedTo=TRUE

	bMovable=TRUE

	Begin Object Class=DrawSphereComponent Name=DrawSphere0
		SphereColor=(R=64,G=70,B=255,A=255)
		SphereRadius=64.f
		//HiddenGame=FALSE
	End Object
	SphereRenderComponent=DrawSphere0
	Components.Add(DrawSphere0)

	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)
}


