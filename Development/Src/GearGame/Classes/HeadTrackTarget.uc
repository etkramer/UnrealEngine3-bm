/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class HeadTrackTarget extends Actor
	placeable
	native;

/** Headtracking zone is spherical, this is the sphere radius. */
var() float		Radius;

/** True if this repulsor is currently active, false otherwise. */
var() bool		bEnabled;

/** Optionally look at this actor, instead of the HeadTrackTarget itself. */
var() Actor		AlternateLookatActor;

/** Which bone to look at in the AlternateLookatActor, assuming it has bones */
var() Name		AlternateLookAtActorBoneName;

/** True if we've registered with the GRI, false otherwise */
var private transient bool bRegistered;

enum EHeadTrackPawnFilter
{
	HT_PlayersOnly,
	HT_PlayerTeamOnly,
	HT_AllPawns,
};

/** Specify which pawns this should affect */
var() EHeadTrackPawnFilter PawnFilter;

var	private DrawSphereComponent					SphereRenderComponent;

cpptext
{
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);
}

simulated function PostBeginPlay()
{
	if (WorldInfo.GRI != None)
	{
		GearGRI(WorldInfo.GRI).RegisterHeadTrackTarget(self);
		bRegistered = TRUE;
	}
	super.PostBeginPlay();
}

/** Handle kismet Toggle action */
simulated function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if( Action.InputLinks[0].bHasImpulse )
	{
		bEnabled = TRUE;
	}
	// Turn OFF
	else if( Action.InputLinks[1].bHasImpulse )
	{
		bEnabled = FALSE;
	}
	// Toggle
	else if( Action.InputLinks[2].bHasImpulse )
	{
		bEnabled = !bEnabled;
	}
}

simulated function SetRadius(float NewRadius)
{
	Radius = NewRadius;
	SphereRenderComponent.SphereRadius = NewRadius;
}

/** Returns true if TestLoc is inside the repulsor, false otherwise. */
simulated function bool Encompasses(vector TestLoc)
{
	local float DistSq;
	DistSq = VSizeSq(TestLoc - Location);
	return (DistSq < (Radius*Radius));
}

simulated function Tick(float DeltaTime)
{
	if (!bRegistered)
	{
		if (WorldInfo.GRI != None)
		{
			GearGRI(WorldInfo.GRI).RegisterHeadTrackTarget(self);
			bRegistered = TRUE;
		}
	}
	super.Tick(DeltaTime);
}

/** Called via timer. */
simulated protected final function DisableSelf()
{
	bEnabled = FALSE;
}

simulated protected native final function bool CanAffectPawn(GearPawn WP) const;

defaultproperties
{
	bEnabled=TRUE
	Radius=128

	RemoteRole=ROLE_SimulatedProxy

	bMovable=TRUE
	bNoDelete=TRUE

	Begin Object Class=DrawSphereComponent Name=DrawSphere0
		SphereColor=(R=200,G=70,B=255,A=255)
		SphereRadius=128.f
	//	HiddenGame=FALSE
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


