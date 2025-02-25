/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTArmorPickupFactory extends UTItemPickupFactory
	abstract
	native;

var int		ShieldAmount;
var UTParticleSystemComponent ParticleEffects;

cpptext
{
	virtual void CheckForErrors();
}

simulated static function UpdateHUD(UTHUD H)
{
	Super.UpdateHUD(H);
	H.LastArmorPickupTime = H.LastPickupTime;
}

simulated function SetPickupVisible()
{
	if(ParticleEffects != none)
	{
		ParticleEffects.SetActive(true);
	}
	super.SetPickupVisible();
}
simulated function SetPickupHidden()
{
	if(ParticleEffects != none)
		ParticleEffects.DeactivateSystem();
	super.SetPickupHidden();
}

simulated function PostBeginPlay()
{
	if(!bPickupHidden)
	{
		SetPickupVisible();
	}
	Super.PostBeginPlay();
}

function SpawnCopyFor( Pawn Recipient )
{
	if ( UTPawn(Recipient) == None )
		return;

	if ( UTPlayerReplicationInfo(Recipient.PlayerReplicationInfo) != None )
	{
		UTPlayerReplicationInfo(Recipient.PlayerReplicationInfo).IncrementPickupStat(GetPickupStatName());
	}

	// Give armor to recipient
	Recipient.MakeNoise(0.2);
	AddShieldStrength(UTPawn(Recipient));
	super.SpawnCopyFor(Recipient);
}

/** CanUseShield()
returns how many shield units P could use
*/
function int CanUseShield(UTPawn P)
{
	return 0;
}

/** AddShieldStrength()
add shield to appropriate P armor type.
*/
function AddShieldStrength(UTPawn P);

//=============================================================================
// Pickup state: this inventory item is sitting on the ground.

auto state Pickup
{
	/* DetourWeight()
	value of this path to take a quick detour (usually 0, used when on route to distant objective, but want to grab inventory for example)
	*/
	function float DetourWeight(Pawn P,float PathWeight)
	{
		local float Need;
		local UTPawn Other;

		Other = UTPawn(P);
		if ( Other == None )
			return 0;
		Need = CanUseShield(Other);
		if ( AIController(Other.Controller).PriorityObjective() && (Need < 0.4 * ShieldAmount) )
			return (0.005 * MaxDesireability * Need)/PathWeight;
		if ( Need <= 0 )
		{
			if ( !WorldInfo.Game.bTeamGame )
				Need = 0.5;
			else
				return 0;
		}
		else if ( !WorldInfo.Game.bTeamGame )
			Need = FMax(Need,0.6);
		return (0.013 * MaxDesireability * Need)/PathWeight;
	}

	/* ValidTouch()
	 Validate touch (if valid return true to let other pick me up and trigger event).
	*/
	function bool ValidTouch( Pawn Other )
	{
		if ( !Super.ValidTouch(Other) )
		{
			return false;
		}

		// does Other need armor?
		return ( (CanUseShield(UTPawn(Other)) > 0) || !WorldInfo.Game.bTeamGame );
	}
}

function float BotDesireability(Pawn Bot, Controller C)
{
	local float Desire;

	if ( UTPawn(Bot) == None )
		return 0;

	Desire = (0.013 * MaxDesireability * CanUseShield(UTPawn(Bot)));
	if (!WorldInfo.Game.bTeamGame && UTBot(C) != None && UTBot(C).Skill >= 4.0)
	{
		// high skill bots keep considering powerups that they don't need if they can still pick them up
		// to deny the enemy any chance of getting them
		Desire = FMax(Desire, 0.001);
	}
	return Desire;
}

defaultproperties
{
	bMovable=FALSE
	bStatic=FALSE

    PickupStatName=PICKUPS_ARMOR
    MaxDesireability=1.500000
	YawRotationRate=24000
	bRotatingPickup=true
	bPredictRespawns=true

    ShieldAmount=20

	RespawnSound=SoundCue'A_Pickups.Armor.Cue.A_Pickups_Armor_Respawn_Cue'

	Begin Object Name=BaseMeshComp
		StaticMesh=StaticMesh'Pickups.Base_Armor.Mesh.S_Pickups_Base_Armor'
		Translation=(X=0.0,Y=0.0,Z=-44.0)
	End Object

	BaseBrightEmissive=(R=25.0,G=25.0,B=1.0)
	BaseDimEmissive=(R=1.0,G=1.0,B=0.01)

	Begin Object Class=UTParticleSystemComponent Name=ArmorParticles
		Translation=(X=0.0,Y=0.0,Z=-25.0)
		Template=ParticleSystem'Pickups.Base_Armor.Effects.P_Pickups_Base_Armor_Glow'
		SecondsBeforeInactive=2.0f
	End Object
	ParticleEffects=ArmorParticles
	Components.Add(ArmorParticles)

	Begin Object Class=StaticMeshComponent Name=ArmorPickUpComp
		AlwaysLoadOnClient=true
		AlwaysLoadOnServer=true

		CastShadow=FALSE
		bAcceptsLights=TRUE
		bForceDirectLightMap=TRUE
		bCastDynamicShadow=FALSE
		LightEnvironment=PickupLightEnvironment

		CollideActors=false
		MaxDrawDistance=8000
		bUseAsOccluder=FALSE
	End Object
	PickupMesh=ArmorPickUpComp
	Components.Add(ArmorPickUpComp)
}
