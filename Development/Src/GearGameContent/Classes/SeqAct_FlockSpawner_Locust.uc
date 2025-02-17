/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqAct_FlockSpawner_Locust extends SeqAct_FlockSpawner;

defaultproperties
{
	ObjName="Flock Spawner - Locust"

	FlockMeshes(0)=SkeletalMesh'Locust_Grunt.Locust_Grunt_Crowdv1'
	FlockAnimTree=AnimTree'Locust_Crowd.CrowdTest_AnimTree'

	WalkAnimName="AR_Walk_Fwd"
	RunAnimName="AR_Run_Fwd"

	FlockAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel')
	FlockAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Grenade')

	ActionAnimNames.Add("AR_Injured_HeadCover")
	ActionAnimNames.Add("AR_Idle_Headset")
	ActionAnimNames.Add("AR_Cough")
	ActionAnimNames.Add("ar_injurede_idle")

	TargetActionAnimNames.Add("AR_Idle_Ready_Aim")
	TargetActionAnimNames.Add("GR_Throw")
	TargetActionAnimNames.Add("AR_Idle_DownSights_Aim")

	DeathAnimNames.Add("ar_death_standing_a")
	DeathAnimNames.Add("ar_death_standing_b")
	DeathAnimNames.Add("ar_death_standing_c")
	DeathAnimNames.Add("ar_death_standing_d")
	DeathAnimNames.Add("AR_Death_Running_A")
	DeathAnimNames.Add("AR_Death_Running_C")
	DeathAnimNames.Add("AR_Death_Running_E")

	ExplosiveDeathEffect=ParticleSystem'Effects_Gameplay.Blood.P_Blood_Crowd_Gib_Medium'
	ExplosiveDeathEffectNonExtremeContent=ParticleSystem'Effects_Gameplay.Blood.P_Blood_Crowd_Gib_Medium_NoGore'
	ExplosiveDeathEffectScale=2.0

	bSpawnAtEdge=TRUE
}