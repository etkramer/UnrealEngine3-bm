/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_PropaneTank extends GearDestructibleObject
	config(Weapon);

/** Damage to give at explosion epicenter. Duplicated from Explosion template so it can be config. */
var() config float	ExploBaseDamage;

/** Radius to apply damage.  Damage falls off linearly from center to edge of radius.  Duplicated from Explosion template so it can be config. */
var() config float	ExploDamageRadius;

/** Damage falloff exponent (1 = linear).  Duplicated from Explosion template so it can be config. */
var() config float	ExploDamageFalloff;

/** TRUE to burn forever after exploding, false to respect BurnTimeSeconds. */
var() bool			bBurnForever;

/** Length of time, in seconds, to remain burning after explosion. */
var() const float	BurnTimeSeconds;

/** The light used for the persistent flame, used if bRemainBurning is TRUE */
var() PointLightComponent		PersistentFlameLight;

/** particles component for persistent flame */
var() ParticleSystemComponent	PS_PersistentFlame;
var() ParticleSystemComponent	PS_PreFire;

/** TRUE if it already blew up. */
var repnotify bool				bExploded;

/** TRUE if we have explosed from being damaged by Explosive Damage Type **/
var repnotify bool bExplodedFromExplosiveDamageType;

/** TRUE if it's sitting there burning, FALSE otherwise */
var repnotify bool				bBurning;

var() const protected StaticMesh			DestroyedStaticMesh;
//var() const protected CylinderComponent		DestroyedCollisionCylinder;

var bool bFadeOutLight;

var AudioComponent FireCracklingSound;

/** Defines the explosion. */
var() editinline GearExplosion			ExplosionTemplate;

/** Flicker vars. */
var() private const vector2d			FlameLightBrightnessFlickerRange;
var() private float						FlameLightFlickerInterpSpeed;
var private transient float				LastFlameLightBrightness;

/** This is needed to allow for propane tanks to correctly propagate the instigator of the explosion for kills **/
var private Controller TheExplosionInstigator;

struct CheckpointRecord
{
	var bool bIsShutDown;
};

replication
{
	if (bNetDirty)
		bExploded, bExplodedFromExplosiveDamageType, bBurning, TheExplosionInstigator;
}

simulated private function float GetFlickerVal(vector2d Range, float Last, float DeltaTime, float InterpSpeed)
{
	local float GoalVal;
	GoalVal = RandRange(Range.X, Range.Y);
	return FInterpTo(Last, GoalVal, DeltaTime, InterpSpeed);
}


simulated function Tick(float DeltaTime)
{
	//local float NewBrightness;
	// hacky flicker for muzzle light.
	//NewBrightness = FMax(0.f, GetFlickerVal(FlameLightBrightnessFlickerRange, LastFlameLightBrightness, DeltaTime, FlameLightFlickerInterpSpeed));
	//PersistentFlameLight.SetLightProperties(NewBrightness);
//
// 	if (bFadeOutLight)
// 	{
// 		if (PersistentFlameLight.Brightness > 0.f)
// 		{
// 			// fade out over 3 seconds
// 			PersistentFlameLight.SetLightProperties(PersistentFlameLight.Brightness - (PersistentFlameLight.default.Brightness * DeltaTime/3.f));
// 			FlameLightFlickerInterpSpeed -= default.FlameLightFlickerInterpSpeed * DeltaTime/3.f;
// 		}
// 		if (PersistentFlameLight.Brightness <= 0.f)
// 		{
// 			bFadeOutLight = FALSE;
// 			PersistentFlameLight.SetEnabled(FALSE);
// 		}
// 	}
//
// 	LastFlameLightBrightness = PersistentFlameLight.Brightness;

	Super.Tick(DeltaTime);
}

simulated event ReplicatedEvent( name VarName )
{

	if( VarName == 'bExplodedFromExplosiveDamageType' )
	{
		if( bExplodedFromExplosiveDamageType == TRUE )
		{
			GotoState('ExploSequenceExplosiveDamageType');
		}
	}
	else if (VarName == 'bExploded')
	{
		if (bExploded)
		{
			GotoState('ExploSequence');
		}
	}
	else if (VarName == 'bBurning')
	{
		if (!bBurning)
		{
			Extinguish();
		}
	}
	else
	{
		super.ReplicatedEvent(VarName);
	}
}

simulated protected function Explode()
{
	local rotator Rot;
	local DestructibleSubobject MainPiece;
	local GearExplosionActor ExplosionActor;

	// spawn explosion
	ExplosionActor = Spawn(class'GearExplosionActor', self,, Location, rot(16384,0,0));

	ExplosionActor.InstigatorController = TheExplosionInstigator;  // will have be replicated out
	ExplosionTemplate.Damage = ExploBaseDamage;
	ExplosionTemplate.DamageRadius = ExploDamageRadius;
	ExplosionTemplate.DamageFalloffExponent = ExploDamageFalloff;
	ExplosionActor.Explode(ExplosionTemplate);

	if (DestroyedStaticMesh != None)
	{
		// add collision component for destroyed version.  not sure why this is necessary, frankly.
//		if (DestroyedCollisionCylinder != None)
//		{
//			CollisionComponent = DestroyedCollisionCylinder;
//			AttachComponent(DestroyedCollisionCylinder);
//		}

		// swap mesh
		// compiler won't let us pass individual elements of a dynamic array as the value for an out parm, so use a local
		//@fixme ronp - TTPRO #40059
		MainPiece = SubObjects[0];
		SetSubObjectStaticMesh(MainPiece, DestroyedStaticMesh);
		SubObjects[0] = MainPiece;

		// randomize yaw so all tanks don't look the same.
		Rot = Rotation;
		Rot.Yaw = Rand(65535);
		SubObjects[0].Mesh.SetRotation(Rot);
	}
}

protected function DamageSubObject(int ObjIdx, int Damage, Controller EventInstigator, class<DamageType> DamType)
{
	super.DamageSubObject(ObjIdx, Damage, EventInstigator, DamType);

	// died, do explosion sequence
	if ((SubObjectHealths[ObjIdx] <= 0) && !bExploded && !bExplodedFromExplosiveDamageType)
	{
		TheExplosionInstigator = EventInstigator;

		if( ClassIsChildOf( DamType, class'GDT_Explosive') == TRUE )
		{
			GotoState('ExploSequenceExplosiveDamageType');
		}
		else
		{
			GotoState('ExploSequence');
		}
	}
}

simulated function UnDestroy()
{
	Super.UnDestroy();

	GotoState('');
	bExploded = false;
	bBurning = false;
	SubObjects[0].Mesh.SetRotation(rot(0,0,0));
//	DetachComponent(DestroyedCollisionCylinder);
	CollisionComponent = SubObjects[0].Mesh;
}

simulated state ExploSequenceExplosiveDamageType
{
	simulated event BeginState(name PreviousStateName)
	{
		// blow up
		if( Role == Role_Authority )
		{
 			bExplodedFromExplosiveDamageType = TRUE;
			bForceNetUpdate = TRUE;
		}
	}

begin:
	Explode();

	if (!bBurnForever)
	{
		if (BurnTimeSeconds >= 0.f)
		{
			Sleep(BurnTimeSeconds);
		}
		ShutDownObject();
		Extinguish();
	}

	GotoState('');
}


simulated state ExploSequence
{
	simulated event BeginState(name PreviousStateName)
	{
		// catch fire
		if (Role == Role_Authority)
		{
			bExploded = TRUE;
			bForceNetUpdate = TRUE;
		}

		// do pre-explosion fire emitters
		PS_PreFire.ActivateSystem();
		PS_PersistentFlame.ActivateSystem();

		PlaySound(SoundCue'Weapon_Grenade.Weapons.PropaneGasIgniteCue', true);
	}

Begin:
	Sleep(1.2f);

	// bewm!
// 	if(FireCracklingSound == none)
// 	{
// 		FireCracklingSound = CreateAudioComponent(SoundCue'Weapon_Grenade.Weapons.PropaneGasFireCue',false,true);
// 		FireCracklingSound.bUseOwnerLocation = true;
// 		FireCracklingSound.bAutoDestroy = true;
// 		FireCracklingSound.Location = location;
// 		AttachComponent(FireCracklingSound);
// 	}
// 	if(FireCracklingSound != none)
// 	{
// 		FireCracklingSound.FadeIn(0.2f,1.0f);
// 	}

	Explode();
	PS_PreFire.DeactivateSystem();

	if ( (BurnTimeSeconds <= 0.f) && (!bBurnForever) )
	{
		PS_PersistentFlame.DeactivateSystem();
		ShutDownObject();
		if(FireCracklingSound != none)
		{
			FireCracklingSound.FadeOut(0.3f,0.0f);
		}
	}
	else
	{
		// turn on persistent light
		if (Role == Role_Authority)
		{
			bBurning = TRUE;
			bForceNetUpdate = TRUE;
		}
		//PersistentFlameLight.SetEnabled(TRUE);

		if (!bBurnForever)
		{
			Sleep(BurnTimeSeconds);
			Extinguish();
		}
	}

	GotoState('');
}

simulated function Extinguish()
{
	// @fixme polish, fade this out over a second or so, instead of popping
	if (Role == Role_Authority)
	{
		bBurning = FALSE;
		bForceNetUpdate = TRUE;
	}

	bFadeOutLight = TRUE;

	// flame off
	PS_PersistentFlame.DeactivateSystem();

	if(FireCracklingSound != none)
	{
		FireCracklingSound.FadeOut(0.3f,0.0f);
	}

	GotoState('');
}

/**
 * Toggling a tank OFF will extinguish the barrel if it's burning.  Toggling it on will
 * detonate it if it hasn't blown up.
 */
function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if( Action.InputLinks[0].bHasImpulse )
	{
		if (!bExploded)
		{
			GotoState('ExploSequence');
		}
	}
	// Turn OFF
	else if( Action.InputLinks[1].bHasImpulse )
	{
		if (bBurning)
		{
			Extinguish();
		}
	}
	// Toggle
	else if( Action.InputLinks[2].bHasImpulse )
	{
		if (!bExploded)
		{
			GotoState('ExploSequence');
		}
		else
		{
			Extinguish();
		}
	}
}

function bool ShouldSaveForCheckpoint()
{
	// basically only save if we're completely disabled
	return (bHidden && bExploded);
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
	Record.bIsShutDown = (bHidden && bExploded);
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	if (Record.bIsShutDown)
	{
		ShutDown();
	}
}

defaultproperties
{
	bBurnForever=TRUE

	bLimitDamageTypes=TRUE
	VulnerableToDamageType(0)=class'GDT_Explosive'
	VulnerableToDamageType(1)=class'GDT_Ballistic'
	VulnerableToDamageType(2)=class'GDT_Melee'
	VulnerableToDamageType(3)=class'GDT_Fire'

	Begin Object Class=ParticleSystemComponent Name=PersistentFlame
		//Template=ParticleSystem'COG_City_Architecture.Particle_Effects.CP_Fire1_PE'
		Translation=(Z=140)
		bAutoActivate=FALSE
		// what else?
	End Object
	PS_PersistentFlame=PersistentFlame
	//Components.Add(PersistentFlame)

	Begin Object Class=ParticleSystemComponent Name=PreFire
		Template=ParticleSystem'Effects_Gameplay.Fire.P_Propane_Tank_Flame'
		Translation=(Z=-12)
		Rotation=(Pitch=16384)
		bAutoActivate=FALSE
		// what else
	End Object
	PS_PreFire=PreFire
	Components.Add(PreFire)

	//point light
// 	Begin Object Class=PointLightComponent Name=FlameLight
// 		Radius=600.000000
// 		Brightness=8.000000
// 		LightColor=(B=83,G=129,R=234,A=255)
// 		Translation=(Z=128)
// 		CastShadows=TRUE
// 		CastStaticShadows=TRUE
// 		CastDynamicShadows=TRUE
// 		bForceDynamicLight=FALSE
// 		bEnabled=FALSE
// 		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
// 	End Object
// 	PersistentFlameLight=FlameLight
	//Components.Add(FlameLight)

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_Tank ObjName=StaticMeshComponent_Tank Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		StaticMesh=StaticMesh'COG_City_Ephyra7.COG_Propane_Tank_MS_SMesh'
		bCastDynamicShadow=FALSE
		//bForceDirectLightMap=TRUE
		LightingChannels=(bInitialized=True,BSP=TRUE,Static=TRUE,Dynamic=True,CompositeDynamic=TRUE)
		Name="StaticMeshComponent_Tank"
		ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		RBChannel=RBCC_Default
	End Object
	Components.Add(StaticMeshComponent_Tank)
	CollisionComponent=StaticMeshComponent_Tank

	DestroyedStaticMesh=StaticMesh'COG_City_Ephyra7.COG_Propane_Tank_Busted'

	//Begin Object Class=CylinderComponent Name=CollisionCylinder
	//	CollisionRadius=+0020.000000
	//	CollisionHeight=+0060.000000
	//	BlockNonZeroExtent=true
	//	BlockZeroExtent=true
	//	BlockActors=true
	//	CollideActors=true
	//End Object
	//DestroyedCollisionCylinder=CollisionCylinder

	SubObjects(0)=(Mesh=StaticMeshComponent_Tank,DefaultHealth=100.000000)
	SubObjectHealths(0)=100.000000

	// explosion point light
    Begin Object Class=PointLightComponent Name=ExploLight0
		Radius=600.00000
		Brightness=50.000000
		LightColor=(B=83,G=129,R=234,A=255)
		Translation=(X=140,Y=0,Z=0)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
    End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_Explosive'
		MomentumTransferScale=20.f	// Scale momentum defined in DamageType

		ParticleEmitterTemplate=ParticleSystem'SP_GasStation_Effects.Gas_Pump_explo'
		ExplosionSound=SoundCue'Weapon_Grenade.Weapons.PropaneExplosion01Cue'
		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		FractureMeshRadius=220.0
		FracturePartVel=500.0

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
		ExploShakeInnerRadius=650
		ExploShakeOuterRadius=1500
	End Object
	ExplosionTemplate=ExploTemplate0

	FlameLightFlickerInterpSpeed=1.f
	FlameLightBrightnessFlickerRange=(X=-20.f,Y=40.f)

	bCanSpecialMeleeAttacked=FALSE
}




