/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_COGDom extends GearPawn_COGGear
	config(Pawn)
	native(Pawn);

/** The amount of HP dom should simulate having @see AdjustPawnDamage **/
var config float DomAIHitPoints;
var float RatioOfDefaultToSimulatedHP;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// save off the ratio so we don't have to calc it each time
	RatioOfDefaultToSimulatedHP = DomAIHitPoints / 175;
}

simulated event Destroyed()
{
	CharacterTexturesForceMipsNonResident();

	Super.Destroyed();
}


simulated function ClientRestart()
{
	Super.ClientRestart();

	if( GearPC(Controller) != None )
	{
		// for all human controlled pawns we want to make certain their textures are streamed in
		CharacterTexturesForceMipsResident();
	}
}


simulated function ChainSawGore()
{
	BreakConstraint( vect(100,0,0), vect(0,10,0), 'b_MF_Spine_03' );
	BreakConstraint( vect(0,100,0), vect(0,0,10), 'b_MF_UpperArm_R' );
}

defaultproperties
{
	CharacterFootStepType=CFST_COG_Dom

	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=49,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	ControllerClass=class'GearAI_Dom'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGDom'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGDom"
	NeedsRevivedGUDSEvent=GUDEvent_DomNeedsRevived;
	WentDownGUDSEvent=GUDEvent_DomWentDown;
	FAS_ChatterNames.Add("COG_Dom.FaceFX.Chatter")
	FAS_ChatterNames.Add("COG_Dom.FaceFX.Dom_Gears1_Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'COG_Dom.FaceFX.Dom_FaceFX_Efforts'


	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
    GoreSkeletalMesh=SkeletalMesh'COG_Dom.Cog_Dom_Game_Gore_CamSkel'
	GorePhysicsAsset=PhysicsAsset'COG_Dom.COG_Dom_CamSkel_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_R","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_Calf_R","b_MF_Calf_L","b_MF_Foot_R")
	HostageHealthBuckets=("b_MF_Hand_L","b_MF_UpperArm_R","b_MF_Calf_L","b_MF_Calf_R")

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Dom.Game.COG_Dom_Game_CamSkel'
		PhysicsAsset=PhysicsAsset'COG_Dom.COG_Dom_CamSkel_Physics'
	End Object

	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGame.GearWeap_AssaultRifle'
	DefaultInventory(1)=class'GearGame.GearWeap_COGPistol'
	DefaultInventory(2)=class'GearGame.GearWeap_ShotGun'

	bKillDuringLevelTransition=FALSE
}
