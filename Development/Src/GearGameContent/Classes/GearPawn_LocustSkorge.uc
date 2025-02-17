
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustSkorge extends GearPawn_LocustSkorgeBase
	config(Pawn);

var ParticleSystem	PS_TakeOffDust;
var ParticleSystem	PS_LandingDust;
var ParticleSystem	PS_BlockBullet;
var Emitter			Emitter_BlockBullet;

var SkeletalMeshComponent StaffComponent;
var SkeletalMeshComponent RightStickComponent, LeftStickComponent;

/** Dueling effects */
var ParticleSystemComponent			ChainSawDuelClashEffect;
var PointLightComponent				ChainSawDuelClashLightComponent;
/** Plays while the dual is going**/
var AudioComponent                  AC_LoopingDuelSound, AC_LoopingChainsawSound, AC_LoopingChainsawSlashSound;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if( StaffComponent != None )
	{
		Mesh.AttachComponentToSocket( StaffComponent, 'Staff' );
		StaffComponent.SetShadowParent( Mesh );
		StaffComponent.SetLightEnvironment( LightEnvironment );
		StaffComponent.SetScale( DrawScale );
	}
	if( RightStickComponent != None )
	{
		Mesh.AttachComponentToSocket( RightStickComponent, 'Stick_R' );
		RightStickComponent.SetShadowParent( Mesh );
		RightStickComponent.SetLightEnvironment( LightEnvironment );
		RightStickComponent.SetScale( DrawScale );
	}
	if( LeftStickComponent != None )
	{
		Mesh.AttachComponentToSocket( LeftStickComponent, 'Stick_L' );
		LeftStickComponent.SetShadowParent( Mesh );
		LeftStickComponent.SetLightEnvironment( LightEnvironment );
		LeftStickComponent.SetScale( DrawScale );
	}

	HideGun( TRUE );
}

simulated function UpdateStage()
{
	if( Stage == SKORGE_Staff )
	{
		if( RightStickComponent != None ) { RightStickComponent.SetHidden( TRUE ); }
		if( LeftStickComponent  != None ) { LeftStickComponent.SetHidden( TRUE );  }
		if( StaffComponent		!= None ) { StaffComponent.SetHidden( FALSE );	   }
		ChargeAnimNode.SetAnim( 'Staff_Charge_Fwd_Block' );
	}
	else if( Stage == SKORGE_TwoStick )
	{
		if( RightStickComponent != None ) { RightStickComponent.SetHidden( FALSE ); }
		if( LeftStickComponent  != None ) { LeftStickComponent.SetHidden( FALSE );  }
		if( StaffComponent		!= None ) { StaffComponent.SetHidden( TRUE );		}
		ChargeAnimNode.SetAnim( '2stick_Charge_Fwd_Block' );
	}
	else /* SKORGE_OneStick */
	{
		if( RightStickComponent != None ) { RightStickComponent.SetHidden( FALSE ); }
		if( LeftStickComponent  != None ) { LeftStickComponent.SetHidden( TRUE );	}
		if( StaffComponent		!= None ) { StaffComponent.SetHidden( TRUE );		}
		ChargeAnimNode.SetAnim( '1stick_Charge_Fwd_Block' );
	}
}

simulated function HideGun( bool bHide )
{
	bHideGun = bHide;
	if( bHideGun )
	{
		if( Weapon != None && Weapon.Mesh != None )
		{
			Weapon.Mesh.SetHidden( TRUE );
		}
		UpdateStage();
	}
	else
	{
		if( Weapon != None && Weapon.Mesh != None )
		{
			Weapon.Mesh.SetHidden( FALSE );
		}
		if( RightStickComponent != None ) { RightStickComponent.SetHidden( TRUE ); }
		if( LeftStickComponent  != None ) { LeftStickComponent.SetHidden( TRUE );  }
		if( StaffComponent		!= None ) { StaffComponent.SetHidden( TRUE );	   }
	}
}

simulated function AttachWeaponToHand(GearWeapon Weap)
{
	// attach to right hand socket
	`LogInv("attach" @ Weap @ "to" @ GetLeftHandSocketName());

	Weap.AttachWeaponTo(Mesh, GetLeftHandSocketName());
}

function AdjustPawnDamage
(
	out	int					Damage,
	Pawn				InstigatedBy,
	Vector				HitLocation,
	out Vector				Momentum,
	class<GearDamageType>	GearDamageType,
	optional	out	TraceHitInfo		HitInfo
)
{
	if( bBlockingBullets )
	{
		PlayBlockedBulletEffect( InstigatedBy );
	}

	CompleteStrafeAttackTime -= 0.1f;
	Damage = 0;
}

simulated function PlayTakeOffEffects()
{
	local Actor			HitActor;
	local Vector		HitLocation, HitNormal, TraceEnd, TraceStart;
	local TraceHitInfo	HitInfo;
	local Emitter		Dust;

	TraceStart	= Location;
	TraceEnd	= TraceStart + vect(0,0,-1000);

	// trace down and see what we are standing on
	HitActor = Trace( HitLocation, HitNormal, TraceEnd, TraceStart, FALSE,, HitInfo );
	if( HitActor != None )
	{
		Dust = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_TakeOffDust, HitLocation, Rotation );
		Dust.ParticleSystemComponent.ActivateSystem();
	}
}

simulated function PlayLandingEffects()
{
	local Actor			HitActor;
	local Vector		HitLocation, HitNormal, TraceEnd, TraceStart;
	local TraceHitInfo	HitInfo;
	local Emitter		Dust;

	TraceStart	= Location;
	TraceEnd	= TraceStart + vect(0,0,-1000);

	// trace down and see what we are standing on
	HitActor = Trace( HitLocation, HitNormal, TraceEnd, TraceStart, FALSE,, HitInfo );
	if( HitActor != None )
	{
		Dust = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_LandingDust, HitLocation, Rotation );
		Dust.ParticleSystemComponent.ActivateSystem();
	}
}

simulated function PlayBlockedBulletEffect( Pawn Shooter )
{
	local Vector SpawnLoc;

	if( Emitter_BlockBullet == None )
	{
		SpawnLoc = Location + Normal(Shooter.Location-Location) * 64.f;
		Emitter_BlockBullet = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_BlockBullet, SpawnLoc, Rotation );
		Emitter_BlockBullet.SetBase( self );
	}
	if( Emitter_BlockBullet != None )
	{
		Emitter_BlockBullet.ParticleSystemComponent.ActivateSystem();
		SetTimer( 0.5f, FALSE, nameof(DisableBlockedBulletEffect) );
	}
}

simulated function DisableBlockedBulletEffect()
{
	if( Emitter_BlockBullet != None )
	{
		Emitter_BlockBullet.ParticleSystemComponent.DeactivateSystem();
	}
}

/** Heats up the skin by the given amount.  Skin heat constantly diminishes in Tick(). */
simulated function HeatSkin( float HeatIncrement )
{
	// skorge can't be affected by heat!
}

simulated function CharSkin( float CharIncrement )
{
	// skorge can't be affected by heat!
}

/** Do not force bAnimRotationOnly on those guys. */
simulated function AnimSetsListUpdated();

simulated function StartChainsawAnimation()
{
	AnimNodeSequence(RightStickComponent.Animations).bPlaying = TRUE;
	AnimNodeSequence(LeftStickComponent.Animations).bPlaying = TRUE;
	AnimNodeSequence(StaffComponent.Animations).bPlaying = TRUE;
}

simulated function StopChainsawAnimation()
{
	AnimNodeSequence(RightStickComponent.Animations).bPlaying = FALSE;
	AnimNodeSequence(LeftStickComponent.Animations).bPlaying = FALSE;
	AnimNodeSequence(StaffComponent.Animations).bPlaying = FALSE;
}

simulated function CheckChainsawStopped()
{
	if( AC_LoopingChainsawSound == None &&
		AC_LoopingChainsawSlashSound == None &&
		AC_LoopingDuelSound == None )
	{
		StopChainsawAnimation();
	}
}

/** Be safe, force chainsaw sounds to stop after duel is finished */
simulated function ForceStopChainsaw()
{
	if( AC_LoopingChainsawSound != None )
	{
		 EndChainsawSound();
	}
	if( AC_LoopingChainsawSlashSound != None )
	{
		EndChainsawSlashSound();
	}
	if( AC_LoopingDuelSound != None )
	{
		EndClashEffect();
	}
}

simulated function StartChainsawSound()
{
	if( AC_LoopingChainsawSound == None )
	{
		AC_LoopingChainsawSound = CreateAudioComponent( SoundCue'Weapon_AssaultRifle.Melee.ChainsawLoopIdle01Cue', FALSE, TRUE, TRUE );
	}
	if( AC_LoopingChainsawSound != None )
	{
		AC_LoopingChainsawSound.FadeIn( 0.1f, 1.0f );
	}
	PlaySound(SoundCue'Weapon_AssaultRifle.Melee.ChainsawStart01Cue', FALSE);
	StartChainsawAnimation();
}

simulated function EndChainsawSound()
{
	if( AC_LoopingChainsawSound != None )
	{
		AC_LoopingChainsawSound.FadeOut( 0.1f, 0.0f );
		AC_LoopingChainsawSound = None;
	}
	PlaySound(SoundCue'Weapon_AssaultRifle.Melee.ChainsawStop01Cue', FALSE);
	CheckChainsawStopped();
}


simulated function StartChainsawSlashSound()
{
	if( AC_LoopingChainsawSlashSound == None )
	{
		AC_LoopingChainsawSlashSound = CreateAudioComponent( SoundCue'Weapon_AssaultRifle.Melee.ChainsawSlashLoop01Cue', FALSE, TRUE, TRUE );
	}
	if( AC_LoopingChainsawSlashSound != None )
	{
		AC_LoopingChainsawSlashSound.FadeIn( 0.1f, 1.0f );
	}
	PlaySound(SoundCue'Weapon_AssaultRifle.Melee.ChainsawSlashStart01Cue', FALSE);
	StartChainsawAnimation();
}

simulated function EndChainsawSlashSound()
{
	if( AC_LoopingChainsawSlashSound != None )
	{
		AC_LoopingChainsawSlashSound.FadeOut( 0.1f, 0.0f );
		AC_LoopingChainsawSlashSound = None;
	}
	PlaySound(SoundCue'Weapon_AssaultRifle.Melee.ChainsawSlashStop01Cue', FALSE);
	CheckChainsawStopped();
}

simulated function StartClashEffect()
{
	if( ChainSawDuelClashEffect != None )
	{
		Mesh.AttachComponentToSocket(ChainSawDuelClashEffect, 'DuelEffects');
		ChainSawDuelClashEffect.SetHidden( FALSE );
		ChainSawDuelClashEffect.ActivateSystem();
	}
	if( ChainSawDuelClashLightComponent != None )
	{
		Mesh.AttachComponentToSocket(ChainSawDuelClashLightComponent, 'DuelEffects');
		ChainSawDuelClashLightComponent.SetEnabled(TRUE);
	}
	if( AC_LoopingDuelSound == None )
	{
		AC_LoopingDuelSound = CreateAudioComponent( SoundCue'Weapon_AssaultRifle.Dueling.ChainsawDuelingLoop01Cue', FALSE, TRUE, TRUE );
	}
	if( AC_LoopingDuelSound != None )
	{
		AC_LoopingDuelSound.FadeIn( 0.1f, 1.0f );
	}
	PlaySound(SoundCue'Weapon_AssaultRifle.Dueling.ChainsawDuelingStart01Cue', FALSE);
	StartChainsawAnimation();
}

simulated function EndClashEffect()
{
	if( ChainSawDuelClashEffect != None )
	{
		ChainSawDuelClashEffect.DeactivateSystem();
		if( Mesh.IsComponentAttached(ChainSawDuelClashEffect) )
		{
			Mesh.DetachComponent(ChainSawDuelClashEffect);
		}
	}
	if( ChainSawDuelClashLightComponent != None )
	{
		ChainSawDuelClashLightComponent.SetEnabled(FALSE);
		if( Mesh.IsComponentAttached(ChainSawDuelClashLightComponent) )
		{
			Mesh.DetachComponent(ChainSawDuelClashLightComponent);
		}
	}
	if( AC_LoopingDuelSound != None )
	{
		AC_LoopingDuelSound.FadeOut( 0.1f, 0.0f );
		AC_LoopingDuelSound = None;
	}
	PlaySound(SoundCue'Weapon_AssaultRifle.Dueling.ChainsawDuelingStop01Cue', FALSE);
	CheckChainsawStopped();
}

defaultproperties
{
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_LocustBurstPistol_Skorge'
	DefaultInventory(1)=class'GearGameContent.GearWeap_InkGrenadeSkorge'

	bAllowInventoryDrops=FALSE
	bRespondToExplosions=FALSE
	bCanDBNO=FALSE

	Begin Object Name=GearPawnMesh
		bEnableFullAnimWeightBodies=TRUE
		SkeletalMesh=SkeletalMesh'Locust_Skorge.Mesh.Locust_Skorge'
		PhysicsAsset=PhysicsAsset'Locust_Skorge.Mesh.Locust_Skorge_Physics'
		AnimSets.Empty
		AnimSets(0)=AnimSet'Locust_Skorge.Anims.Locust_Skorge'
		AnimTreeTemplate=AnimTree'Locust_Skorge.SkorgeAnimTree'
		Translation=(Z=-110)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0060.000000
		CollisionHeight=+0110.000000
	End Object

	Begin Object Class=AnimNodeSequence Name=AnimNodeSeqStaff0
		AnimSeqName="saw_loop"
		bLooping=TRUE
	End Object
	Begin Object Class=SkeletalMeshComponent Name=StaffComp0
		SkeletalMesh=SkeletalMesh'Locust_Skorge_SawStaff.Meshes.Locust_Skorge_Staff'
		Animations=AnimNodeSeqStaff0
		AnimSets.Add( AnimSet'Locust_Skorge_SawStaff.Anims.Staff_Double' )
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
	End Object
	StaffComponent=StaffComp0
	Components.Add(StaffComp0)

	Begin Object Class=AnimNodeSequence Name=AnimNodeSeqStick0
		AnimSeqName="saw_loop"
		bLooping=TRUE
	End Object
	Begin Object Class=SkeletalMeshComponent Name=StickComp0
		SkeletalMesh=SkeletalMesh'Locust_Skorge_SawStaff.Meshes.Locust_Skorge_Staff_Single_R'
		Animations=AnimNodeSeqStick0
		AnimSets.Add( AnimSet'Locust_Skorge_SawStaff.Anims.Staff_Single_R' )
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
	End Object
	RightStickComponent=StickComp0
	Components.Add(StickComp0)

		Begin Object Class=AnimNodeSequence Name=AnimNodeSeqStick1
		AnimSeqName="saw_loop"
		bLooping=TRUE
	End Object
	Begin Object Class=SkeletalMeshComponent Name=StickComp1
		SkeletalMesh=SkeletalMesh'Locust_Skorge_SawStaff.Meshes.Locust_Skorge_Staff_Single_R'
		Animations=AnimNodeSeqStick1
		AnimSets.Add( AnimSet'Locust_Skorge_SawStaff.Anims.Staff_Single_R' )
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
	End Object
	LeftStickComponent=StickComp1
	Components.Add(StickComp1)

	PS_TakeOffDust=ParticleSystem'Locust_Skorge.Effects.P_Skorge_Jump_Takeoff_Dust_01'
	PS_LandingDust=ParticleSystem'Locust_Skorge.Effects.P_Skorge_Jump_Landing_Dust_01'
	PS_BlockBullet=ParticleSystem'Locust_Skorge.Effects.P_Skorge_Chainsaw_BulletBlock_01'


	Begin Object Class=AudioComponent Name=AudioComponent0
		SoundCue=SoundCue'Locust_Skorge_Efforts.Staff.Skorge_BladeTwirlLoopB_Slow01Cue'
		bStopWhenOwnerDestroyed=TRUE
	End Object
	AC_StaffTwirl=AudioComponent0
	Components.Add(AudioComponent0)

	SpecialMoveClasses(GSM_LeapToCeiling)		=class'GSM_Skorge_LeapToCeiling'
	SpecialMoveClasses(GSM_DropFromCeiling)		=class'GSM_Skorge_DropFromCeiling'
	SpecialMoveClasses(SM_ChainsawDuel_Leader)	=class'GSM_Skorge_Duel_Leader'
	SpecialMoveClasses(SM_ChainSawAttack)		=class'GSM_Skorge_Duel_Outcome'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_Skorge'

	Begin Object Class=ParticleSystemComponent Name=ChainSawDuelClashEffect0
		bAutoActivate=FALSE
		Template=ParticleSystem'Effects_Gameplay.Chainsaw.P_Chainsaw_Duel_Clash_Looping'
		TickGroup=TG_PostUpdateWork
	End Object
	ChainSawDuelClashEffect=ChainSawDuelClashEffect0

	Begin Object Class=LightFunction Name=ChainSawDuelClashLightFunction0
		SourceMaterial=Material'Effects_Gameplay.Materials.M_Chainsaw_Duel_Lightfunction'
	End Object

	Begin Object Class=PointLightComponent Name=ChainSawDuelClashLightComponent0
		bEnabled=FALSE
		LightColor=(R=252,G=207,B=148,A=255)
		Brightness=6
		Radius=256
		Function=ChainSawDuelClashLightFunction0
	End Object
	ChainSawDuelClashLightComponent=ChainSawDuelClashLightComponent0
}
