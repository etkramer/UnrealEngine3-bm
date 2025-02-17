/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearExplosionActor extends Actor
	notplaceable
	config(Weapon)
	dependson(GearMapSpecificInfo)
	native(Weapon);

/** Logging pre-processor macros */
/** GoW global macros */

/** True if explosion has occurred. */
var protected transient bool bHasExploded;

/** The actual light used for the explosion. */
var protected transient PointLightComponent ExplosionLight;

/** Temp data for light fading. */
var protected transient float LightFadeTime;
var protected transient float LightInitialBrightness;

/** Temp reference to the explosion template, used for delayed damage */
var GearExplosion ExplosionTemplate;

/** Whether or not this Explosion is from something that has an ActiveReload bonus that is active **/
var bool bActiveReloadBonusActive;

/** Used to push physics when explosion goes off. */
var protected RB_RadialImpulseComponent	RadialImpulseComponent;

/** player responsible for damage */
var Controller InstigatorController;

/** This the saved off hit actor and location from the GetPhysicalMaterial trace so we can see if it is a FluidSurfaceActor and then apply some forces to it **/
var Actor HitActorFromPhysMaterialTrace;
var vector HitLocationFromPhysMaterialTrace;

event PreBeginPlay()
{
	Super.PreBeginPlay();

	if (Instigator != None && InstigatorController == None)
	{
		InstigatorController = Instigator.Controller;
	}
}

/**
 * Internal. Tries to find a physical material for the surface the explosion occurred upon.
 * @note: It sucks doing an extra trace here.  We could conceivably pass the physical material info around
 * by changing the lower level physics code (e.g. processHitWall), but that's a big engine-level change.
 */
simulated protected function PhysicalMaterial GetPhysicalMaterial()
{
	local PhysicalMaterial Retval;
	local vector TraceStart, TraceDest, OutHitNorm, ExploNormal;
	local TraceHitInfo OutHitInfo;

	// here we have to do an additional trace shooting straight down to see if we are under water.
	TraceStart = Location + (vect(0,0,1) * 256.f);
	TraceDest = Location - (vect(0,0,1) * 16.f);

	HitActorFromPhysMaterialTrace = Trace(HitLocationFromPhysMaterialTrace, OutHitNorm, TraceDest, TraceStart, TRUE, vect(0,0,0), OutHitInfo, TRACEFLAG_Bullet|TRACEFLAG_PhysicsVolumes);
	//DrawDebugLine( TraceStart, TraceDest, 0, 255, 0, TRUE);
	//`log("EXPLOSION SURFACE:"@HitActorFromPhysMaterialTrace);
	//DrawDebugCoordinateSystem( TraceStart, Rotation, 10.0f, TRUE );

	if( FluidSurfaceActor(HitActorFromPhysMaterialTrace) != None )
	{
		Retval = OutHitInfo.PhysMaterial;
		return Retval;
	}

	if( GearWaterVolume(HitActorFromPhysMaterialTrace) != None )
	{
		Retval = GearWaterVolume(HitActorFromPhysMaterialTrace).WaterPhysMaterial;
		return Retval;
	}

	ExploNormal = vector(Rotation);
	TraceStart = Location + (ExploNormal * 8.f);
	TraceDest = TraceStart - (ExploNormal * 64.f);

	HitActorFromPhysMaterialTrace = Trace(HitLocationFromPhysMaterialTrace, OutHitNorm, TraceDest, TraceStart, TRUE, vect(0,0,0), OutHitInfo, TRACEFLAG_Bullet);
	//DrawDebugLine( TraceStart, TraceDest, 0, 255, 0, TRUE);
	//DrawDebugCoordinateSystem( TraceStart, Rotation, 10.0f, TRUE );

	if( HitActorFromPhysMaterialTrace != None )
	{
		Retval = OutHitInfo.PhysMaterial;
	}

	if( Retval == None )
	{
		Retval = Physicalmaterial'GearPhysMats.DefaultPhysMat';
	}

	return Retval;
}

/** Does damage modeling and application for explosions */
protected final simulated function HurtExplosion
(
	float				BaseDamage,
	float				DamageRadius,
	float				DamageFalloffExp,
	class<DamageType>	DamageType,
	float				MomentumScale,
	vector				ExploOrigin,
	Actor		IgnoredActor,
	class<Actor> ActorClassToIgnoreForDamage,
	Controller InstigatedByController,
	bool       bDoFullDamage
)
{
	local Actor		Victim, HitActor;
	local vector	HitL, HitN, Dir;
	local bool		bDamageBlocked;
	local float		ColRadius, ColHeight;
	local float		DamageScale, Dist;
	local GearPawn	HitGearPawn, Hostage, Kidnapper;
	local array<Actor> VictimsList;

	local Box BBox;
	local vector BBoxCenter;

	local Controller ModInstigator;
	local Pawn VictimPawn;
	local class<DamageType> ModDamageType;

	//`log(self@`showvar(Instigator)@`showvar(InstigatedByController)@`showvar(InstigatorController));

	// GearPawn hit by explosion projectile.
	HitGearPawn = GearPawn(ExplosionTemplate.HitActor);
	// If we've hit a hostage with an explosive projectile, then spare kidnapper and kill hostage.
	if( HitGearPawn != None && HitGearPawn.IsAHostage() )
	{
		Hostage = HitGearPawn;
		Kidnapper = Hostage.InteractionPawn;
	}

//	FlushPersistentDebugLines();
//	DrawDebugSphere(ExploOrigin, DamageRadius, 12, 255, 0, 0, TRUE);

	//debug
	`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "BaseDamage:" @ BaseDamage @ "DamageRadius:" @ DamageRadius @ "DamageFalloffExp:" @ DamageFalloffExp @ "IgnoredActor:" @ IgnoredActor @ "HitActor:" @ ExplosionTemplate.HitActor @ "Hostage:" @ Hostage @ "Kidnapper:" @ Kidnapper, bDebug);
	`log( ".... Attachee:"@ExplosionTemplate.Attachee@"AttachCont"@ExplosionTemplate.AttacheeController@"HitPawn"@HitGearPawn, bDebug );
	`log(`showvar(ExplosionTemplate.bUseOverlapCheck), bDebug);

	foreach CollidingActors(class'Actor', Victim, DamageRadius, ExploOrigin, ExplosionTemplate.bUseOverlapCheck)
	{
		bDamageBlocked = FALSE;

		// Look for things that are not yourself and not world geom (or an e-hole)
		if ( Victim != Self
			&& Victim != IgnoredActor
			&& Victim != Kidnapper
			&& (Victim.Role == ROLE_Authority || Victim.bTearOff || Hydra_Base(Victim) != None || GearKnockableActor(Victim) != None)
			&& (!Victim.bWorldGeometry || GearSpawner_EmergenceHoleBase(Victim) != None || GearDestructibleObject(Victim) != None)
			&& ( ClassIsChildOf( Victim.Class, ActorClassToIgnoreForDamage ) == FALSE )
			)
		{
			ModDamageType = DamageType;
			ModInstigator = InstigatedByController;

			// If attached to a pawn and victim is a pawn on other team
			VictimPawn = Pawn(Victim);

			//debug
			`log( "Check vs Victim"@Victim@"CurDamInfo"@ModDamageType@ModInstigator@"Checks..."@ExplosionTemplate.AttacheeController@VictimPawn@WorldInfo.GRI.OnSameTeam(ExplosionTemplate.AttacheeController, VictimPawn.Controller), bDebug );

			if( ExplosionTemplate.AttacheeController != None && VictimPawn != None && !WorldInfo.GRI.IsCoopMultiplayerGame() && !WorldInfo.GRI.OnSameTeam(ExplosionTemplate.AttacheeController, VictimPawn.Controller) )
			{
				//debug
				`log( "Change instigator to"@ExplosionTemplate.AttacheeController@ExplosionTemplate.Attachee, bDebug );

				ModInstigator = ExplosionTemplate.AttacheeController;			// Make the instigator the base pawn's controller
			}

			// skip line check for physics objects
			if ( Victim.bStatic ||
				 Victim.IsA('KActor') ||
				 Victim.IsA('InterpActor') ||
				 Victim.IsA('GearDestructibleObject') ||
				 Victim.IsA('FracturedStaticMeshPart') ||
				 Victim.IsA('GearProj_Grenade') )
			{
				bDamageBlocked = FALSE;
				bDoFullDamage = TRUE;			// force full damage for physics objects
			}
			else
			{
				// note: using bbox center instead of location, because that's what visiblecollidingactors does
				// and we use that function for knockdowns.  it also claims better results.
				// Use per-poly collision, as that is what grenade movement uses, and that way damage uses line-of-sight
				Victim.GetComponentsBoundingBox(BBox);
				BBoxCenter = (BBox.Min + BBox.Max) * 0.5f;
				HitActor = Trace(HitL, HitN, BBoxCenter, ExploOrigin, FALSE,,,TRACEFLAG_Bullet);
				if (HitActor != None && HitActor != Victim)
				{
					bDamageBlocked = TRUE;
				}
			}

			if( !bDamageBlocked )
			{
				// Calculate and apply radius damage
				Victim.GetBoundingCylinder(ColRadius, ColHeight);

				dir	= Victim.Location - ExploOrigin;
				dist = VSize(dir);
				dir	= Normal(Dir);

				if( Hostage != None && Victim == Hostage )
				{
					`log("Hostage hit"@`showvar(Hostage),bDebug);
					if (Hostage.IsA('GearPawn_MeatflagBase') && Hostage.InteractionPawn != None)
					{
						DamageScale = 0.f;
						if (ClassIsChildOf(ModDamageType, class'GDT_Mortar'))
						{
							// if it's a mortar then delay longer since it's far likely we'll die soon
							Hostage.InteractionPawn.SetTimer(0.5f,FALSE,'SavedByAMeatshieldCallback');
						}
						else
						{
							Hostage.InteractionPawn.SetTimer(0.1f,FALSE,'SavedByAMeatShieldCallback');
						}
						Hostage.InteractionPawn.ServerEndSpecialMove(Hostage.InteractionPawn.SpecialMove);
					}
					else
					{
						// Make sure hostage dies!
						DamageScale = Hostage.DefaultHealth - Hostage.DBNODeathThreshold + 1;
					}
				}
				else if( bDoFullDamage )
				{
					DamageScale = 1.f;
				}
				else
				{
					//`log(`showvar(Dist)@`showvar(ColRadius));
					Dist = FMax(Dist - ColRadius,0.f);
					DamageScale = FClamp(1.f - Dist/DamageRadius, 0.f, 1.f);
					DamageScale = DamageScale ** DamageFalloffExp;
				}

				if( DamageScale > 0.f )
				{
					//debug
					`log("*****"@self@"Hurting"@Victim@Pawn(Victim).Health@"for"@BaseDamage*DamageScale@ModDamageType@"by"@ModInstigator@ModInstigator.Pawn, bDebug	);
					Victim.TakeDamage
					(
						BaseDamage * DamageScale,
						ModInstigator,
						Victim.Location - 0.5 * (ColHeight + ColRadius) * dir,
						(DamageScale * MomentumScale * dir),
						ModDamageType,,
						self
					);
					//debug
					`log("     Newhealth"@Victim@Pawn(Victim).Health, bDebug );

					VictimsList[VictimsList.Length] = Victim;
				}
			}
		}
	}

	if( ExplosionTemplate.bFullDamageToAttachee && VictimsList.Find( ExplosionTemplate.Attachee ) < 0 )
	{
		Victim	= ExplosionTemplate.Attachee;

		Victim.GetBoundingCylinder(ColRadius, ColHeight);
		dir		= Victim.Location - ExploOrigin;
		dist	= VSize(dir);
		dir		= Normal(Dir);

		Victim.TakeDamage
		(
			BaseDamage,
			InstigatedByController,
			Victim.Location - 0.5 * (ColHeight + ColRadius) * dir,
			(MomentumScale * dir),
			DamageType,,
			self
		);
	}
}

/** Look for FSMAs in radius and break parts off. */
protected simulated function DoBreakFracturedMeshes(vector ExploOrigin, float DamageRadius, float RBStrength, class<DamageType> DmgType)
{
	local FracturedStaticMeshActor FracActor;
	local byte bWantPhysChunksAndParticles;

	foreach CollidingActors(class'FracturedStaticMeshActor', FracActor, DamageRadius, ExploOrigin, TRUE)
	{
		if((FracActor.Physics == PHYS_None) && FracActor.IsFracturedByDamageType(DmgType))
		{
			// Make sure the impacted fractured mesh is visually relevant
			if( FracActor.FractureEffectIsRelevant( false, Instigator, bWantPhysChunksAndParticles ) )
			{
				FracActor.BreakOffPartsInRadius(
					ExploOrigin,
					DamageRadius,
					RBStrength,
					bWantPhysChunksAndParticles == 1 ? true : false);
			}
		}
	}
}

// @todo, do we want to handle single player and multi differently for this?
/** Internal.  Handle making pawns cringe or fall down from nearby explosions.  Server only. */
protected function DoCringesAndKnockdowns()
{
	local GearPawn Victim;
	local float DistFromExplosion;
	local vector ExplosionToVictimDir, LinearVel, KnockdownExplosionLoc;
	local float KnockDownRad, CringeRad;

	// should only happen on the server
	if (ExplosionTemplate != None && WorldInfo.NetMode != NM_Client)
	{
		KnockDownRad = ExplosionTemplate.KnockDownRadius;
		CringeRad = ExplosionTemplate.CringeRadius;

		// @fixme, unify this with the HurtRadius stuff.  Explosions are doing 2 separate collections
		// of nearby actors at the moment
		foreach VisibleCollidingActors( class'GearPawn', Victim, FMax(CringeRad, KnockDownRad), Location, TRUE )
		{
			if( !Victim.bRespondToExplosions || Victim.InGodMode() )
			{
				continue;
			}

			// only cringe if on opposite team OR you hit yourself and you're human controlled OR the explosion is attached to you
			if ( ( (InstigatorController == None)
				   || (!WorldInfo.GRI.OnSameTeam(InstigatorController, Victim) || ExplosionTemplate.bAllowTeammateCringes)
				   || GearGame(WorldInfo.Game).bAllowFriendlyFire
				   || (Victim == ExplosionTemplate.Attachee)
				   || (Instigator == Victim && Instigator.IsHumanControlled())
				   )
				&& ( ClassIsChildOf( Victim.Class, ExplosionTemplate.ActorClassToIgnoreForKnockdownsAndCringes ) == FALSE )
				&& (Victim.DrivenVehicle == None || !Victim.DrivenVehicle.IsA('Turret_TroikaCabal')) ) // need to check this otherwise you end up in lala land with camera all crazy and knocked down guy shooting troika
			{
				ExplosionToVictimDir = Victim.Location - Location;
				DistFromExplosion = VSize(ExplosionToVictimDir);
				if (DistFromExplosion == 0.f)
				{
					// if explosion is directly on this guy, pretend it's actually 1 unit below him
					// it's an arbitrary convention, but chosen to tend to blow things upward in more spectacular fashion.
					DistFromExplosion = 1.f;
					ExplosionToVictimDir = vect(0,0,1);
				}
				else
				{
					ExplosionToVictimDir /= DistFromExplosion;
				}

				if (!Victim.IsProtectedByShield(ExplosionToVictimDir,FALSE,Location))
				{
					if (DistFromExplosion < KnockDownRad)
					{
						// adjust the explosion location to eliminate z
						KnockdownExplosionLoc = Location;
						KnockdownExplosionLoc.Z = Victim.Location.Z + 50.f;
						// @fixme, these ragdoll params are just copied smoke grenade concussive prototypes
						// prolly want to parametrize and expose this for tweaking
						LinearVel = Normal(Victim.Location - KnockdownExplosionLoc) * 192.f * FClamp(KnockDownRad/DistFromExplosion,0.5,2.f);
						LinearVel.X = int(LinearVel.X);
						LinearVel.Y = int(LinearVel.Y);
						LinearVel.Z = Min(LinearVel.Z,0);	// clamp the z to only push down

						Victim.Knockdown(LinearVel, vect(0,0,0));

						// do any desired knockdown effort
						if (ExplosionTemplate.PawnKnockdownEffort != GearEffort_None)
						{
							Victim.SoundGroup.PlayEffort(Victim, ExplosionTemplate.PawnKnockdownEffort);
						}
					}
					else if (DistFromExplosion < CringeRad)
					{
						// outside of knockdown radius, do cringe
						Victim.Cringe();
					}
				}
				// if they are carrying a meatshield
				if (Victim.IsAKidnapper())
				{
					// drop it
					Victim.SetTimer(0.1f,FALSE,'SavedByAMeatShieldCallback');
					Victim.ServerEndSpecialMove(Victim.SpecialMove);
					// award bonus points to the instigator if this is the meatflag
					if (GearGameCTM_Base(WorldInfo.Game) != None && GearPRI(Instigator.PlayerReplicationInfo) != None && IsA('GearPawn_MeatFlagBase'))
					{
						GearPRI(Instigator.PlayerReplicationInfo).ScoreGameSpecific2('FlagBreak',"",50);
					}
				}
			}
		}
	}
}


/**
 * Internal.  Extract what data we can from the physical material-based effects system
 * and stuff it into the ExplosionTemplate.
 * Data in the physical material will take precedence.
 *
 * We are also going to be checking for relevance here as when any of these params are "none" / invalid we do not
 * play those effects in Explode().  So this way we avoid any work on looking things up in the physmaterial
 *
 */
simulated protected function UpdateExplosionTemplateWithPerMaterialFX(PhysicalMaterial PhysMaterial)
{
	local DecalData DecalData;
	local ParticleSystem ExplosionPS;
	local SoundCue ExploSound;
	local FogVolumeSphericalDensityInfo FogVolumeArchetype;
	local class<Emit_CameraLensEffectBase> CameraEffect;
	local float CameraEffectRadius;
	local class<GearDamageType> MyGearDamageType;
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	MyGearDamageType = class<GearDamageType>(ExplosionTemplate.MyDamageType);


	if( GRI.IsEffectRelevant( Instigator, Location, 6000, FALSE, GRI.CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		// so if we are not manually overriding the PS then look it up
		if( ExplosionTemplate.bParticleSystemIsBeingOverriddenDontUsePhysMatVersion == FALSE )
		{
			ExplosionPS = class'GearPhysicalMaterialProperty'.static.DetermineImpactParticleSystem( PhysMaterial, MyGearDamageType, bActiveReloadBonusActive, WorldInfo, InstigatorController != None ? InstigatorController.Pawn : None );
			if( ExplosionPS != None )
			{
				// copy for actual use
				ExplosionTemplate.ParticleEmitterTemplate = ExplosionPS;
			}
		}
	}
	else
	{
		ExplosionTemplate.ParticleEmitterTemplate = none;
	}


	if( GRI.IsEffectRelevant( Instigator, Location, 6000, FALSE, GRI.CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		// decal data
		DecalData = class'GearPhysicalMaterialProperty'.static.DetermineImpactDecalData( PhysMaterial, MyGearDamageType, bActiveReloadBonusActive, WorldInfo, InstigatorController != None ? InstigatorController.Pawn : None);
		if( DecalData.bIsValid )
		{
			// copy for actual use
			ExplosionTemplate.DecalData = DecalData;
		}
	}
	else
	{
		ExplosionTemplate.DecalData = DecalData;
	}


	// this is "always" relevant so the broken sound cue cross faders can be fixed up
	if( GRI.IsEffectRelevant( Instigator, Location, 100000, FALSE, GRI.CheckEffectDistance_SpawnWithinCullDistance ) )
	{
		// audio
		ExploSound = class'GearPhysicalMaterialProperty'.static.DetermineImpactSound( PhysMaterial, MyGearDamageType, bActiveReloadBonusActive, WorldInfo );
		if( ExploSound != None )
		{
			// copy for actual use
			ExplosionTemplate.ExplosionSound = ExploSound;
		}
	}
	else
	{
		ExplosionTemplate.ExplosionSound = none;
	}


	// Fog volumes are always relevant as they have gameplay repercussions in terms of visibility
	// fog volume
	if( ExplosionTemplate.bUsePerMaterialFogVolume == TRUE )
	{
		FogVolumeArchetype = class'GearPhysicalMaterialProperty'.static.DetermineFogVolumeArchetype( PhysMaterial, MyGearDamageType, bActiveReloadBonusActive, WorldInfo );
		if( FogVolumeArchetype != None )
		{
			// copy for actual use
			ExplosionTemplate.FogVolumeArchetype = FogVolumeArchetype;
		}
	}


	if( GRI.IsEffectRelevant( Instigator, Location, 2048, FALSE, GRI.CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		CameraEffect = class'GearPhysicalMaterialProperty'.static.DetermineImpactCameraEffect( PhysMaterial, MyGearDamageType, bActiveReloadBonusActive, WorldInfo, CameraEffectRadius );
		if( CameraEffect != None )
		{
			// copy for actual use
			ExplosionTemplate.CameraEffect = CameraEffect;
			ExplosionTemplate.CameraEffectRadius = CameraEffectRadius;
		}
	}
	else
	{
		ExplosionTemplate.CameraEffect = none;
	}

}


/**
 * @todo break this up into the same methods that GearWeapon uses (SpawnImpactEffects, SpawnImpactSounds, SpawnImpactDecal) as they are all
 * orthogonal and so indiv subclasses can choose to have base functionality or override
 **/
simulated event Explode(GearExplosion NewExplosionTemplate)
{
	local GearDecal GD;
	local Emitter ExplosionEmitter;
	local float HowLongToLive;
	local PhysicalMaterial PhysMat;

	local Actor TraceActor;
	local vector out_HitLocation;
	local vector out_HitNormal;
	local vector TraceExtent;
	local TraceHitInfo HitInfo;
	local float RandomScale;
	local GearMapSpecificInfo GSI;
	local EffectParam EP;
	local VectorParam VP;
	local FloatParam FP;

	local GearFogVolume_Spawnable FogVolume;
	local MaterialInstanceTimeVarying MITV;
	local float FogVolumeDuration;

	local PlayerController PC;
	local bool bIsHorde;
	local bool bIsSplitScreen;
	local float DecalLifeSpan;

	ExplosionTemplate = NewExplosionTemplate;

	// by default, live just long enough to go boom
	HowLongToLive = ExplosionTemplate.DamageDelay + 0.01f;

	if (!bHasExploded)
	{
		// maybe find the physical material and extract the properties we need
		if (ExplosionTemplate.bAllowPerMaterialFX)
		{
			PhysMat = GetPhysicalMaterial();
			if (PhysMat != None)
			{
				UpdateExplosionTemplateWithPerMaterialFX(PhysMat);
			}
		}

		// spawn explosion effects
		if( WorldInfo.NetMode != NM_DedicatedServer )
		{
			if( ExplosionTemplate.ParticleEmitterTemplate != none )
			{
				// spawn particle effect
				ExplosionEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ExplosionTemplate.ParticleEmitterTemplate, Location, Rotation );
				if (ExplosionEmitter != None)
				{
					// attach the emitter to the attachee if available and if we were asked to actually attach this
					if( (ExplosionTemplate.Attachee != None) && (ExplosionTemplate.bAttachExplosionEmitterToAttachee) )
					{
						ExplosionEmitter.SetBase(ExplosionTemplate.Attachee);
					}

					ExplosionEmitter.SetDrawScale(ExplosionTemplate.ExplosionEmitterScale);

					// now check to see if this Explosion is using the per map values
					if( ExplosionTemplate.bUseMapSpecificValues == TRUE )
					{
						GSI = GearMapSpecificInfo(WorldInfo.GetMapInfo());
						if( GSI != none )
						{
							if( GSI.ColorConfig != none )
							{
								// look over all of the effects
								foreach GSI.ColorConfig.EffectParams( EP )
								{
									// if there are any for my damage type
									if( ExplosionTemplate.MyDamageType == EP.DamageType )
									{
										// do all of the setting
										foreach EP.VectorParams( VP )
										{
											ExplosionEmitter.ParticleSystemComponent.SetVectorParameter( VP.Name, VP.Value );
											//`log( "Setting Vect:" @ VP.Name @ VP.Value );
										}

										foreach EP.FloatParams( FP )
										{
											ExplosionEmitter.ParticleSystemComponent.SetFloatParameter( FP.Name, FP.Value );
											//`log( "Setting Float:" @ FP.Name @ FP.Value );
										}
									}
								}
							}
						}
					}

					ExplosionEmitter.ParticleSystemComponent.ActivateSystem();
				}
			}

			// spawn a decal
			// @todo when GearPawnFX.LeaveADecal is promoted to global object update this
			if( ExplosionTemplate.DecalData.bIsValid )
			{
				TraceActor = Trace( out_HitLocation, out_HitNormal, (Location + (-ExplosionTemplate.HitNormal * ExplosionTemplate.DecalTraceDistance)), Location, FALSE, TraceExtent, HitInfo, TRACEFLAG_Bullet );
				//DrawDebugLine( Location, (Location + (-ExplosionTemplate.HitNormal * ExplosionTemplate.DecalTraceDistance)) , 255, 1, 1, TRUE );
				if( TraceActor != None )
				{

					RandomScale = GetRangeValueByPct( ExplosionTemplate.DecalData.RandomScalingRange, FRand() );
					ExplosionTemplate.DecalData.Width *= RandomScale;
					ExplosionTemplate.DecalData.Height *= RandomScale;

					GD = GearGRI(WorldInfo.GRI).GOP.GetDecal_Explosion( out_HitLocation );
					if( GD != none )
					{
						bIsHorde = WorldInfo.GRI.IsCoopMultiplayerGame();
						bIsSplitScreen = class'Engine'.static.IsSplitScreen();
						DecalLifeSpan = (bIsHorde || bIsSplitScreen) ? ExplosionTemplate.DecalData.LifeSpan : 999999.0;

						//`log( "Explo DM: " $ ExplosionTemplate.DecalData.DecalMaterial );
						GD.MITV_Decal.SetParent( ExplosionTemplate.DecalData.DecalMaterial );

						// @TODO:  pass in a struct with the decal params you want to use
						WorldInfo.MyDecalManager.SetDecalParameters(
							GD,
							GD.MITV_Decal,
							out_HitLocation,
							rotator(-out_HitNormal),
							bIsHorde ? (ExplosionTemplate.DecalData.Width*0.80f) : ExplosionTemplate.DecalData.Width,
							bIsHorde ? (ExplosionTemplate.DecalData.Height*0.80f) : ExplosionTemplate.DecalData.Height,
							Max(ExplosionTemplate.DecalData.Thickness,class'GearDecal'.default.MinExplosionThickness),
							(bIsHorde || bIsSplitScreen) ? FALSE : !ExplosionTemplate.DecalData.ClipDecalsUsingFastPath,
							(FRand() * 360.0f),
							HitInfo.HitComponent,
							TRUE,
							FALSE,
							HitInfo.BoneName,
							INDEX_NONE,
							INDEX_NONE,
							DecalLifeSpan,
							INDEX_NONE,
							class'GearDecal'.default.DepthBias,
							ExplosionTemplate.DecalData.BlendRange
							);

						TraceActor.AttachComponent( GD );

						// if horde or splitscren then fade decals out instead of leaving them around for ever
						GD.MITV_Decal.SetDuration( DecalLifeSpan );

						//DrawDebugCoordinateSystem( out_HitLocation, rotator(-out_HitNormal), 3.0f, TRUE );
					}
				}
			}

			// turn on the light
			if (ExplosionTemplate.ExploLight != None)
			{
				// construct a copy of the PLC, turn it on
				ExplosionLight = new(self) class'PointLightComponent' (ExplosionTemplate.ExploLight);
				if (ExplosionLight != None)
				{
					AttachComponent(ExplosionLight);
					ExplosionLight.SetEnabled(TRUE);
					SetTimer(ExplosionTemplate.ExploLightFadeOutTime);
					LightFadeTime = ExplosionTemplate.ExploLightFadeOutTime;
					HowLongToLive = FMax( LightFadeTime + 0.2f, HowLongToLive );
					LightInitialBrightness = ExplosionTemplate.ExploLight.Brightness;
				}
			}

			// play the sound
			if ( (ExplosionTemplate.ExplosionSound != None) && !ExplosionTemplate.bSuppressAudio )
			{
				//`log( "Playing Explosion Sound (debug left in to test distance)" @ ExplosionTemplate.ExplosionSound );
				PlaySound( ExplosionTemplate.ExplosionSound, TRUE, TRUE, FALSE, Location, TRUE );
				//PlaySound(ExplosionTemplate.ExplosionSound, TRUE, TRUE);
			}

			// do camera shake(s)
			if (ExplosionTemplate.bDoExploCameraShake)
			{
				class'GearPlayerCamera'.static.PlayWorldCameraShake(ExplosionTemplate.ExploShake, self, Location, ExplosionTemplate.ExploShakeInnerRadius, ExplosionTemplate.ExploShakeOuterRadius, ExplosionTemplate.ExploShakeFalloff, TRUE );
			}
			if (ExplosionTemplate.bDoExploCameraAnimShake)
			{
				class'GearPlayerCamera'.static.PlayWorldCameraShakeAnim(ExplosionTemplate.ExploAnimShake, self, Location, ExplosionTemplate.ExploShakeInnerRadius, ExplosionTemplate.ExploShakeOuterRadius, ExplosionTemplate.ExploShakeFalloff, TRUE );
			}

			// Apply impulse to physics stuff (before we do fracture)
			RadialImpulseComponent.ImpulseRadius = FMax(ExplosionTemplate.DamageRadius, ExplosionTemplate.KnockDownRadius);
			RadialImpulseComponent.ImpulseStrength = ExplosionTemplate.MyDamageType.default.RadialDamageImpulse;
			RadialImpulseComponent.bVelChange = ExplosionTemplate.MyDamageType.default.bRadialDamageVelChange;
			RadialImpulseComponent.ImpulseFalloff = RIF_Constant;
			//`log("AA"@ExplosionTemplate.MyDamageType@RadialImpulseComponent.ImpulseStrength@RadialImpulseComponent.ImpulseRadius);
			RadialImpulseComponent.FireImpulse(Location);

			// Do fracturing
			if(ExplosionTemplate.bCausesFracture)
			{
				DoBreakFracturedMeshes(Location, ExplosionTemplate.FractureMeshRadius, ExplosionTemplate.FracturePartVel, ExplosionTemplate.MyDamageType);
			}

			// spawn a fog volume
			if( ( ExplosionTemplate.bUsePerMaterialFogVolume == TRUE ) && ( ExplosionTemplate.FogVolumeArchetype != none ) )
			{
				FogVolume = Spawn( class'GearFogVolume_Spawnable',,,, rot(0,0,0) );
				FogVolume.FogVolumeArchetype = ExplosionTemplate.FogVolumeArchetype;
				FogVolume.LifeSpan = 6.0f;
				//`log( "just spawned:" @ ExplosionTemplate.FogVolumeArchetype );

				// now check to see if we have an MITV and then start it
				if( MaterialInstanceTimeVarying(FogVolume.DensityComponent.FogMaterial) != none )
				{
					//`log( "MITV! " $ FogVolume.DensityComponent.FogMaterial );
					MITV = new(outer) class'MaterialInstanceTimeVarying';
					MITV.SetParent( FogVolume.DensityComponent.FogMaterial );
					FogVolumeDuration = MITV.GetMaxDurationFromAllParameters();
					if( FogVolumeDuration == 0.0f )
					{
						FogVolumeDuration = 6.0f;
					}

					FogVolumeDuration = Min( FogVolumeDuration, 30.0f ); // safeguard
					//`log( "MITV! setting duration " @ FogVolumeDuration);
					FogVolume.DensityComponent.FogMaterial = MITV;

					MITV.SetDuration( FogVolumeDuration );
					FogVolume.LifeSpan = FogVolumeDuration;
				}

				FogVolume.StartFogVolume();
			}

			if( ExplosionTemplate.CameraEffect != None )
			{
				foreach WorldInfo.LocalPlayerControllers( class'PlayerController', PC )
				{
					// splatter some blood on their camera if they are a human and decently close
					if( ( PC.Pawn != none && VSize(PC.Pawn.Location - Location) < ExplosionTemplate.CameraEffectRadius )
						&& PC.IsAimingAt( self, 0.1f ) // if we are semi looking in the direction of the explosion
						)
					{
						PC.ClientSpawnCameraLensEffect( ExplosionTemplate.CameraEffect );
					}
				}
			}

			if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, HitLocationFromPhysMaterialTrace, 1024.0f, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistance ) )
			{
				if( FluidSurfaceActor(HitActorFromPhysMaterialTrace) != none )
				{
					FluidSurfaceActor(HitActorFromPhysMaterialTrace).FluidComponent.ApplyForce( HitLocationFromPhysMaterialTrace, 1024.0f, 20.0f, FALSE );
				}
			}
		}

		// do damage
		// delay the damage if necessary,
		if (ExplosionTemplate.DamageDelay > 0.0f)
		{
			SetTimer( ExplosionTemplate.DamageDelay, FALSE, nameof(DoExplosionDamage) );
		}
		else
		{
			// otherwise apply immediately
			DoExplosionDamage();
		}
		if( Role == Role_Authority )
		{
			MakeNoise(1.0f);
		}

`if(`notdefined(FINAL_RELEASE))
		//if( GearPawn(Instigator).bWeaponDebug_DamageRadius == TRUE )
		//{
		//	// debug spheres
		//	FlushPersistentDebugLines();
		//	DrawDebugSphere(HitLocation, InnerExploRadius, 12, 255, 128, 16, TRUE);
		//	DrawDebugSphere(HitLocation, OuterCoreDamageRadius, 12, 255, 16, 16, TRUE);
		//	DrawDebugLine(HitLocation, HitLocation + HitNormal*16, 255, 255, 255, TRUE);
		//}
`endif

		bHasExploded = TRUE;

		// done with it
		if (!bPendingDelete && !bDeleteMe)
		{
			LifeSpan = HowLongToLive;
		}
	}
}

simulated function DoExplosionDamage()
{
	if (ExplosionTemplate != None)
	{
		// Actually do the damage
		if ( (ExplosionTemplate.Damage > 0.f) && (ExplosionTemplate.DamageRadius > 0.f) )
		{
			HurtExplosion(ExplosionTemplate.Damage, ExplosionTemplate.DamageRadius, ExplosionTemplate.DamageFalloffExponent, ExplosionTemplate.MyDamageType, ExplosionTemplate.MomentumTransferScale, Location, ExplosionTemplate.ActorToIgnoreForDamage, ExplosionTemplate.ActorClassToIgnoreForDamage, InstigatorController, FALSE );
		}

		if (Role == ROLE_Authority)
		{
			DoCringesAndKnockdowns();
		}
	}
}

simulated function Tick( float DeltaTime )
{
	local float TimerCount, Pct;

	if (ExplosionLight != None)
	{
		TimerCount = GetTimerCount();
		if( TimerCount >= 0 )
		{
			Pct = 1.f - (TimerCount / LightFadeTime);
			Pct *= Pct;		// note that fades out as a square
			ExplosionLight.SetLightProperties(LightInitialBrightness * Pct);
		}
		else
		{
			ExplosionLight.SetEnabled(FALSE);
		}
	}
}

defaultproperties
{
	// Note that this actor does not replicate.  If you want the server to dictate an explosion on
	// all clients, use a GearExplosionActorReplicated.
	RemoteRole=ROLE_None

	Begin Object Class=RB_RadialImpulseComponent Name=ImpulseComponent0
	End Object
	RadialImpulseComponent=ImpulseComponent0
	Components.Add(ImpulseComponent0)

	//bDebug=TRUE
}
