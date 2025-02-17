/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTPickupFactory extends PickupFactory
	abstract
	native
	nativereplication
	hidecategories(Display,Collision,PickupFactory);

var		bool			bRotatingPickup;	// if true, the pickup mesh rotates
var		float			YawRotationRate;
var		Controller		TeamOwner[4];		// AI controller currently going after this pickup (for team coordination)

/**  pickup base mesh */
var transient StaticMeshComponent BaseMesh;

/** Used to pulse the emissive on the base */
var MaterialInstanceConstant BaseMaterialInstance;

/** The baseline material colors */
var LinearColor BaseBrightEmissive;		// When the pickup is on the base
var LinearColor	BaseDimEmissive;		// When the pickup isn't on the base

/** When set to true, this base will begin pulsing it's emissive */
var repnotify bool bPulseBase;

/** How fast does the base pulse */
var float BasePulseRate;

/** How much time left in the current pulse */
var float BasePulseTime;

/** This pickup base will begin pulsing when there are PulseThreshold seconds left
    before respawn. */
var float PulseThreshold;

/** The TargetEmissive Color */
var LinearColor BaseTargetEmissive;
var LinearColor BaseEmissive;

/** This material instance parameter for adjusting the emissive */
var name BaseMaterialParamName;

// Used when floating
var	bool	bFloatingPickup;	// if true, the pickup mesh floats (bobs) slightly
var(floating)	bool	bRandomStart;		// if true, this pickup will start at a random height
var				float 	BobTimer;			// Tracks the bob time.  Used to create the position
var				float 	BobOffset;			// How far to bob.  It will go from +/- this number
var				float 	BobSpeed;			// How fast should it bob
var				float	BobBaseOffset;		// The base offset (Translation.Y) cached

/** sound played when the pickup becomes available */
var SoundCue RespawnSound;

/** Sound played on base while available to pickup*/
var AudioComponent PickupReadySound;

/** The pickup's light environment */
var DynamicLightEnvironmentComponent LightEnvironment;

/** In disabled state */
var repnotify bool bIsDisabled;

/** whether this pickup is updating */
var bool bUpdatingPickup;

/** Translation of pivot point */
var vector PivotTranslation;

/** Name used for the stats system */
var name PickupStatName;

/**
 * This determines whether this health pickup fades in or not.
 *
 * NOTE:  need to move this up to the highest pickup factory so all items will fade in
 **/
var bool bDoVisibilityFadeIn;
var name VisibilityParamName;

/** holds the pickups material so parameters can be set **/
var MaterialInstanceConstant MIC_Visibility;
/** holds the pickups 2nd material so parameters can be set **/
var MaterialInstanceConstant MIC_VisibilitySecondMaterial;

var repnotify bool bIsRespawning;

var ParticleSystemComponent Glow; // the glowing effect that comes from the base on spawn
var name GlowEmissiveParam;

var bool bHasLocationSpeech;

var Array<SoundNodeWave> LocationSpeech;

var float LastSeekNotificationTime;

var	ForceFeedbackWaveform	PickUpWaveForm;


cpptext
{
	virtual void TickSpecial( FLOAT DeltaSeconds );
	virtual void PostEditMove(UBOOL bFinished);
	virtual void Spawned();
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	if ( bNetDirty && (Role == Role_Authority) )
		bPulseBase,bIsRespawning;
	if (bNetInitial && ROLE==ROLE_Authority )
		bIsDisabled;
}

simulated function PostBeginPlay()
{
	if ( bIsDisabled )
	{
		return;
	}

	Super.PostBeginPlay();

	bUpdatingPickup = (WorldInfo.NetMode != NM_DedicatedServer) && (bRotatingPickup || bFloatingPickup);
	if(Glow != none)
	{
		Glow.SetFloatParameter('LightStrength',0.0f);
		Glow.SetActive(true);
	}

	// Grab a reference to the material so we can adjust the parameter.  This adjustment occurs natively
	// in TickSpecial()
	if ( WorldInfo.NetMode != NM_DedicatedServer && BaseMesh != none && BaseMaterialParamName != '' )
	{
		BaseMaterialInstance = BaseMesh.CreateAndSetMaterialInstanceConstant(0);

		BaseTargetEmissive = bPickupHidden ? BaseDimEmissive : BaseBrightEmissive;
		BaseEmissive = BaseTargetEmissive;

		BaseMaterialInstance.SetVectorParameterValue( BaseMaterialParamName, BaseEmissive );
	}

	if(!bIsSuperItem && PickupReadySound != none)
	{
		PickupReadySound.Play();
	}

	// increase pickup radius on console
	if ( WorldInfo.bUseConsoleInput )
	{
		SetCollisionSize(1.5*CylinderComponent.CollisionRadius, CylinderComponent.CollisionHeight);
	}
}

simulated function SetResOut()
{
	if (bDoVisibilityFadeIn && MIC_Visibility != None)
	{
		MIC_Visibility.SetScalarParameterValue(VisibilityParamName, 1.f);
	}
}

simulated function DisablePickup()
{
	bIsDisabled = true;
	GotoState('Disabled');
}

simulated function ShutDown()
{
	DisablePickup();
}

/**
  * Look for changes in bPulseBase or bPickupHidden and set the TargetEmissive accordingly
  */
simulated function ReplicatedEvent(name VarName)
{
	if ( VarName == 'bIsDisabled' )
	{
		Global.SetInitialState();
	}
	else
	{
		if(VarName == 'bPickupHidden' && bPickupHidden)
		{
			setResOut();
		}
		if ( VarName == 'bPulseBase' || VarName == 'bPickupHidden' )
		{
			StartPulse((bPickupHidden && !bPulseBase) ? BaseDimEmissive : BaseBrightEmissive);
		}
		if(VarName == 'bIsRespawning' )
		{
			if(bIsRespawning)
			{
				RespawnEffect();
			}
		}
		Super.ReplicatedEvent(VarName);
	}
}

/**
  * Returns true if Bot should wait for me
  */
function bool ShouldCamp(UTBot B, float MaxWait)
{
	return ( ReadyToPickup(MaxWait) && (B.RatePickup(self, InventoryType) > 0) && !ReadyToPickup(0) );
}

/* UpdateHUD()
Called for the HUD of the player that picked it up
*/
simulated static function UpdateHUD(UTHUD H)
{
	H.LastPickupTime = H.WorldInfo.TimeSeconds;
}

simulated function RespawnEffect()
{
	bIsRespawning = true;
	PlaySound(RespawnSound, true);
	if (PickupMesh != None)
	{
		PickupMesh.SetHidden(false);
	}
}

/* epic ===============================================
* ::StopsProjectile()
*
* returns true if Projectiles should call ProcessTouch() when they touch this actor
*/
simulated function bool StopsProjectile(Projectile P)
{
	local Actor HitActor;
	local vector HitNormal, HitLocation;

	if ( (P.CylinderComponent.CollisionRadius > 0) || (P.CylinderComponent.CollisionHeight > 0) )
	{
		// only collide if zero extent trace would also collide
		HitActor = Trace(HitLocation, HitNormal, P.Location, P.Location - 100*Normal(P.Velocity), true, vect(0, 0, 0));
		if ( HitActor != self )
			return false;
	}

	return bProjTarget || bBlockActors;
}


/**
 * Pulse to the Brightest point
 */
simulated function StartPulse(LinearColor TargetEmissive)
{
	if ( WorldInfo.NetMode != NM_DedicatedServer && BaseMesh != none && BaseMaterialParamName != '' )
	{
		BaseTargetEmissive = TargetEmissive;
		if ( BasePulseTime <= 0.0 )
		{
			BasePulseTime = BasePulseRate;
		}
		else
		{
			BasePulseTime = BasePulseRate - BasePulseTime;
		}
	}
}

auto state Pickup
{
	function BeginState(Name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		bPulseBase = false;
		StartPulse( BaseBrightEmissive );
    }
	simulated function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);
		bIsRespawning = false;
		SetResOut();
	}

}

State Sleeping
{
	function BeginState(Name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		bPulseBase = false;
		StartPulse( BaseDimEmissive );
		SetTimer( GetReSpawnTime() - PulseThreshold, false, 'PulseThresholdMet');
	}

	function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);
		ClearTimer('PulseThresholdMet');
	}

	function PulseThresholdMet()
	{
		bPulseBase = true;
		StartPulse(BaseBrightEmissive);
	}
}

simulated function SetPickupMesh()
{
	Super.SetPickupMesh();

	InitPickupMeshEffects();

}

/**
 * Stats
 */
function name GetPickupStatName()
{
/*	local UTItemPickupFactory myType;

	//TEMP
	MyType = UTItemPickupFactory(self);
	if (MyType != none)
	{
		//A non inventory things (like Ammo/Health/Powerup)
		`log("Pickedup:"@class.name);
	}
	else
	{
		//Actual inventory (like weapons)
		`log("Pickedup Inv:"@InventoryType.name);
	}
	//ENDTEMP
*/
	if ( Default.PickupStatName != '' )
	{
		//`log("Returning:"@Default.PickupStatName);
		return Default.PickupStatName;
	}

	return 'INVALID_PICKUPSTAT';
}

/** split out from SetPickupMesh() for subclasses that don't want to do the base PickupFactory implementation */
simulated event InitPickupMeshEffects()
{
	if ( PickupMesh != none )
	{
		PickupMesh.SetLightEnvironment(LightEnvironment);

		// Create a material instance for the pickup
		if (bDoVisibilityFadeIn && MeshComponent(PickupMesh) != None)
		{
			MIC_Visibility = MeshComponent(PickupMesh).CreateAndSetMaterialInstanceConstant(0);
			MIC_Visibility.SetScalarParameterValue(VisibilityParamName, bIsSuperItem ? 1.f : 0.f);
		}

		BobBaseOffset = PickupMesh.Translation.Y;
		if ( bRandomStart )
		{
			BobTimer = 1000 * frand();
		}
	}
}

simulated function SetPickupVisible()
{
	if(PickupReadySound != none)
	{
		PickupReadySound.Play();
	}

	if(Glow != none)
	{
		Glow.SetFloatParameter('LightStrength', 1.0f);
	}

	Super.SetPickupVisible();
}

simulated function SetPickupHidden()
{
	if(PickupReadySound != none)
	{
		PickupReadySound.Stop();
	}

	if(Glow != none)
	{
		Glow.SetFloatParameter('LightStrength',0.0f);
	}

	// need to hide also for low end video cards
	Super.SetPickupHidden();
}

simulated event SetInitialState()
{
	bScriptInitialized = true;

	if ( bIsDisabled )
	{
		GotoState('Disabled');
	}
	else
	{
		super.SetInitialState();
	}
}

function PickedUpBy(Pawn P)
{
	local UTBot B;
	local PlayerController PC;

	Super.PickedUpBy(P);

	// clear bot NoVehicleGoal so if it got out of vehicle to get a pickup it will consider getting back in now
	B = UTBot(P.Controller);
	if (B != None && B.NoVehicleGoal == self)
	{
		B.NoVehicleGoal = None;
	}

	PC = PlayerController(P.Controller);
	if(PC != None)
	{
		PC.ClientPlayForceFeedbackWaveform(PickUpWaveForm);
	}
}

State Disabled
{
	function float BotDesireability(Pawn P, Controller C)
	{
		return 0;
	}
}

defaultproperties
{
	Components.Remove(Sprite)
	Components.Remove(Sprite2)
	Components.Remove(Arrow)
	GoodSprite=None
	BadSprite=None

	// setting bMovable=FALSE will break pickups and powerups that are on movers.
	// I guess once we get the LightEnvironment brightness issues worked out and if this is needed maybe turn this on?
	// will need to look at all maps and change the defaults for the pickups that move tho.
	bMovable=FALSE
    bStatic=FALSE

	RespawnSound=SoundCue'A_Pickups.Generic.Cue.A_Pickups_Generic_ItemRespawn_Cue'

	YawRotationRate=32768

	Begin Object NAME=CollisionCylinder
		CollisionRadius=+00040.000000
		CollisionHeight=+00044.000000
		CollideActors=true
	End Object

	// define here as lot of sub classes which have moving parts will utilize this
 	Begin Object Class=DynamicLightEnvironmentComponent Name=PickupLightEnvironment
 	    bDynamic=FALSE
 	    bCastShadows=FALSE
		AmbientGlow=(R=0.3f,G=0.3f,B=0.3f,A=1.0f)
 	End Object
  	LightEnvironment=PickupLightEnvironment
  	Components.Add(PickupLightEnvironment)

	BasePulseRate=0.5
	PulseThreshold=5.0
	BaseMaterialParamName=BaseEmissiveControl

	Begin Object Class=StaticMeshComponent Name=BaseMeshComp
		CastShadow=FALSE
		bCastDynamicShadow=FALSE
		bAcceptsLights=TRUE
		bForceDirectLightMap=TRUE
		LightingChannels=(BSP=TRUE,Dynamic=FALSE,Static=TRUE,CompositeDynamic=FALSE)
		LightEnvironment=PickupLightEnvironment

		CollideActors=false
		MaxDrawDistance=7000
		bUseAsOccluder=FALSE
	End Object
	BaseMesh=BaseMeshComp
	Components.Add(BaseMeshComp)

	GlowEmissiveParam=LightStrength
	bDoVisibilityFadeIn=TRUE
	VisibilityParamName=ResIn

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformPickUp
		Samples(0)=(LeftAmplitude=80,RightAmplitude=30,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearIncreasing,Duration=0.20)
	End Object
	PickUpWaveForm=ForceFeedbackWaveformPickUp
}
