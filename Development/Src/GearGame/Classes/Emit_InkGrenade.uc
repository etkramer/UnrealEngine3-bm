/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_InkGrenade extends Emit_SmokeGrenade
	dependson(GearPerMapColorConfig)
	config(Weapon);

var()			const config float	InkDamageRadius;
var() protected const config float	InkDamagePerSecond;
var() protected const float			SecondsPerInkDamageTick;

var protected transient float		NextCoughCheckTime;

var protected AIAvoidanceCylinder AvoidanceCylinder;

function SetupAvoidanceCylinder()
{
	if( AvoidanceCylinder == None )
	{
		AvoidanceCylinder = Spawn(class'AIAvoidanceCylinder',self,,Location,,,TRUE);
		if(AvoidanceCylinder!=none)
		{
			AvoidanceCylinder.SetCylinderSize(InkDamageRadius,InkDamageRadius*2.0f);
			AvoidanceCylinder.SetEnabled(true);
		}
	}
}

simulated function PostBeginPlay()
{
	local GearMapSpecificInfo GSI;
	local EffectParam EP;
	local VectorParam VP;
	local FloatParam FP;

	super.PostBeginPlay();

	if (WorldInfo.NetMode != NM_Client)
	{
		// set up damage timer
		SetTimer( SecondsPerInkDamageTick, TRUE, nameof(ApplyGasDamage) );

		SetupAvoidanceCylinder();
	}
	
	// now check to see if this is using the per map values
	GSI = GearMapSpecificInfo(WorldInfo.GetMapInfo());
	if( GSI != none )
	{
		if( GSI.ColorConfig != none )
		{
			// look over all of the effects
			foreach GSI.ColorConfig.EffectParams( EP )
			{
				// if there are any for my damage type
				if( class'GDT_InkGrenade' == EP.DamageType )
				{
					// do all of the setting
					foreach EP.VectorParams( VP )
					{
						ParticleSystemComponent.SetVectorParameter( VP.Name, VP.Value );
						//`log( "Setting Vect:" @ VP.Name @ VP.Value );
					}

					foreach EP.FloatParams( FP )
					{
						ParticleSystemComponent.SetFloatParameter( FP.Name, FP.Value );
						//`log( "Setting Float:" @ FP.Name @ FP.Value );
					}
				}
			}
		}
	}
}

simulated event Destroyed()
{
	Super.Destroyed();
	if(AvoidanceCylinder != none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder=none;
	}
}


// overloaded to call super.TurnOffSmokeEmitter() since we don't want to kill the damage
simulated function CheckPerformance()
{
	if (WorldInfo.bDropDetail)
	{
		`log(self@"disabling emitter due to performance");
		// turn off emitter, let particles dissipate
		ParticleSystemComponent.DeactivateSystem();
	
		// turn off sound
		TurnOffSmokeSpewSound();
	}
	else
	{
		SetTimer(0.5f,FALSE,nameof(CheckPerformance));
	}
}


/** Sets duration of smoke emission.  Does not include any fadeout/dissipation time. */
simulated function SetEmissionDuration(float Duration)
{
	Super.SetEmissionDuration( Duration );
	ParticleSystemComponent.SetFloatParameter( 'InkGrenadeDuration', Duration );
}

simulated protected function TurnOffSmokeEmitter()
{
	super.TurnOffSmokeEmitter();
	ClearTimer('ApplyGasDamage');
}

private function ApplyGasDamage()
{
	local GearPawn GP;
	local float DamageThisTick;
	local bool bCoughCheck, bDoDamage;

	DamageThisTick = InkDamagePerSecond * SecondsPerInkDamageTick;

	if (NextCoughCheckTime < WorldInfo.TimeSeconds)
	{
		bCoughCheck = TRUE;
		NextCoughCheckTime = WorldInfo.TimeSeconds + RandRange(CoughCheckInterval.X, CoughCheckInterval.Y);
	}

	//DrawDebugSphere(Location, InkDamageRadius, 10, 255, 255, 255, FALSE);
	// Note: not doing colliding actors, since we don't care the origin of the damage
	foreach CollidingActors(class'GearPawn', GP, InkDamageRadius, Location)
	{
		if ( IgnoringDamagePawn != GP )
		{
			bDoDamage = TRUE;
			if (GP.IsDBNO())
			{
				// don't hurt DBNO humans in Horde
				if (WorldInfo.GRI.IsCoopMultiplayerGame() && GP.IsHumanControlled())
				{
					bDoDamage = FALSE;
				}
				// don't execute DBNO COGs in Campaign
				else if (!WorldInfo.GRI.IsMultiplayerGame() && GP.IsA('GearPawn_COGGear'))
				{
					bDoDamage = FALSE;
				}
			}
			if (bDoDamage)
			{
				// ink hurts everyone, even friendlies at the moment
				GP.TakeRadiusDamage(InstigatorController, DamageThisTick, InkDamageRadius, GetDamageType(), 1.f, Location, FALSE, self);
			}
		}

		// cough
		if (bCoughCheck)
		{
			GP.SoundGroup.PlayEffort(GP, GearEffort_InInkCough);
		}
	}
}

/** Returns the damagetype to use */
private function class<DamageType> GetDamageType()
{
	if ( bTriggeredByMartyr )
	{
		return class'GDT_InkMartyr';
	}
	else if ( bTriggeredBySticky )
	{
		return class'GDT_InkSticky';
	}
	else
	{
		return class'GDT_InkGrenade';
	}
}

defaultproperties
{
	Begin Object Name=ParticleSystemComponent0
		TranslucencySortPriority=2.0f
		Template=ParticleSystem'Weap_Ink_Grenade.Effects.P_Weap_Ink_Grenade'
	End Object

	FogVolumeClass=class'GearFogVolume_InkGrenade'

	TickGroup=TG_PreAsyncWork

	SmokeSpewStartCue=None
	SmokeSpewFinishCue=None
	SmokeSpewLoopCue=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeSprayingLoopCue'

	bDoCoughChecks=FALSE			// cough checking handled by hand above
	CoughCheckInterval=(X=1.f,Y=2.f)
	SecondsPerInkDamageTick=0.5f
}
