/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustBoomerFlame extends GearPawn_LocustBoomer
	config(Pawn)
	implements(AIObjectAvoidanceInterface);

var	StaticMeshComponent	TankStaticMesh;

var repnotify GDO_FlameDroneTank FlameTanks;

/** General material used to control common pawn material parameters (e.g. burning) */
var protected transient MaterialInstanceConstant MIC_FlameTanks;

var protected const array<SoundCue>		BoomerAboutToExplodeDialogue;

replication
{
	if(ROLE==ROLE_Authority)
		FlameTanks;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'FlameTanks')
	{
		if(FlameTanks != none)
		{
			FlameTanks.SetHardAttach(true);
			FlameTanks.SetBase(self,,Mesh,'Flametanks');
			FlameTanks.FlameTankMesh.SetLightEnvironment( LightEnvironment );
			FlameTanks.FlameTankMesh.SetShadowParent( Mesh );

			if( MIC_FlameTanks == none )
			{
				MIC_FlameTanks = FlameTanks.FlameTankMesh.CreateAndSetMaterialInstanceConstant(0);
			}
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}
/**
---> AIobjectAvoidanceInterface
**/
function bool ShouldAvoid(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	if(FastTrace(Location, AskingAI.Pawn.Location,,true))
	{
		return true;
	}

	return false;
}
function bool ShouldEvade(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	// evade if it's really close
	if(VSizeSq(AskingAI.Pawn.Location - Location) < 300.f*300.f)
	{
		return true;
	}
	return false;
}
function bool ShouldRoadieRun(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	return true;
}


simulated function SetupHelmet()
{
	Super.SetupHelmet();
}

simulated function Destroyed()
{
	super.Destroyed();

	if(FlameTanks != none)
	{
		FlameTanks.Destroy();
	}
}

/** This will create the MIC for this pawn's materials **/
simulated protected function InitMICPawnMaterial()
{
	Super.InitMICPawnMaterial();

	if(ROLE == ROLE_Authority)
	{
		// Attach flame tanks
		FlameTanks = Spawn(class'GDO_FlameDroneTank_Boomer',self,,Location,,,true);
		FlameTanks.Init(self);
		FlameTanks.SetHardAttach(true);
		FlameTanks.SetBase(self,,Mesh,'Flametanks');
		FlameTanks.FlameTankMesh.SetLightEnvironment( LightEnvironment );
		FlameTanks.FlameTankMesh.SetShadowParent( Mesh );
	}

	MIC_FlameTanks = FlameTanks.FlameTankMesh.CreateAndSetMaterialInstanceConstant(0);
}


/** Heats up the skin by the given amount.  Skin heat constantly diminishes in Tick(). */
simulated function HeatSkin(float HeatIncrement)
{
	Super.HeatSkin( HeatIncrement );

	MIC_FlameTanks.SetScalarParameterValue('Heat', CurrentSkinHeat);
}

/** Char skin to the given absolute char amount [0..1].  Charring does not diminish over time. */
simulated function CharSkin(float CharIncrement)
{
	Super.CharSkin( CharIncrement );

	MIC_FlameTanks.SetScalarParameterValue('Burn', CurrentSkinChar);
}

simulated function FlameTankAboutToBlow()
{
	local int RandIdx;

	if (FRand() < 0.5f)
	{
		super.FlameTankAboutToBlow();
	}
	else
	{
		if ( (Health > 0) && (BoomerAboutToExplodeDialogue.length > 0) )
		{
			RandIdx = Rand(BoomerAboutToExplodeDialogue.length);

			// make addressee AI's target?
			SpeakLine(None, BoomerAboutToExplodeDialogue[RandIdx], "", 0.f, Speech_GUDS,,,,,, TRUE);
		}
	}
}


defaultproperties
{
	bCanBeBaseForPawns=TRUE
	bNoEncroachCheck=TRUE

	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer2Chatter")
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer2Chatter_Dup")

	HelmetType=class'Item_Helmet_LocustBoomerFlame'
	DefaultWeapon=class'GearGameContent.GearWeap_Boomer_FlameThrower'
	DefaultInventory(0)=class'GearGameContent.GearWeap_Boomer_FlameThrower'

	Begin Object Class=StaticMeshComponent Name=SlotTankStaticMesh1
		CollideActors=FALSE
		BlockRigidBody=FALSE
		HiddenGame=FALSE
		AlwaysLoadOnClient=TRUE
		StaticMesh=StaticMesh'Locust_Hunter.flamethrower_tanks'
		bCastDynamicShadow=FALSE
		LightEnvironment=MyLightEnvironment
	End Object
	TankStaticMesh=SlotTankStaticMesh1

	// flame boomer is boomerB, says "burn"
	BoomerAttackTelegraphDialogue.Empty
	BoomerAttackTelegraphDialogue(0)=SoundCue'Locust_Boomer2_Chatter_Cue.DupedRefsForCode.Boomer2Chatter_Burn_Medium01Cue_Code'
	BoomerAttackTelegraphDialogue(1)=SoundCue'Locust_Boomer2_Chatter_Cue.DupedRefsForCode.Boomer2Chatter_Burn_Medium02Cue_Code'

	BoomerAttackChuckleDialogue.Empty
	BoomerAttackChuckleDialogue(0)=SoundCue'Locust_Boomer1_Chatter_Cue.DupedRefsForCode.Boomer1Chatter_Laugh_Medium09Cue_Code'
	BoomerAttackChuckleDialogue(1)=SoundCue'Locust_Boomer2_Chatter_Cue.KilledEnemy.Boomer2Chatter_Laugh_Loud02Cue'
	BoomerAttackChuckleDialogue(2)=SoundCue'Locust_Boomer2_Chatter_Cue.DupedRefsForCode.Boomer2Chatter_Laugh_Medium09Cue_Code'
	BoomerAttackChuckleDialogue(3)=SoundCue'Locust_Boomer2_Chatter_Cue.DupedRefsForCode.Boomer2Chatter_Laugh_Medium10Cue_Code'
	BoomerAttackChuckleChance=0.7f

	BoomerAboutToExplodeDialogue(0)=SoundCue'Locust_Boomer1_Chatter_Cue.DupedRefsForCode.Boomer1Chatter_No_Loud01Cue_Code'
	BoomerAboutToExplodeDialogue(1)=SoundCue'Locust_Boomer1_Chatter_Cue.DupedRefsForCode.Boomer1Chatter_No_Loud02Cue_Code'
	BoomerAboutToExplodeDialogue(2)=SoundCue'Locust_Boomer1_Chatter_Cue.DupedRefsForCode.Boomer1Chatter_No_Loud03Cue_Code'
	bPlayWeaponFireAnim=false
}
