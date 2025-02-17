/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustPalaceGuard extends GearPawn_LocustTheron
	config(Pawn);

var	Array<StaticMeshComponent>	AttachmentSM;
var Array<Name>					AttachmentSocketName;

/** The item spawnable class to use when detataching and spawning the attachments **/
var array< class<ItemSpawnable> > AttachmentSpawnClasses;

simulated function SetupHelmet()
{
	local int i;

	Super.SetupHelmet();

	// Attachments
	for(i=0; i<AttachmentSM.Length; i++)
	{
		// Attach tanks to chest
		AttachmentSM[i].SetShadowParent(Mesh);
		AttachmentSM[i].SetLightEnvironment(LightEnvironment);
		Mesh.AttachComponentToSocket(AttachmentSM[i], AttachmentSocketName[i]);
	}
}


/** This will return whether or not this mob can be special melee attacked **/
simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	// if we're on a bloodmount you can't chainsaw us!
	if(GearPawn_LocustBloodmount(Base) != none)
	{
		return false;
	}

	return Super.CanBeSpecialMeleeAttacked(Attacker);
}

/** This will do what ever is needed to reset materials to allow the gore overlay material to show up **/
simulated function SetMainBodyMaterialToNoneToClearItForGoreMaterial()
{
	Mesh.SetMaterial( 0, Material'Locust_Theron_Guard.LOD_MERGE.Locust_Theron_Guard_Copper_MERGEGORE_UV_Mat' ); // this is needed as we have to "clear" the material instance on the mesh to make the gore material which is affecting the same verts can be seen. 
	Mesh.SetMaterial( 1, Material'COG_Gore.COG_NEWGore_Mat' );
}

function RemoveAndDropAttachments( Vector ApplyImpulse, class<DamageType> DamageType )
{
	local int AttachIdx;
	local StaticMeshComponent SMC;
	local ItemSpawnable	AnItem;
	local vector SpawnLoc;
	local rotator SpawnRot;
	local SpawnableItemDatum ItemDatum;

	for( AttachIdx = 0; AttachIdx < AttachmentSM.length; ++AttachIdx )
	{
		if( AttachmentSpawnClasses[AttachIdx] == none )
		{
			continue;
		}

		ItemDatum = AttachmentSpawnClasses[AttachIdx].static.GetSpawnableItemDatum();

		Mesh.GetSocketWorldLocationAndRotation( ItemDatum.AttachSocketName, SpawnLoc, SpawnRot );

		SMC = AttachmentSM[AttachIdx];

		SMC.SetHidden( TRUE );
		//Mesh.DetachComponent( SMC );

		AnItem = Spawn( AttachmentSpawnClasses[AttachIdx],,, SpawnLoc, SpawnRot,, TRUE );
		AnItem.StaticMeshComponent.SetStaticMesh( ItemDatum.TheMesh );
		AnItem.StaticMeshComponent.SetLightEnvironment( LightEnvironment );

		AnItem.CollisionComponent.AddImpulse( ApplyImpulse * 1.2, vect(0,0,100) );
		AnItem.CollisionComponent.SetRBAngularVelocity( vect(0,100,300), TRUE );

		if( AnItem != None )
		{
			AnItem.TurnCollisionOff();
			AnItem.SetTimer( 0.100f, FALSE, nameof(AnItem.TurnCollisionOn) ); // to stop inter penetrating and then OOE as physics corrects and shoots it off
		}
	}
}



defaultproperties
{
	HeadSocketName="palaceguard_helmet" // needed as the helmet's rotation doesn't match the other helmets :-\
	HelmetType=class'Item_Helmet_LocustPalaceGuard'

	Begin Object Name=GearPawnMesh
		Materials(0)=Material'Locust_Theron_Guard.LOD_MERGE.Locust_Theron_Gaurd_Copper_UV_MERGE_M'
		AnimSets.Add(AnimSet'Locust_Theron_Guard.Animations.Bloodmount_Animset') // this could be moved to a subclass
	End Object

	Begin Object Class=StaticMeshComponent Name=AttachmentSM0
	    CollideActors=FALSE
		BlockRigidBody=FALSE
		HiddenGame=FALSE
		AlwaysLoadOnClient=TRUE
		StaticMesh=StaticMesh'Locust_Theron_Guard.palaceguard_arm_LEFT'
		bCastDynamicShadow=FALSE
		LightEnvironment=MyLightEnvironment
	End Object
	AttachmentSM(0)=AttachmentSM0
	AttachmentSocketName(0)="palaceguard_arm_LEFT"
	AttachmentSpawnClasses(0)=none // class'Item_PalaceGuardAttachment_ArmLeft'

	Begin Object Class=StaticMeshComponent Name=AttachmentSM1
	    CollideActors=FALSE
		BlockRigidBody=FALSE
		HiddenGame=FALSE
		AlwaysLoadOnClient=TRUE
		StaticMesh=StaticMesh'Locust_Theron_Guard.palaceguard_arm_RIGHT'
		bCastDynamicShadow=FALSE
		LightEnvironment=MyLightEnvironment
	End Object
	AttachmentSM(1)=AttachmentSM1
	AttachmentSocketName(1)="palaceguard_arm_RIGHT"
	AttachmentSpawnClasses(1)=none // class'Item_PalaceGuardAttachment_ArmRight'


	Begin Object Class=StaticMeshComponent Name=AttachmentSM2
	    CollideActors=FALSE
		BlockRigidBody=FALSE
		HiddenGame=FALSE
		AlwaysLoadOnClient=TRUE
		StaticMesh=StaticMesh'Locust_Theron_Guard.palaceguard_shoulder_LEFT'
		bCastDynamicShadow=FALSE
		LightEnvironment=MyLightEnvironment
	End Object
	AttachmentSM(2)=AttachmentSM2
	AttachmentSocketName(2)="palaceguard_shoulder_LEFT"
	AttachmentSpawnClasses(2)=class'Item_PalaceGuardAttachment_ShoulderLeft'


	Begin Object Class=StaticMeshComponent Name=AttachmentSM3
	    CollideActors=FALSE
		BlockRigidBody=FALSE
		HiddenGame=FALSE
		AlwaysLoadOnClient=TRUE
		StaticMesh=StaticMesh'Locust_Theron_Guard.palaceguard_shoulder_RIGHT'
		bCastDynamicShadow=FALSE
		LightEnvironment=MyLightEnvironment
	End Object
	AttachmentSM(3)=AttachmentSM3
	AttachmentSocketName(3)="palaceguard_shoulder_RIGHT"
	AttachmentSpawnClasses(3)=class'Item_PalaceGuardAttachment_ShoulderRight'

	NoticedGUDSEvent=GUDEvent_NoticedPalaceGuard
	NoticedGUDSPriority=100

	SpeechPitchMultiplier=0.67f
}