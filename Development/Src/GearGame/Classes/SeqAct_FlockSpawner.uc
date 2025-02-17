/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class SeqAct_FlockSpawner extends SeqAct_CrowdSpawner
	native(Sequence);

/** internal, the temporary weapon we spawn to do the effects */
var		GearWeapon			SpawnedWeapon;
/** Which type of weapon to shoot */
var()	class<GearWeapon>	WeaponClass;
/** Error when firing - in degrees */
var()	float				AimError;
/** Offset to fire weapon from. */
var()	vector				FireOffset;

var()	DecalData			OverlapDeathDecalInfo;

var()	SoundCue			OverlapDeathSound;

var()	ParticleSystem		OverlapDeathEffect;

var()	float				OverlapDeathRadius;

var()	bool				bOverlapDeathPlayAnim;

var()	bool				bCountForSeriouslyAchievement;

cpptext
{
	virtual void CleanUp();

	void SpawnDummyWeapon();

	/** Use the SpawnedWeapon (will create if missing) to dummy fire at target. */
	void TriggerDummyFire(const FVector& SourceLocation, const FRotator& SourceRotation, const FVector& TargetLocation);
};

defaultproperties
{
	ObjName="Gears Flock Spawner"
	ObjCategory="Crowd"

	AgentClass=class'GearGame.FlockTestLocust'

	FireOffset=(X=30.0,Z=70.0)

	ActionBlendTime=0.3
	CollisionBoxScaling=(X=0.3,Y=0.3,Z=0.5)
	RotateToTargetSpeed=0.05

	MinVelDamping=0.005
	MaxVelDamping=0.007

	OverlapDeathRadius=45.0

	bCountForSeriouslyAchievement=TRUE
}