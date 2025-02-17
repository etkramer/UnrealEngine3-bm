//=============================================================================
// ForcedDirVolume
// used to force UTVehicles [of a certain class if wanted] in a certain direction
//
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

class ForcedDirVolume extends PhysicsVolume
	placeable
	native;

var() bool bAllowBackwards; // allows negative forces, thus trapping vehicles on a line instead of in a direction
var() class<UTVehicle> TypeToForce;
var() bool bIgnoreHoverboards;
var() const ArrowComponent Arrow;
var() bool bDenyExit; // if the vehicle is being effected by a force volume, the player cannot exit the vehicle.
var() bool bBlockPawns;
var() bool bBlockSpectators;
var vector ArrowDirection;
var array<UTVehicle> TouchingVehicles;

cpptext
{
	virtual void PostEditChange( UProperty* PropertyThatChanged );
	UBOOL IgnoreBlockingBy( const AActor *Other ) const;
	virtual void TickSpecial(FLOAT DeltaSeconds );
}

simulated function PostBeginPlay()
{
	super.PostBeginPlay();
	
	if ( !bBlockSpectators && (BrushComponent != None) )
	{
		BrushComponent.SetTraceBlocking(false,true);
	}
}

event ActorEnteredVolume(Actor Other)
{
	if ( PlayerController(Other) != None )
	{
		Other.FellOutOfWorld(None);
	}
}

simulated event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	local UTVehicle V;

	Super.Touch(Other, OtherComp, HitLocation, HitNormal);

	V = UTVehicle(Other);
	if ((V != None) && ClassIsChildOf(V.Class, TypeToForce) && V.OnTouchForcedDirVolume(self))
	{
		TouchingVehicles.AddItem(V);
		if (bDenyExit)
		{
			V.bAllowedExit = false;
		}
	}
}

simulated event UnTouch(Actor Other)
{
	local bool bInAnotherVolume;
	local ForcedDirVolume AnotherVolume;

	if (ClassIsChildOf(Other.class, TypeToForce))
	{
		TouchingVehicles.RemoveItem(UTVehicle(Other));
		if (bDenyExit)
		{
			foreach Other.TouchingActors(class'ForcedDirVolume', AnotherVolume)
			{
				if (AnotherVolume.bDenyExit)
				{
					bInAnotherVolume = true;
					break;
				}
			}
			if (!bInAnotherVolume)
			{
				UTVehicle(Other).bAllowedExit = UTVehicle(Other).default.bAllowedExit;
			}
		}
	}
}

simulated function bool StopsProjectile(Projectile P)
{
	return false;
}

defaultproperties
{
	Begin Object Class=ArrowComponent Name=AC
		ArrowColor=(R=150,G=100,B=150)
		ArrowSize=5.0
		AbsoluteRotation=true
	End Object
	Components.Add(AC)
	Arrow=AC

	TypeToForce=class'UTVehicle'

	Begin Object Name=BrushComponent0
		CollideActors=true
		BlockActors=true
		BlockZeroExtent=true
		BlockNonZeroExtent=true
		BlockRigidBody=TRUE
		RBChannel=RBCC_Untitled4
	End Object

	bPushedByEncroachers=FALSE
	bMovable=FALSE
	bWorldGeometry=false
	bCollideActors=true
	bBlockActors=true
	bBlockSpectators=true
	bStatic=false
	bNoDelete=true
}
