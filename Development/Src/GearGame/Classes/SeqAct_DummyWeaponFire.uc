/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_DummyWeaponFire extends SeqAct_Latent
	native(Sequence);

cpptext
{
protected:
	void UpdateObjectList(FDummyFireObjectListParams& ListParams, TArray<UObject**> Objects, FLOAT DeltaTime);
	AActor* GetCurrentActorFromObjectList(FDummyFireObjectListParams const& ListParams, TArray<UObject**> Objects) const;

public:
	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT DeltaTime);
	virtual void DeActivated();

	/** called when the level that contains this sequence object is being removed/unloaded */
	virtual void CleanUp();
};

/** Number of shots to fire in sequence */
var() int				ShotsToFire;

/** TRUE to keep shooting forever (ignore ShotsToFire) */
var() bool				bShootUntilStopped;

/** Shooting inaccuracy, in degrees */
var() float				InaccuracyDegrees;

/** Which type of weapon to shoot */
var() class<GearWeapon>	WeaponClass;

/** Which firing mode to fire the weapon. */
var() byte				FiringMode;

/** true if action was stopped via the Stop input */
var protected bool		bStopped;

/** true if action finished naturally */
var protected bool		bFinished;

/** internal, the temporary weapon we spawn to do the effects */
var	GearWeapon			SpawnedWeapon;

/** internal, time in seconds until next shot fires */
var private float		RemainingFireTime;

/** internal, number of shots that have been fired */
var private int			ShotsFired;

/** if TRUE, do not show weapon muzzle flash */
var() bool				bSuppressMuzzleFlash;

/** if TRUE, do not show weapon tracers if appropriate */
var() bool				bSuppressTracers;

/** if TRUE, do not show weapon impact effects */
var() bool				bSuppressImpactFX;

/** if TRUE, do not play any weapon-fire-related audio (incl impacts) */
var() bool				bSuppressAudio;

/** if TRUE, does not do damage */
var() bool				bSuppressDamage;

/** True if weapon is trying to fire, false if not (e.g. in a delay) */
var() bool				bFiring;

/** If set, will try and find a socket with this name on the Origin Actor and fire from there. */
var() name				OriginSocketName;

/** our replication handler */
var DummyWeaponFireActor ReplicatedActor;

/** Enumerate the supported objet list cycle methods */
enum DummyFireObjectCyclingMethod
{
	/** move through targets array in order */
	DFOCM_Sequential,

	/** choose targets randomly.  can result in a target chosen multiple times in a row */
	DFOCM_Random,

	// support random without repeats?
};


/** Convenient wrapper for parameters used to define behavior when dealing with multiple objects */
struct native DummyFireObjectListParams
{
	/** Range, in seconds, for how long to fire at each target. */
	var() const vector2d						SecondsPerObject;

	/**
	 * Range, in seconds, for how long to wait between stopping using
	 * one object and starting to use another.  Object is considered NULL during this period.
	 */
	var() const vector2d						ObjectChangeDelay;

	/** Target cycling method, affects how targets are chosen */
	var() const DummyFireObjectCyclingMethod	CyclingMethod;

	/** Internal.  Seconds until next object change. */
	var float							TimeUntilObjectChange;

	/** Internal.  True if a delay is active.  Current object is considered to be NULL. */
	var bool							bDelay;

	/** Internal.  Seconds until delay is finished and we move onto the next object. */
	var float							DelayTimeRemaining;

	/** Internal.  Index of current object that we're using. */
	var int								CurrentObjIdx;

	structdefaultproperties
	{
		SecondsPerObject=(X=1.f,Y=2.f)
		ObjectChangeDelay=(X=0.5f,Y=1.5f)
		CyclingMethod=DFOCM_Sequential
	}
};


/** Parameters defining multiple-target behavior.  No effect if only 1 target actor is specified. */
var() protected DummyFireObjectListParams MultipleTargetParams;

/** Parameters defining multiple-source behavior.  No effect if only 1 origin actor is specified.  */
var() protected DummyFireObjectListParams MultipleOriginParams;




/** Spawns a temporary weapon to do the firing. */
native final function SpawnDummyWeapon(Actor OriginActor, Actor TargetActor);

/** Aligns the dummy weapon such that it's muzzle point is on AlignTo, and is pointing at AimAt. */
native final function AlignWeaponMuzzleToActor(Actor AlignTo, Actor AimAt);

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Dummy Weapon Fire"
	ObjCategory="Gear"
	bCallHandler=FALSE

	ShotsToFire=1

	InputLinks(0)=(LinkDesc="Start Firing")
	InputLInks(1)=(LinkDesc="Stop Firing")

	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Finished")
	OutputLinks(2)=(LinkDesc="Stopped")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Origin",PropertyName=Origin,MaxVars=255)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Target,MaxVars=255)
}
