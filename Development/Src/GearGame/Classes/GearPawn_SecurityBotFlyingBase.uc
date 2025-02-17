/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearPawn_SecurityBotFlyingBase extends GearPawn
	native(Pawn)
	config(Pawn)
	abstract;

var() float WobbleMagMax,WobbleMagMin;
var() float MaxWobbleDist;
var transient vector CurrentWobble;
var transient vector CurrentAccumulatedWobbleOffset;
var transient float LastWobbleDot;

/** death explosion vars */
var() instanced editinline GearExplosion	ExplosionTemplate;
var() config float ExploDamage;
var() config float ExploDamageRadius;
var() config float ExploDamageFalloffExp;

/** Mommy, Where do bullets come from? */
var name PhysicalFireLocBoneName;

/** Whether damage effect is active or not */
var() repnotify float	DamageTakenPct;
/** Whether bot is in angry mode - update colors */
var() repnotify bool	bAngryBot;

/** Percentage of default health take before damage effect shows up */
var() config float	DamageEffectPct;



/** Effects */
/** Particle systems for eye glow/lights */
var ParticleSystem			PS_Eye, PS_Eye_Angry;
var ParticleSystemComponent PSC_Eye;
/** Particle systems for jets */
var ParticleSystem			PS_Jets;
var ParticleSystemComponent PSC_Jets;
/** Particle system for damaged bot */
var ParticleSystem			PS_Damage;
var ParticleSystemComponent PSC_Damage;



cpptext
{
	void CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant);
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	PSC_Eye = new(self) class'ParticleSystemComponent';
	PSC_Eye.SetTemplate( PS_Eye );
	PSC_Eye.SetHidden( FALSE );
	PSC_Eye.ActivateSystem();
	Mesh.AttachComponentToSocket( PSC_Eye, 'Eye' );

	PSC_Jets = new(self) class'ParticleSystemComponent';
	PSC_Jets.SetTemplate( PS_Jets );
	PSC_Jets.SetHidden( FALSE );
	PSC_Jets.ActivateSystem();
	Mesh.AttachComponentToSocket( PSC_Jets, 'Jets' );
}

simulated event ReplicatedEvent( name VarName )
{
	Super.ReplicatedEvent( VarName );

	switch( VarName )
	{
		case 'DamageTakenPct':
			PlayDamageEffect();
			break;
		case 'bAngryBot':
			PlayAngryEffect();
			break;
	}
}

simulated native event Vector GetPhysicalFireStartLoc( vector FireOffset );

singular function BaseChange();
function AddVelocity(vector NewVelocity, vector HitLocation, class<DamageType> DamageType, optional TraceHitInfo HitInfo );
function SetMovementPhysics()
{
	SetPhysics(PHYS_Flying);
}

simulated function AttachWeaponToSlot(GearWeapon W);
simulated function AdjustWeaponDueToMirror();


event HitWall( vector HitNormal, actor Wall, PrimitiveComponent WallComp)
{
	MessagePlayer(GetFuncName()@self@HitNormal);
	Super.HitWall(HitNormal,Wall,WallComp);

	SetPhysics(PHYS_Falling);
	SetTimer( 0.5,FALSE,nameof(SetMovementPhysics) );
	Velocity -= (2) * HitNormal * (Velocity Dot HitNormal);

	Controller.HandlePathObstruction(Wall);
}

simulated function OnAcquireEnemy(GearPawn Enemy); // stub
simulated function OnLostEnemy();

simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	local GearExplosionActor ExplosionActor;
	// spawn explosion
	ExplosionActor = Spawn(class'GearExplosionActor', self,, Location, Rotation);
	ExplosionTemplate.Damage = ExploDamage;
	ExplosionTemplate.DamageRadius = ExploDamageRadius;
	ExplosionTemplate.DamageFalloffExponent = ExploDamageFalloffExp;
	ExplosionTemplate.ExploShakeOuterRadius = ExploDamageRadius;;
	ExplosionActor.Explode(ExplosionTemplate);
	//Super.PlayDying(DamageType,HitLoc);

	// Turn off effects
	if( PSC_Eye		!= None ) { PSC_Eye.DeactivateSystem();		}
	if( PSC_Jets	!= None ) { PSC_Jets.DeactivateSystem();	}
	if( PSC_Damage	!= None ) { PSC_Damage.DeactivateSystem();	}
	
	// just explode, don't leave a corpse
	Destroy();
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

function DoDamageEffects(float Damage, Pawn InstigatedBy, vector HitLocation, class<DamageType> DamageType, vector Momentum, TraceHitInfo HitInfo)
{
	Super.DoDamageEffects( Damage, InstigatedBy, HitLocation, DamageType, Momentum, HitInfo );

	DamageTakenPct = (1.f - (Health/float(DefaultHealth)));
	PlayDamageEffect();
}

simulated function PlayDamageEffect();
simulated function PlayAngryEffect();

defaultProperties
{
	bCanDoRun2Cover=FALSE
	AirSpeed=300.f
	RotationRate=(Pitch=25,Yaw=25,Roll=50)
	HearingThreshold=6000.f
	bCanFly=true
	WobbleMagMax=250.f
	WobbleMagMin=100.f
	MaxWobbleDist=100.f
	bCanDBNO=false

	// explosion point light
	Begin Object Class=PointLightComponent Name=ExploLight0
		Radius=800.000000
		Brightness=1000.000000
		LightColor=(B=60,G=107,R=249,A=255)
		Translation=(X=16)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
	End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_Explosive'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType

		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
		ExploShakeInnerRadius=325
		ExploShakeOuterRadius=750

		FractureMeshRadius=120.0
		FracturePartVel=500.0
	End Object
	ExplosionTemplate=ExploTemplate0
}
