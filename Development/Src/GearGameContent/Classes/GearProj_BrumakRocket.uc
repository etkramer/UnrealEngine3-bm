/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_BrumakRocket extends GearProj_ExplosiveBase
	config(Weapon);

/** Acceleration magnitude. By default, acceleration is in the same direction as velocity */
var float AccelRate;
var float SpiralForceMag;
var float InwardForceMag;
var float ForwardForceMag;
var float DesiredDistanceToAxis;
var float DesiredDistanceDecayRate;
var float InwardForceMagGrowthRate;

var float CurSpiralForceMag;
var float CurInwardForceMag;
var float CurForwardForceMag;

var float DT;

var vector AxisOrigin;
var vector AxisDir;

var vector Target, SecondTarget, InitialDir;
var float KillRange;
var bool bFinalTarget;
var float SwitchTargetTime;

var ParticleSystem ProjIgnitedFlightTemplate;

var float IgniteTime;
var repnotify vector InitialAcceleration;

/** if hit a GearPawn do damage a damage bonus! **/
var bool bDoHitPawnDamageBonus;


replication
{
	if( bNetInitial && Role == ROLE_Authority )
		IgniteTime, InitialAcceleration, Target, SecondTarget, SwitchTargetTime, bFinalTarget;
}

function SetupProjectile( GearWeapon W, Controller C, Pawn Brumak, Vector StartLocation, Rotator AimRot, float RocketAccelRate, optional bool bStraightRocket )
{
	local GearAI_BrumakDriver AI;
	local Vector AccelAdjustment, CrossVec;
	local Vector LockedTargetVect;
	local float	 Dist, TravelTime, VelBoost;
	local Actor	 FireTarget;

	AI = GearAI_BrumakDriver(C);
	if( AI != None )
	{
		FireTarget	 = AI.GetFireTarget();
		Dist		 = VSize(FireTarget.Location - StartLocation);
		TravelTime	 = Dist / Speed;
		Target		 = FireTarget.Location + (FireTarget.Velocity * TravelTime * 0.1f);
		Target.X	+= RandRange( -128.f, 128.f );
		Target.Y	+= RandRange( -128.f, 128.f );
	}
	LockedTargetVect = Target;

	VelBoost = 700.f;
	
	// Set their initial velocity
	CrossVec	= Vect(0,0,1);
	CrossVec   *= (FRand()>0.5f ? 1 : -1);

	AccelAdjustment = (vector(AimRot) Cross CrossVec) * RocketAccelRate;
	AccelAdjustment.Z += ((300.0 * FRand()) - 100.0) * ( FRand() * 2.f);

	KillRange			= 1024;
	bFinalTarget		= FALSE;
	SecondTarget		= LockedTargetVect;
	SwitchTargetTime	= 0.5;
	
	ArmMissile(AccelAdjustment, Vector(AimRot) * (VelBoost + VSize(Brumak.Velocity)) );
}

simulated function ReplicatedEvent(name VarName)
{
	if( VarName == 'InitialAcceleration' )
	{
		SetTimer( IgniteTime , FALSE, nameof(Ignite) );
		Acceleration = InitialAcceleration;
	}
}

function Init(vector Direction);

function ArmMissile(vector InitAccel, vector InitVelocity)
{
	Velocity = InitVelocity;
	InitialAcceleration = InitAccel;

	IgniteTime = (FRand() * 0.25) + 0.35;

	// Seed the acceleration/timer on a server
	ReplicatedEvent('InitialAcceleration');
}


simulated function ChangeTarget()
{
	Target = SecondTarget;
	bFinalTarget = TRUE;
	SwitchTargetTime = 0;
}

simulated function Ignite()
{
	local float Dist, TravelTime;

	SetCollision(TRUE, TRUE);
	if(!bFinalTarget)
	{
		// Look for a pending collision

		Dist = VSize(Target - Location);
		if ( Dist < KillRange )
		{
			ChangeTarget();
		}
		else if (!IsZero(Velocity))	// No pending collision, look for a collision before the switch
		{
			TravelTime = (Dist / VSize(Velocity));
			if ( TravelTime < SwitchTargetTime  )
			{
				// 3/4 life the travel time and then switch
				SwitchTargetTime = TravelTime * 0.75;
			}
		}
		GotoState('Spiraling');
	}
	else
	{
		GotoState( 'Homing' );
	}
	
}


state Spiraling
{
	simulated function ChangeTarget()
	{
		Global.ChangeTarget();
		BeginState('none');
	}

	simulated function BeginState(name PreviousStateName)
	{
		CurSpiralForceMag = SpiralForceMag;
		CurInwardForceMag = InwardForceMag;
		CurForwardForceMag = ForwardForceMag;

		AxisOrigin = Location;
		AxisDir =  Normal(Target - AxisOrigin);

		Velocity = AxisDir * Speed;

		SetTimer(DT, TRUE);
	}

	simulated function Timer()
	{
		local vector ParallelComponent, PerpendicularComponent, NormalizedPerpendicularComponent;
		local vector SpiralForce, InwardForce, ForwardForce;
		local float InwardForceScale;

		// Add code to switch directions

		// Update the inward force magnitude.
		CurInwardForceMag += InwardForceMagGrowthRate * DT;

		ParallelComponent = ((Location - AxisOrigin) dot AxisDir) * AxisDir;
		PerpendicularComponent = (Location - AxisOrigin) - ParallelComponent;
		NormalizedPerpendicularComponent = Normal(PerpendicularComponent);

		InwardForceScale = VSize(PerpendicularComponent) - DesiredDistanceToAxis;

		SpiralForce = CurSpiralForceMag * Normal(AxisDir cross NormalizedPerpendicularComponent);
		InwardForce = -CurInwardForceMag * InwardForceScale * NormalizedPerpendicularComponent;
		ForwardForce = CurForwardForceMag * AxisDir;

		Acceleration = SpiralForce + InwardForce + ForwardForce;

		DesiredDistanceToAxis -= DesiredDistanceDecayRate * DT;
		DesiredDistanceToAxis = FMax(DesiredDistanceToAxis, 0.0);

		// Update rocket so it faces in the direction its going.
		SetRotation(rotator(Velocity));

		// Check to see if we should switch to Home in Mode
		if (!bFinalTarget)
		{
			SwitchTargetTime -= DT;
			if (SwitchTargetTime <= 0)
			{
				ChangeTarget();
			}
		}

		if (VSize(Location - Target) <= KillRange)
		{
			GotoState('Homing');
		}
	}
}

state Homing
{
	simulated function Timer()
	{
		local vector ForceDir;
		local float VelMag;

		if (InitialDir == vect(0,0,0))
		{
			InitialDir = Normal(Velocity);
		}

		Acceleration = vect(0,0,0);
		Super.Timer();

		// do normal guidance to target.
		ForceDir = Normal(Target - Location);

		if ((ForceDir Dot InitialDir) > 0.f)
		{
			VelMag = VSize(Velocity);

			ForceDir = Normal(ForceDir * 0.9 * VelMag + Velocity);
			Velocity =  VelMag * ForceDir;
			Acceleration += 5 * ForceDir;
		}
		// Update rocket so it faces in the direction its going.
		SetRotation(rotator(Velocity));
	}

	simulated function BeginState(name PreviousStateName)
	{
		Velocity *= 0.6;
		SetTimer(0.1, TRUE);
	}
}


simulated function Landed(vector HitNormal, Actor FloorActor)
{
	Internal_Explode( Location, HitNormal, FloorActor );
}

simulated function HitWall(vector HitNormal, actor Wall, PrimitiveComponent WallComp)
{
	Internal_Explode( Location, HitNormal, Wall );
}

simulated function ProcessTouch(Actor Other, vector HitLocation, vector HitNormal)
{
	if( Other != Instigator &&
		Other != Instigator.Base &&
		(!Other.IsA('Projectile') || Other.bProjTarget))
	{
		Internal_Explode(HitLocation, vect(0,0,1), Other );
	}
}

simulated function Internal_Explode( Vector HitLocation, Vector HitNormal, Actor HitActor )
{
	Super.TriggerExplosion( HitLocation, HitNormal, HitActor );
}


defaultproperties
{
	Speed=350.0
	AccelRate=750
	MaxSpeed=4000.0
	SpiralForceMag=600.0
	InwardForceMag=30.0
	ForwardForceMag=15000.0
	DesiredDistanceToAxis=150.0
	DesiredDistanceDecayRate=500.0
	InwardForceMagGrowthRate=0.0
	DT=0.1
	MomentumTransfer=1.f	// Scale momentum defined in DamageType

	MyDamageType=class'GDT_BrumakCannon'

	RemoteRole=ROLE_SimulatedProxy

	Lifespan=10.0
	bNetInitialRotation=TRUE

	RotationRate=(Roll=50000)
	DesiredRotation=(Roll=900000)
	bCollideWorld=TRUE
	bCollideActors=TRUE
	bCollideComplex=TRUE	// Ignore simple collision on StaticMeshes, and collide per poly
	bNetTemporary=FALSE
	KillRange=512

	bRotationFollowsVelocity=TRUE

	InFlightSoundTemplate=SoundCue'Weapon_Boomer.Firing.BoomerInAirLoopCue'
	TrailTemplate=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Rocket_Trail'

	// brumak rocket is really dark and shot in dark levels so lighten it up some so it doesn't look unlit
	Begin Object Name=MyLightEnvironment
	    AmbientGlow=(R=0.3,G=0.3,B=0.3,A=1.0)
	End Object

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
    	StaticMesh=StaticMesh'Locust_Brumak.Locust_Brumak_Rocket_SM'
		HiddenGame=false
		CastShadow=false
		CollideActors=false
		BlockActors=false
		BlockZeroExtent=false
		BlockNonZeroExtent=false
		BlockRigidBody=false
		bDisableAllRigidBody=TRUE
		Scale=2.25
		Rotation=(Yaw=-16384)
    End Object
	Components.Add(StaticMeshComponent0)

	// remove light environment
	Components.Remove(MyLightEnvironment)
	ProjLightEnvironment=None



	// explosion point light
	Begin Object Class=PointLightComponent Name=ExploLight0
		Radius=400.000000
		Brightness=500.000000
		LightColor=(B=35,G=185,R=255,A=255)
		Translation=(X=16)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
	End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_BrumakCannon'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType

		ParticleEmitterTemplate=ParticleSystem'Locust_Boomshot.Effects.P_Boomshot_Explo_Main'
		ExplosionSound=SoundCue'Weapon_Boomer.Impacts.BoomerExplosionCue'
		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
		ExploShakeInnerRadius=650
		ExploShakeOuterRadius=1500

		FractureMeshRadius=220.0
		FracturePartVel=500.0

		bAllowPerMaterialFX=TRUE
	End Object
	ExplosionTemplate=ExploTemplate0
}
