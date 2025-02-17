
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * base class, accessible from main package, but not spawnable.
 */
class GearPawn_LocustNemacystBase extends GearPawn
	abstract
	config(Pawn)
	native(Pawn);

var repnotify BYTE	PlaySwoopAnimCount;
var repnotify bool	bStartAttack;

var BodyStance SwoopAnim;
var BodyStance StartAttackAnim;
var BodyStance LoopAttackAnim;

/** charge parameters */
// top speed of charge
var() config float ChargeMaxSpeed;
// amount of time it should take to get to full speed
var() config float ChargeSpeedRampTime;

var() name PawnViewSocketName;

cpptext
{
	virtual void physicsRotation(FLOAT deltaTime, FVector OldVelocity);
}

replication
{
	// Replicated to ALL
	if( Role == Role_Authority )
		PlaySwoopAnimCount, bStartAttack;
}

simulated native event vector GetPawnViewLocation();

/** replicated event */
simulated event ReplicatedEvent( name VarName )
{
	switch( VarName )
	{
		case 'PlaySwoopAnimCount':
			if( PlaySwoopAnimCount > 0 )
			{
				PlaySwoopAnimation();
			}
			else
			{
				StopSwoopAnimation();
			}
			break;
		case 'bStartAttack':
			PlayAttackAnim( FALSE );
			break;
	}

	Super.ReplicatedEvent(VarName);
}

function HomingInOnEnemy(Pawn Enemy);//stub

simulated function float PlaySwoopAnimation()
{
	local float Time;

	// On server, trigger replication for clients
	if( WorldInfo.NetMode == NM_ListenServer || WorldInfo.NetMode == NM_DedicatedServer )
	{
		PlaySwoopAnimCount	= Max(PlaySwoopAnimCount + 1, 1);	// Make sure we don't wrap over to 0
		bForceNetUpdate = TRUE;
	}

	Mesh.bForceDiscardRootMotion = FALSE;
	Mesh.RootMotionMode = RMM_Accel;

	Time = BS_Play(SwoopAnim, 0.8f, 0.4f, -1.f, FALSE, TRUE) - 0.2f;
	BS_SetRootBoneAxisOptions(SwoopAnim, RBA_Translate, RBA_Translate, RBA_Translate);
	return Time;
}


simulated function StopSwoopAnimation()
{
	// On server, trigger replication for clients
	if( WorldInfo.NetMode == NM_ListenServer || WorldInfo.NetMode == NM_DedicatedServer )
	{
		PlaySwoopAnimCount	= 0;
		bForceNetUpdate = TRUE;
	}

	Mesh.RootMotionMode				= RMM_Ignore;
	Mesh.bForceDiscardRootMotion	= TRUE;
	BS_Stop(SwoopAnim, 0.2f);
	BS_SetRootBoneAxisOptions(SwoopAnim, RBA_Discard, RBA_Discard, RBA_Discard);
}

function SetAttackAnim( bool binAttack )
{
	if( bStartAttack != binAttack )
	{
		bStartAttack = binAttack;
		PlayAttackAnim(	FALSE );
	}
}

simulated function PlayAttackAnim( bool bAttackLoop )
{
	local float Duration;

	if( bStartAttack )
	{
		if( !bAttackLoop )
		{
			Duration = BS_Play( StartAttackAnim, 1.f, 0.2f, -1.f, FALSE, TRUE );
			SetTimer( Duration, FALSE, 'PlayAttackLoopAnim' );
		}
		else
		{
			BS_Play( LoopAttackAnim, 1.f, 0.1f, -1.f, TRUE, TRUE );
		}
	}
	else
	{
		BS_Stop( StartAttackAnim, 0.3f );
		BS_Stop( LoopAttackAnim, 0.3f );
	}
}

simulated function PlayAttackLoopAnim()
{
	PlayAttackAnim( TRUE );
}

simulated function bool ShouldTorqueBowArrowGoThroughMe(GearProjectile Proj, TraceHitInfo HitInfo, vector HitLocation, vector Momentum, optional out byte out_bHasHelmet)
{
	// kill it and keep flying
	TakeDamage(99999.f, Proj.InstigatorController, HitLocation, vect(0,0,0), class'GDT_TorqueBow_Impact', HitInfo);
	return true;
}


defaultproperties
{
	SwoopAnim=(AnimName[BS_FullBody]="Swoop_A")
	StartAttackAnim=(AnimName[BS_FullBody]="Attack_Start")
	LoopAttackAnim=(AnimName[BS_FullBody]="Attack_Start")
	bCanDBNO=false
	bTranslateMeshByCollisionHeight=FALSE

	PelvisBoneName=none
}
