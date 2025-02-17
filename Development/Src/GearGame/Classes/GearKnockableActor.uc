/** base class of Actors that fall over when hit with damage or by a vehicle */
class GearKnockableActor extends Actor
	abstract
	placeable;

var() editconst MeshComponent Mesh;
var() editconst LightEnvironmentComponent LightEnvironment;
var() editconst ParticleSystemComponent HitEffect;
var() SoundCue HitSound;
/** played one second after being hit */
var() SoundCue DelayedHitSound;
/** size of impulse to cause in the direction of the hit */
var() float ImpulseStrength;
/** how long to keep around after falling out of sight of the player */
var() float FallenLifetime;
/** whether this object can be knocked over by weapon damage */
var() bool bAffectedByWeapons;
/** whether we should play a camera shake when a human player crashes into this Actor */
var() bool bPlayerCollisionShake;
/** the parameters for any camera shaking */
var() ScreenShakeStruct	ShakeParams;

/** knocks down the tree, applying the appropriate starting impulse from the inputs provided */
simulated function KnockDown(vector HitLocation, vector HitDirection)
{
	Velocity = HitDirection * ImpulseStrength;
	GotoState('KnockedDown');
}

simulated event TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	if (bAffectedByWeapons)
	{
		KnockDown(HitLocation, Normal(Momentum));
	}
}

simulated event Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	local GearPC PC;

	if (Other.Physics == PHYS_RigidBody || Other.IsA('InterpActor') || Other.IsA('SkeletalMeshActor'))
	{
		KnockDown(HitLocation, Normal(Other.Velocity));

		foreach LocalPlayerControllers(class'GearPC', PC)
		{
			if (PC.ViewTarget == Other || PC.ViewTarget.Base == Other)
			{
				PC.ClientPlayCameraShake(ShakeParams, true);
			}
		}
	}
}

simulated state KnockedDown
{
	simulated event BeginState(name PreviousStateName)
	{
		Mesh.SetBlockRigidBody(true);
		SetPhysics(PHYS_RigidBody);
		SetCollision(false);
		HitEffect.ActivateSystem();
		PlaySound(HitSound, true);
		bStasis = false;
	}

	simulated event Tick(float DeltaTime)
	{
		if (WorldInfo.TimeSeconds - LastRenderTime > FallenLifetime)
		{
			GotoState('Dead');
		}
	}
Begin:
	Sleep(1.0);
	PlaySound(DelayedHitSound, true);
}

state Dead
{
	simulated event BeginState(name PreviousStateName)
	{
		SetPhysics(PHYS_None);
		Mesh.SetBlockRigidBody(false);
		SetHidden(true);
		bStasis = true;
	}
}

defaultproperties
{
	Begin Object Class=ParticleSystemComponent Name=HitFX
		bAutoActivate=false
		SecondsBeforeInactive=0.0
	End Object
	Components.Add(HitFX)
	HitEffect=HitFX

	TickGroup=TG_PostAsyncWork
	bCollideActors=true
	bBlockActors=true
	bCollideWorld=false
	bProjTarget=true
	FallenLifetime=10.0
	ImpulseStrength=1200.0
	bAffectedByWeapons=true
	bNoDelete=true
	bStasis=true

	bPlayerCollisionShake=true
	ShakeParams=(RotAmplitude=(X=150.000000,Y=75.000000,Z=5.000000),RotFrequency=(X=100.000000,Y=50.000000,Z=10.000000),RotParam=(X=ESP_OffsetZero,Y=ESP_OffsetZero,Z=ESP_OffsetZero),LocAmplitude=(X=2.5000000,Y=1.500000,Z=4.000000),LocFrequency=(X=25.000000,Y=10.000000,Z=35.000000),LocParam=(X=ESP_OffsetZero,Y=ESP_OffsetZero,Z=ESP_OffsetZero),FOVAmplitude=0.500000,FOVFrequency=0.500000,ShakeName="Explosive_hit",bOverrideTargetingDampening=True)
}
