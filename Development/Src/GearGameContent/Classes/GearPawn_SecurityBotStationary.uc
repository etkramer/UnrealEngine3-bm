/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearPawn_SecurityBotStationary extends GearPawn_SecurityBotStationaryBase
	placeable;

/** GoW global macros */

var instanced ParticleSystemComponent FrickinLaserBeam;
var ParticleSystem LaserTemplate;



var		MaterialInstanceConstant		EmissiveMIC;
var		MaterialInstanceConstant		BulletsMIC;

var		LinearColor						PissedColor;
var		LinearColor						HappyColor;

var		BodyStance						BS_Fire;

var		SoundCue	LostTargetSound;
var		SoundCue	AmbientSound;
var		SoundCue	TargetAcquiredSound;
var		SoundCue	FireStopSound;
var		SoundCue	BarrelSpinDown;
var		SoundCue	TurretRotationLoop;
var		SoundCue	PissedLoop;
var		SoundCue	EnabledAudioSound;
var		SoundCue	ExploSound;
var		SoundCue	DeploymentBlockedSound;

var		SoundCue	ShutDownSound1,ShutDownSound2,ShutDownSound3;
var		SoundCue	StartUpSound1, StartUpSound2,StartUpSound3,StartUpSound4;

Var AudioComponent TurretRotationAudioComp;
var AudioComponent PissedLoopAudioComp;
var AudioComponent EnabledAudioComp;

var float BaseLaserDist;

var particleSystem ExploParticleEmitterTemplate;

/** whether or not we're allowed to play sounds from anims */
var bool bPlayAnimSounds;

simulated function PostbeginPlay()
{
	Super.PostBeginPlay();
	Mesh.AttachComponentToSocket(FrickinLaserBeam,RightHandSocketName);

	EmissiveMIC = Mesh.CreateAndSetMaterialInstanceConstant(0);
	BulletsMIC =  Mesh.CreateAndSetMaterialInstanceConstant(1);

	BS_Fire.AnimName[BS_FullBody] = 'Firing';

	GetHappy();
}

simulated function ANIMNOTIFY_PlayShutDownSound1()
{
	PlayAnimNotifySound(ShutDownSound1);
}
simulated function ANIMNOTIFY_PlayShutDownSound2()
{
	PlayAnimNotifySound(ShutDownSound2);
}
simulated function ANIMNOTIFY_PlayShutDownSound3()
{
	PlayAnimNotifySound(ShutDownSound3);
}
simulated function ANIMNOTIFY_PlayStartUpSound1()
{
	PlayAnimNotifySound(StartUpSound1);
}
simulated function ANIMNOTIFY_PlayStartUpSound2()
{
	PlayAnimNotifySound(StartUpSound2);
}
simulated function ANIMNOTIFY_PlayStartUpSound3()
{
	PlayAnimNotifySound(StartUpSound3);
}
simulated function ANIMNOTIFY_PlayStartUpSound4()
{
	PlayAnimNotifySound(StartUpSound4);
}

simulated function PlayAnimNotifySound(SoundCue Sound)
{
	if(bPlayAnimSounds)
	{
		PlaySound(Sound,TRUE);
	}
}

simulated singular event Rotator GetBaseAimRotation()
{
	return Rotator(-Mesh.GetBoneAxis(BarrelRotationBoneName,AXIS_X));
}

function TargetAcquired(Actor Target) 
{
	PlaySound(TargetAcquiredSound);
}

simulated function StartedRotating()
{
	Super.StartedRotating();

	if(TurretRotationAudioComp == none && TurretRotationLoop != none && WorldInfo.NetMode != NM_DedicatedServer)
	{
		TurretRotationAudioComp = CreateAudioComponent(TurretRotationLoop,TRUE,TRUE,TRUE);
		TurretRotationAudioComp.bShouldRemainActiveIfDropped=true;
		AttachComponent(TurretRotationAudioComp);
	}
	TurretRotationAudioComp.Play();
}

simulated function StoppedRotating()
{
	Super.StoppedRotating();
	if(TurretRotationAudioComp != none)
	{
		TurretRotationAudioComp.FadeOut(0.3f,0);
	}	
}

function FailedDeployment()
{
	PlaySound(DeploymentBlockedSound);
}

simulated function TurnOn()
{
	local BodyStance	BS_Deploy;
	Super.TurnOn();
	
	if(EnabledAudioComp == none && EnabledAudioSound != none && WorldInfo.NetMode != NM_DedicatedServer)
	{
		EnabledAudioComp = CreateAudioComponent(EnabledAudioSound,TRUE,TRUE,TRUE);
		EnabledAudioComp.bShouldRemainActiveIfDropped=true;
		AttachComponent(EnabledAudioComp);
	}
	EnabledAudioComp.Play();

	BS_Deploy.AnimName[BS_FullBody] = 'Startup';
	BS_Play(BS_Deploy, 1.f, 0.f, -1.f, FALSE);
	BS_SetAnimEndNotify(BS_Deploy,TRUE);
	SetCollision(,TRUE);
}

simulated function TurnOff()
{
	local BodyStance	BS_Deploy;
	Super.TurnOff();
	FrickinLaserBeam.DeactivateSystem();
	FrickinLaserBeam.KillParticlesForced();

	BS_Deploy.AnimName[BS_FullBody] = 'ShutDown';
	BS_Play(BS_Deploy, 1.f, 0.f, -1.f, FALSE);
	BS_SetAnimEndNotify(BS_Deploy,TRUE);
	if(PissedLoopAudioComp != none)
	{
		PissedLoopAudioComp.FadeOut(0.5f,0);
	}

	if(TurretRotationAudioComp != none)
	{
		TurretRotationAudioComp.Stop();
	}

	EnabledAudioComp.Stop();

	SetCollision(,FALSE);
	
}

simulated event OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	//MessagePlayer(GetFuncName()@PlayedTime);
	Super.OnAnimEnd(SeqNode, PlayedTime, ExcessTime);

	bPlayAnimSounds=true;
	if(bEnabled)
	{
		bDeployed=true;
		InitSkelControl();
		FrickinLaserBeam.ActivateSystem();
	}
	else
	{
		bDeployed=false;
	}
}


simulated function vector GetVectColorFromLinearColor(LinearColor Col)
{
	local vector ret;

	ret.x = Col.R * 255;
	ret.y = Col.G * 255;
	ret.z = Col.B * 255;

	return ret;
}
simulated function GetPissed()
{
	bTrackingAnEnemy=true;
	EmissiveMIC.SetVectorParameterValue('Weap_EmisColor',PissedColor);
	//FrickinLaserBeam.SetVectorParameter('Laser_Color',GetVectColorFromLinearColor(PissedColor));
	FrickinLaserBeam.SetFloatParameter('Laser_Color_Switch',1);
	
	if(PissedLoopAudioComp == none && PissedLoop != none && WorldInfo.NetMode != NM_DedicatedServer)
	{
		PissedLoopAudioComp = CreateAudioComponent(PissedLoop,TRUE,TRUE,TRUE);
		PissedLoopAudioComp.bShouldRemainActiveIfDropped=true;
		AttachComponent(TurretRotationAudioComp);
	}
	PissedLoopAudioComp.Play();

}

simulated function GetHappy()
{
	PlaySound(LostTargetSound);
	bTrackingAnEnemy=false;
	EmissiveMIC.SetVectorParameterValue('Weap_EmisColor',HappyColor);
	//FrickinLaserBeam.SetVectorParameter('Laser_Color',GetVectColorFromLinearColor(HappyColor));
	FrickinLaserBeam.SetFloatParameter('Laser_Color_Switch',0);
	
	if(PissedLoopAudioComp != none)
	{
		PissedLoopAudioComp.FadeOut(0.5f,0);
	}	
}

simulated function StartedFiring()
{
	Super.StartedFiring();
	BS_Play(BS_Fire,1.0f,0.25f,0.25f,TRUE,FALSE);
	BS_SetAnimEndNotify(BS_Fire,FALSE);
}

simulated function StoppedFiring(optional bool bFromRep)
{
	if(bFiring || bFromRep)
	{
		PlaySound(FireStopSound);
		PlaySound(BarrelSpinDown);
		BS_SetPlayingFlag(BS_Fire,FALSE);
	}
	Super.StoppedFiring();
}

function bool Died(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{
	FrickinLaserBeam.DeactivateSystem();
	return Super.Died(Killer,DamageType,HitLocation);
}



simulated event Tick(float DeltaTime)
{
	local vector muzzleloc;
	local float Dist;
	Super.Tick(DeltaTime);
	muzzleloc = GetPhysicalFireStartLoc(vect(0,0,0));
	FrickinLaserBeam.SetVectorParameter('Start',muzzleloc);
	//FrickinLaserBeam.SetVectorParameter('End',muzzleloc+( (vect(1.f,0,0) * (DetectionRange)) >> GetBaseAimRotation() ));
	FrickinLaserBeam.SetVectorParameter('End',LaserHitPoint);

	if(LaserHitPoint != vect(0,0,0))
	{
		// scale up effect to hit end point
		Dist = VSize(muzzleloc - LaserHitPoint);
		//`log(Dist@Dist/default.DetectionRange);
		FrickinLaserBeam.SetScale(Dist/BaseLaserDist);
	}

	if(bFiring)
	{
		if( BulletsMIC != none)
		{
			BulletsMIC.SetScalarParameterValue('Firing',WorldInfo.TimeSeconds);
		}
	}
	
}

/** Called when pawn imulsion effect is done, so we can clean up. */
simulated function OnPawnDeathEffectFinished(ParticleSystemComponent PSC)
{
	Mesh.DetachComponent(PSC);
}

simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	local particleSystemComponent PSC;
	PSC = new class'ParticleSystemComponent';
	PSC.SetTemplate(ExploParticleEmitterTemplate);
	Mesh.Attachcomponenttosocket(PSC,RightHandSocketName);
	PSC.ActivateSystem();
	PSC.OnSystemFinished = OnPawnDeathEffectFinished;

	PlaySound(ExploSound);
	Controller.Destroy();
	TurnOff();
	

}

simulated function bool ShouldTurnCursorRedFor(GearPC PC)
{
	return Super.ShouldTurnCursorRedFor(PC) && bEnabled;
}


DefaultProperties
{
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_SecurityBotGunStationary'
	
	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'GOW_Outpost_Turret.Mesh.Skel_GOW_OutPost_Turret'
		AnimSets(0)=AnimSet'GOW_Outpost_Turret.Mesh.GOW_OutPost_Turret_Anims'
		PhysicsAsset=PhysicsAsset'GOW_Outpost_Turret_Aux.Skel_GOW_OutPost_Turret_Physics'
		AnimTreeTemplate=AnimTree'GOW_Outpost_Turret_Aux.GOW_Outpost_Turret_AnimTree'
		Translation=(Z=-74)
		Rotation=(Yaw=16384)
		bIgnoreControllersWhenNotRendered=FALSE
		bAcceptsDynamicDecals=FALSE
	End Object


	Begin Object Class=ParticleSystemComponent Name=Laser0	
		Template=ParticleSystem'Trap_LaserGrid.Effects.P_FX_LaserBeam_Red_Narrows_Vector'
	End Object
	FrickinLaserBeam=Laser0
	EnabledAudioSound=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_Beam02Cue'	
	TargetAcquiredSound=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_BreakBeamCue'
	LostTargetSound=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_EnemyLossCue'
	FireStopSound=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_Lock01Cue'
	TurretRotationLoop=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_TurretRotate01Cue'	
	PissedLoop=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_Alarm01Cue'
	BarrelSpinDown=SoundCue'Weapon_Troika.Weapons.Troika_FireStopCue'
	DeploymentBlockedSound=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_EnemyLossCue'

	StartUpSound1=SoundCue'Ambient_NonLoop.AmbientNonLoop.MetalSlamMediumCue'
	StartUpSound2=SoundCue'Ambient_NonLoop.AmbientNonLoop.MetalMovesCue'
	StartUpSound3=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_PowerOn01Cue'
	StartUpSound4=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_Beep01Cue'

	ShutDownSound1=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_PowerOff01Cue'
	ShutDownSound2=SoundCue'Ambient_NonLoop.AmbientNonLoop.MetalMovesCue'
	ShutDownSound3=SoundCue'Ambient_NonLoop.AmbientNonLoop.MetalSlamMediumCue'

	ExploParticleEmitterTemplate=ParticleSystem'Sentry_Bot.Effects.P_Sentry_Bot_Explosion'
	ExploSound=SoundCue'Weapon_SecurityBots.SecurityBots.SecurityBot_ExplodeCue'
	PhysicalFireLocBoneName=laser

	BarrelRotationSkelControlName=TurretRot
	BarrelRotationBoneName=Dummy_Upper

	HappyColor=(R=0.5,G=1.0,B=2.0,A=2.0)
	PissedColor=(R=1.0,G=0,B=0,A=2.0)

	DrawScale=1.5f
	RightHandSocketName=laser

	Begin Object Name=CollisionCylinder
		CollisionHeight=+00100.000000
		CollisionRadius=+00100.000000
	End Object

	

	PelvisBoneName="Dummy_Upper"

	AimAttractors(0)=(OuterRadius=96.f,InnerRadius=50.f,BoneName="Dummy_Upper")

	BaseLaserDist=512

	RotationRate=(Pitch=8000)
}