/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTWeap_SniperRifle extends UTWeapon;

var class<UTDamageType> HeadShotDamageType;
var float HeadShotDamageMult;

var Texture2D HudMaterial;

var array<MaterialInterface> TeamSkins;

/** headshot scale factor when moving slowly or stopped */
var float SlowHeadshotScale;

/** headshot scale factor when running or falling */
var float RunningHeadshotScale;

/** Zoom minimum time*/
var bool bAbortZoom;

/** sound while the zoom is in progress */
var audiocomponent ZoomLoop;
var soundcue ZoomLoopCue;

/** Whether the standard crosshair should be displayed */
var bool bDisplayCrosshair;

/** tracks number of zoom started calls before zoom is ended */
var int ZoomCount;

var float	FadeTime;

//-----------------------------------------------------------------
// AI Interface

function float SuggestAttackStyle()
{
    return -0.4;
}

function float SuggestDefenseStyle()
{
    return 0.2;
}

function float GetAIRating()
{
	local UTBot B;
	local float ZDiff, dist, Result;

	B = UTBot(Instigator.Controller);
	if ( B == None )
		return AIRating;
	if ( B.IsShootingObjective() )
		return AIRating - 0.15;
	if ( B.Enemy == None )
		return AIRating;

	if ( B.Stopped() )
		result = AIRating + 0.1;
	else
		result = AIRating - 0.1;
	if ( Vehicle(B.Enemy) != None )
		result -= 0.2;
	ZDiff = Instigator.Location.Z - B.Enemy.Location.Z;
	if ( ZDiff < -200 )
		result += 0.1;
	dist = VSize(B.Enemy.Location - Instigator.Location);
	if ( dist > 2000 )
	{
		if ( !B.LineOfSightTo(B.Enemy) )
			result = result - 0.15;
		return ( FMin(2.0,result + (dist - 2000) * 0.0002) );
	}
	if ( !B.LineOfSightTo(B.Enemy) )
		return AIRating - 0.1;

	return result;
}

function bool RecommendRangedAttack()
{
	local UTBot B;

	B = UTBot(Instigator.Controller);
	if ( (B == None) || (B.Enemy == None) )
		return true;

	return ( VSize(B.Enemy.Location - Instigator.Location) > 2000 * (1 + FRand()) );
}

simulated function ProcessInstantHit( byte FiringMode, ImpactInfo Impact )
{
	local float Scaling;
	local int HeadDamage;

	if( (Role == Role_Authority) && !bUsingAimingHelp )
	{
		if (Instigator == None || VSize(Instigator.Velocity) < Instigator.GroundSpeed * Instigator.CrouchedPct)
		{
			Scaling = SlowHeadshotScale;
		}
		else
		{
			Scaling = RunningHeadshotScale;
		}

		HeadDamage = InstantHitDamage[FiringMode]* HeadShotDamageMult;
		if ( (UTPawn(Impact.HitActor) != None && UTPawn(Impact.HitActor).TakeHeadShot(Impact, HeadShotDamageType, HeadDamage, Scaling, Instigator.Controller)) ||
			(UTVehicleBase(Impact.HitActor) != None && UTVehicleBase(Impact.HitActor).TakeHeadShot(Impact, HeadShotDamageType, HeadDamage, Scaling, Instigator.Controller)) )
		{
			SetFlashLocation(Impact.HitLocation);
			return;
		}
	}

	super.ProcessInstantHit( FiringMode, Impact );
}


simulated function DrawZoomedOverlay( HUD H )
{
	local float ScaleX, ScaleY, StartX;
	local float OldOrgX, OldOrgY, OldClipX, OldClipY;

	// the sniper overlay is a special case that we want to ignore the safe region
	OldOrgX = H.Canvas.OrgX;
	OldOrgY = H.Canvas.OrgY;
	OldClipX = H.Canvas.ClipX;
	OldClipY = H.Canvas.ClipY;
	H.Canvas.OrgX = 0.0;
	H.Canvas.OrgY = 0.0;
	H.Canvas.ClipX = H.Canvas.SizeX;
	H.Canvas.ClipY = H.Canvas.SizeY;

	bDisplayCrosshair = false;

	// FIXME - USE ART DESIGNED FOR 1280x720 natively
	ScaleY = H.Canvas.ClipY/768.0;
	ScaleX = ScaleY;
	StartX = 0.5*H.Canvas.ClipX - 512.0*ScaleX;

	if ( (Instigator == None) || (Instigator.PlayerReplicationInfo == None)
		|| (Instigator.PlayerReplicationInfo.Team == None) || (Instigator.PlayerReplicationInfo.Team.TeamIndex == 0) )
	{
		H.Canvas.SetDrawColor(255,48,0);
	}
	else
	{
		H.Canvas.SetDrawColor(64,64,255);
	}

	// Draw the crosshair
	// Draw the 4 corners
	H.Canvas.SetPos(StartX, 0.0);
	H.Canvas.DrawTile(HudMaterial, 512.0 * ScaleX, 384.0 * ScaleY, 2, 0, 510, 383);

	H.Canvas.SetPos(H.Canvas.ClipX*0.5, 0.0);
	H.Canvas.DrawTile(HudMaterial, 512.0 * ScaleX, 384.0 * ScaleY, 510, 0, -510, 383);

	H.Canvas.SetPos(StartX, H.Canvas.ClipY*0.5);
	H.Canvas.DrawTile(HudMaterial, 512.0 * ScaleX, 384.0 * ScaleY, 2, 383, 510, -383);

	H.Canvas.SetPos(H.Canvas.ClipX*0.5, H.Canvas.ClipY*0.5);
	H.Canvas.DrawTile(HudMaterial, 512.0 * ScaleX, 384.0 * ScaleY, 510, 383, -510, -383);

	if ( StartX > 0 )
	{
		// Draw the Horizontal Borders
		H.Canvas.SetPos(0.0, 0.0);
		H.Canvas.DrawTile(HudMaterial, StartX, 384.0 * ScaleY, 1, 0, 3, 383);

		H.Canvas.SetPos(H.Canvas.ClipX - StartX, 0.0);
		H.Canvas.DrawTile(HudMaterial, StartX, 384.0 * ScaleY, 4, 0, -3, 383);

		H.Canvas.SetPos(0.0, H.Canvas.ClipY*0.5);
		H.Canvas.DrawTile(HudMaterial, StartX, 384.0 * ScaleY, 1, 383, 3, -383);

		H.Canvas.SetPos(H.Canvas.ClipX - StartX, H.Canvas.ClipY*0.5);
		H.Canvas.DrawTile(HudMaterial, StartX, 384.0 * ScaleY, 4, 383, -3, -383);
	}

	// restore the canvas parameters
	H.Canvas.OrgX = OldOrgX;
	H.Canvas.OrgY = OldOrgY;
	H.Canvas.ClipX = OldClipX;
	H.Canvas.ClipY = OldClipY;
}

simulated function DrawWeaponCrosshair( Hud HUD )
{
	local EZoomState ZoomState;
	local float FadeValue;
	local UTPlayerController PC;

	ZoomState = GetZoomedState();

	if ( ZoomState != ZST_NotZoomed )
	{
		if ( ZoomState > ZST_ZoomingOut )
		{
			DrawZoomedOverlay(HUD);
		}
		else
		{
			FadeValue = 255 * ( 1.0 - (WorldInfo.TimeSeconds - ZoomFadeTime)/FadeTime);
			HUD.Canvas.DrawColor.A = FadeValue;
			HUD.Canvas.SetPos(0,0);
			HUD.Canvas.DrawTile( Texture2D 'EngineResources.Black', HUD.Canvas.SizeX, HUD.Canvas.SizeY, 0.0, 0.0, 16, 16);
		}
	}
	if( bDisplayCrosshair )
	{
		PC = UTPlayerController(Instigator.Controller);
		if ( (PC == None) || PC.bNoCrosshair )
		{
			return;
		}
		super.DrawWeaponCrosshair(HUD);
	}
}

simulated function PreloadTextures(bool bForcePreload)
{
	Super.PreloadTextures(bForcePreload);

	if (HUDMaterial != None)
	{
		HUDMaterial.bForceMiplevelsToBeResident = bForcePreload;
	}
}

/** Called when zooming starts
 * @param PC - cast of Instigator.Controller for convenience
 */
simulated function StartZoom(UTPlayerController PC)
{
	ZoomCount++;
	if (ZoomCount == 1 && !IsTimerActive('Gotozoom') && IsActiveWeapon() && HasAmmo(0) && Instigator.IsFirstPerson())
	{
		bDisplayCrosshair = false;
		PlayWeaponAnimation('WeaponZoomIn',0.2);
		PlayArmAnimation('WeaponZoomIn',0.2);
		bAbortZoom = false;
		SetTimer(0.2, false, 'Gotozoom');
	}
}

simulated function EndFire(Byte FireModeNum)
{
	local UTPlayerController PC;

	// Don't bother performing if this is a dedicated server

	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		PC = UTPlayerController(Instigator.Controller);
		if (PC != None && LocalPlayer(PC.Player) != none && FireModeNum < bZoomedFireMode.Length && bZoomedFireMode[FireModeNum] != 0 )
		{
			if(GetZoomedState() == ZST_NotZoomed && ZoomCount != 0)
			{
				if(bAbortZoom)
				{
					ClearTimer('Gotozoom');
					LeaveZoom();
				}
				else
				{
					bAbortZoom=true;
				}
			}
			if(ZoomLoop != none)
			{
				ZoomLoop.Stop();
				ZoomLoop = none;
			}
		}
	}
	super.EndFire(FireModeNum);
}

/** Called when zooming ends
 * @param PC - cast of Instigator.Controller for convenience
 */
simulated function EndZoom(UTPlayerController PC)
{
	PlaySound(ZoomOutSound, true);
	bAbortZoom = false;
	if (IsTimerActive('Gotozoom'))
	{
		ClearTimer('Gotozoom');
	}
	SetTimer(0.001,false,'LeaveZoom');

}
simulated function LeaveZoom()
{
	local UTPlayerController PC;

	bAbortZoom = false;
	PC = UTPlayerController(Instigator.Controller);
	if (PC != none)
	{
		PC.EndZoom();
	}
	ZoomCount = 0;
	if(Instigator.IsFirstPerson())
	{
		ChangeVisibility(true);
	}

	PlayWeaponAnimation('WeaponZoomOut',0.3);
	PlayArmAnimation('WeaponZoomOut',0.3);
	SetTimer(0.3,false,'RestartCrosshair');

}
simulated function ChangeVisibility(bool bIsVisible)
{
	super.Changevisibility(bIsvisible);
	if(bIsVisible)
	{
		PlayArmAnimation('WeaponZoomOut',0.00001); // to cover zooms ended while in 3p
	}
	if(!Instigator.IsFirstPerson()) // to be consistent with not allowing zoom from 3p
	{
		LeaveZoom();
	}

}
simulated function RestartCrosshair()
{
	bDisplayCrosshair = true;
}

simulated function PutDownWeapon()
{
	ClearTimer('GotoZoom');
	ClearTimer('StopZoom');
	LeaveZoom();
	super.PutDownWeapon();
}

simulated function bool DenyClientWeaponSet()
{
	// don't autoswitch while zoomed
	return (GetZoomedState() != ZST_NotZoomed);
}

simulated function HolderEnteredVehicle()
{
	local UTPawn UTP;

	// clear timers and reset anims
	ClearTimer('GotoZoom');
	ClearTimer('StopZoom');
	PlayWeaponAnimation('WeaponZoomOut', 0.3);

	// partially copied from PlayArmAnimation() - we don't want to abort if third person here because they definitely will
	// since they just got in a vehicle
	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		UTP = UTPawn(Instigator);
		if (UTP != None)
		{
			// Check we have access to mesh and animations
			if (UTP.ArmsMesh[0] != None && ArmsAnimSet != None && GetArmAnimNodeSeq() != None)
			{
				UTP.ArmsMesh[0].PlayAnim('WeaponZoomOut', 0.3, false);
			}
		}
	}
}

simulated function Gotozoom()
{
	local UTPlayerController PC;

	PC = UTPlayerController(Instigator.Controller);
	if (GetZoomedState() == ZST_NotZoomed)
	{
		PC.FOVAngle = 40;
		Super.StartZoom(PC);
		ChangeVisibility(false);
		if (bAbortZoom) // stop the zoom after 1 tick
		{
			SetTimer(0.0001, false, 'StopZoom');
		}
		else
		{
			if(ZoomLoop == none)
			{
				ZoomLoop = CreateAudioComponent(ZoomLoopCue, false, true);
			}
			if(ZoomLoop != none)
			{
				ZoomLoop.Play();
			}
		}
	}

}
simulated function PlayWeaponPutDown()
{
	ClearTimer('GotoZoom');
	ClearTimer('StopZoom');
	if(UTPlayerController(Instigator.Controller) != none)
	{
		UTPlayerController(Instigator.Controller).EndZoom();
	}
	super.PlayWeaponPutDown();
}

simulated function StopZoom()
{
	local UTPlayerController PC;

	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		PC = UTPlayerController(Instigator.Controller);
		if (PC != None && LocalPlayer(PC.Player) != none)
		{
			PC.StopZoom();
		}
	}
}

simulated event CauseMuzzleFlash()
{
	if(GetZoomedState() == ZST_NotZoomed)
	{
		super.CauseMuzzleFlash();
	}
}
function byte BestMode()
{
	return 0;
}

simulated function SetSkin(Material NewMaterial)
{
	local int TeamIndex;

	if( ( Instigator != none ) && ( NewMaterial == none ) ) 	// Clear the materials
	{
		TeamIndex = Instigator.GetTeamNum();
		if (TeamIndex > TeamSkins.length)
		{
			TeamIndex = 0;
		}
		Mesh.SetMaterial(0,TeamSkins[TeamIndex]);
	}
	else
	{
		Super.SetSkin(NewMaterial);
	}
}

simulated function vector GetEffectLocation()
{
	// tracer comes from center if zoomed in
	return (GetZoomedState() != ZST_NotZoomed) ? Instigator.Location : Super.GetEffectLocation();
}

defaultproperties
{
	WeaponColor=(R=255,G=0,B=64,A=255)
	PlayerViewOffset=(X=0,Y=0,Z=0)

	Begin Object class=AnimNodeSequence Name=MeshSequenceA
	End Object

	// Weapon SkeletalMesh
	Begin Object Name=FirstPersonMesh
	    SkeletalMesh=SkeletalMesh'WP_SniperRifle.Mesh.SK_WP_SniperRifle_1P'
		AnimSets(0)=AnimSet'WP_SniperRifle.Anims.K_WP_SniperRifle_1P_Base'
		Animations=MeshSequenceA
		Scale=1.5
		bForceUpdateAttachmentsInTick=true
		FOV=65
	End Object
	AttachmentClass=class'UTAttachment_SniperRifle'

	ArmsAnimSet=AnimSet'WP_SniperRifle.Anims.K_WP_SniperRifle_1P_Arms'

	// Pickup staticmesh
	Components.Remove(PickupMesh)
	Begin Object Name=PickupMesh
		SkeletalMesh=SkeletalMesh'WP_SniperRifle.Mesh.SK_WP_SniperRifle_3P_Mid'
	End Object

	ZoomLoopCue=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ZoomLoop_Cue'

	InstantHitDamage(0)=70
	InstantHitDamage(1)=0
	InstantHitMomentum(0)=10000.0

	FireInterval(0)=+1.33
	FireInterval(1)=+1.33

	FiringStatesArray(1)=Active

	InstantHitDamageTypes(0)=class'UTDmgType_SniperPrimary'
	InstantHitDamageTypes(1)=None

	WeaponFireSnd[0]=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_Fire_Cue'
	//WeaponFireSnd[1]=SoundCue'A_Weapon.SniperRifle.Cue.A_Weapon_SN_Fire01_Cue'
	WeaponEquipSnd=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_Raise_Cue'
	WeaponPutDownSnd=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_Lower_Cue'
	ZoomOutSound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ZoomOut_Cue'
	ZoomInSound=SoundCue'A_Weapon_Sniper.Sniper.A_Weapon_Sniper_ZoomIn_Cue'

	WeaponFireTypes(0)=EWFT_InstantHit

	LockerRotation=(pitch=0,yaw=0,roll=-16384)

	MaxDesireability=0.63
	AIRating=+0.7
	CurrentRating=+0.7
	bInstantHit=true
	bSplashJump=false
	bRecommendSplashDamage=false
	bSniping=true
	bWarnIfInLocker=true
	ShouldFireOnRelease(0)=0
	ShouldFireOnRelease(1)=0
	InventoryGroup=9
	GroupWeight=0.5
	AimError=600

	PickupSound=SoundCue'A_Pickups.Weapons.Cue.A_Pickup_Weapons_Sniper_Cue'

	AmmoCount=10
	LockerAmmoCount=10
	MaxAmmoCount=40

	HeadShotDamageType=class'UTDmgType_SniperHeadShot'
	HeadShotDamageMult=2.0
	SlowHeadshotScale=1.75
	RunningHeadshotScale=0.8

	// Configure the zoom

	bZoomedFireMode(0)=0
	bZoomedFireMode(1)=1

	ZoomedTargetFOV=12.0
	ZoomedRate=30.0

	FadeTime=0.3

	HudMaterial=Texture2D'WP_SniperRifle.Textures.T_SniperCrosshair'

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'WP_SniperRifle.Effects.P_WP_SniperRifle_MuzzleFlash'
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'UTGame.UTRocketMuzzleFlashLight'

	IconX=451
	IconY=448
	IconWidth=54
	IconHeight=51

	EquipTime=+0.6
	PutDownTime=+0.45
	CrossHairCoordinates=(U=256,V=64,UL=64,VL=64)
	IconCoordinates=(U=726,V=532,UL=165,VL=51)
	QuickPickGroup=7
	QuickPickWeight=0.8

	TeamSkins[0]=MaterialInterface'WP_SniperRifle.Materials.M_WP_SniperRifle'
	TeamSkins[1]=MaterialInterface'WP_SniperRifle.Materials.M_WP_SniperRifle_Blue'

	bDisplaycrosshair = true;

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=30,RightAmplitude=50,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.200)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1
}
