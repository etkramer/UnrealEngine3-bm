/**
 * Spotlight turret
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Turret_Spotlight extends Turret_TroikaCabal
	config(Pawn);

var()	SpotLightComponent SpotLightComp;

var		repnotify	bool	bDisabled;

replication
{
	if( Role == ROLE_Authority )
		bDisabled;
}

simulated function PostBeginPlay()
{
	// attach light
	Mesh.AttachComponentToSocket(SpotLightComp, MuzzleSocketName);

	super.PostBeginPlay();
}

simulated event ReplicatedEvent( name VarName )
{
	if( VarName == 'bDisabled' )
	{
		SetDisabled( bDisabled );
	}
	else
	{
		Super.ReplicatedEvent( VarName );
	}
}

simulated function WeaponFired(bool bViaReplication, optional vector HitLocation)
{
	// do nothing
}

simulated function WeaponStoppedFiring(bool bViaReplication)
{
	// do nothing
}

function OnToggle( SeqAct_Toggle inAction )
{
	if( inAction.InputLinks[0].bHasImpulse )
	{
		SetDisabled( FALSE );
	}
	else
	if( inAction.InputLinks[1].bHasImpulse )
	{
		SetDisabled( TRUE );
	}
	else
	{
		SetDisabled( !bDisabled );
	}
}

simulated function SetDisabled( bool inbDisabled )
{
	bDisabled = inbDisabled;
	SpotLightComp.SetEnabled( !bDisabled );
}

defaultproperties
{
	MuzzleSocketName="Muzzle"

	// camera vars
	CameraViewOffsetHigh=(X=-240,Y=0,Z=35)
	CameraViewOffsetLow=(X=-200,Y=0,Z=70)
	CameraViewOffsetMid=(X=-200,Y=0,Z=55)
	CameraTargetingViewOffsetHigh=(X=-140,Y=0,Z=45)
	CameraTargetingViewOffsetLow=(X=-140,Y=0,Z=45)
	CameraTargetingViewOffsetMid=(X=-140,Y=0,Z=45)
	ViewRotInterpSpeed=9.f

	Begin Object Name=CollisionCylinder
		CollisionHeight=50.000000
		CollisionRadius=20.000000
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=TRUE
		BlockActors=TRUE
		CollideActors=TRUE
		Translation=(X=70)
	End Object

	Begin Object Name=SkelMeshComponent0
		SkeletalMesh=SkeletalMesh'MS_Spotlight.SpotLightTurret'
		PhysicsAsset=PhysicsAsset'MS_Spotlight.SpotLightTurret_Physics'
		AnimTreeTemplate=AnimTree'Locust_TroikaCabal.AnimTree_TroikaCabal'
		BlockZeroExtent=TRUE
		CollideActors=TRUE
		BlockRigidBody=TRUE
		AlwaysLoadOnClient=TRUE
		AlwaysLoadOnServer=TRUE
		Scale=1.0
        Translation=(X=0,Z=-50) 
	End Object
	Mesh=SkelMeshComponent0
	Components.Add(SkelMeshComponent0)

	Begin Object Class=SpotLightComponent Name=SpotLight0
		InnerConeAngle=8.f
		OuterConeangle=12.f
		Radius=12000.f
		Brightness=12.0
		LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING
		CastShadows=TRUE
		CastStaticShadows=TRUE
		CastDynamicShadows=TRUE
		bForceDynamicLight=FALSE
		UseDirectLightMap=FALSE
		LightColor=(B=255,G=255,R=255,A=255)
		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
	End Object
	SpotLightComp=SpotLight0

	TrackSpeed=3.f
	SearchSpeed=0.5f

	TurretTurnRateScale=0.33
	ExitPositions(0)=(X=-110)
	EntryPosition=(X=-110)
	EntryRadius=100
	bRelativeExitPos=TRUE
	PitchBone=Gun_pivot_bone
	BaseBone=Rt_Tripod_bone
	ViewPitchMin=-6144
	ViewPitchMax=1024
	POV=(DirOffset=(X=-9,Y=2,Z=4.5),Distance=25,fZAdjust=-100)
	CannonFireOffset=(X=160,Y=0,Z=24)
	
	InventoryManagerClass=class'GearInventoryManager'
	
	DefaultInventory.Empty()
	//DefaultInventory(0)=class'GearWeap_UVSpotlight'

	InteractAction={(
	ActionName=UseSpotlight,
	ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=284,V=437,UL=59,VL=59)))	),
	)}
}
