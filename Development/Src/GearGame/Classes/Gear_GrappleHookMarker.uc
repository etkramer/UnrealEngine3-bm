
/**
 * Gear_GrappleHookMarker
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Gear_GrappleHookMarker extends Actor
	placeable
	native;

/** This is the root motion offset used to reach the marker */
var const Vector	MarkerOffset;

defaultproperties
{
	MarkerOffset=(X=-33.947,Y=0,Z=-487.047)

	// Show a good ref animation, so it can be positioned properly
	Begin Object Class=AnimNodeSequence Name=GrapplingLocustAnimNode
	    AnimSeqName="GP_Climb"
		CurrentTime=3.5
	End Object

	Begin Object Class=SkeletalMeshComponent Name=GrapplingLocustMesh
		SkeletalMesh=SkeletalMesh'Locust_Grunt.Mesh.Locust_Grunt_CamSkel'
		AnimSets(0)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Grapple'
		Animations=GrapplingLocustAnimNode
		bUpdateSkelWhenNotRendered=FALSE
		HiddenGame=TRUE	// @fixme
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
		HiddenEditor=FALSE
		Translation=(X=-33.947,Y=0,Z=-487.047)
	End Object
	Components.Add(GrapplingLocustMesh)

	// Show a good ref animation, so it can be positioned properly
	Begin Object Class=AnimNodeSequence Name=GrapplingHookAnimNode
	    AnimSeqName="GP_Climb"
		CurrentTime=3.5
	End Object

	//@todo move to GearGameContent
	Begin Object Class=SkeletalMeshComponent Name=GrappleHookMesh
		SkeletalMesh=SkeletalMesh'Locust_GrappleHook.Mesh.Locust_GrappleHook'
		AnimSets(0)=AnimSet'Locust_GrappleHook.Anims.Locust_GrappleHook_Animset'
		Animations=GrapplingHookAnimNode
		bUpdateSkelWhenNotRendered=FALSE
		HiddenGame=TRUE	// @fixme
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
		HiddenEditor=FALSE
		Translation=(X=-33.947,Y=0,Z=-487.047)
	End Object
	Components.Add(GrappleHookMesh)

	bHidden=TRUE	//@fixme
	bHardAttach=TRUE
	bNoDelete=true
}
