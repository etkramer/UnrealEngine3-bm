/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTWeap_Translocator extends UTWeapon
	HideDropDown
	abstract;

/*********************************************************************************************
 Misc Weapon Vars
********************************************************************************************* */

/** Access to the disc in the world */
var repnotify UTProj_TransDisc 	TransDisc;

/** How fast the TL recharges */
var float				RechargeRate;

/** Anim when the disc is deployed */
var name EmptyIdleAnim;
var name EmptyPutDownAnim;
var name EmptyEquipAnim;

/*********************************************************************************************
 Animations and Sounds
********************************************************************************************* */

var SoundCue 			TransFailedSound;
var SoundCue			TransRecalledSound;
var SoundCue			DisruptionDeath;

/*********************************************************************************************
 Skins
********************************************************************************************* */

var MaterialInterface	TeamSkins[4];
var MaterialInstanceConstant	GeneralSkin;
var LinearColor				SkinColors[2];
var class<UTDamageType> FailedTranslocationDamageClass;
var ParticleSystemComponent DiskEffect;

/** true only during translocation (for kill credit) */
var bool bTranslocationInProgress;

replication
{
	if (bNetDirty)
		TransDisc;
}

simulated function PostBeginPlay()
{
	local SkeletalMeshComponent SKMesh;

	// Attach the Muzzle Flash
	SKMesh = SkeletalMeshComponent(Mesh);
	super.PostBeginPlay();
	if (  SKMesh != none )
	{
		SKMesh.AttachComponentToSocket(DiskEffect, MuzzleFlashSocket);
	}
	GeneralSkin=(Mesh.CreateAndSetMaterialInstanceConstant(0));
}

simulated function SetSkin(Material NewMaterial)
{
	local int TeamIndex;

	TeamIndex = Instigator.GetTeamNum();
	if ( TeamIndex == 255 )
		TeamIndex = 0;
	if ( NewMaterial == None ) 	// Clear the materials
	{

		Mesh.SetMaterial(1,TeamSkins[TeamIndex]);
		GeneralSkin.SetVectorParameterValue('Team_Color',SkinColors[TeamIndex]);
	}
	else
	{
		Super.SetSkin(NewMaterial);
	}
}

simulated event float GetPowerPerc()
{
	local int i;
	local float t;

	t = 1.0;

    if( Timers.Length > 0 )
    {
    	for (i=0;i<Timers.Length;i++)
    	{
    		if ( Timers[i].FuncName == 'ReAddAmmo' )
    		{
    			t = Timers[i].Count/Timers[i].Rate;
    			break;
    		}
		}
    }
    t = FClamp(t,0.0,1.0);
    return t;
}

function GivenTo(Pawn ThisPawn, optional bool bDoNotActivate)
{
	Super.GivenTo(ThisPawn, bDoNotActivate);

	if (UTBot(ThisPawn.Controller) != None)
	{
		UTBot(ThisPawn.Controller).bHasTranslocator = true;
	}
}

function ItemRemovedFromInvManager()
{
	Super.ItemRemovedFromInvManager();

	if (TransDisc != None)
	{
		TransDisc.Destroy();
	}
	if(DiskEffect != none)
	{
		DiskEffect.DeactivateSystem();
	}
}

/**
 *  We need to override ConsumeAmmo and set the recharge timer.
 */
function ConsumeAmmo( byte FireModeNum )
{
	super.ConsumeAmmo(FireModeNum);

	if ( UTBot(Instigator.Controller) != None )
	{
		UTBot(Instigator.Controller).NextTranslocTime = WorldInfo.TimeSeconds + 2.5;
	}
}

simulated function PlayWeaponPutDown()
{
	if(	WeaponIdleAnims[0] == EmptyIdleAnim)
	{
		WeaponPutDownAnim = EmptyPutDownAnim;
	}
	else
	{
		WeaponPutDownAnim = default.WeaponPutDownAnim;
	}
	super.PlayWeaponPutDown();
}

simulated function PlayWeaponEquip()
{
	if(	WeaponIdleAnims[0] == EmptyIdleAnim)
	{
		WeaponEquipAnim = EmptyEquipAnim;
	}
	else
	{
		WeaponEquipAnim = default.WeaponEquipAnim;
	}
	super.PlayWeaponEquip();
}
/**
 * Override CheckAmmo to return 0 if ammo count = 0 regardless of the cost
 */
simulated function bool HasAmmo(byte FireModeNum, optional int Amount)
{
	if (AmmoCount == 0)
	{
		return false;
	}
	else
	{
		return super.HasAmmo(FireModeNum, Amount);
	}
}

/**
 * Add ammo back in to the weapon
 */
simulated function ReAddAmmo()
{
	AmmoCount = Min(AmmoCount + 1, MaxAmmoCount);
	if (AmmoCount == MaxAmmoCount)
	{
		ClearTimer('ReAddAmmo');
	}
}

/**
 * If there isn't a disc, launch one, if there is, recall it.
 */
simulated function Projectile ProjectileFire()
{
	local byte TeamNum;

	// If there isn't a disc in the wild, spawn one

	if (TransDisc == None || TransDisc.bDeleteMe)
	{
		if (Instigator != None)
		{
			TeamNum = Instigator.GetTeamNum();
		}
		WeaponProjectiles[0] = default.WeaponProjectiles[(TeamNum < default.WeaponProjectiles.length) ? int(TeamNum) : 0];
		TransDisc = UTProj_TransDisc(Super.ProjectileFire());
		if(TransDisc != none)
		{
			TransDisc.MyTranslocator = self;
			TransDisc.SetOwner(TransDisc.InstigatorController); // so it is always relevant to player who fired
		}
		if(DiskEffect != none)
		{
			DiskEffect.deactivateSystem();
		}

		if ( (ROLE< Role_Authority || (TransDisc != none && !TransDisc.bDeleteMe )) )
		{
			SetTimer(RechargeRate,true,'ReAddAmmo');
		}
		WeaponIdleAnims[0] = EmptyIdleAnim;
		GeneralSkin.SetScalarParameterValue('Mode',1);
	}
	else if (TransDisc != None && !TransDisc.bDeleteMe && !TransDisc.Disrupted() )
	{
		WeaponIdleAnims[0] = default.WeaponIdleAnims[0];
		GeneralSkin.SetScalarParameterValue('Mode',0);
		if(Role == ROLE_Authority)
		{
			TransDisc.Recall();
		}
		TransDisc=none;
		if(DiskEffect != none)
		{
			DiskEffect.ActivateSystem();
		}

	}

	return TransDisc;
}

/**
 * The function will kill the player holding the TL if it's disrupted
 */
function FragInstigator(class<DamageType> DmgClass)
{
	Instigator.TakeDamage(2000, TransDisc.DisruptedController, TransDisc.Location, Vect(0,0,0), DmgClass,, self);
	WeaponPlaySound(DisruptionDeath);
	TransDisc.Recall();
	TransDisc = none;
}


/**
 * Alt-Fire on this weapon is custom and this is it's stub.  It should attempt to translocate.
 */
simulated function CustomFire()
{
	local Actor HitActor;
	local Vector HitNormal,HitLocation, Dest, NewDest, Vel2D, PrevLocation, Diff;
	local bool bFailedTransloc;
	local Controller C;
	local float DiffZ;
	local UTBot B;

	IncrementFlashCount();
	if(AmmoCount != 0)
	{
		WeaponIdleAnims[0] = default.WeaponIdleAnims[0];
		GeneralSkin.SetScalarParameterValue('Mode',0);
		PlayWeaponAnimation(WeaponIdleAnims[0],0.001);
	}
	if (Role==ROLE_Authority)
	{
		// If there isn't a TL disc, exit right away
		if ( (TransDisc==None) || TransDisc.bDeleteMe)		// FIXME: Play a beep or something to notify the player there is no TL disc out
		{
			return;
		}

		// If the disc is disrupted, kill the player
		if ( TransDisc.Disrupted() )
		{
			FragInstigator(FailedTranslocationDamageClass);
			return;
		}

		// Figure out the start location and make sure it's not in the floor.
		dest = TransDisc.Location;
		HitActor = Trace(HitLocation,HitNormal,dest-vect(0,0,1)*Instigator.GetCollisionHeight(),dest);
		if ( HitActor != None )
		{
			dest += HitNormal*Instigator.GetCollisionHeight();
		}

		TransDisc.SetCollision(false, false);
		HitActor = Trace(HitLocation,HitNormal,dest,TransDisc.Location,true,,,TRACEFLAG_Blocking);
		if ( HitActor != None )
		{
			dest = TransDisc.Location;
		}

		if (Instigator.PlayerReplicationInfo.bHasFlag )
		{
			UTPlayerReplicationInfo(Instigator.PlayerReplicationInfo).GetFlag().Drop();
		}

		PrevLocation = Instigator.Location;

		// verify won't telefrag teammate or recently spawned player
		foreach WorldInfo.AllControllers(class'Controller', C)
		{
			if ( (C.Pawn != None) && (C.Pawn != Instigator) )
			{
				Diff = Dest - C.Pawn.Location;
				DiffZ = Diff.Z;
				Diff.Z = 0;
				if ( (Abs(DiffZ) < C.Pawn.GetCollisionHeight() + 2 * Instigator.GetCollisionHeight() )
					&& (VSize(Diff) < C.Pawn.GetCollisionRadius() + Instigator.GetCollisionRadius() + 8) )
				{
					if ( WorldInfo.Game.GameReplicationInfo.OnSameTeam(C.Pawn,Instigator) || (WorldInfo.TimeSeconds - C.Pawn.SpawnTime < UTGame(WorldInfo.Game).SpawnProtectionTime) )
					{
						bFailedTransloc = true;
						break;
					}
					else
					{
						if ( (UTPawn(C.Pawn) != None) && (UTPawn(C.Pawn).ShieldBeltArmor > 0) )
						{
							// very hard to telefrag shield belted enemy
							NewDest = Dest + C.Pawn.GetCollisionRadius() * Normal(Diff);
						}
						else
							NewDest = Dest + 0.1 * C.Pawn.GetCollisionRadius() * Normal(Diff);

						if ( FastTrace(NewDest ,dest) )
							Dest = NewDest;
					}
				}
			}
		}

		if ( !bFailedTransloc && AttemptTranslocation(dest) )
		{
			// Cause any effects to occur
			if ( UTPawn(Instigator) != none )
				UTPawn(Instigator).DoTranslocate(PrevLocation);

			// bound XY velocity to prevent cheats
			Vel2D = Instigator.Velocity;
			Vel2D.Z = 0;
			Vel2D = Normal(Vel2D) * FMin(Instigator.GroundSpeed,VSize(Vel2D));

			// prevent cheat using jumppad in one part of level to get giant jump somewhere else
			Vel2D.Z = FMin(Instigator.JumpZ, Instigator.Velocity.Z);
			Instigator.Velocity = Vel2D;
			if ( !Instigator.PhysicsVolume.bWaterVolume )
			{
				if ( UTBot(Instigator.Controller) != None )
				{
					Instigator.Velocity.X = 0;
					Instigator.Velocity.Y = 0;
					Instigator.Velocity.Z = -150;
					Instigator.Acceleration = vect(0,0,0);
				}
				Instigator.SetPhysics(PHYS_Falling);
			}
			// don't need to adjust anymore because we teleported away from whatever it was
			Instigator.Controller.bAdjusting = false;
		}
		else
		{
			// translocation failed
			if (Instigator.Controller.bPreparingMove)
			{
				// abort bot's move
				B = UTBot(Instigator.Controller);

				if ( (B != None) && B.bPreparingMove )
				{
					B.bForceRefreshRoute = TRUE;
					B.MoveTimer = -1.0f;
					B.NextTranslocTime = WorldInfo.TimeSeconds + 10;
				}
			}
			if( PlayerController(Instigator.Controller) != None && TransFailedSound != none)
			{
				PlayerController(Instigator.Controller).ClientPlaySound(TransFailedSound);
			}
		}

		TransDisc.Recall();
		TransDisc = None;
		if(DiskEffect != none)
		{
			DiskEffect.ActivateSystem();
		}
	}
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'TransDisc')
	{
		if(TransDisc != none)
		{
			if(DiskEffect != none)
			{
				DiskEffect.DeactivateSystem();
			}
		}
		else if(DiskEffect != none)
		{
			DiskEffect.ActivateSystem();
		}
	}

	Super.ReplicatedEvent(VarName);
}

function bool AttemptTranslocation(vector dest)
{
	local vector OldLocation, HitLocation, HitNormal, DestFinalZ;

	bTranslocationInProgress = true;
	OldLocation = Instigator.Location;
	if (!TranslocSucceeded(dest))
	{
		bTranslocationInProgress = false;
		return false;
	}

	// try trace down to dest
	if (Trace(HitLocation, HitNormal, Instigator.Location, dest, false,,, TRACEFLAG_Bullet) == None)
	{
		bTranslocationInProgress = false;
		return true;
	}
	// try trace straight up, then sideways to final location
	DestFinalZ = dest;
	dest.Z = Instigator.Location.Z;
	if ( Trace(HitLocation, HitNormal, DestFinalZ, dest, false) == None &&
		Trace(HitLocation, HitNormal, Instigator.Location, DestFinalZ, false,,, TRACEFLAG_Bullet) == None )
	{
		bTranslocationInProgress = false;
		return true;
	}

	bTranslocationInProgress = false;
	Instigator.SetLocation(OldLocation);
	return false;
}

function bool TranslocSucceeded(vector dest)
{
	local vector newdest;

	if ( Instigator.SetLocation(dest) || BotTranslocation() )
		return true;
	if ( TransDisc.Physics != PHYS_None )
	{
		newdest = TransDisc.Location - Instigator.GetCollisionRadius() * Normal(TransDisc.Velocity);
		if ( Instigator.SetLocation(newdest) )
			return true;
	}
	if ( (dest != TransDisc.Location) && Instigator.SetLocation(TransDisc.Location) )
		return true;

	newdest = dest + Instigator.GetCollisionRadius()  * vect(1,1,0);
	if ( Instigator.SetLocation(newdest) )
		return true;
	newdest = dest + Instigator.GetCollisionRadius()  * vect(1,-1,0);
	if ( Instigator.SetLocation(newdest) )
		return true;
	newdest = dest + Instigator.GetCollisionRadius() * vect(-1,1,0);
	if ( Instigator.SetLocation(newdest) )
		return true;
	newdest = dest + Instigator.GetCollisionRadius() * vect(-1,-1,0);
	if ( Instigator.SetLocation(newdest) )
		return true;

	return false;
}

simulated function PlayFiringSound()
{
	if ( CurrentFireMode==0 && TransDisc!=None && !TransDisc.bDeleteMe )
	{
		WeaponPlaySound( TransRecalledSound );
	}
	else
	{
		super.PlayFiringSound();
	}
}

simulated function WeaponEmpty();

function HolderDied()
{
	super.HolderDied();
	if ( (TransDisc!=none) && !TransDisc.bDeleteMe )
	{
		TransDisc.Recall();
	}
}

// AI Interface...
function bool ShouldTranslocatorHop(UTBot B)
{
	local float dist;
	local Actor N;
	local bool bHop;

	bHop = B.bTranslocatorHop;
	B.bTranslocatorHop = false;

	if ( B.Pawn.PhysicsVolume.bWaterVolume )
		return false;

	if ( bHop && (B.Focus == B.TranslocationTarget) && (B.NextTranslocTime < WorldInfo.TimeSeconds)
		 && B.InLatentExecution(B.LATENT_MOVETOWARD) && B.Squad.AllowTranslocationBy(B) )
	{
		if ( (TransDisc != None) && !TransDisc.bDeleteMe && TransDisc.IsMonitoring(B.Focus) )
			return false;
		dist = VSize(B.TranslocationTarget.Location - B.Pawn.Location);
		if ( dist < 300 )
		{
			// see if next path is possible
			N = B.AlternateTranslocDest();
			if ( (N == None) || ((vector(B.Rotation) Dot Normal(N.Location - B.Pawn.Location)) < 0.5)  )
			{
				if ( dist < 200 )
				{
					B.TranslocationTarget = None;
					return false;
				}
			}
			else
			{
				B.TranslocationTarget = N;
				B.Focus = N;
				return true;
			}
		}
		if ( (vector(B.Rotation) Dot Normal(B.TranslocationTarget.Location - B.Pawn.Location)) < 0.5 )
		{
			SetTimer(0.1,false,'FireHackTimer');
			return false;
		}
		return true;
	}
	return false;
}

simulated function FireHackTimer()
{
	local UTBot B;

	if ( Instigator != None )
	{
		B = UTBot(Instigator.Controller);
		if ( (B != None) && (B.TranslocationTarget != None) && (B.bPreparingMove || ShouldTranslocatorHop(B)) )
		{
			FireHack();
		}
	}
}

function FireHack()
{
	local Actor TTarget;
	local vector TTargetLoc;
	local UTBot B;

	CurrentFireMode = 0;
	B = UTBot(Instigator.Controller);
	TTarget = B.TranslocationTarget;
	if ( TTarget == None )
	{
		return;
	}
	if (TransDisc != None && !TransDisc.bDeleteMe)
	{
		if (TransDisc.Disrupted() || TransDisc.IsMonitoring(TTarget))
		{
			return;
		}
		TransDisc.bNoAI = true;
		TransDisc.Destroy();
		TransDisc = None;
	}

	ProjectileFire();
	if (TransDisc != None && !TransDisc.bDeleteMe)
	{
		TTargetLoc = B.GetTranslocationDestination();
		// find correct initial velocity - FIXMESTEVE is this needed?
		if ( TTarget.Velocity != vect(0,0,0) )
		{
			TTargetLoc += 0.3 * TTarget.Velocity;
			TTargetLoc.Z = 0.5 * (TTargetLoc.Z + TTarget.Location.Z);
		}
		// if the LD placed a specific point, use its velocity instead of calculating it ourselves
		if (UTTranslocatorDest(B.MoveTarget) != None && UTTranslocatorReachSpec(B.CurrentPath) != None)
		{
			TransDisc.Velocity = UTTranslocatorReachSpec(B.CurrentPath).CachedVelocity;
		}
		else
		{
			if ( B.bPreparingMove && (Instigator.Anchor != None) && Instigator.ReachedDestination(Instigator.Anchor)
				&& SuggestTossVelocity(TransDisc.Velocity, TTarget.Location, Instigator.Anchor.Location, TransDisc.Speed, TransDisc.TossZ,,,TransDisc.GetTerminalVelocity()) )
			{
				// fixmesteve - only do this if previously failed translocation to TTarget (add flag to NavigationPoint)
				TransDisc.SetLocation(Instigator.Anchor.Location);
			}
			// FIXMESTEVE- pass proper collisionsize
			else if ( !SuggestTossVelocity(TransDisc.Velocity, TTargetLoc, TransDisc.Location, TransDisc.Speed, TransDisc.TossZ,,, TransDisc.GetTerminalVelocity()) &&
				!SuggestTossVelocity(TransDisc.Velocity, TTargetLoc + vect(0,0,50), TransDisc.Location, TransDisc.Speed, TransDisc.TossZ,,, TransDisc.GetTerminalVelocity()) &&
				!SuggestTossVelocity(TransDisc.Velocity, TTargetLoc, TransDisc.Location, TransDisc.Speed, TransDisc.TossZ, 0.50,, TransDisc.GetTerminalVelocity()) )
			{
				SuggestTossVelocity(TransDisc.Velocity, TTargetLoc + vect(0,0,100), TransDisc.Location, TransDisc.Speed, TransDisc.TossZ,,, TransDisc.GetTerminalVelocity());
			}
			TransDisc.Velocity.Z += TransDisc.TossZ;
		}
		TransDisc.SetTranslocationTarget(B.TranslocationTarget, TTargetLoc);
	}
}

// super desireable for bot waiting to translocate
function float GetAIRating()
{
	local UTBot B;

	B = UTBot(Instigator.Controller);
	if ( B == None )
	{
		return AIRating;
	}
	// useless if disc disrupted
	if (TransDisc != None && !TransDisc.bDeleteMe && TransDisc.Disrupted())
	{
		return -10.0;
	}
	// verify bHasTranslocator is set, since it might have gotten set to false if a disc was previously disrupted
	B.bHasTranslocator = true;
	if ( B.bPreparingMove && (B.TranslocationTarget != None) )
	{
		return 10;
	}
	if ( B.bTranslocatorHop && ((B.Focus == B.MoveTarget) || ((B.TranslocationTarget != None) && (B.Focus == B.TranslocationTarget))) && B.Squad.AllowTranslocationBy(B) )
	{
		if ( (TransDisc != None) && !TransDisc.bDeleteMe && TransDisc.IsMonitoring(TransDisc.TranslocationTarget) )
		{
			if ( (UTCarriedObject(B.Focus) != None) && (B.Focus != TransDisc.TranslocationTarget) )
			{
				if ( Instigator.Weapon == self )
					SetTimer(0.2,false,'FireHackTimer');
				return 4;
			}
			if  ( B.ValidTranslocationTarget(TransDisc.TranslocationTarget) )
				return 4;
		}

		if ( Instigator.Weapon == self )
			SetTimer(0.2,false,'FireHackTimer');
		return 4;
	}
	return AIRating;
}

simulated function StartFire( byte FireModeNum )
{
	if (UTBot(Instigator.Controller) == None)
	{
		Super.StartFire(FireModeNum);
	}
}

function bool BotTranslocation()
{
	local UTBot B;

	B = UTBot(Instigator.Controller);
	if ( (B == None) || !B.bPreparingMove || (B.TranslocationTarget == None) )
	{
		return false;
	}

	// if bot failed to translocate, event though beacon was in target cylinder,
	// try at center of cylinder
	return ( Instigator.SetLocation(B.TranslocationTarget.Location) );
}

simulated state Active
{
	simulated function BeginState(Name PreviousStateName)
	{
		super.BeginState(PreviousStateName);

		if ( UTBot(Instigator.Controller) != None )
		{
			SetTimer(0.01,false,'FireHackTimer');
		}
	}
}
// END AI interface

state WeaponFiring
{
	simulated function FireAmmunition()
	{
		if (CurrentFireMode==1 && ((TransDisc == none) || TransDisc.bDeleteMe) )
		{
			TimeWeaponFiring(CurrentFireMode);
			return;
		}

		Global.FireAmmunition();
	}

	/** @see WarWeapon::ShouldRefire() */
	simulated function bool ShouldRefire()
	{
		// in single fire more, it is not possible to refire. You have to release and re-press fire to shot every time
		if (CurrentFireMode==0)
			EndFire( CurrentFireMode );

		return false;
	}
}

simulated function bool DenyClientWeaponSet()
{
	return (TransDisc != none) && !TransDisc.bDeleteMe;
}

state RecallFire extends WeaponFiring
{
	simulated function bool ShouldLagRot()
	{
		// allow when recalling as there's no animation
		return true;
	}
}

defaultproperties
{
	WeaponColor=(R=255,G=255,B=128,A=255)
	PlayerViewOffset=(X=0.0,Y=0.0,Z=0.0)

	FireInterval(0)=+0.25
	FireInterval(1)=+0.25

	FireOffset=(X=15,Y=8,Z=-10)

	AIRating=-1.0
	CurrentRating=-1.0
	bInstantHit=false
	bSplashJump=false
	bRecommendSplashDamage=false
	bSniping=false
	ShouldFireOnRelease(0)=0
	ShouldFireOnRelease(1)=0

	InventoryGroup=0
	GroupWeight=0.6

	AmmoCount=7
	MaxAmmoCount=7

	RechargeRate=1.25

	ShotCost(0)=0
	ShotCost(1)=1

	bCanThrow=false
	bExportMenuData=false
}
