/**
 * Locust Ticker Bomb
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustTicker extends GearPawn_LocustTickerBase
	config(Pawn)
	implements(AIObjectAvoidanceInterface);


var() name ExplosionBoneName;
var GearExplosion ExplosionTemplate;

/** These guys have a dust cloud around them all the time **/
var transient ParticleSystem DustCloudTemplate;
var transient ParticleSystemComponent PSC_DustCloud;
var name DustCloudSocketName;



struct ChatterZone
{
	var float Range;
	var SoundCue Sound;
	var float NextPlayTime;
};
/** List of ranges which represent different zones to play different speed chatter sounds in **/
var() array<ChatterZone> ChatterZones;
var float LastChatterSoundTime;
var float MinTimeBetweenAllSounds;

/** the current zone for chatter sounds **/
var int CurrentChatterZone;

/** minimum time between chatters from the same zone **/
var() float MinChatterReplayTime;
/** maximum time between chatters from the same zone **/
var() float MaxChatterReplayTime;

var() float MinAmbientChatterReplayTime;
var() float MaxAmbientChatterReplayTime;

/** Cue to play every now and then when we have no enemy in range **/
var SoundCue AmbientSound;
/** next world time to play an ambient cue **/
var float NextAmbientSoundTime;

/** sound to play when the Ticker charges his enemy **/
var() SoundCue ChargeSound;
/** sound to play when the Ticker is scared and hides **/
var() SoundCue HideSound;
/** footstep sound **/
var() SoundCue FootstepLoop;
/** death sound, played just before explode */
var() SoundCue DeathSound;
/** sound that plays when the ticker gets flipped onto its back */
var() SoundCue FlipOverSound;

/** Audio component used to play footstep audio **/
var AudioComponent FootstepComp;

var		MaterialInstanceConstant		GlowMIC;
var		vector2d						GlowRange;
var		float							CurrentGlowVal;
var		float							ExplodeAnimDuration;

var protected AIAvoidanceCylinder AvoidanceCylinder;

// once we do damage, this is on and we don't do damage any more
var bool	bPopped;
// if this is true we exploded on our own volition
var bool	bExplosionInvoluntary;
// person who shot at us last (person who set off the explosion)
var Controller	LastDamageInstigator;
var config float DamageMultiplierToSquadMatesOfExploder;



simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	PSC_DustCloud = new(self) class'ParticleSystemComponent';
	DetermineDustCloudType();
	SetTimer( 1.0f, TRUE, nameof(DetermineDustCloudType) );
	SetTimer( 0.25f, TRUE, nameof(PlayFootStepSounds) );
	SetTimer( MinTimeBetweenAllSounds, TRUE, nameof(PlayChatterSounds) );

	LastChatterSoundTime = WorldInfo.TimeSeconds + RandRange(MinChatterReplayTime,MaxChatterReplayTime);

	GlowMIC = Mesh.CreateAndSetMaterialInstanceConstant(0);
}


simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'bCharging')
	{
		if(bCharging)
		{
			PlayChargeSound();
		}
	}
	else if(VarName == 'bHiding')
	{
		if(bHiding)
		{
			PlayHideSound();
		}
	}
	else if(VarName == 'bExplode')
	{
		if(bExplode)
		{
			PlayExplodeAnim();
		}
		else
		{
			StopExplodeAnim();
		}
	}
	else if(VarName == 'bSpawnedFromkantus')
	{
		if(bSpawnedFromkantus)
		{
			PlayKantusSpawnEffects();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

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
	LastDamageInstigator = InstigatedBy;
	Super.TakeDamage(Damage,InstigatedBy,HitLocation,Momentum,DamageType,HitInfo,DamageCauser);
}

simulated function PlayExplodeAnim()
{
	local BodyStance	BS_Explode;
	Super.PlayExplodeAnim();
	BS_Explode.AnimName[BS_FullBody] = 'Explode';
	BS_Play(BS_Explode, 1.f, 0.f, -1.f, FALSE);
	ExplodeAnimDuration = BS_GetTimeLeft(BS_Explode);
	CurrentGlowVal=GlowRange.X;
	PlayDeathSound();
	SetupAvoidanceCylinder();
}

function SetupAvoidanceCylinder()
{
	local float AvoidRadius;

	if( !bDeleteMe && AvoidanceCylinder == None )
	{
		AvoidRadius = ExplosionDamageRadius*0.65;
		AvoidanceCylinder = Spawn(class'AIAvoidanceCylinder',self,,Location,,,TRUE);
		if(AvoidanceCylinder!=none)
		{
			AvoidanceCylinder.SetBase(self);
			AvoidanceCylinder.SetCylinderSize(AvoidRadius,AvoidRadius*2.0f);
			AvoidanceCylinder.SetEnabled(true);
		}
		//DrawDebugSphere(Location,AvoidRadius,16,255,0,0,TRUE);
	}

}

/**
---> AIobjectAvoidanceInterface
**/
function bool ShouldAvoid(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	if(AskingAI != Controller && FastTrace(Location, AskingAI.Pawn.Location,,true))
	{
		return true;
	}

	return false;
}
function bool ShouldEvade(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	local float Thresh;

	Thresh = ExplosionDamageRadius * 0.75f;
	Thresh *= Thresh;
	// evade if it's really close
	if(VSizeSq(AskingAI.Pawn.Location - Location) < Thresh)
	{
		return true;
	}
	return false;
}
function bool ShouldRoadieRun(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent);

simulated function StopExplodeAnim()
{
	//`log(GetFuncName());
	//ScriptTrace();
	if(AvoidanceCylinder != none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder=none;
	}

	bExplode=false;
	BS_StopAll(0.1f);
}

simulated event Tick(FLOAT DeltaTime)
{
	Super.Tick(DeltaTime);
	if(bExplode)
	{
		CurrentGlowVal += ((GlowRange.Y - GlowRange.X)/ExplodeAnimDuration)*DeltaTime;
		GlowMIC.SetScalarParameterValue('TickerBrightness',CurrentGlowVal);
	}
	else if(CurrentGlowVal > GlowRange.X)
	{
		CurrentGlowVal -= ((GlowRange.Y - GlowRange.X)/ExplodeAnimDuration)*DeltaTime;
		GlowMIC.SetScalarParameterValue('TickerBrightness',CurrentGlowVal);
	}
	
}

simulated final function int DetermineZoneForRange(float Range)
{
	local int i;

	for(i=0;i<ChatterZones.Length;i++)
	{
		if(Range < ChatterZones[i].Range )
		{
			return i;
		}
	}

	return -1;
}

simulated final function float GetRangeToPlayer()
{
	local PlayerController	PC;
	local float Closest;
	local float CurDist;
	Closest = 100000000.f;
	ForEach LocalPlayerControllers(class'PlayerController', PC)
	{
		if(PC.Pawn != none && PC.Pawn.Health > 0)
		{
			CurDist = VSize(PC.Pawn.Location - Location);
			Closest = FMin(CurDist,Closest);
		}
	}

	return Closest;
}

simulated final function PlayChatterSounds()
{
	local int NewZone;

	NewZone = -1;

	if(Controller != none && GearAI(Controller).HasValidEnemy())
	{
		NewZone = DetermineZoneForRange(GetRangeToPlayer());
	}


	if(NewZone != CurrentChatterZone)
	{
		CurrentChatterZone = NewZone;
	}

	if(WorldInfo.TimeSeconds >= LastChatterSoundTime + MinTimeBetweenAllSounds)
	{
		if(CurrentChatterZone != -1)
		{
			if(WorldInfo.TimeSeconds >= ChatterZones[CurrentChatterZone].NextPlayTime)
			{
				//DEBUG
				//`log(GetFuncName()@self@"Playing "@ChatterZones[CurrentChatterZone].Sound@"Zone:"@CurrentChatterZone@"Range:"@GetRangeToPlayer());
				PlaySound(ChatterZones[CurrentChatterZone].Sound);
				ChatterZones[CurrentChatterZone].NextPlayTime = WorldInfo.TimeSeconds + RandRange(MinChatterReplayTime,MaxChatterReplayTime);
				LastChatterSoundTime=WorldInfo.TimeSeconds;
			}
		}
		else
		{
			if(WorldInfo.TimeSeconds >= NextAmbientSoundTime)
			{
				PlaySound(AmbientSound);
				NextAmbientSoundTime = WorldInfo.TimeSeconds + (RandRange(MinAmbientChatterReplayTime,MaxAmbientChatterReplayTime*2.0f));
				LastChatterSoundTime=WorldInfo.TimeSeconds;
			}

		}
	}
}

simulated function PlayDeathSound()
{
	PlaySound(DeathSound);
}

simulated function PlayHideSound()
{
	Super.PlayHideSound();
	//DEBUG
	//`log(GetFuncname()@self);
	PlaySound(HideSound);
}

simulated function PlayChargeSound()
{
	Super.PlayChargeSound();
	//DEBUG
	//`log(GetFuncName()@self);
	PlaySound(ChargeSound);
}

/** This is called every second and will look to see if we are on top of a different PhysicalMaterial than before and update our dust cloud **/
simulated function DetermineDustCloudType()
{
	local ParticleSystem DustPS;

	DustPS = GetDustPS();
	//`log( "DustPS " $ DustPS );

	if( DustCloudTemplate != DustPS )
	{
		DustCloudTemplate = DustPS;
		PSC_DustCloud.SetTemplate( DustCloudTemplate );
		Mesh.AttachComponentToSocket( PSC_DustCloud, DustCloudSocketName );
		PSC_DustCloud.ActivateSystem();
		//`log( "attaching: " $ DustCloudTemplate );
	}
}

simulated function PlayFootStepSounds()
{

	if((FootstepLoop != None) &&
	   (WorldInfo.NetMode != NM_DedicatedServer)
	   )
	{

		if(VSizeSq(Velocity) > 16*16)
		{
			if(FootstepComp == none)
			{
				//`log(GetFuncName()@self@"creating audio component using "$FootStepLoop);
				FootstepComp = CreateAudioComponent( FootstepLoop, TRUE, TRUE );
			}

			if(FootstepComp != None)
			{
				//`log(GetFuncName()@Self@"Attaching audio comp");
				FootstepComp.bAutoDestroy = TRUE;
				FootstepComp.Location = Location;
				AttachComponent( FootstepComp );
				FootstepComp.FadeIn(0.3f, 1.f);
			}
		}
		else if(FootstepComp != none)
		{
			FootstepComp.FadeOut(0.5f,0.f);
		}
	}

}

simulated event Destroyed()
{
	Super.Destroyed();
	if(FootstepComp != none)
	{
		FootstepComp.FadeOut(0.1f,0.f);
	}

	if(AvoidanceCylinder!=none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder=none;
	}
}


/** Looks at the physical material below pawn and then determines the physical material and then gets the particle system to play out of it **/
simulated function ParticleSystem GetDustPS()
{
	local vector PawnLoc;
	local float CurrHeight;

	local vector out_HitLocation;
	local vector out_HitNormal;
	local vector TraceDest;
	local vector TraceStart;
	local vector TraceExtent;
	local TraceHitInfo HitInfo;

	local GearPhysicalMaterialProperty PhysMatProp;
	local ParticleSystem ImpactPS;
	local PhysicalMaterial PhysMaterial;

	PawnLoc = Location;
	CurrHeight = GetCollisionHeight();

	TraceStart = PawnLoc;
	TraceDest = TraceStart - ( Vect(0, 0, 1 ) * CurrHeight ) - Vect(0, 0, 16 );

	// trace down and see what we are standing on
	Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, false, TraceExtent, HitInfo, TRACEFLAG_PhysicsVolumes );

	// grab the physical material to get things rolling
	PhysMaterial = class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo );

	//`log( "PhysMaterial " $ PhysMaterial );

	// until we find a particle system or run out of stuff to look in
	while( ImpactPS == None && PhysMaterial != None )
	{
		PhysMatProp = GearPhysicalMaterialProperty(PhysMaterial.PhysicalMaterialProperty);
		if (PhysMatProp != None)
		{
			ImpactPS = PhysMatProp.FootStepInfo[CFST_Locust_Ticker].FootstepEffectPS;
		}

		//`LogExt("- Valid material property");

		PhysMaterial = PhysMaterial.Parent;
	}

	return ImpactPS;
}

function AdjustTickerDamage(out int Damage, GearPawn Damagee)
{
	local GearSquad DamagerSquad;
	local GearPC GPC;
	local GearAI GAI;
	// if someone else shooting us is causing this explosion, lower damage to the human squad mates of the guy that shot it
	if(bExplosionInvoluntary && LastDamageInstigator != none && Damagee.IsHumanControlled() && Damagee.Controller != LastDamageInstigator)
	{
		GAI = GearAI(LastDamageInstigator);
		if(GAI != none)
		{
			DamagerSquad = GAI.Squad;
		}
		else 
		{
			GPC = GearPC(LastDamageInstigator);
			if(GPC != none)
			{
				DamagerSquad = GPC.Squad;
			}
		}

		// if the dude that blew me up is in the same squad as the player taking damage, drop the damage by 50%
		if(GearPC(Damagee.Controller).Squad == DamagerSquad)
		{
			//MessagePlayer("Adjusting damage done by "@self@"to"@Damagee@"because we exploded by "@LastDamageInstigator@"who is in his squad");
			Damage *= DamageMultiplierToSquadMatesOfExploder;
		}
	}
}



simulated function Explode()
{
	local GearExplosionActor ExplosionActor;
	local Vector ExplosionLoc;
	local rotator ExplosionRot;

	//`log(GetFuncName()@bPopped@self);
	if(!bPopped)
	{
		//MessagePlayer(bExplosionInvoluntary);

		bPopped = TRUE;
		Mesh.GetSocketWorldLocationAndRotation(ExplosionBoneName,ExplosionLoc,ExplosionRot);

		// spawn explosion
		ExplosionActor = Spawn(class'GearExplosionActor', self,, ExplosionLoc, ExplosionRot);
		if(bExplosionInvoluntary)
		{
			ExplosionActor.InstigatorController = LastDamageInstigator;
		}
		ExplosionTemplate.Damage = ExplosionBaseDamage;
		ExplosionTemplate.DamageRadius = ExplosionDamageRadius;
		ExplosionTemplate.DamageFalloffExponent = ExplosionDamageFalloff;
		ExplosionActor.Explode(ExplosionTemplate);

		if( WorldInfo.GRI.ShouldShowGore() )
		{
			// delay by a tick to spread out the cost a bit
			SetTimer(WorldInfo.DeltaSeconds + 0.01, false, nameof(SpawnABloodTrail_GibExplode_360));

			SpawnHeadChunks();
		}


		PSC_DustCloud.DeactivateSystem();

		if(AvoidanceCylinder != none)
		{
			AvoidanceCylinder.Destroy();
			AvoidanceCylinder = none;
		}	
	}
	
}

simulated function PlayRagDollDeath(class<GearDamageType> GearDamageType, vector HitLoc, bool bShowGore)
{
	// give time for any clients to get updated state
	LifeSpan = 0.5;
	SetHidden(true);
}


simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	Super.PlayDying(DamageType,HitLoc);
	bExplosionInvoluntary=true;
	Explode();
}

simulated function FlipMeOver(Controller Inst)
{
	local rotator NewRot;
	Super.FlipMeOver(Inst);

	PlaySound(FlipOverSound);
	if(bAllowFlipOver)
	{
		// hax 
		GroundSpeed=0;
		NewRot = Rotation;
		NewRot.Pitch += 32768;
		SetRotation(NewRot);
		MyGearAI.DesiredRotation=NewRot;
	}
}

simulated function UnFlipMeOver()
{
	local rotator NewRot;
	Super.UnFlipMeOver();

	GroundSpeed=DefaultGroundSpeed;
	NewRot = Rotation;
	NewRot.Pitch -= 32768;
	//SetRotation(NewRot);
	MyGearAI.DesiredRotation=NewRot;
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

simulated function UpdateMeshBoneControllers(float DeltaTime);
simulated function HandleWeaponFiring();

defaultproperties
{
	ExplosionBoneName=Bomb
	ControllerClass=class'GearGame.GearAI_Ticker'
	bCanBeBaseForPawns=false

	MeleeDamageBoneName="pelvis"

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Ticker.Locust_Ticker'
		PhysicsAsset=PhysicsAsset'Locust_Ticker.Meshes.Locust_Ticker_Physics'
		AnimTreeTemplate=AnimTree'Locust_Ticker.Locust_Ticker_AnimTree'
		AnimSets(0)=AnimSet'Locust_Ticker.Locust_Ticker_Animset'
		Translation=(z=-44)
		Rotation=(Yaw=-16384)  // fix this up for gears3 meshes should NOT be imported flipped like this.  Content MUST fix it
	End Object

	Begin Object Name=CollisionCylinder
		CollisionHeight=+0044.000000
//		CollisionRadius=+0056.000000
	End Object

	bCanStrafe=true
	PeripheralVision=-0.75f
	RotationRate=(Pitch=20000,Yaw=65535,Roll=20000)


	// explosion point light
    Begin Object Class=PointLightComponent Name=ExploLight0
		Radius=600.00000
		Brightness=95.000000
		LightColor=(R=241,G=139,B=78,A=255)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
    End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_TickerExplosion'
		MomentumTransferScale=20.f	// Scale momentum defined in DamageType

		ParticleEmitterTemplate=ParticleSystem'Locust_Ticker.Effects.P_Ticker_Explode'
		ExplosionSound=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_Explosion_Cue'
		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		FractureMeshRadius=200.0
		FracturePartVel=500.0

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
		ExploShakeInnerRadius=300
		ExploShakeOuterRadius=1200
		ExploShakeFalloff=2.f
	End Object
	ExplosionTemplate=ExploTemplate0

	DustCloudSocketName=Ground_Dust

	InventoryManagerClass=none
	SightBoneName=none
	PelvisBoneName=none

	// sound stuff
	MinChatterReplayTime=1.5
	MaxChatterReplayTime=5.0
	MinAmbientChatterReplayTime=4.0
	MaxAmbientChatterReplayTime=7.0
	ChatterZones(0)=(Range=200,Sound=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_ChatterFaster_Cue')
	ChatterZones(1)=(Range=600,Sound=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_ChatterFast_Cue')
	ChatterZones(2)=(Range=1000,Sound=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_ChatterMedium_Cue')
	ChatterZones(3)=(Range=1400,Sound=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_ChatterSlow_Cue')
	AmbientSound=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_Ambient_Cue'
	MinTimeBetweenAllSounds=1.0f

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustTicker'
	HideSound=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_Defense_Cue'
	ChargeSound=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_Attack_Cue'
	FootstepLoop=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_FootstepsLoop_Cue'
	DeathSound=SoundCue'Locust_Ticker_Efforts.Ticker.Ticker_Death_Cue'
	FlipOverSound=SoundCue'Locust_Kryll_Efforts.Kryll_Single_VocalAttack'
	GlowRange=(X=1.0f,Y=20.f)

	AimAttractors(0)=(OuterRadius=96.f,InnerRadius=32.f,BoneName="chest")

	KantusSpawnParticleSystem=ParticleSystem'Locust_Kantus.Effects.P_Kantus_Summon_Ticker'

	HeadChunks.Empty
	HeadChunks(00)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk5_SM' // this is the body
	HeadChunks(01)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk2_SM'
	HeadChunks(02)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk3_SM'
	HeadChunks(03)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk4_SM'
	HeadChunks(04)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk1_SM'
	HeadChunks(05)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk6_SM'
	HeadChunks(06)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk7_SM'
	HeadChunks(07)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk8_SM'
	HeadChunks(08)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk9_SM'
	HeadChunks(09)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk10_SM'
	HeadChunks(10)=StaticMesh'Locust_Ticker.Meshes.Ticker_Gore_Chunk11_SM'

}
