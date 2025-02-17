/**
 * Locust Ticker Bomb
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustTickerBase extends GearPawn
	abstract
	native(Pawn);

var() config float TimeToStayFlippedOverMin;
var() config float TimeToStayFlippedOverMax;

var repnotify bool bCharging;
var repnotify bool bHiding;
var repnotify bool bExplode;
var repnotify bool bSpawnedFromKantus;

var bool bAllowFlipOver;


var() config vector AngularFlipImpulse;
var() config float LinearFlipImpulseStrength;
var() config float GravityScaleZ;

var config float ExplosionBaseDamage;
var config float ExplosionDamageFalloff;
var config float ExplosionDamageRadius;

var ParticleSystem KantusSpawnParticleSystem;

var bool bFlipped;

replication
{
	if (Role == ROLE_Authority)
		bCharging,bHiding,bExplode,bSpawnedFromKantus;
}

cpptext
{
	virtual FLOAT GetGravityZ();

	virtual void PostProcessRBPhysics(FLOAT DeltaSeconds, const FVector& OldVelocity){/* Shhhh... it's cool */}

};


native function bool IsValidTargetFor( const Controller C) const;

simulated function PlayKantusSpawnEffects()
{
	local Emitter EffectEmitter;
	bSpawnedFromKantus=true;
	EffectEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( KantusSpawnParticleSystem, Location, rot(16384,0,0));
	EffectEmitter.ParticleSystemComponent.ActivateSystem();
	//DesiredKantusFadeVal=0.4f;
}

function PlayChargeSound()
{
	if(Role == ROLE_Authority)
	{
		bCharging=true;
	}
}

function PlayHideSound()
{
	if(Role == ROLE_Authority)
	{
		bHiding=true;
	}
}

function PlayExplodeAnim()
{
	if(Role == ROLE_Authority)
	{
		bExplode=true;
	}
}

function StopExplodeAnim()
{
	bExplode=false;
	BS_StopAll(0.1f);
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

simulated function GetWeaponHandPosition(out vector HandLoc, out rotator HandRot)
{
	// ticker has no hand so just return the Pawn location
	HandLoc = Location;
	HandRot = Rotation;
}

simulated function bool TookHeadShot(Name BoneName, vector HitLocation, vector Momentum, optional out byte out_bHasHelmet);

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
	local GearPC PC;

	if( InstigatedBy != Controller && 
			(
			ClassIsChildOf(DamageType,class'GDT_TickerExplosion') || 
			(ClassIsChildOf(DamageType,class'GDT_Melee') && Health > 1)
			)
	   )
	{
		// See if we should update the ticker melee achievement
		if (ClassIsChildOf(DamageType,class'GDT_Melee'))
		{
			PC = GearPC(InstigatedBy);
			if (PC != none)
			{
				PC.ClientCheckForMeleeTickerAchievement();
			}
		}
		FlipMeOver(InstigatedBy);
		// make sure we survive the hit
		Damage = Min(Health-1,Damage);
	}

	Super.TakeDamage(Damage,InstigatedBy,HitLocation,Momentum,DamageType,HitInfo,DamageCauser);
}

// stub
function AdjustTickerDamage(out int Damage, GearPawn Damagee);

function FlipMeOver(Controller Inst)
{
	local vector Impulse;
	if(bAllowFlipOver)
	{

		//MessagePlayer(GetFuncName()@self@MyGearAI);
		`AILog_Ext(GetFuncName()@self,,MyGearAI);
		bFlipped=true;
		StopExplodeAnim();
		if(Inst != none)
		{
			Impulse = Normal((Location+vect(0,0,200))-Inst.Pawn.Location);
		}
		else
		{
			Impulse = Vector(Inst.Pawn.Rotation + rot(10,0,0));
		}

		Super.Knockdown(Impulse*LinearFlipImpulseStrength,AngularFlipImpulse>>rotator(Impulse));
		//FlushPersistentDebugLines();
		//DrawDebugLine(Location,Location + (AngularFlipImpulse>>rotator(Impulse)) * 100,255,0,0,TRUE);
	}
}

function UnFlipMeOver()
{
	bFlipped=false;
	GotoState('');
}

// called from anim notify
simulated function JumpStarted()
{
	//MessagePlayer(GetFuncName()@self@MyGearAI);
	bAllowFlipOver = false;
}

state KnockedDown
{
Begin:
	// apply the impulse
	ApplyKnockdownImpulse();
	do
	{
		// delay
		Sleep(0.35f);
		// until we're snapped out of physics, stopped moving, or the physics has gone to sleep
	} until (Physics != PHYS_RigidBody || VSize(Velocity) < 32.f || !Mesh.RigidBodyIsAwake());

	Sleep(RandRange(TimeToStayFlippedOverMin,TimeToStayFlippedOverMax));

	// if we're still alive at this point
	if (Health > 0)
	{
		ServerDoSpecialMove(SM_RecoverFromRagdoll, TRUE);
		UnFlipMeOver();
		GotoState('');
	}
}

function float GetAIAccuracyModWhenTargetingMe(GearAI GuyTargetingMe)
{
	return 0.6f;
}

/** Applies the contents of KnockdownImpulse.  Clients shouldn't call this function directly, only from repnotify. */
simulated protected function ApplyKnockdownImpulse()
{
	`log("!!!"@`showvar(KnockdownImpulse.LinearVelocity)@`showvar(Location)@`showvar(Mesh.GetPosition()));
	// Record when knockdown happened
	KnockDownStartTime = WorldInfo.TimeSeconds;
	// bail out of cover
	if (IsInCover())
	{
		BreakFromCover();
	}
	if (Physics != PHYS_RigidBody || Mesh != CollisionComponent)
	{
		// first send them to ragdoll
		PlayFallDown();
	}
	if (Physics == PHYS_RigidBody)
	{
		// and apply the forces additively
		if (!IsZero(KnockdownImpulse.AngularVelocity))
		{
			Mesh.SetRBAngularVelocity(KnockdownImpulse.AngularVelocity,TRUE);
		}
		Mesh.SetRBLinearVelocity(KnockdownImpulse.LinearVelocity,TRUE);
	}
	else
	{
		`warn(self@"failed to transition to RigidBody for"@GetFuncName());
	}
}

function EndKnockedDownState()
{
	ClearTimer(nameof(EndKnockedDownState));
	ClearTimer(nameof(CheckGetUp));
	ClearTimer(nameof(KnockdownFailsafe));
	ClearTimer(nameof(EnableKnockdownFallChecking));

`if(`notdefined(FINAL_RELEASE))
	messagePlayer(GetFuncName());
	DebugFreezeGame();
`endif

	KnockdownImpulse.LinearVelocity = vect(0,0,0);
	KnockdownImpulse.AngularVelocity = vect(0,0,0);

	// if we're still alive at this point
	if( Health > 0 && Physics == PHYS_RigidBody )
	{
		ServerDoSpecialMove(SM_RecoverFromRagdoll, TRUE);
		GotoState('');
	}
}

defaultproperties
{
	bCanDBNO=false
	bCanClimbLadders=FALSE
	bAllowFlipOver=true
	bForceMaxAccel=true
	SpecialMoveClasses(SM_RecoverFromRagdoll)	=class'GSM_RecoverFromRagdoll_Ticker'
	SpecialMoveClasses(SM_Emerge_Type1)			=class'GSM_Emerge'
	SpecialMoveClasses(SM_Emerge_Type2)			=class'GSM_Emerge'
	SpecialMoveClasses(SM_DeathAnimFire)        =none // this is needed to make it so the pawn doesn't play the fire death anim and then be stuck in ragdoll forever

	bCanPlayPhysicsHitReactions=FALSE

	NoticedGUDSPriority=105
	NoticedGUDSEvent=GUDEvent_NoticedTickers


}
