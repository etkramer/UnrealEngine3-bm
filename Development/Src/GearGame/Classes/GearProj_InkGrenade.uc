/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_InkGrenade extends GearProj_SmokeGrenade;

var config float AttachDamage;
var config float AttachDamageRate;

var GearPawn TaggedPawn;


simulated function TriggerExplosion(vector HitLocation, vector HitNormal, Actor HitActor)
{
	local PlayerController PC;

	if (!bIsSimulating && !bGrenadeHasExploded)
	{
		Super.TriggerExplosion(HitLocation, HitNormal, HitActor);

		foreach WorldInfo.LocalPlayerControllers( class'PlayerController', PC )
		{
			// splatter some inksplat on their camera if they are a human and decently close
			if( ( PC.Pawn != none && VSize(PC.Pawn.Location - Location) < 800.0f )
				&& PC.IsAimingAt( self, 0.1f ) // if we are semi looking in the direction of the explosion
				)
			{
				PC.ClientSpawnCameraLensEffect( class'Emit_Camera_InkSplat' );
			}
		}
	}
}

// avoidance cylinder lives on the emitter
function SetupAvoidanceCylinder();



simulated function name AttachInit(Actor Attachee)
{
	local name BoneName;

	BoneName = super.AttachInit(Attachee);

	if (Role == ROLE_Authority)
	{
		TaggedPawn = GearPawn(Attachee);
		if (TaggedPawn != None)
		{
			// set the dot timer
			SetTimer( AttachDamageRate,TRUE,nameof(DoInkAttachDamage) );
			SetTimer( 10.f,FALSE,nameof(StopInkAttachDamage) );
		}
	}

	return BoneName;
}

function DoInkAttachDamage()
{
	if (TaggedPawn != None && TaggedPawn.Health > 0)
	{
		TaggedPawn.TakeDamage(AttachDamage,InstigatorController,TaggedPawn.Location,vect(0,0,1),class'GDT_InkTag');
	}
	else
	{
		StopInkAttachDamage();
	}
}

function StopInkAttachDamage()
{
	ClearTimer('DoInkAttachDamage');
	ClearTimer('StopInkAttachDamage');
}

static function float GetAIEffectiveRadius()
{
	return class'Emit_InkGrenade'.default.InkDamageRadius;
}

defaultproperties
{

	InFlightSoundTemplate=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeWhipInAirLoopCue'
	GrenadeAttachingToFoeSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeStickEnemyCue'
	GrenadeAttachingToWorldSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeStickSurfaceCue'
	BeepBeepAboutToExplodeSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeWarningCue'
	GrenadeBounceSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeBounceCue'

	Begin Object Name=COGGrenadeMesh0
	    SkeletalMesh=SkeletalMesh'Locust_PoisonGrenade.Locust_Poison_Grenade'
		PhysicsAsset=PhysicsAsset'Locust_PoisonGrenade.Poison_Grenade_Physics'
	bAcceptsDynamicDecals=FALSE
	End Object

	TrailTemplate=ParticleSystem'Weap_Ink_Grenade.FX.P_Weap_Ink_Grenade_ThrowTrail'

	Begin Object Name=ExploTemplate0
	    MyDamageType=class'GDT_InkGrenade'
		ParticleEmitterTemplate=ParticleSystem'Weap_Ink_Grenade.Effects.P_Weap_Ink_Grenade_Explo01'
		ExploAnimShake=(Anim=CameraAnim'Effects_Camera.Explosions.CA_Ink_Grenade_Explo')
		ExplosionSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeExplosionCue'
		bUseMapSpecificValues=TRUE
	End Object
	ExplosionTemplate=ExploTemplate0

	GrenadeTagPreExplosionSmokeEffect=ParticleSystem'Weap_Ink_Grenade.Effects.P_Weap_Ink_Grenade_Stick_On'
	GrenadeTagPreExplosionSmokeEffectPlanted=None

	SmokeEmitterClass=class'Emit_InkGrenade'

	MartyrDamageType=class'GDT_InkMartyr'
}


