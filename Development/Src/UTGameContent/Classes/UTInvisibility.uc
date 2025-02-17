/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class UTInvisibility extends UTTimedPowerup;

var Material InvisibilityMaterial;
var SoundCue AmbientSound;
var SoundCue WarningSound;

function GivenTo(Pawn NewOwner, optional bool bDoNotActivate)
{
	local UTPawn P;

	Super.GivenTo(NewOwner, bDoNotActivate);

	// boost damage
	P = UTPawn(NewOwner);
	if (P != None)
	{
		// apply invisibility material
		P.SetSkin(InvisibilityMaterial);
		P.SetInvisible(true);
		P.SetPawnAmbientSound(AmbientSound);
		SetTimer(TimeRemaining - 3.0, false, 'PlayWarningSound');
	}
}

/** Plays a warning the powerup is running low */
function PlayWarningSound()
{
	// reset timer if time got added
	if (TimeRemaining > 3.0)
	{
		SetTimer(TimeRemaining - 3.0, false, 'PlayWarningSound');
	}
	else
	{
		Instigator.PlaySound(WarningSound);
		SetTimer(0.75, false, 'PlayWarningSound');
	}
}


function ItemRemovedFromInvManager()
{
	local UTPlayerReplicationInfo UTPRI;
	local UTPawn P;

	P = UTPawn(Owner);
	if (P != None)
	{
		P.SetSkin(None);
		P.SetInvisible(P.default.bIsInvisible);
		P.SetPawnAmbientSound(None);
		ClearTimer('PlayWarningSound');

		//Stop the timer on the powerup stat
		if (P.DrivenVehicle != None)
		{
			UTPRI = UTPlayerReplicationInfo(P.DrivenVehicle.PlayerReplicationInfo);
		}
		else
		{
			UTPRI = UTPlayerReplicationInfo(P.PlayerReplicationInfo);
		}
		if (UTPRI != None)
		{
			UTPRI.StopPowerupTimeStat(GetPowerupStatName());
		}
	}
}

defaultproperties
{
	PowerupStatName=POWERUPTIME_INVISIBILITY
	Begin Object Class=StaticMeshComponent Name=MeshComponentA
		StaticMesh=StaticMesh'Pickups.Invis.Mesh.S_Pickups_Invisibility'
		AlwaysLoadOnClient=true
		AlwaysLoadOnServer=true
		CastShadow=false
		bForceDirectLightMap=true
		bCastDynamicShadow=false
		bAcceptsLights=true
		CollideActors=false
		BlockRigidBody=false
		MaxDrawDistance=8000
		bUseAsOccluder=FALSE
		Translation=(X=0.0,Y=0.0,Z=-20.0)
	End Object
	DroppedPickupMesh=MeshComponentA
	PickupFactoryMesh=MeshComponentA

	Begin Object Class=UTParticleSystemComponent Name=InvisParticles
		Template=ParticleSystem'Pickups.Invis.Effects.P_Pickups_Invis_Idle'
		bAutoActivate=false
		SecondsBeforeInactive=1.0f
		Translation=(X=0.0,Y=0.0,Z=-20.0)
	End Object
	DroppedPickupParticles=InvisParticles

	bReceiveOwnerEvents=true
	bRenderOverlays=true
	PickupSound=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_Invisibility_PickupCue'

	InvisibilityMaterial=Material'Pickups.Invis.M_Invis_01'
	HudIndex=2

	AmbientSound=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_Invisibility_PowerLoopCue'

	WarningSound=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_Invisibility_WarningCue'
	PowerupOverSound=SoundCue'A_Pickups_Powerups.PowerUps.A_Powerup_Invisibility_EndCue'
	IconCoords=(U=744,UL=48,V=55,VL=54)
	PP_Scene_Highlights=(Y=-0.07,Z=-0.14)
	PP_Scene_Desaturation=0.16
}
