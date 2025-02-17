/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustBrumak extends GearPawn_LocustBrumakBase;

//////// EFFECT VARIABLES //////////
var bool bTookDamage;
/** Pain vocal **/
var SoundCue PainSound;
/** List of Roar sound cues */
var array<SoundCue> RoarSound;

/** Particle effects and template for side gun destruction */
var ParticleSystem PS_SideGunDestroyed;
var ParticleSystemComponent PSC_LeftGunDestroyed, PSC_RightGunDestroyed;

var ParticleSystem PS_SideGunDamaged_One, PS_SideGunDamaged_Two;
var ParticleSystemComponent PSC_LeftGunDamaged, PSC_RightGunDamaged;

var ParticleSystem PS_SideGunSmoke;
var ParticleSystemComponent PSC_LeftGunSmoke, PSC_RightGunSmoke;

/** Overrides physical material on side guns when they are destroyed */
var PhysicalMaterial PhysMat_SideGunDestroyed;

var StaticMeshComponent	LeftShieldComponent, RightShieldComponent;
var PrimitiveComponent	LastTakeDamageComponent;

/** Proximity warning cue. Dom yells at Marcus when too close from Brumak. */
var Array<SoundCue>	DomProximityCues;

var repnotify bool			CallLeftGunDestroyed;
var repnotify bool			CallRightGunDestroyed;
var repnotify bool			LeftGunPawnIsNone;
var repnotify bool			RightGunPawnIsNone;

/** Cached pointer to gore manager node. */
var		GearAnim_GoreSystem		GoreNode;

var		array<name>				LeftGunGoreBones;
var		array<name>				RightGunGoreBones;

/** Set of damage types that should not cause gore to appear on this mesh */
var() array< class<DamageType> > NoGoreDamageTypes;

replication
{
	if( Role==ROLE_Authority )
		CallLeftGunDestroyed, CallRightGunDestroyed, LeftGunPawnIsNone, RightGunPawnIsNone;
}

simulated function PostBeginPlay()
{
	local GearAI_BrumakDriver	DriverController;
	local GearAI_Brumak_SideGun	RightGunController, LeftGunController;
	local GearPawn				NewDriver;
	local Pawn					NewLeftGun, NewRightGun;

	super.PostBeginPlay();

	if( Role == ROLE_Authority )
	{
		// Spawn Driver and Attach him to the Brumak mesh
		NewDriver = Spawn(DriverClass,,,,,,TRUE); // ignore encroach on spawn
		if( NewDriver != None )
		{
			SetDriver( NewDriver );

			DriverController = Spawn( class'GearAI_BrumakDriver' );
			DriverController.SetTeam( 1 );
			DriverController.Possess( Driver, FALSE );

			// Initialize health & other variables
			WorldInfo.Game.SetPlayerDefaults(NewDriver);
		}

		// Spawn faux pawns for side guns
		NewRightGun = Spawn(class'GearPawn_LocustBrumak_SideGun',,,,,,TRUE); // ignore encroach on spawn
		if( NewRightGun != None )
		{
			SetGunner( NewRightGun, 1 );

			RightGunController = Spawn( class'GearAI_Brumak_SideGun' );
			RightGunController.SetTeam( 1 );
			RightGunController.Possess( RightGunPawn, FALSE );

			// Initialize health & other variables
			WorldInfo.Game.SetPlayerDefaults(NewRightGun);
		}
		NewLeftGun = Spawn(class'GearPawn_LocustBrumak_SideGun',,,,,,TRUE); // ignore encroach on spawn
		if( NewLeftGun != None )
		{
			SetGunner( NewLeftGun, -1 );

			LeftGunController = Spawn( class'GearAI_Brumak_SideGun' );
			LeftGunController.SetTeam( 1 );
			LeftGunController.Possess( LeftGunPawn, FALSE );

			// Initialize health & other variables
			WorldInfo.Game.SetPlayerDefaults(NewLeftGun);
		}

/*
		// init the driver POI
		DriverPOI = Spawn( class'GearPointOfInterest' );
		DriverPOI.SetBase( Driver,, Driver.Mesh, Driver.NeckBoneName );
		DriverPOI.SetRelativeLocation( DriverPOI.RelativeLocation + vect(0,0,50) );
		DriverPOI.bEnabled = FALSE;
		DriverPOI.IconDuration = 5.0f;
		DriverPOI.FOVCount=0;
		DriverPOI.DesiredFOV = 30;
		DriverPOI.bDoTraceForFOV = FALSE;
		DriverPOI.DisplayName = "BRUMAK_DRIVER";

		// init the leg POI
		LegPOI = Spawn( class'GearPointOfInterest' );
		LegPOI.SetBase( self );
		LegPOI.SetRelativeLocation( vect(0,0,0) );
		LegPOI.bEnabled = FALSE;
		LegPOI.IconDuration = 5.0f;
		LegPOI.FOVCount=0;
		LegPOI.DesiredFOV = 30;
		LegPOI.bDoTraceForFOV = FALSE;
		LegPOI.DisplayName = "BRUMAK_LEGS";
*/
		CallRightGunDestroyed = false;
		CallLeftGunDestroyed = false;
		LeftGunPawnIsNone = false;
		RightGunPawnIsNone = false;
	}

	if( LeftShieldComponent != None )
	{
		Mesh.AttachComponentToSocket( LeftShieldComponent, 'LeftDriverShield' );
		LeftShieldComponent.SetShadowParent( Mesh );
		LeftShieldComponent.SetLightEnvironment( LightEnvironment );
		LeftShieldComponent.SetScale( DrawScale );
	}
	if( RightShieldComponent != None )
	{
		Mesh.AttachComponentToSocket( RightShieldComponent, 'RightDriverShield' );
		RightShieldComponent.SetShadowParent( Mesh );
		RightShieldComponent.SetLightEnvironment( LightEnvironment );
		RightShieldComponent.SetScale( DrawScale );
	}

	if( LeftGunComponent != None )
	{
		Mesh.AttachComponentToSocket( LeftGunComponent, 'leftgun' );
		LeftGunComponent.SetShadowParent( Mesh );
		LeftGunComponent.SetLightEnvironment( LightEnvironment );
		LeftGunComponent.SetScale( DrawScale );
	}

	if( RightGunComponent != None )
	{
		Mesh.AttachComponentToSocket( RightGunComponent, 'rightgun' );
		RightGunComponent.SetShadowParent( Mesh );
		RightGunComponent.SetLightEnvironment( LightEnvironment );
		RightGunComponent.SetScale( DrawScale );
	}

// 	if( MaskComponent != None )
// 	{
// 		Mesh.AttachComponentToSocket( MaskComponent, 'head' );
// 		MaskComponent.SetShadowParent( Mesh );
// 		MaskComponent.SetLightEnvironment( LightEnvironment );
// 		MaskComponent.SetScale( DrawScale );
// 	}

	// Add 5 secs to driver safe time to account for length of animation
	MinDriverSafeTime += 5.f;

	// If not on a dedicated server, check for proximity to Brumak to warn player to get out of the way.
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		CheckMarcusProximity();
	}
}

/** Interpolate side gun aiming toward respective targets */
simulated function UpdateMeshBoneControllers(float DeltaTime)
{
	local Rotator	DeltaRot;
	local Vector2D	NewAimPct;
	local bool		bUpdateLeftGun, bUpdateRightGun, bUpdateMainGun;

	Super.UpdateMeshBoneControllers(DeltaTime);

	AimOffsetViewLoc		= GetAimOffsetViewLocation();

	if( Role == ROLE_Authority )
	{
		if( LeftGunAimController != None )
		{
			if( LeftGunTarget != None )
			{
				LeftGunAimLocation = GetGunLookTargetLocation( LeftGunTarget, LeftGunAimController );
				if( LeftGunPawn != None && IsWithinLeftGunFOV( LeftGunAimLocation ) )
				{
					bUpdateLeftGun = TRUE;
				}
			}

			if( !bUpdateLeftGun )
			{
				LeftGunAimLocation = vect(0,0,0);
			}
		}

		if( RightGunAimController != None )
		{
			if( RightGunTarget != None )
			{
				RightGunAimLocation = GetGunLookTargetLocation( RightGunTarget, RightGunAimController );
				if( RightGunPawn != None && IsWithinRightGunFOV( RightGunAimLocation ) )
				{
					bUpdateRightGun = TRUE;
				}
			}

			if( !bUpdateRightGun )
			{
				RightGunAimLocation = vect(0,0,0);
			}
		}

		if( MainGunAimController1 != None && MainGunAimController2 != None )
		{
			// If there is a target for the main gun
			if( MainGunTarget != None )
			{
				MainGunAimLocation = GetGunLookTargetLocation( MainGunTarget, MainGunAimController1 );
				// If target is not in FOV
				if( Driver != None && IsWithinMainGunFOV( MainGunAimLocation ) )
				{
					bUpdateMainGun = TRUE;
				}
			}

			if( !bUpdateMainGun )
			{
				MainGunAimLocation = vect(0,0,0);
			}
		}
	}

///// UPDATE CONTROLLERS ON CLIENT AND SERVER
	if( LeftGunAimController != None )
	{
		if( !IsZero(LeftGunAimLocation) )
		{
			// Target is in FOV ... make sure we are blend tracking it
			if( LeftGunAimController.LookAtAlphaTarget != 1.f )
			{
				LeftGunAimController.SetLookAtAlpha(  1.f, 0.45f );
				LeftArmAimOffsetBlend.SetBlendTarget( 1.f, 0.45f );
			}

			// Update the target location and update gun
			LeftGunAimController.SetTargetLocation( LeftGunAimLocation );
			LeftGunAimController.InterpolateTargetLocation( DeltaTime );

			// Update AimOffset
			DeltaRot = Normalize(rotator(LeftGunAimLocation - AimOffsetViewLoc) - Rotation);
			NewAimPct.X = DeltaRot.Yaw	 / 16384.f;
			NewAimPct.Y = DeltaRot.Pitch / 16384.f;
			LeftGunAimPct.X = AimInterpTo(LeftGunAimPct.X, NewAimPct.X, DeltaTime, AimOffsetInterpSpeedAI / 2.f);
			LeftGunAimPct.Y = AimInterpTo(LeftGunAimPct.Y, NewAimPct.Y, DeltaTime, AimOffsetInterpSpeedAI / 2.f);

			// Update Recoil Controllers
			IKRecoilCtrl[0].Aim = LeftGunAimPct;
		}
		else
		// Blend out until target is in the FOV
		if( LeftGunAimController.LookAtAlphaTarget != 0.f )
		{
			LeftGunAimController.SetLookAtAlpha(  0.f, 0.45f );
			LeftArmAimOffsetBlend.SetBlendTarget( 0.f, 0.45f );
		}
	}

	if( RightGunAimController != None )
	{
		if( !IsZero(RightGunAimLocation) )
		{
			// Target is in FOV ... make sure we are blend tracking it
			if( RightGunAimController.LookAtAlphaTarget != 1.f )
			{
				RightGunAimController.SetLookAtAlpha(  1.f, 0.45f );
				RightArmAimOffsetBlend.SetBlendTarget( 1.f, 0.45f );
			}

			// Update the target location and update gun
			RightGunAimController.SetTargetLocation( RightGunAimLocation );
			RightGunAimController.InterpolateTargetLocation( DeltaTime );

			// Update AimOffset
			DeltaRot = Normalize(rotator(RightGunAimLocation - AimOffsetViewLoc) - Rotation);
			NewAimPct.X = DeltaRot.Yaw	 / 16384.f;
			NewAimPct.Y = DeltaRot.Pitch / 16384.f;
			RightGunAimPct.X = AimInterpTo(RightGunAimPct.X, NewAimPct.X, DeltaTime, AimOffsetInterpSpeedAI / 2.f);
			RightGunAimPct.Y = AimInterpTo(RightGunAimPct.Y, NewAimPct.Y, DeltaTime, AimOffsetInterpSpeedAI / 2.f);

			// Update Recoil Controllers
			IKRecoilCtrl[1].Aim = RightGunAimPct;
		}
		else
		// Blend out until target is in the FOV
		if( RightGunAimController.LookAtAlphaTarget != 0.f )
		{
			RightGunAimController.SetLookAtAlpha(  0.f, 0.45f );
			RightArmAimOffsetBlend.SetBlendTarget( 0.f, 0.45f );
		}
	}

	if( MainGunAimController1 != None && MainGunAimController2 != None )
	{
		if( !IsZero(MainGunAimLocation) )
		{
			// Target is in FOV ... make sure we are blend tracking it
			if( MainGunAimController1.LookAtAlphaTarget != 1.f )
			{
				MainGunAimController1.SetLookAtAlpha( 1.f, 0.75f );
				MainGunAimController2.SetLookAtAlpha( 1.f, 0.75f );
			}

			// Update the target location and update gun
			MainGunAimController1.SetTargetLocation( MainGunAimLocation );
			MainGunAimController2.SetTargetLocation( MainGunAimLocation );

			MainGunAimController1.InterpolateTargetLocation( DeltaTime );
			MainGunAimController2.InterpolateTargetLocation( DeltaTime );
		}
		else
		// Blend out until target is in the FOV
		if( MainGunAimController1.LookAtAlphaTarget != 0.f )
		{
			MainGunAimController1.SetLookAtAlpha( 0.f, 0.75f );
			MainGunAimController2.SetLookAtAlpha( 0.f, 0.75f );
		}
	}
}


/** Used to find the gore node in this anim tree */
simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	local GearAnim_GoreSystem Node;

	Super.PostInitAnimTree(SkelComp);

	if(Mesh == SkelComp)
	{
		// Iterate over all GearAnim_GoreSystem - but should only have one
		foreach SkelComp.AllAnimNodes(class'GearAnim_GoreSystem', Node)
		{
			if(GoreNode == None)
			{
				GoreNode = Node;
			}
			else
			{
				`log("GearPawn_LocustBrumak: More than one gore node found!"@SkelComp.AnimTreeTemplate);
			}
		}

		if(GoreNode == None)
		{
			`log("GearPawn_LocustBrumak: No gore node found!"@SkelComp.AnimTreeTemplate);
		}
	}
}

/**
* Checks at regular intervals if Marcus is too close to the Brumak.
* And if that's the case, Dom shouts a warning to move away from it.
*/
simulated function CheckMarcusProximity()
{
	local GearPawn_COGMarcus	MarcusPawn;
	local GearPawn_COGDom		DomPawn;
	local bool				bPlayedLine;

	// If Marcus is within melee distance of brumak, issue a warning from Dom.
	foreach WorldInfo.AllPawns(class'GearPawn_COGMarcus', MarcusPawn, Location, class'GearAI_Brumak'.default.EnemyDistance_Melee)
	{
		// Find Dom
		foreach WorldInfo.AllPawns(class'GearPawn_COGDom', DomPawn)
		{
			DomPawn.SpeakLine(MarcusPawn, DomProximityCues[Rand(DomProximityCues.Length)], "", 0.f, Speech_GUDS);
			bPlayedLine = TRUE;
			break;
		}
		break;
	}

	// Set Timer to check again. If just played a line, delay check for a little bit.
	SetTimer( bPlayedLine ? (4.f + FRand()*2.f): 1.f, FALSE, nameof(CheckMarcusProximity) );
}

function SetDriver( GearPawn NewDriver )
{
	//debug
	`log( GetFuncName()@Driver@NewDriver,bDebug );

	MainGunTarget	= None;

	// Assign new driver
	Driver = NewDriver;

	if( Driver != None )
	{
		Health = DefaultHealth;

		Driver.SetPhysics( PHYS_None );
		Driver.SetOwner( self );
		Driver.SetBase( None );
		Driver.SetBase( self,, Mesh, DriverSocket );

		Driver.CreateInventory( class'GearGameContent.GearWeap_BrumakMainGun');
		MainGunAimController1.SetLookAtAlpha( 0.f, 0.1f );
		MainGunAimController2.SetLookAtAlpha( 0.f, 0.1f );

		DoDriverAttachment(Driver);
	}
	else
	{
		LeftGunAimController.SetLookAtAlpha(  0.f, 0.1f );
		RightGunAimController.SetLookAtAlpha( 0.f, 0.1f );
	}
}

function SetGunner( Pawn NewGunner, optional int Side )
{
	//debug
	`log( GetFuncName()@LeftGunPawn@RightGunPawn@NewGunner@Side,bDebug );

	if( Side == 0 )
		return;

	if( Side < 0 ) // Left side
	{
		LeftGunTarget	= None;
		if( LeftGunPawn != None && !LeftGunPawn.IsHumanControlled() )
		{
			LeftGunPawn.Controller.PawnDied( LeftGunPawn );
			LeftGunPawn.Destroy();
		}
		LeftGunPawn = NewGunner;
		if( LeftGunPawn != None )
		{
			LeftGunPawn.SetPhysics( PHYS_None );
			LeftGunPawn.SetOwner( self );
			LeftGunPawn.SetBase( self,, Mesh, DriverSocket );
			GearPawn_LocustBrumak_SideGun(LeftGunPawn).bIsLeftGun = TRUE;

			LeftGunPawn.CreateInventory( class'GearGameContent.GearWeap_BrumakSideGun' );

			LeftGunAimController.SetLookAtAlpha( 0.f, 0.1f );
		}
	}
	else
	if( Side > 0 ) // Right side
	{
		RightGunTarget	= None;
		if( RightGunPawn != None && !RightGunPawn.IsHumanControlled() )
		{
			RightGunPawn.Controller.PawnDied( RightGunPawn );
			RightGunPawn.Destroy();
		}
		RightGunPawn = NewGunner;
		if( RightGunPawn != None )
		{
			RightGunPawn.SetPhysics( PHYS_None );
			RightGunPawn.SetOwner( self );
			RightGunPawn.SetBase( self,, Mesh, DriverSocket );
			RightGunPawn.CreateInventory( class'GearGameContent.GearWeap_BrumakSideGun' );

			RightGunAimController.SetLookAtAlpha( 0.f, 0.1f );
		}
	}
	SetHumanGunner( None );
}

simulated function Tick(float DeltaTime)
{
	Super.Tick( DeltaTime );

	if( Role == ROLE_Authority )
	{
		// Heal legs over time
		if( LegHealTime > 0.f )
		{
			LegHealth = FMin( LegHealth + ((default.LegHealth/LegHealTime) * DeltaTime), default.LegHealth );
		}
		if( HeadHealTime > 0.f )
		{
			HeadHealth = FMin( HeadHealth + ((default.HeadHealth/HeadHealTime) * DeltaTime), default.HeadHealth );
		}
	}
}

function PruneDamagedBoneList( out array<Name> Bones )
{
	local bool	bLegs, bUsed;
	local int	Idx;

	for( Idx = 0; Idx < Bones.Length; Idx++ )
	{
		bUsed = FALSE;
		if( !bLegs && IsLegBone( Bones[Idx] ) )
		{
			bUsed = TRUE;
			bLegs = TRUE;
		}

		if( !bUsed )
		{
			Bones.Remove(Idx--, 1);
		}
	}
}

simulated event ReplicatedEvent( Name VarName )
{
	if( VarName == 'CallLeftGunDestroyed' )
	{
		//	`Log("GearPawn_LocustBrumak.ReplicatedEvent.LeftGunPawn: " @ LeftGunPawn);
		PlaySideGunDestroyed( LeftGunComponent, LeftGunPawnIsNone );

	}
	else if( VarName == 'CallRightGunDestroyed' )
	{
		//	`Log("GearPawn_LocustBrumak.ReplicatedEvent.RightGunPawn: " @ RightGunPawn);
		PlaySideGunDestroyed( RightGunComponent, RightGunPawnIsNone );
	}
	else
	{
		Super.ReplicatedEvent( VarName );
	}
}

/**
* Overridden for GoW's specific damage taking needs, whee!
*
* @see Actor::TakeDamage
*/
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
	// We will modify damage by AdjustPawnDamage function
	SetDontModifyDamage(false);

	LastTakeDamageComponent = HitInfo.HitComponent;

	// call Actor's version to handle any SeqEvent_TakeDamage for scripting
	Super.TakeDamage( Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );

	if (InGodMode())
	{
		PlayTakeHit(Damage, (InstigatedBy != None) ? InstigatedBy.Pawn : None, HitLocation, DamageType, Momentum, HitInfo);
	}
}

simulated function ReplicatedPlayTakeHitEffects()
{
	local int i;
	local bool bNoGore;
	local name HitBone;

	Super.ReplicatedPlayTakeHitEffects();

	// Check if this is a damage type we should ignore
	for(i=0; i<NoGoreDamageTypes.length; i++)
	{
		if(ClassIsChildOf(LastTakeHitInfo.DamageType, NoGoreDamageTypes[i]))
		{
			bNoGore = TRUE;
			break;
		}
	}

	// If we are not ignoring
	if(!bNoGore && GoreNode != None)
	{
		// pass info to cosmetic gore system
		if(ClassIsChildOf(LastTakeHitInfo.DamageType, class'GDT_Explosive'))
		{
			GoreNode.UpdateGoreDamageRadial(LastTakeHitInfo.HitLocation, LastTakeHitInfo.Damage, FALSE);
		}
		else
		{
			HitBone = (LastTakeHitInfo.HitBoneIndex != 255) ? Mesh.GetBoneName(LastTakeHitInfo.HitBoneIndex) : 'None';
			GoreNode.UpdateGoreDamage(HitBone, LastTakeHitInfo.HitLocation, LastTakeHitInfo.Damage);
		}
	}
}

State Dying
{
	// For shooting off gore after death
	function TakeDamage(int Damage, Controller InstigatedBy, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
	{
		local bool bNoGore;
		local int i;

		Super.TakeDamage(Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);

		// Check if this is a damage type we should ignore
		for(i=0; i<NoGoreDamageTypes.length; i++)
		{
			if(ClassIsChildOf(DamageType, NoGoreDamageTypes[i]))
			{
				bNoGore = TRUE;
				break;
			}
		}

		// If we are not ignoring
		if(!bNoGore && GoreNode != None)
		{
			// pass info to cosmetic gore system
			if(ClassIsChildOf(DamageType, class'GDT_Explosive'))
			{
				GoreNode.UpdateGoreDamageRadial(HitLocation, Damage, FALSE);
			}
			else
			{
				GoreNode.UpdateGoreDamage(HitInfo.BoneName, HitLocation, Damage);
			}
		}
	}
}

/** Different from GearPawn - don't want to stop ticking because of gore */
simulated function StopAllAnimations()
{
	local AnimNodeSequence	SeqNode;

	foreach Mesh.AllAnimNodes(class'AnimNodeSequence', SeqNode)
	{
		SeqNode.bPlaying = FALSE;
	}
}

/** Handle taking damage in various body locations */
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
	local bool bSideGunDestroyed;

	bTookDamage = FALSE;

	if( InGodMode() )
	{
		Damage = 0;
		return;
	}

	// If shot in the left arm gun and gun is not destroyed and not doing hit reaction
	if( HitLeftGunBone( LastTakeDamageComponent ) && CanShootLeftGun() )
	{
		// Only let players finish of the guns, not AI
		if( InstigatedBy.IsHumanControlled() || LeftGunHealth > Damage )
		{
			bTookDamage		= TRUE;
			LeftGunHealth  -= Damage;

			if( IsLeftGunDestroyed() )
			{
				LeftGunLives--;
				if( LeftGunLives == 0 )
				{
					LeftGunPawn.Destroy();
					LeftGunPawn = None;
					LeftGunPawnIsNone = true;
				}
				else
				{
					LeftGunHealth = default.LeftGunHealth;
				}

				bSideGunDestroyed = (LeftGunPawn==None);

				PlaySideGunDestroyed( LastTakeDamageComponent, bSideGunDestroyed );
				CallLeftGunDestroyed = !CallLeftGunDestroyed;
			}
		}
	}
	else
	// Otherwise, if shot in the right arm gun and gun is not destroyed and not doing hit reaction
	if( HitRightGunBone( LastTakeDamageComponent ) && CanShootRightGun() )
	{
		// Only let players finish of the guns, not AI
		if( InstigatedBy.IsHumanControlled() || RightGunHealth > Damage )
		{
			bTookDamage		= TRUE;
			RightGunHealth -= Damage;

			if( IsRightGunDestroyed() )
			{
				RightGunLives--;
				if( RightGunLives == 0 )
				{
					RightGunPawn.Destroy();
					RightGunPawn = None;
					RightGunPawnIsNone = true;
				}
				else
				{
					RightGunHealth = default.RightGunHealth;
				}

				bSideGunDestroyed = (RightGunPawn==None);
				PlaySideGunDestroyed( LastTakeDamageComponent, bSideGunDestroyed );
				CallRightGunDestroyed = !CallRightGunDestroyed;
			}
		}
	}
	else
	// Otherwise, if shot in the legs and driver has been hidden a while
	if( IsLegBone( HitInfo.BoneName ) && CanShootLegs() )
	{
		bTookDamage = TRUE;
		LegHealth  -= Damage;

		if( LegHealth <= 0 )
		{
			LegHealth = default.LegHealth;
			LastExposeDriverTime = WorldInfo.TimeSeconds;

			// If both side guns are destroyed, expose driver.
			if( IsLeftGunDestroyed() && IsRightGunDestroyed() )
			{
				PlayExposeDriver();
			}
			else
			{
				// Otherwise play a frustration ROAR
				UpdateRoarCount();
			}
		}
	}
	else if( HitHead( HitInfo.BoneName ) )
	{
		HeadHealth = FMax( HeadHealth - Damage, 0.f );
	}
	else if( IsMainGunBone(HitInfo.BoneName) && CanShootMainGun() )
	{
		MainGunHealth -= Damage;
	}

	if( Tag != Class.Name || bTookDamage )
	{
		// Brumak doesn't take any damage itself
		Damage = 0;
	}

	if( bSideGunDestroyed )
	{
		Damage = SideGunDestroyedHitPointPenalty;
	}

	PlayTakeHit(Damage, InstigatedBy, HitLocation, GearDamageType, Momentum, HitInfo);
}

simulated function bool TookHeadShot(Name BoneName, vector HitLocation, vector Momentum, optional out byte out_bHasHelmet)
{
	return FALSE;
}

/** Both side guns have to be destroyed before we can shoot the main gun. Because until then, driver is immortal. */
simulated final function bool CanShootMainGun()
{
	return TRUE;
}

simulated final function bool CanShootLeftGun()
{
	return (!IsLeftGunDestroyed()						&&
			!IsDoingSpecialMove(GSM_Brumak_LtGunHit)	&&
			!IsDoingSpecialMove(GSM_Brumak_LtGunPain)	);
}

simulated final function bool CanShootRightGun()
{
	return (!IsRightGunDestroyed()					 &&
			!IsDoingSpecialMove(GSM_Brumak_RtGunHit) &&
			!IsDoingSpecialMove(GSM_Brumak_RtGunPain) );
}

simulated final function bool CanShootLegs()
{
	return FALSE;
//	return (TimeSince(LastExposeDriverTime) > MinDriverSafeTime &&
//		!IsDoingASpecialMove());
}

function float GetHeadDamageInaccuracyScale()
{
	local float Pct;

	Pct = 1.f - FPctByRange( HeadHealth, 0.f, default.HeadHealth );
	return 1.f + (Pct * 2.f);
}

simulated final function bool IsMainGunBone(Name InBoneName)
{
	return (InBoneName == 'MachinGun');
}

simulated final function bool HitRightGunBone( PrimitiveComponent HitComponent )
{
	return (HitComponent == RightGunComponent);
//	return (InBoneName == 'Rt_Arm_Gun' || InBoneName == 'RightWrist');
}

simulated final function bool HitLeftGunBone( PrimitiveComponent HitComponent )
{
	return (HitComponent == LeftGunComponent);
//	return (InBoneName == 'Lf_Arm_Gun' || InBoneName == 'LeftWrist');
}

simulated final function bool IsLegBone(Name InBoneName)
{
	switch( InBoneName )
	{
	case 'LeftKnee':
	case 'LeftAnkle':
	case 'LeftBall':
	case 'LeftThigh':
	case 'RightKnee':
	case 'RightAnkle':
	case 'RightBall':
	case 'RightThigh':
		return TRUE;
	}

	return FALSE;
}

/** Util to see if a bone it relevant to a particular damage bone */
simulated function bool BoneForDamage(name Bone, name Parent)
{
	return (Bone == Parent || Mesh.BoneIsChildOf(Bone, Parent));
}

simulated final function bool HitHead( name BoneName )
{
	return BoneForDamage(BoneName, 'Head');
}


/**
*	Handle driver being killed and disabling the main gun
*	Check to see if stage should progress
*/
function NotifyDriverDied( Pawn DeadPawn, Controller Killer )
{
	//debug
	`log( GetFuncName()@DeadPawn@Killer,bDebug );

	// Play driver killed animation
	if( CanDoSpecialMove(GSM_Brumak_CannonHit) )
	{
		ServerDoSpecialMove(GSM_Brumak_CannonHit, TRUE);
	}

	// Trigger kismet event
//	TriggerEventClass( class'SeqEvent_BrumakDriverKilled', self );
}

function PlayHurtSound()
{
	PlaySound( PainSound );
}

/** Upon death, all pieces can be shot off */
simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	local int i;

	Super.PlayDying(DamageType, HitLoc);

	for(i=0; i<GoreNode.GoreSetup.length; i++)
	{
		GoreNode.GoreSetup[i].bEnabled = TRUE;
	}
}


simulated function PlaySideGunDestroyed( PrimitiveComponent HitComponent, bool bDestroyed )
{
	local RB_BodyInstance BodyInst;
	local int i;

	//	`Log("Pawn.LocustBrumak.PlaySideGunDestroyed.BoneName: " @ BoneName);
	// If left gun was destroyed
	if( HitLeftGunBone( HitComponent ) )
	{
		PlaySound( SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_ArmBlow_NoVoice_Cue', TRUE );

		//		`Log("Pawn.LocustBrumak.PlaySideGunDestroyed.bDestroyed: " @ bDestroyed);
		if( bDestroyed )
		{
			//			`Log("Pawn.LocustBrumak.PlaySideGunDestroyed.PSC_LeftGunDestroyed: " @ PSC_LeftGunDestroyed);
			// Create a new particle system comp for the smoke
			if( PSC_LeftGunDestroyed == None )
			{
				PSC_LeftGunDestroyed = new(self) class'ParticleSystemComponent';
				PSC_LeftGunDestroyed.SetTemplate( PS_SideGunDestroyed );
				PSC_LeftGunDestroyed.SetScale( DrawScale );
				PSC_LeftGunDestroyed.bAutoActivate = FALSE;
				//	PSC_LeftGunDestroyed.SetOnlyOwnerSee(TRUE);
				Mesh.AttachComponent( PSC_LeftGunDestroyed, 'LeftWrist', vect(0,0,0),,);
				//	`Log("Pawn.LocustBrumak.PlaySideGunDestroyed.PSC_LeftGunDestroyed Created");
				//	`Log("Pawn.LocustBrumak.PlaySideGunDestroyed.PSC_LeftGunDestroyed.AttachComponent (BoneName): " @ BoneName);
			}

			// Show/start the new particle effect
			PSC_LeftGunDestroyed.SetHidden( FALSE );
			PSC_LeftGunDestroyed.ActivateSystem();
			//	`Log("Pawn.LocustBrumak.PlaySideGunDestroyed.PSC_LeftGunDestroyed.ActivateSystem");

			// Disable aiming of left gun
			SetSideGunAimTarget( None, TRUE );

			if( Mesh.PhysicsAsset != None )
			{
				BodyInst = Mesh.PhysicsAssetInstance.FindBodyInstance( 'Lf_Arm_Gun', Mesh.PhysicsAsset );
				if( BodyInst != None )
				{
					BodyInst.SetPhysMaterialOverride( PhysMat_SideGunDestroyed );
				}
			}

			//	`Log("Pawn.LocustBrumak.PlaySideGunDestroyed.CanDoSpecialMove(GSM_Brumak_LtGunHit): " @ CanDoSpecialMove(GSM_Brumak_LtGunHit));

			// Play destroyed animation
			if( CanDoSpecialMove(GSM_Brumak_LtGunHit) )
			{
				ServerDoSpecialMove(GSM_Brumak_LtGunHit, TRUE);
			}

			if( LeftGunComponent != None )
			{
				LeftGunComponent.SetHidden( TRUE );
			}

			// Gore up the left hand
			for(i=0; i<LeftGunGoreBones.length; i++)
			{
				GoreNode.ForceLoseGorePiece(LeftGunGoreBones[i]);
			}
		}
		else
		{
			if( PSC_LeftGunDamaged == None )
			{
				PSC_LeftGunDamaged = new(self) class'ParticleSystemComponent';
				PSC_LeftGunDamaged.SetTemplate( PS_SideGunDamaged_One );
				PSC_LeftGunDamaged.SetScale( DrawScale );
				PSC_LeftGunDamaged.bAutoActivate = FALSE;
				Mesh.AttachComponent( PSC_LeftGunDamaged, 'LeftWrist', vect(0,0,0),,);
			}
			else
			{
				PSC_LeftGunDamaged.SetTemplate( PS_SideGunDamaged_Two );
				PSC_LeftGunDamaged.SetScale( DrawScale );
			}
			// Show/start the new particle effect
			PSC_LeftGunDamaged.SetHidden( FALSE );
			PSC_LeftGunDamaged.ActivateSystem();

			// Pain
			if( CanDoSpecialMove(GSM_Brumak_LtGunPain) )
			{
				ServerDoSpecialMove(GSM_Brumak_LtGunPain, TRUE);
			}
		}
	}
	else
	// Otherwise, if right gun was destroyed
	if( HitRightGunBone( HitComponent ) )
	{
		PlaySound( SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_ArmBlow_NoVoice_Cue', TRUE );

		if( bDestroyed )
		{
			// Create a new particle system comp for the smoke
			if( PSC_RightGunDestroyed == None )
			{
				PSC_RightGunDestroyed = new(self) class'ParticleSystemComponent';
				PSC_RightGunDestroyed.SetTemplate( PS_SideGunDestroyed );
				PSC_RightGunDestroyed.SetScale( DrawScale );
				PSC_RightGunDestroyed.bAutoActivate = FALSE;
				//	PSC_RightGunDestroyed.SetOnlyOwnerSee(TRUE);
				Mesh.AttachComponent( PSC_RightGunDestroyed, 'RightWrist', vect(0,0,0),,);
			}

			// Show/start the new particle effect
			PSC_RightGunDestroyed.SetHidden( FALSE );
			PSC_RightGunDestroyed.ActivateSystem();

			// Disable aiming of right gun
			SetSideGunAimTarget( None, FALSE );

			if( Mesh.PhysicsAsset != None )
			{
				BodyInst = Mesh.PhysicsAssetInstance.FindBodyInstance( 'Rt_Arm_Gun', Mesh.PhysicsAsset );
				if( BodyInst != None )
				{
					BodyInst.SetPhysMaterialOverride( PhysMat_SideGunDestroyed );
				}
			}

			// Play destroyed animation
			if( CanDoSpecialMove(GSM_Brumak_RtGunHit) )
			{
				ServerDoSpecialMove(GSM_Brumak_RtGunHit, TRUE);
			}

			if( RightGunComponent != None )
			{
				RightGunComponent.SetHidden( TRUE );
			}

			// Gore up the right hand
			for(i=0; i<RightGunGoreBones.length; i++)
			{
				GoreNode.ForceLoseGorePiece(RightGunGoreBones[i]);
			}
		}
		else
		{
			if( PSC_RightGunDamaged == None )
			{
				PSC_RightGunDamaged = new(self) class'ParticleSystemComponent';
				PSC_RightGunDamaged.SetTemplate( PS_SideGunDamaged_One );
				PSC_RightGunDamaged.SetScale( DrawScale );
				PSC_RightGunDamaged.bAutoActivate = FALSE;
				Mesh.AttachComponent( PSC_RightGunDamaged, 'RightWrist', vect(0,0,0),,);
			}
			else
			{
				PSC_RightGunDamaged.SetTemplate( PS_SideGunDamaged_Two );
				PSC_RightGunDamaged.SetScale( DrawScale );
			}
			// Show/start the new particle effect
			PSC_RightGunDamaged.SetHidden( FALSE );
			PSC_RightGunDamaged.ActivateSystem();

			// Pain
			if( CanDoSpecialMove(GSM_Brumak_RtGunPain) )
			{
				ServerDoSpecialMove(GSM_Brumak_RtGunPain, TRUE);
			}
		}
	}

	// If both side guns are destroyed, then driver can be killed now.
	if( IsRightGunDestroyed() && IsLeftGunDestroyed() && Driver != None && Driver.Controller != None )
	{
		// Make driver vulnerable now for PHASE 2
		Driver.Controller.bGodMode = FALSE;

		LeftShieldComponent.SetHidden( TRUE );
		RightShieldComponent.SetHidden( TRUE );

		// Show leg POI after guns killed
		//SetTimer( 3.f, FALSE, nameof(EnableLegPOI) );
	}
}

// Get the enemy of the gun on the other side
function Pawn GetOtherGunEnemy( GearPawn_LocustBrumak_SideGun ThisGun )
{
	if( ThisGun == LeftGunPawn )
	{
		if( RightGunPawn != None )
		{
			return RightGunPawn.Controller.Enemy;
		}
	}
	else
	if( ThisGun == RightGunPawn )
	{
		if( LeftGunPawn != None )
		{
			return LeftGunPawn.Controller.Enemy;
		}
	}

	return None;
}

function PlayExposeDriver()
{
	local GearAI_Brumak AI;

	// trigger roar animation.
	if( CanDoSpecialMove(GSM_Brumak_ExposeDriver) )
	{
		AI = GearAI_Brumak(Controller);
		if( AI != None )
		{
			AI.DoExposeDriver();
		}
		else
		{
			ServerDoSpecialMove(GSM_Brumak_ExposeDriver, TRUE);
		}

		// enables the driver poi, but only once
		//EnableDriverPOI();
	}
}

simulated function PlayRoar()
{
	local GearAI_Brumak AI;

	// trigger roar animation.
	if( CanDoSpecialMove(GSM_Brumak_Roar) )
	{
		if( PSC_BreathEffect == None )
		{
			PSC_BreathEffect = new(self) class'ParticleSystemComponent';
			PSC_BreathEffect.SetTemplate( PS_BreathEffect );
			PSC_BreathEffect.SetScale( DrawScale );
			PSC_BreathEffect.bAutoActivate = FALSE;
			Mesh.AttachComponent( PSC_BreathEffect, 'head', vect(0,0,0),,);
		}
		PSC_BreathEffect.SetHidden( FALSE );
		PSC_BreathEffect.ActivateSystem();

		PlaySound( RoarSound[Rand(RoarSound.Length)], TRUE );

		AI = GearAI_Brumak(Controller);
		if( AI != None )
		{
			AI.DoRoar();
		}
	}
}

/** Enable the Driver POI */
function EnableDriverPOI()
{
	// enable the poi, but only after leg has been shown
	if( bLegPOIEnabled && !IsTimerActive('DisableDriverPOI', self) )
	{
		DriverPOI.bEnabled = TRUE;
		DriverPOI.EnablePOI();
		SetTimer( 8.0f, FALSE, nameof(self.DisableDriverPOI), self );
	}
}

/** Disable the Driver POI */
function DisableDriverPOI()
{
	// disable the poi
	bDriverPOIEnabled	= TRUE;
	DriverPOI.bEnabled	= FALSE;
	DriverPOI.DisablePOI();
}


/** Enable the legs POI */
function EnableLegPOI()
{
	// enable the poi, but only the first time
	if( !bLegPOIEnabled && !IsTimerActive('DisableLegPOI', self) )
	{
		LegPOI.bEnabled = TRUE;
		LegPOI.EnablePOI();
		SetTimer( 5.0f, FALSE, nameof(self.DisableLegPOI), self );
	}
}

/** Disable the legs POI */
function DisableLegPOI()
{
	// disable the poi
	if( !bLegPOIEnabled )
	{
		bLegPOIEnabled	= TRUE;
		LegPOI.bEnabled = FALSE;
		LegPOI.DisablePOI();
	}
}

event BaseChange()
{
	local GearAI_Brumak AI;

	Super.BaseChange();

	AI = GearAI_Brumak(Controller);
	if( AI != None && Base == None && Physics == PHYS_Falling )
	{
		if( AI.LeftGunAI != None )
		{
			AI.LeftGunAI.StopFiring();
		}
		if( AI.RightGunAI != None )
		{
			AI.RightGunAI.StopFiring();
		}
		if( AI.DriverAI != None )
		{
			AI.DriverAI.StopFiring();
		}
	}
}


simulated function bool ShouldRefireWeapon(GearWeap_BrumakWeaponBase Gun)
{
	local Vector TestLocation;

	if( Physics == PHYS_Falling )
		return FALSE;

	if( Gun.Instigator == LeftGunPawn )
	{
		TestLocation = GetGunLookTargetLocation(LeftGunTarget, LeftGunAimController);
		return IsWithinLeftGunFOV(TestLocation);
	}
	else if( Gun.Instigator == RightGunPawn )
	{
		TestLocation = GetGunLookTargetLocation(RightGunTarget, RightGunAimController);
		return IsWithinRightGunFOV(TestLocation);
	}
	else if( Gun.Instigator == Driver )
	{
		TestLocation = GetGunLookTargetLocation(MainGunTarget, MainGunAimController2);
		return IsWithinMainGunFOV(TestLocation);
	}

	return TRUE;
}


defaultproperties
{
	ControllerClass=class'GearAI_Brumak'

	Begin Object Name=GearPawnMesh
		//SkeletalMesh=SkeletalMesh'Locust_Brumak.BrumakMesh'
		SkeletalMesh=SkeletalMesh'Locust_Brumak.Gore.Brumak_Gore'
		PhysicsAsset=PhysicsAsset'Locust_Brumak.BrumakMesh_Physics'
		AnimTreeTemplate=AnimTree'Locust_Brumak.Brumak_AnimTree'
		AnimSets(0)=AnimSet'Locust_Brumak.Brumak_Animset'
		bHasPhysicsAssetInstance=TRUE
		bEnableFullAnimWeightBodies=TRUE
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		CollideActors=TRUE
		BlockRigidBody=TRUE
		Translation=(Z=-120)
	End Object

	//GoreSkeletalMesh=SkeletalMesh'Locust_Brumak.Brumak_GoreMesh'
	//GorePhysicsAsset=PhysicsAsset'Locust_Brumak.BrumakMesh_Physics'
	//GoreBreakableJoints=("Head","Jaw","RightElbow","RightWrist","LeftWrist","LeftKnee","Spine","Spine2","RightAnkle","LeftAnkle")

	Begin Object Name=CollisionCylinder
		CollisionRadius=100.0 //200.0
		CollisionHeight=120.0
		BlockActors=FALSE
		CollideActors=FALSE
		BlockNonZeroExtent=FALSE
		BlockZeroExtent=FALSE
	End Object

	Begin Object Class=StaticMeshComponent Name=RightGunSMC
		StaticMesh=StaticMesh'Locust_Brumak.right_gun'
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=FALSE
	End Object
	RightGunComponent=RightGunSMC
	Components.Add(RightGunSMC)

	Begin Object Class=StaticMeshComponent Name=LeftGunSMC
		StaticMesh=StaticMesh'Locust_Brumak.left_gun'
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockRigidBody=FALSE
	End Object
	LeftGunComponent=LeftGunSMC
	Components.Add(LeftGunSMC)

// 	Begin Object Class=StaticMeshComponent Name=MaskSMC
// 		StaticMesh=StaticMesh'Locust_Brumak.Helmet'
// 		CollideActors=TRUE
// 		BlockActors=FALSE
// 		BlockZeroExtent=TRUE
// 		BlockNonZeroExtent=TRUE
// 		BlockRigidBody=FALSE
// 	End Object
// 	MaskComponent=MaskSMC
// 	Components.Add(MaskSMC)

	DriverClass=class'GearPawn_LocustBrumakDriver'
	DriverSocket="Driver"
	DriverAnimTree=AnimTree'Locust_Brumak.BrumakRider_AnimTree'
	DriverAnimSet=AnimSet'Locust_Brumak.Rider_animset'
	SideGunClass=class'GearPawn_LocustBrumak_SideGun'

	LeftMuzzleSocketName=LeftMuzzle
	RightMuzzleSocketName=RightMuzzle
	NeckBoneName=head

	PeripheralVision=-0.1
	SightBoneName="head"

	PS_SideGunDamaged_One=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Gun_Damage_1'
	PS_SideGunDamaged_Two=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Gun_Damage_2'
	PS_SideGunDestroyed=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Gun_Damage_3'
	PS_FootstepDust=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Foot_Effect'
	PS_BreathEffect=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Breath_Effect'


	PainSound=SoundCue'Locust_Brumak_Efforts.BrumakEfforts.BrumakVocalPainSmall01_Cue'
	FootSound=SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_FootstepClose_Cue'
	RoarSound(0)=SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_RoarGutteralCShort_Cue'
	DyingSound=SoundCue'Locust_Brumak_Efforts.BrumakEfforts.BrumakVocalDeath_Cue'

//	PhysMat_SideGunDestroyed=PhysicalMaterial'Locust_Brumak.Gun_No_Damage'

	DomProximityCues(0)=SoundCue'Human_Dom_Chatter_Cue.DomGroup.DomChatter_Brumak_LoudCue'
	DomProximityCues(1)=SoundCue'Human_Dom_Chatter_Cue.DomSingle.DomChatter_Careful_Loud01Cue'
	DomProximityCues(2)=SoundCue'Human_Dom_Chatter_Cue.DomSingle.DomChatter_FallBack_Loud02Cue'
	DomProximityCues(3)=SoundCue'Human_Dom_Chatter_Cue.DomSingle.DomChatter_LookOut_Loud01Cue'
	DomProximityCues(4)=SoundCue'Human_Dom_Chatter_Cue.DomGroup.DomChatter_Marcus_LoudCue'
	DomProximityCues(5)=SoundCue'Human_Dom_Chatter_Cue.DomGroup.DomChatter_Move_LoudCue'
	DomProximityCues(6)=SoundCue'Human_Dom_Chatter_Cue.DomGroup.DomChatter_OutOfTheWay_LoudCue'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustBrumak'

	LeftGunGoreBones=("Gore_Lt_Cuff01","Gore_Lt_Cuff02","Gore_Lt_Gun_03","LeftWrist")
	RightGunGoreBones=("Gore_Rt_Cuff01","Gore_Rt_Cuff02","Gore_Rt_Gun_03","Gore_Rt_Finger")

	NoGoreDamageTypes.Add(class'GDT_AssaultRifle')
	NoGoreDamageTypes.Add(class'GDT_LocustAssaultRifle')
	NoGoreDamageTypes.Add(class'GDT_LocustBurstPistol')
	NoGoreDamageTypes.Add(class'GDT_LocustPistol')
	NoGoreDamageTypes.Add(class'GDT_Melee')
	NoGoreDamageTypes.Add(class'GDT_Shotgun')
	NoGoreDamageTypes.Add(class'GDT_SniperRifle')
}

