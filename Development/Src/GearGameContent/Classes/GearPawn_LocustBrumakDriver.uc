/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustBrumakDriver extends GearPawn_LocustHunterNoArmorNoGrenades
	config(Pawn);

function AddDefaultInventory();

function TakeDamage
(
	int					Damage,
	Controller			InstigatedBy,
	Vector				HitLocation,
	Vector				Momentum,
	class<DamageType>	DamageType,
	optional	TraceHitInfo		HitInfo,
	optional	Actor				DamageCauser
)
{
	if( InstigatedBy.Pawn == None || !IsSameTeam( InstigatedBy.Pawn ) )
	{
		Super.TakeDamage( Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
	}
}

defaultproperties
{
	bIgnoreForces=TRUE
	bRespondToExplosions=FALSE
	bNeverAValidEnemy=TRUE

	IdleBreakFreq=(X=0.f,Y=0.f)

	Begin Object Name=GearPawnMesh
		AnimTreeTemplate=AnimTree'Locust_Brumak.BrumakRider_AnimTree'
		AnimSets.Empty
		AnimSets(0)=AnimSet'Locust_Brumak.Rider_animset'
		bHasPhysicsAssetInstance=FALSE
		BlockRigidBody=FALSE
		Translation=(Z=0)
	End Object

	Begin Object Name=CollisionCylinder
		BlockActors=FALSE
		CollideActors=FALSE
		BlockNonZeroExtent=FALSE
		BlockZeroExtent=FALSE
	End Object

	bCollideWorld=FALSE
	bAllowInventoryDrops=FALSE
	bEnableEncroachCheckOnRagdoll=TRUE

	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=392,V=0,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'
	
	PeripheralVision=-0.1
}
