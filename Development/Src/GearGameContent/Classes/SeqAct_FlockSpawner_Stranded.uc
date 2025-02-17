/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqAct_FlockSpawner_Stranded extends SeqAct_FlockSpawner;

defaultproperties
{
	ObjName="Flock Spawner - Stranded"

	FlockMeshes(0)=SkeletalMesh'Locust_Grunt.Locust_Grunt_Crowdv1'
	FlockAnimTree=AnimTree'Locust_Crowd.CrowdTest_AnimTree'

	WalkAnimName="Panic_Walk"
	RunAnimName="Panic_Run"

	FlockAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel')
	FlockAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Grenade')
	FlockAnimSets.Add(AnimSet'Locust_Crowd.PanicCrowdAnims')

	ActionAnimNames.Add("AR_Injured_HeadCover")
	//ActionAnimNames.Add("AR_Idle_Headset")
	ActionAnimNames.Add("AR_Cough")
	ActionAnimNames.Add("Panic_Idle")

	TargetActionAnimNames.Add("AR_Idle_Ready_Aim")
	TargetActionAnimNames.Add("GR_Throw")
	TargetActionAnimNames.Add("AR_Idle_DownSights_Aim")
}