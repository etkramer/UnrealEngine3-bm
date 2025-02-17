
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustBoomerButcher extends GearPawn_LocustBoomer
	config(Pawn);

/** Item Held by that character */
var class<ItemSpawnable>	HeldItem;
var StaticMeshComponent		HeldItemStaticMesh;

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	SetupHeldItem();
}

simulated function SetupHeldItem()
{
	local SpawnableItemDatum ItemDatum;

	ItemDatum = HeldItem.static.GetSpawnableItemDatum();

	if( ItemDatum.TheMesh != None )
	{
		// figure out the mesh to attach
		HeldItemStaticMesh.SetStaticMesh( ItemDatum.TheMesh );

		// need to possibly adjust the location (e.g. goggles)
		//`log( "Setting trans: " $ ItemDatum.Translation $ " for " $ ItemDatum.TheMesh @ HelmetType );
		HeldItemStaticMesh.SetTranslation( ItemDatum.Translation );

		// attach the head mesh to the body
		HeldItemStaticMesh.SetShadowParent(Mesh);
		HeldItemStaticMesh.SetLightEnvironment(LightEnvironment);
		HeldItemStaticMesh.SetHidden(FALSE);
		Mesh.AttachComponentToSocket(HeldItemStaticMesh, ItemDatum.AttachSocketName);

		`DLog("Attached" @ ItemDatum.TheMesh @ "to" @  ItemDatum.AttachSocketName);
	}
	else
	{
		`DLog("ItemDatum.TheMesh == None!!");
	}
}

simulated function HideMesh()
{
	Super.HideMesh();

	if( HeldItemStaticMesh != None )
	{
		HeldItemStaticMesh.SetHidden(TRUE);
	}
}

simulated function SetDepthPriorityGroup(ESceneDepthPriorityGroup NewDepthPriorityGroup)
{
	local bool bUseViewOwnerDepthPriorityGroup;

	Super.SetDepthPriorityGroup( NewDepthPriorityGroup );

	// If we're a kidnapper or a hostage, ignore DepthPriorityGroup setting
	// As this is creating sorting issues because both actors are not owned by the same viewport.
	// We don't really care because you can't go in cover in this context.
	if( IsAHostage() || IsAKidnapper() )
	{
		NewDepthPriorityGroup = SDPG_World;
	}

	if( Mesh != None )
	{
		bUseViewOwnerDepthPriorityGroup = NewDepthPriorityGroup != SDPG_World;
		if( HeldItemStaticMesh != None )
		{
			HeldItemStaticMesh.SetViewOwnerDepthPriorityGroup(bUseViewOwnerDepthPriorityGroup, NewDepthPriorityGroup);
		}
	}
}

simulated function RemoveAndSpawnHeldItem(Vector ApplyImpulse, class<DamageType> DamageType)
{
	local vector				SpawnLoc;
	local rotator				SpawnRot;
	local float					Radius, Height;
	local SpawnableItemDatum	ItemDatum;
	local ItemSpawnable			AnItem;

	GetBoundingCylinder( Radius, Height );

	HeldItemStaticMesh.SetHidden( TRUE );
	Mesh.DetachComponent(HeldItemStaticMesh);

	ItemDatum = HeldItem.static.GetSpawnableItemDatum();

	if( ItemDatum.TheMesh != None )
	{
		Mesh.GetSocketWorldLocationAndRotation(ItemDatum.AttachSocketName, SpawnLoc, SpawnRot);

		AnItem = Spawn(HeldItem,,, SpawnLoc,,, TRUE);
		AnItem.StaticMeshComponent.SetStaticMesh( ItemDatum.TheMesh );
		AnItem.StaticMeshComponent.SetLightEnvironment( LightEnvironment );
		AnItem.StaticMeshComponent.SetPhysMaterialOverride( PhysicalMaterial'GearPhysMats.Metal' );

		AnItem.CollisionComponent.AddImpulse( ApplyImpulse * 1.2, vect(0,0,100) );
		AnItem.CollisionComponent.SetRBAngularVelocity( vect(0,100,300), TRUE );

		AnItem.PlayShotOffSound();
	}

	if( AnItem != None )
	{
		AnItem.TurnCollisionOff();
		AnItem.SetTimer( 0.100f, FALSE, nameof(AnItem.TurnCollisionOn) ); // to stop inter penetrating and then OOE as physics corrects and shoots it off
	}
}

simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	local vector ApplyImpulse;

	Super.PlayDying(DamageType, HitLoc);

	ApplyImpulse = TearOffMomentum * DamageType.default.KDamageImpulse + VSize(TearOffMomentum) * Vect(0, 0, 1);
	RemoveAndSpawnHeldItem(ApplyImpulse, DamageType);
}

defaultproperties
{
	HelmetType=class'Item_Helmet_LocustBoomerButcher'
	HeldItem=class'Item_BoomerCleaver'
	DefaultInventory(0)=class'GearWeap_BoomerButcherMelee'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustBoomerB'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustBoomerB"

	FAS_ChatterNames.Empty
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.Chatter")
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.Chatter_Dup")
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer2Chatter")
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer2Chatter_Dup")
	// need boomer1chatter too?

	GoreSkeletalMesh=SkeletalMesh'Locust_Boomer.Gore.Locust_BoomerButcher_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Boomer.Mesh.Locust_Butcher_Boomer_CamSkel_Physics'
	GoreBreakableJointsTest=("b_MF_Head","b_MF_Face","b_MF_Forearm_L","b_MF_Hand_L","b_MF_Forearm_R","b_MF_Hand_R","b_MF_Calf_R","b_MF_Foot_L","b_Boomer_Hook1","b_Boomer_Hook2")
	GoreBreakableJoints=("b_MF_Head","b_MF_Face","b_MF_Forearm_L","b_MF_Hand_L","b_MF_Forearm_R","b_MF_Hand_R","b_MF_Calf_R","b_MF_Foot_L","b_Boomer_Hook1","b_Boomer_Hook2")

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Boomer.Mesh.Locust_Butcher_Boomer'
		PhysicsAsset=PhysicsAsset'Locust_Boomer.Mesh.Locust_Butcher_Boomer_CamSkel_Physics'
		AnimTreeTemplate=AnimTree'Locust_Boomer.Animations.AT_LocustBoomer_Butcher'
		AnimSets.Add(AnimSet'Locust_Boomer.Animations.AnimSetBoomer_CamSkel_Cleaver')
		bEnableFullAnimWeightBodies=TRUE
	End Object

	BS_MeleeAttack=(AnimName[BS_Std_Up]="CL_MeleeAttack",AnimName[BS_Std_Idle_Lower]="CL_MeleeAttack")

	Begin Object Class=StaticMeshComponent Name=HeldItemStaticMesh0
	    CollideActors=FALSE
		BlockRigidBody=FALSE
		HiddenGame=TRUE
		AlwaysLoadOnClient=TRUE
		bCastDynamicShadow=FALSE
		LightEnvironment=MyLightEnvironment
		MotionBlurScale=0.0
	End Object
	HeldItemStaticMesh=HeldItemStaticMesh0

	// butcher boomer is boomerB, says "hunger"
	BoomerAttackTelegraphDialogue.Empty
	BoomerAttackTelegraphDialogue(0)=SoundCue'Locust_Boomer_Chatter_Cue.BoomerSingle.BoomerChatter_Hunger03Cue'
	BoomerAttackTelegraphDialogue(1)=SoundCue'Locust_Boomer_Chatter_Cue.DupedRefsForCode.BoomerChatter_Hunger01Cue_Code'
	BoomerAttackTelegraphDialogue(2)=SoundCue'Locust_Boomer_Chatter_Cue.BoomerSingle.BoomerChatter_Hunger02Cue'
	bPlayWeaponFireAnim=false
}
