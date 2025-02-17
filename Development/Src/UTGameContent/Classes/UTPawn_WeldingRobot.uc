/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTPawn_WeldingRobot extends UTPawn;


event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	// hack here so we don't have to modify Pawn's TakeDamage this late in the dev cycle.
	if( !AffectedByHitEffects() )
	{
		Momentum = vect(0,0,0);
	}

	Super.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
}


DefaultProperties
{
    ControllerClass=class'UTPawn_WeldingRobotController'
	
	SoundGroupClass=class'UTPawnSoundGroup_Liandri'

	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh=SkeletalMesh'CH_Corrupt_Cine.Mesh.SK_CH_WeldingRobot'
		PhysicsAsset=PhysicsAsset'CH_AnimCorrupt.Mesh.SK_CH_Corrupt_Male_Physics'
		AnimSets[0]=AnimSet'CH_AnimHuman.Anims.K_AnimHuman_BaseMale'
		AnimSets[1]=AnimSet'CH_Corrupt_Cine.Mesh.K_CH_WeldingRobot'
		Translation=(X=0.0,Y=0.0,Z=-50.0)
		Scale3D=(X=1.0,Y=1.0,Z=1.0)
	End Object
}

// this is the anim he has
//Robot_WeldingIdle

