//=============================================================================
// Pawn, the base class of all actors that can be controlled by players or AI.
//
// Pawns are the physical representations of players and creatures in a level.
// Pawns have a mesh, collision, and physics.  Pawns can take damage, make sounds,
// and hold weapons and other inventory.  In short, they are responsible for all
// physical interaction between the player or AI and the world.
//
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class Pawn extends Actor
	abstract
	native
	placeable
	config(Game)
	nativereplication;

var const float			MaxStepHeight,
						MaxJumpHeight;
var const float			WalkableFloorZ;		/** minimum z value for floor normal (if less, not a walkable floor for this pawn) */

//-----------------------------------------------------------------------------
// Pawn variables.

var repnotify Controller Controller;

/** Chained pawn list */
var const Pawn NextPawn;

// cache net relevancy test
var float				NetRelevancyTime;
var playerController	LastRealViewer;
var actor				LastViewer;

// Physics related flags.
var bool		bUpAndOut;			// used by swimming
var bool		bIsWalking;			// currently walking (can't jump, affects animations)

// Crouching
var				bool	bWantsToCrouch;		// if true crouched (physics will automatically reduce collision height to CrouchHeight)
var		const	bool	bIsCrouched;		// set by physics to specify that pawn is currently crouched
var		const	bool	bTryToUncrouch;		// when auto-crouch during movement, continually try to uncrouch
var()			bool	bCanCrouch;			// if true, this pawn is capable of crouching
var		const	float	UncrouchTime;		// when auto-crouch during movement, continually try to uncrouch once this decrements to zero
var				float	CrouchHeight;		// CollisionHeight when crouching
var				float	CrouchRadius;		// CollisionRadius when crouching
var		const	int		FullHeight;			// cached for pathfinding

var bool		bCrawler;			// crawling - pitch and roll based on surface pawn is on

/** Used by movement natives to slow pawn as it reaches its destination to prevent overshooting */
var const bool	bReducedSpeed;

var bool		bJumpCapable;
var	bool		bCanJump;			// movement capabilities - used by AI
var	bool		bCanWalk;
var	bool		bCanSwim;
var	bool		bCanFly;
var	bool		bCanClimbLadders;
var	bool		bCanStrafe;
var	bool		bAvoidLedges;		// don't get too close to ledges
var	bool		bStopAtLedges;		// if bAvoidLedges and bStopAtLedges, Pawn doesn't try to walk along the edge at all
var const bool	bSimulateGravity;	// simulate gravity for this pawn on network clients when predicting position (true if pawn is walking or falling)
var	bool		bIgnoreForces;		// if true, not affected by external forces
var	bool		bCanWalkOffLedges;	// Can still fall off ledges, even when walking (for Player Controlled pawns)
var bool		bCanBeBaseForPawns;	// all your 'base', are belong to us
var const bool	bSimGravityDisabled;	// used on network clients
var bool		bDirectHitWall;		// always call pawn hitwall directly (no controller notifyhitwall)
var const bool	bPushesRigidBodies;	// Will do a check to find nearby PHYS_RigidBody actors and will give them a 'soft' push.
var	bool		bForceFloorCheck;	// force the pawn in PHYS_Walking to do a check for a valid floor even if he hasn't moved.	Cleared after next floor check.
var bool		bForceKeepAnchor;	// Force ValidAnchor function to accept any non-NULL anchor as valid (used to override when we want to set anchor for path finding)

//@fixme - remove these post-ship, as they aren't general enough to warrant being placed here
var config bool bCanMantle;			// can this pawn mantle over cover
var config bool bCanClimbUp;		// can this pawn climb up cover wall
var		   bool bCanClimbCeilings;	// can this pawn climb ceiling nodes
var config bool bCanSwatTurn;		// can this pawn swat turn between cover
var config bool bCanLeap;			// can this pawn use LeapReachSpec
var config bool	bCanCoverSlip;		// can this pawn coverslip

/** if set, display "MAP HAS PATHING ERRORS" and message in the log when a Pawn fails a full path search */
var globalconfig bool bDisplayPathErrors;

// AI related flags
var		bool	bIsFemale;
var		bool	bCanPickupInventory;	// if true, will pickup inventory when touching pickup actors
var		bool	bAmbientCreature;		// AIs will ignore me
var(AI) bool	bLOSHearing;			// can hear sounds from line-of-sight sources (which are close enough to hear)
										// bLOSHearing=true is like UT/Unreal hearing
var(AI) bool	bMuffledHearing;		// can hear sounds through walls (but muffled - sound distance increased to double plus 4x the distance through walls
var(AI) bool	bDontPossess;			// if true, Pawn won't be possessed at game start
var		bool	bAutoFire;				// used for third person weapon anims/effects
var		bool	bRollToDesired;			// Update roll when turning to desired rotation (normally false)
var		bool	bStationary;			// pawn can't move

var		bool	bCachedRelevant;		// network relevancy caching flag
var		bool	bSpecialHUD;
var		bool	bNoWeaponFiring;
var		bool	bCanUse;				// can this pawn use things?
var		bool	bModifyReachSpecCost;	// pawn should call virtual function to modify reach spec costs
var		bool	bModifyNavPointDest;	// pawn should call virtual function to modify destination location when moving to nav point
/** set if Pawn counts as a vehicle for pathfinding checks (so don't use bBlockedForVehicles nodes, etc) */
var bool bPathfindsAsVehicle;
/** Pawn multiplies cost of NavigationPoints that don't have bPreferredVehiclePath set by this number */
var float NonPreferredVehiclePathMultiplier;

// AI basics.
enum EPathSearchType
{
	PST_Default,
	PST_Breadth,
	PST_NewBestPathTo,
	PST_Constraint,
};
var EPathSearchType	PathSearchType;

/** List of search constraints for pathing */
var PathConstraint		PathConstraintList;
var PathGoalEvaluator	PathGoalList;

var		float	DesiredSpeed;
var		float	MaxDesiredSpeed;
var(AI) float	HearingThreshold;	// max distance at which a makenoise(1.0) loudness sound can be heard
var(AI)	float	Alertness;			// -1 to 1 ->Used within specific states for varying reaction to stimuli
var(AI)	float	SightRadius;		// Maximum seeing distance.
var(AI)	float	PeripheralVision;	// Cosine of limits of peripheral vision.
var const float	AvgPhysicsTime;		// Physics updating time monitoring (for AI monitoring reaching destinations)
var			  float		  Mass;				// Mass of this pawn.
var			  float		  Buoyancy;			// Water buoyancy. A ratio (1.0 = neutral buoyancy, 0.0 = no buoyancy)
var		float	MeleeRange;			// Max range for melee attack (not including collision radii)
var const NavigationPoint Anchor;			// current nearest path;
var const NavigationPoint LastAnchor;		// recent nearest path
var		float	FindAnchorFailedTime;	// last time a FindPath() attempt failed to find an anchor.
var		float	LastValidAnchorTime;	// last time a valid anchor was found
var		float	DestinationOffset;	// used to vary destination over NavigationPoints
var		float	NextPathRadius;		// radius of next path in route
var		vector	SerpentineDir;		// serpentine direction
var		float	SerpentineDist;
var		float	SerpentineTime;		// how long to stay straight before strafing again
var		float	SpawnTime;
var		int		MaxPitchLimit;		// limit on view pitching

// Movement.
var	bool	bRunPhysicsWithNoController;	// When there is no Controller, Walking Physics abort and force a velocity and acceleration of 0. Set this to TRUE to override.
var bool	bForceMaxAccel;	// ignores Acceleration component, and forces max AccelRate to drive Pawn at full velocity.
var float	GroundSpeed;	// The maximum ground speed.
var float	WaterSpeed;		// The maximum swimming speed.
var float	AirSpeed;		// The maximum flying speed.
var float	LadderSpeed;	// Ladder climbing speed
var float	AccelRate;		// max acceleration rate
var float	JumpZ;			// vertical acceleration w/ jump
var float	OutofWaterZ;	/** z velocity applied when pawn tries to get out of water */
var float	MaxOutOfWaterStepHeight;	/** Maximum step height for getting out of water */
var float	AirControl;		// amount of AirControl available to the pawn
var float	WalkingPct;		// pct. of running speed that walking speed is
var float	CrouchedPct;	// pct. of running speed that crouched walking speed is
var float	MaxFallSpeed;	// max speed pawn can land without taking damage
/** AI will take paths that require a landing velocity less than (MaxFallSpeed * AIMaxFallSpeedFactor) */
var float AIMaxFallSpeedFactor;

var(Camera) float	BaseEyeHeight;	// Base eye height above collision center.
var(Camera) float		EyeHeight;		// Current eye height, adjusted for bobbing and stairs.
var	vector			Floor;			// Normal of floor pawn is standing on (only used by PHYS_Spider and PHYS_Walking)
var float			SplashTime;		// time of last splash
var float			OldZ;			// Old Z Location - used for eyeheight smoothing
var transient PhysicsVolume HeadVolume;		// physics volume of head
var() int Health;		/** amount of health this Pawn has */
var() int HealthMax;		/** normal maximum health of Pawn - defaults to default.Health unless explicitly set otherwise */
var	float			BreathTime;		// used for getting BreathTimer() messages (for no air, etc.)
var float			UnderWaterTime; // how much time pawn can go without air (in seconds)
var	float			LastPainTime;	// last time pawn played a takehit animation (updated in PlayHit())

/** RootMotion derived velocity calculated by APawn::CalcVelocity() (used when replaying client moves in net games (since can't rely on animation when replaying moves)) */
var vector RMVelocity;

/** this flag forces APawn::CalcVelocity() to just use RMVelocity directly */
var bool bForceRMVelocity;

/** this flag forces APawn::CalcVelocity() to never use root motion derived velocity */
var bool bForceRegularVelocity;

// Sound and noise management
// remember location and position of last noises propagated
var const	vector		noise1spot;
var const	float		noise1time;
var const	pawn		noise1other;
var const	float		noise1loudness;
var const	vector		noise2spot;
var const	float		noise2time;
var const	pawn		noise2other;
var const	float		noise2loudness;

var float SoundDampening;
var float DamageScaling;

var localized  string MenuName; // Name used for this pawn type in menus (e.g. player selection)

var class<AIController> ControllerClass;	// default class to use when pawn is controlled by AI

var RepNotify PlayerReplicationInfo PlayerReplicationInfo;

var LadderVolume OnLadder;		// ladder currently being climbed

var name LandMovementState;		// PlayerControllerState to use when moving on land or air
var name WaterMovementState;	// PlayerControllerState to use when moving in water

var PlayerStart LastStartSpot;	// used to avoid spawn camping
var float LastStartTime;

var vector				TakeHitLocation;		// location of last hit (for playing hit/death anims)
var class<DamageType>	HitDamageType;			// damage type of last hit (for playing hit/death anims)
var vector				TearOffMomentum;		// momentum to apply when torn off (bTearOff == true)
var bool				bPlayedDeath;			// set when death animation has been played (used in network games)

var() SkeletalMeshComponent	Mesh;

var	CylinderComponent		CylinderComponent;

var()	float				RBPushRadius; // Unreal units
var()	float				RBPushStrength;

var	repnotify	Vehicle DrivenVehicle;

var float AlwaysRelevantDistanceSquared;	// always relevant to other clients if closer than this distance to viewer, and have controller

/** replicated to we can see where remote clients are looking */
var		const	byte	RemoteViewPitch;

/** Radius that is checked for nearby vehicles when pressing use */
var() float	VehicleCheckRadius;

var Controller LastHitBy; //give kill credit to this guy if hit momentum causes pawn to fall to his death

var()	float	ViewPitchMin;
var()	float	ViewPitchMax;

/** Max difference between pawn's Rotation.Yaw and DesiredRotation.Yaw for pawn to be considered as having reached its desired rotation */
var		int		AllowedYawError;

/** Inventory Manager */
var class<InventoryManager>		InventoryManagerClass;
var repnotify InventoryManager			InvManager;

/** Weapon currently held by Pawn */
var()	Weapon					Weapon;

/**
 * This next group of replicated properties are used to cause 3rd person effects on
 * remote clients.	FlashLocation and FlashCount are altered by the weapon to denote that
 * a shot has occured and FiringMode is used to determine what type of shot.
 */

/** Hit Location of instant hit weapons. vect(0,0,0) = not firing. */
var repnotify	vector	FlashLocation;
/** last FlashLocation that was an actual shot, i.e. not counting clears to (0,0,0)
 * this is used to make sure we set unique values to FlashLocation for consecutive shots even when there was a clear in between,
 * so that if a client missed the clear due to low net update rate, it still gets the new firing location
 */
var vector LastFiringFlashLocation;
/** increased when weapon fires. 0 = not firing. 1 - 255 = firing */
var repnotify	byte	FlashCount;
/** firing mode used when firing */
var	repnotify	byte	FiringMode;
/** tracks the number of consecutive shots. Note that this is not replicated, so it's not correct on remote clients. It's only updated when the pawn is relevant. */
var				int		ShotCount;

/** set in InitRagdoll() to old CollisionComponent (since it must be Mesh for ragdolls) so that TermRagdoll() can restore it */
var PrimitiveComponent PreRagdollCollisionComponent;

/** Physics object created to create contacts with physics objects, used to push them around. */
var	native const pointer	PhysicsPushBody;

/** @HACK: count of times processLanded() was called but it failed without changing physics for some reason
 * so we can detect and avoid a rare case where Pawns get stuck in that state
 */
var int FailedLandingCount;

cpptext
{
	// declare type for node evaluation functions
	typedef FLOAT ( *NodeEvaluator ) (ANavigationPoint*, APawn*, FLOAT);

	virtual void PostBeginPlay();
	virtual void PostScriptDestroyed();

	// AActor interface.
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);

	APawn* GetPlayerPawn() const;
	virtual FLOAT GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, APlayerController* Viewer, UActorChannel* InChannel, FLOAT Time, UBOOL bLowBandwidth);
	virtual UBOOL DelayScriptReplication(FLOAT LastFullUpdateTime);
	virtual INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	virtual void NotifyBump(AActor *Other, UPrimitiveComponent* OtherComp, const FVector &HitNormal);
	virtual void TickSimulated( FLOAT DeltaSeconds );
	virtual void TickSpecial( FLOAT DeltaSeconds );
	UBOOL PlayerControlled();
	void SetBase(AActor *NewBase, FVector NewFloor = FVector(0,0,1), int bNotifyActor=1, USkeletalMeshComponent* SkelComp=NULL, FName BoneName=NAME_None );
	virtual void CheckForErrors();
	virtual UBOOL IsNetRelevantFor(APlayerController* RealViewer, AActor* Viewer, const FVector& SrcLocation);
	UBOOL CacheNetRelevancy(UBOOL bIsRelevant, APlayerController* RealViewer, AActor* Viewer);
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
	virtual void PreNetReceive();
	virtual void PostNetReceiveLocation();
	virtual APawn* GetAPawn() { return this; }
	virtual const APawn* GetAPawn() const { return this; }

	/**
	 * Sets the hard attach flag by first handling the case of already being
	 * based upon another actor
	 *
	 * @param bNewHardAttach the new hard attach setting
	 */
	virtual void SetHardAttach(UBOOL bNewHardAttach);

	// Level functions
	void SetZone( UBOOL bTest, UBOOL bForceRefresh );

	// AI sensing
	virtual void CheckNoiseHearing(AActor* NoiseMaker, FLOAT Loudness, FName NoiseType=NAME_None );
	virtual FLOAT DampenNoise(AActor* NoiseMaker, FLOAT Loudness, FName NoiseType=NAME_None );


	// Latent movement
	virtual void setMoveTimer(FVector MoveDir);
	FLOAT GetMaxSpeed();
	virtual UBOOL moveToward(const FVector &Dest, AActor *GoalActor);
	virtual UBOOL IsGlider();
	virtual void rotateToward(FVector FocalPoint);
	UBOOL PickWallAdjust(FVector WallHitNormal, AActor* HitActor);
	void StartNewSerpentine(const FVector& Dir, const FVector& Start);
	void ClearSerpentine();
	virtual UBOOL SharingVehicleWith(APawn *P);
	void InitSerpentine();
	virtual void HandleSerpentineMovement(FVector& out_Direction, FLOAT Distance, const FVector& Dest);

	// reach tests
	virtual UBOOL ReachedDestination(const FVector &Start, const FVector &Dest, AActor* GoalActor);
	virtual int pointReachable(FVector aPoint, int bKnowVisible=0);
	virtual int actorReachable(AActor *Other, UBOOL bKnowVisible=0, UBOOL bNoAnchorCheck=0);
	virtual int Reachable(FVector aPoint, AActor* GoalActor);
	int walkReachable(const FVector &Dest, const FVector &Start, int reachFlags, AActor* GoalActor);
	int flyReachable(const FVector &Dest, const FVector &Start, int reachFlags, AActor* GoalActor);
	int swimReachable(const FVector &Dest, const FVector &Start, int reachFlags, AActor* GoalActor);
	int ladderReachable(const FVector &Dest, const FVector &Start, int reachFlags, AActor* GoalActor);
	INT spiderReachable( const FVector &Dest, const FVector &Start, INT reachFlags, AActor* GoalActor );
	FVector GetGravityDirection();
	virtual UBOOL TryJumpUp(FVector Dir, FVector Destination, DWORD TraceFlags, UBOOL bNoVisibility);
	virtual UBOOL ReachedBy(APawn* P, const FVector& TestPosition, const FVector& Dest);
	virtual UBOOL ReachThresholdTest(const FVector &TestPosition, const FVector &Dest, AActor* GoalActor, FLOAT UpThresholdAdjust, FLOAT DownThresholdAdjust, FLOAT ThresholdAdjust);
	virtual UBOOL SetHighJumpFlag() { return false; }

	// movement component tests (used by reach tests)
	void TestMove(const FVector &Delta, FVector &CurrentPosition, FCheckResult& Hit, const FVector &CollisionExtent);
	FVector GetDefaultCollisionSize();
	FVector GetCrouchSize();
	ETestMoveResult walkMove(FVector Delta, FVector &CurrentPosition, const FVector &CollisionExtent, FCheckResult& Hit, AActor* GoalActor, FLOAT threshold);
	ETestMoveResult flyMove(FVector Delta, FVector &CurrentPosition, AActor* GoalActor, FLOAT threshold);
	ETestMoveResult swimMove(FVector Delta, FVector &CurrentPosition, AActor* GoalActor, FLOAT threshold);
	virtual ETestMoveResult FindBestJump(FVector Dest, FVector &CurrentPosition);
	virtual ETestMoveResult FindJumpUp(FVector Direction, FVector &CurrentPosition);
	ETestMoveResult HitGoal(AActor *GoalActor);
	virtual UBOOL HurtByDamageType(class UClass* DamageType);
	UBOOL CanCrouchWalk( const FVector& StartLocation, const FVector& EndLocation, AActor* HitActor );
	/** updates the highest landing Z axis velocity encountered during a reach test */
	virtual void SetMaxLandingVelocity(FLOAT NewLandingVelocity) {}

	// Path finding
	UBOOL GeneratePath();
	FLOAT findPathToward(AActor *goal, FVector GoalLocation, NodeEvaluator NodeEval, FLOAT BestWeight, UBOOL bWeightDetours, INT MaxPathLength = 0, UBOOL bReturnPartial = FALSE, INT SoftMaxNodes = 200);
	ANavigationPoint* BestPathTo(NodeEvaluator NodeEval, ANavigationPoint *start, FLOAT *Weight, UBOOL bWeightDetours, INT MaxPathLength = 0, INT SoftMaxNodes = 200);
	virtual ANavigationPoint* CheckDetour(ANavigationPoint* BestDest, ANavigationPoint* Start, UBOOL bWeightDetours);
	virtual INT calcMoveFlags();
	/** returns the maximum falling speed an AI will accept along a path */
	FORCEINLINE FLOAT GetAIMaxFallSpeed() { return MaxFallSpeed * AIMaxFallSpeedFactor; }
	virtual void MarkEndPoints(ANavigationPoint* EndAnchor, AActor* Goal, const FVector& GoalLocation);
	virtual FLOAT SecondRouteAttempt(ANavigationPoint* Anchor, ANavigationPoint* EndAnchor, NodeEvaluator NodeEval, FLOAT BestWeight, AActor *goal, const FVector& GoalLocation, FLOAT StartDist, FLOAT EndDist, INT MaxPathLength, INT SoftMaxNodes);
	/** finds the closest NavigationPoint within MAXPATHDIST that is usable by this pawn and directly reachable to/from TestLocation
	 * @param TestActor the Actor to find an anchor for
	 * @param TestLocation the location to find an anchor for
	 * @param bStartPoint true if we're finding the start point for a path search, false if we're finding the end point
	 * @param bOnlyCheckVisible if true, only check visibility - skip reachability test
	 * @param Dist (out) if an anchor is found, set to the distance TestLocation is from it. Set to 0.f if the anchor overlaps TestLocation
	 * @return a suitable anchor on the navigation network for reaching TestLocation, or NULL if no such point exists
	 */
	ANavigationPoint* FindAnchor(AActor* TestActor, const FVector& TestLocation, UBOOL bStartPoint, UBOOL bOnlyCheckVisible, FLOAT& Dist);
	virtual INT		ModifyCostForReachSpec( UReachSpec* Spec, INT Cost ) { return 0; }
	virtual void	InitForPathfinding( AActor* Goal, ANavigationPoint* EndAnchor ) {}
	// allows pawn subclasses to veto anchor validity
	virtual UBOOL	IsValidAnchor( ANavigationPoint* AnchorCandidate ){ return TRUE; }

	/*
	 * Route finding notifications (sent to target)
	 */
	virtual ANavigationPoint* SpecifyEndAnchor(APawn* RouteFinder);
	virtual UBOOL AnchorNeedNotBeReachable();
	virtual void NotifyAnchorFindingResult(ANavigationPoint* EndAnchor, APawn* RouteFinder);

	// Pawn physics modes
	virtual void performPhysics(FLOAT DeltaSeconds);
	/** Called in PerformPhysics(), after StartNewPhysics() is done moving the Actor, and before the PendingTouch() event is dispatched. */
	virtual void PostProcessPhysics( FLOAT DeltaSeconds, const FVector& OldVelocity );
	virtual FVector CheckForLedges(FVector AccelDir, FVector Delta, FVector GravDir, int &bCheckedFall, int &bMustJump );
	virtual void physWalking(FLOAT deltaTime, INT Iterations);
	virtual void physFlying(FLOAT deltaTime, INT Iterations);
	void physSwimming(FLOAT deltaTime, INT Iterations);
	void physFalling(FLOAT deltaTime, INT Iterations);
	void physSpider(FLOAT deltaTime, INT Iterations);
	void physLadder(FLOAT deltaTime, INT Iterations);
	virtual void startNewPhysics(FLOAT deltaTime, INT Iterations);
	virtual void GetNetBuoyancy(FLOAT &NetBuoyancy, FLOAT &NetFluidFriction);
	void startSwimming(FVector OldLocation, FVector OldVelocity, FLOAT timeTick, FLOAT remainingTime, INT Iterations);
	virtual void physicsRotation(FLOAT deltaTime, FVector OldVelocity);
	void processLanded(FVector const& HitNormal, AActor *HitActor, FLOAT remainingTime, INT Iterations);
	virtual void SetPostLandedPhysics(AActor *HitActor, FVector HitNormal);
	virtual void processHitWall(FCheckResult const& Hit, FLOAT TimeSlice=0.f);
	virtual void Crouch(INT bClientSimulation=0);
	virtual void UnCrouch(INT bClientSimulation=0);
	FRotator FindSlopeRotation(FVector FloorNormal, FRotator NewRotation);
	void SmoothHitWall(FVector const& HitNormal, AActor *HitActor);
	FVector NewFallVelocity(FVector OldVelocity, FVector OldAcceleration, FLOAT timeTick);
	void stepUp(const FVector& GravDir, const FVector& DesiredDir, const FVector& Delta, FCheckResult &Hit);
	virtual FLOAT MaxSpeedModifier();
	virtual FVector CalculateSlopeSlide(const FVector& Adjusted, const FCheckResult& Hit);
	virtual UBOOL IgnoreBlockingBy(const AActor* Other) const;
	virtual void PushedBy(AActor* Other);
	virtual void UpdateBasedRotation(FRotator &FinalRotation, const FRotator& ReducedRotation);
	virtual void ReverseBasedRotation();

	virtual void InitRBPhys();
	virtual void TermRBPhys(FRBPhysScene* Scene);

	/** Update information used to detect overlaps between this actor and physics objects, used for 'pushing' things */
	virtual void UpdatePushBody();

	/** Called when the push body 'sensor' overlaps a physics body. Allows you to add a force to that body to move it. */
	virtual void ProcessPushNotify(const FRigidBodyCollisionInfo& PushedInfo, const TArray<FRigidBodyContactInfo>& ContactInfos);

	virtual UBOOL HasAudibleAmbientSound(const FVector& SrcLocation) { return false; }

	//superville: Chance for pawn to say he has reached a location w/o touching it (ie cover slot)
	virtual UBOOL HasReached( ANavigationPoint *Nav, UBOOL& bFinalDecision ) { return FALSE; }

	virtual FVector GetIdealCameraOrigin()
	{
		return FVector(Location.X,Location.Y,Location.Z + BaseEyeHeight);
	}

protected:
	virtual void CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant);

private:
	UBOOL Pick3DWallAdjust(FVector WallHitNormal, AActor* HitActor);
	FLOAT Swim(FVector Delta, FCheckResult &Hit);
	FVector findWaterLine(FVector Start, FVector End);
	void SpiderstepUp(const FVector& DesiredDir, const FVector& Delta, FCheckResult &Hit);
	int findNewFloor(FVector OldLocation, FLOAT deltaTime, FLOAT remainingTime, INT Iterations);
	int checkFloor(FVector Dir, FCheckResult &Hit);
}

replication
{
	// Variables the server should send ALL clients.
	if( bNetDirty && (Role==ROLE_Authority) )
	FlashLocation, bSimulateGravity, bIsWalking, PlayerReplicationInfo, HitDamageType,
		TakeHitLocation, DrivenVehicle, Health;

	// variables sent to owning client
	if ( bNetDirty && bNetOwner && Role==ROLE_Authority )
		InvManager, Controller, GroundSpeed, WaterSpeed, AirSpeed, AccelRate, JumpZ, AirControl;

	// sent to non owning clients
	if ( bNetDirty && (!bNetOwner || bDemoRecording) && Role==Role_Authority )
		bIsCrouched, FlashCount, FiringMode;

	// variable sent to all clients when Pawn has been torn off. (bTearOff)
	if( bTearOff && bNetDirty && (Role==ROLE_Authority) )
		TearOffMomentum;

	// variables sent to all but the owning client
	if ( (!bNetOwner || bDemoRecording) && Role==ROLE_Authority )
		RemoteViewPitch;
}

/** Generic Set/Get for BasedPosition */
native static final function SetBasedPosition( out BasedPosition BP, Vector Pos, optional Actor ForcedBase );
native static final function Vector GetBasedPosition( BasedPosition BP );

/** Check on various replicated data and act accordingly. */
simulated event ReplicatedEvent( name VarName )
{
	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ "VarName:" @ VarName );

	super.ReplicatedEvent( VarName );

	if( VarName == 'FlashCount' )	// FlashCount and FlashLocation are changed when a weapon is fired.
	{
		FlashCountUpdated( true );
	}
	else if( VarName == 'FlashLocation' ) // FlashCount and FlashLocation are changed when a weapon is fired.
	{
		FlashLocationUpdated( true );
	}
	else if( VarName == 'FiringMode' )
	{
		FiringModeUpdated( true );
	}
	else if ( VarName == 'DrivenVehicle' )
	{
		if ( DrivenVehicle != None )
		{
			// since pawn doesn't have a PRI while driving, and may become initially relevant while driving,
			// we may only be able to ascertain the pawn's team (for team coloring, etc.) through its drivenvehicle
			NotifyTeamChanged();
		}
	}
	else if ( VarName == 'PlayerReplicationInfo' )
	{
		NotifyTeamChanged();
	}
	else if ( VarName == 'Controller' )
	{
		if ( (Controller != None) && (Controller.Pawn == None) )
		{
			Controller.Pawn = self;
			if ( (PlayerController(Controller) != None)
				&& (PlayerController(Controller).ViewTarget == Controller) )
				PlayerController(Controller).SetViewTarget(self);
		}
	}
}


// =============================================================

/** Returns TRUE if Pawn is alive and doing well */
final virtual simulated native function bool IsAliveAndWell() const;

final native virtual function Vector AdjustDestination( Actor GoalActor, optional Vector Dest );

/** Is the current anchor valid? */
final native function bool ValidAnchor();

/**
SuggestJumpVelocity()
returns true if succesful jump from start to destination is possible
returns a suggested initial falling velocity in JumpVelocity
Uses GroundSpeed and JumpZ as limits
*/
native function bool SuggestJumpVelocity(out vector JumpVelocity, vector Destination, vector Start);

/** returns if we are a valid enemy for C
 * checks things like whether we're alive, teammates, etc
 * server only; always returns false on clients
 * obsolete - use IsValidEnemyTargetFor() instead!
 */
native function bool IsValidTargetFor( const Controller C) const;

/** returns if we are a valid enemy for PRI
 * checks things like whether we're alive, teammates, etc
 * works on clients and servers
 */
native function bool IsValidEnemyTargetFor(const PlayerReplicationInfo PRI, bool bNoPRIisEnemy) const;

/**
@RETURN true if pawn is invisible to AI
*/
native function bool IsInvisible();

/**
 * Set Pawn ViewPitch, so we can see where remote clients are looking.
 *
 * @param	NewRemoteViewPitch	Pitch component to replicate to remote (non owned) clients.
 */
native final function SetRemoteViewPitch( int NewRemoteViewPitch );

native function SetAnchor( NavigationPoint NewAnchor );
native function NavigationPoint GetBestAnchor( Actor TestActor, Vector TestLocation, bool bStartPoint, bool bOnlyCheckVisible, out float out_Dist );
native function bool ReachedDestination(Actor Goal);
native function bool ReachedPoint( Vector Point, Actor NewAnchor );
native function ForceCrouch();
native function SetPushesRigidBodies( bool NewPush );
native final virtual function bool ReachedDesiredRotation();

native function GetBoundingCylinder(out float CollisionRadius, out float CollisionHeight) const;

/** Allow pawn to add special cost to path */
function int SpecialCostForPath( ReachSpec Path ) { return NavigationPoint(Path.End.Actor).Cost; }
/** Pawn can be considered a valid enemy */
simulated function bool IsValidEnemy() { return true; }

/**
 * Does the following:
 *	- Assign the SkeletalMeshComponent 'Mesh' to the CollisionComponent
 *	- Call InitArticulated on the SkeletalMeshComponent.
 *	- Change the physics mode to PHYS_RigidBody
 */
native function bool InitRagdoll();
/** the opposite of InitRagdoll(); resets CollisionComponent to the default,
 * sets physics to PHYS_Falling, and calls TermArticulated() on the SkeletalMeshComponent
 * @return true on success, false if there is no Mesh, the Mesh is not in ragdoll, or we're otherwise not able to terminate the physics
 */
native function bool TermRagdoll();

/** Give pawn the chance to do something special moving between points */
function bool SpecialMoveTo( NavigationPoint Start, NavigationPoint End, Actor Next );

simulated function SetBaseEyeheight()
{
	if ( !bIsCrouched )
		BaseEyeheight = Default.BaseEyeheight;
	else
		BaseEyeheight = FMin(0.8 * CrouchHeight, CrouchHeight - 10);
}

function PlayerChangedTeam()
{
	Died( None, class'DamageType', Location );
}

/* Reset()
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	if ( (Controller == None) || Controller.bIsPlayer )
	{
		DetachFromController();
		Destroy();
	}
	else
		super.Reset();
}

function bool StopFiring()
{
	if( Weapon != None )
	{
		 Weapon.StopFire(Weapon.CurrentFireMode);
	}
	return TRUE;
}


/**
 * Pawn starts firing!
 * Called from PlayerController::StartFiring
 * Network: Local Player
 *
 * @param	FireModeNum		fire mode number
 */

simulated function StartFire(byte FireModeNum)
{
	if( bNoWeaponFIring )
	{
		return;
	}

	if( InvManager != None )
	{
		InvManager.StartFire(FireModeNum);
	}
}


/**
 * Pawn stops firing!
 * i.e. player releases fire button, this may not stop weapon firing right away. (for example press button once for a burst fire)
 * Network: Local Player
 *
 * @param	FireModeNum		fire mode number
 */

simulated function StopFire(byte FireModeNum)
{
	if( InvManager != None )
	{
		InvManager.StopFire(FireModeNum);
	}
}


/*********************************************************************************************
 * Remote Client Firing Magic...
 * @See Weapon::IncrementFlashCount()
 ********************************************************************************************/


/**
 * Set firing mode replication for remote clients trigger update notification.
 * Network: LocalPlayer and Server
 */
simulated function SetFiringMode(byte FiringModeNum)
{
	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ "old:" @ FiringMode @  "new:" @ FiringModeNum );

	if( FiringModeNum != FiringMode )
	{
		FiringMode		= FiringModeNum;
		bForceNetUpdate = TRUE;

		// call updated event locally
		FiringModeUpdated(FALSE);
	}
}


/**
 * Called when FiringMode has been updated.
 *
 * Network: ALL
 */
simulated function FiringModeUpdated(bool bViaReplication)
{
	if( Weapon != None )
	{
		Weapon.FireModeUpdated(FiringMode, bViaReplication);
	}
}


/**
 * This function's responsibility is to signal clients that non-instant hit shot
 * has been fired. Call this on the server and local player.
 *
 * Network: Server and Local Player
 */
simulated function IncrementFlashCount( Weapon Who, byte FireModeNum )
{
	bForceNetUpdate = TRUE;	// Force replication
	FlashCount++;
	// Make sure it's not 0, because it means the weapon stopped firing!
	if( FlashCount == 0 )
	{
		FlashCount += 2;
	}

	// Make sure firing mode is updated
	SetFiringMode(FireModeNum);

	// This weapon has fired.
	FlashCountUpdated(FALSE);
}


/**
 * Clear flashCount variable. and call WeaponStoppedFiring event.
 * Call this on the server and local player.
 *
 * Network: Server or Local Player
 */
simulated function ClearFlashCount(Weapon Who)
{
	if( FlashCount != 0 )
	{
		bForceNetUpdate = TRUE;	// Force replication
		FlashCount		= 0;

		// This weapon stopped firing
		FlashCountUpdated(FALSE);
	}
}


/**
 * This function sets up the Location of a hit to be replicated to all remote clients.
 * It is also responsible for fudging a shot at (0,0,0).
 *
 * Network: Server only
 */
function SetFlashLocation(Weapon Who, byte FireModeNum, vector NewLoc)
{
	// Make sure 2 consecutive flash locations are different, for replication
	if( NewLoc == LastFiringFlashLocation )
	{
		NewLoc += vect(0,0,1);
	}

	// If we are aiming at the origin, aim slightly up since we use 0,0,0 to denote
	// not firing.
	if( NewLoc == vect(0,0,0) )
	{
		NewLoc = vect(0,0,1);
	}

	bForceNetUpdate = TRUE; // Force replication
	FlashLocation	= NewLoc;
	LastFiringFlashLocation = NewLoc;

	// Make sure firing mode is updated
	SetFiringMode(FireModeNum);

	// This weapon has fired.
	FlashLocationUpdated(FALSE);
}


/**
 * Reset flash location variable. and call stop firing.
 * Network: Server only
 */
function ClearFlashLocation( Weapon Who )
{
	if( !IsZero( FlashLocation ) )
	{
		bForceNetUpdate = TRUE;	// Force replication
		FlashLocation = vect(0,0,0);

		FlashLocationUpdated(FALSE);
	}
}


/**
 * Called when FlashCount has been updated.
 * Trigger appropritate events based on FlashCount's value.
 * = 0 means Weapon Stopped firing
 * > 0 means Weapon just fired
 *
 * Network: ALL
 */
simulated function FlashCountUpdated( bool bViaReplication )
{
	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ "FlashCount:" @ FlashCount @ "bViaReplication:" @ bViaReplication );

	if( FlashCount > 0 )
	{
		WeaponFired( bViaReplication );
	}
	else
	{
		WeaponStoppedFiring( bViaReplication );
	}
}


/**
 * Called when FlashLocation has been updated.
 * Trigger appropritate events based on FlashLocation's value.
 * == (0,0,0) means Weapon Stopped firing
 * != (0,0,0) means Weapon just fired
 *
 * Network: ALL
 */
simulated function FlashLocationUpdated( bool bViaReplication )
{
	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ "FlashLocation:" @ FlashLocation @ "bViaReplication:" @ bViaReplication );

	if( !IsZero(FlashLocation) )
	{
		WeaponFired( bViaReplication, FlashLocation );
	}
	else
	{
		WeaponStoppedFiring( bViaReplication );
	}
}


/**
 * Called when a pawn's weapon has fired and is responsibile for
 * delegating the creation of all of the different effects.
 *
 * bViaReplication denotes if this call in as the result of the
 * flashcount/flashlocation being replicated. It's used filter out
 * when to make the effects.
 *
 * Network: ALL
 */
simulated function WeaponFired( bool bViaReplication, optional vector HitLocation )
{
	// increment number of consecutive shots.
	ShotCount++;

	// By default we just call PlayFireEffects on the weapon.
	if( Weapon != None )
	{
		Weapon.PlayFireEffects( FiringMode, HitLocation );
	}
}


/**
 * Called when a pawn's weapon has stopped firing and is responsibile for
 * delegating the destruction of all of the different effects.
 *
 * bViaReplication denotes if this call in as the result of the
 * flashcount/flashlocation being replicated.
 *
 * Network: ALL
 */
simulated function WeaponStoppedFiring( bool bViaReplication )
{
	// reset number of consecutive shots fired.
	ShotCount = 0;

	if (Weapon != None)
	{
		Weapon.StopFireEffects( FiringMode );
	}
}


/**
AI Interface for combat
**/

function bool BotFire(bool bFinished)
{
	StartFire(ChooseFireMode());
	return true;
}

function byte ChooseFireMode()
{
	return 0;
}

function bool CanAttack(Actor Other)
{
	if ( Weapon == None )
		return false;
	return Weapon.CanAttack(Other);
}

function bool TooCloseToAttack(Actor Other)
{
	return false;
}

function bool FireOnRelease()
{
	if (Weapon != None)
		return Weapon.FireOnRelease();

	return false;
}

function bool HasRangedAttack()
{
	return ( Weapon != None );
}

function bool IsFiring()
{
	if (Weapon != None)
		return Weapon.IsFiring();

	return false;
}

/** returns whether we need to turn to fire at the specified location */
function bool NeedToTurn(vector targ)
{
	local vector LookDir, AimDir;

	LookDir = Vector(Rotation);
	LookDir.Z = 0;
	LookDir = Normal(LookDir);
	AimDir = targ - Location;
	AimDir.Z = 0;
	AimDir = Normal(AimDir);

	return ((LookDir Dot AimDir) < 0.93);
}

simulated function String GetHumanReadableName()
{
	if ( PlayerReplicationInfo != None )
		return PlayerReplicationInfo.PlayerName;
	return MenuName;
}

function PlayTeleportEffect(bool bOut, bool bSound)
{
	MakeNoise(1.0);
}

/** NotifyTeamChanged()
Called when PlayerReplicationInfo is replicated to this pawn, or PlayerReplicationInfo team property changes.

Network:  client
*/
simulated function NotifyTeamChanged();

/* PossessedBy()
 Pawn is possessed by Controller
*/
function PossessedBy(Controller C, bool bVehicleTransition)
{
	Controller			= C;
	bForceNetUpdate = TRUE;

	if ( C.PlayerReplicationInfo != None )
	{
		PlayerReplicationInfo = C.PlayerReplicationInfo;
	}
	UpdateControllerOnPossess(bVehicleTransition);

	SetOwner(Controller);	// for network replication
	Eyeheight = BaseEyeHeight;

	if ( C.IsA('PlayerController') )
	{
		if ( WorldInfo.NetMode != NM_Standalone )
		{
			RemoteRole = ROLE_AutonomousProxy;
		}

		// inform client of current weapon
		if( Weapon != None )
		{
			Weapon.ClientWeaponSet(FALSE);
		}
	}
	else
	{
		RemoteRole = Default.RemoteRole;

		if (Weapon != None)
		{
			Weapon.AIController = AIController(C);
		}
	}
}

/* UpdateControllerOnPossess()
update controller - normally, just change its rotation to match pawn rotation
*/
function UpdateControllerOnPossess(bool bVehicleTransition)
{
	// don't set pawn rotation on possess if was driving vehicle, so face
	// same direction when get out as when driving
	if ( !bVehicleTransition )
	{
		Controller.SetRotation(Rotation);
	}
}

function UnPossessed()
{
	bForceNetUpdate = TRUE;

	PlayerReplicationInfo = None;
	SetOwner(None);
	Controller = None;
}

/**
 * returns default camera mode when viewing this pawn.
 * Mainly called when controller possesses this pawn.
 *
 * @param	PlayerController requesting the default camera view
 * @return	default camera view player should use when controlling this pawn.
 */
simulated function name GetDefaultCameraMode( PlayerController RequestedBy )
{
	if ( RequestedBy != None && RequestedBy.PlayerCamera != None && RequestedBy.PlayerCamera.CameraStyle == 'Fixed' )
		return 'Fixed';

	return 'FirstPerson';
}

function DropToGround()
{
	bCollideWorld = True;
	if ( Health > 0 )
	{
		SetCollision(true,true);
		SetPhysics(PHYS_Falling);
		if ( IsHumanControlled() )
			Controller.GotoState(LandMovementState);
	}
}

function bool CanGrabLadder()
{
	return ( bCanClimbLadders
			&& (Controller != None)
			&& (Physics != PHYS_Ladder)
			&& ((Physics != Phys_Falling) || (abs(Velocity.Z) <= JumpZ)) );
}

function bool RecommendLongRangedAttack()
{
	return false;
}

function float RangedAttackTime()
{
	return 0;
}

/**
 * Called every frame from PlayerInput or PlayerController::MoveAutonomous()
 * Sets bIsWalking flag, which defines if the Pawn is walking or not (affects velocity)
 *
 * @param	bNewIsWalking, new walking state.
 */
event SetWalking( bool bNewIsWalking )
{
	if ( bNewIsWalking != bIsWalking )
	{
		bIsWalking = bNewIsWalking;
	}
}

simulated function bool CanSplash()
{
	if ( (WorldInfo.TimeSeconds - SplashTime > 0.15)
		&& ((Physics == PHYS_Falling) || (Physics == PHYS_Flying))
		&& (Abs(Velocity.Z) > 100) )
	{
		SplashTime = WorldInfo.TimeSeconds;
		return true;
	}
	return false;
}

function EndClimbLadder(LadderVolume OldLadder)
{
	if ( Controller != None )
		Controller.EndClimbLadder();
	if ( Physics == PHYS_Ladder )
		SetPhysics(PHYS_Falling);
}

function ClimbLadder(LadderVolume L)
{
	OnLadder = L;
	SetRotation(OnLadder.WallDir);
	SetPhysics(PHYS_Ladder);
	if ( IsHumanControlled() )
		Controller.GotoState('PlayerClimbing');
}

/**
 * list important Pawn variables on canvas.	 HUD will call DisplayDebug() on the current ViewTarget when
 * the ShowDebug exec is used
 *
 * @param	HUD		- HUD with canvas to draw on
 * @input	out_YL		- Height of the current font
 * @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.
 */
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local string	T;
	local Canvas	Canvas;
	local AnimTree	AnimTreeRootNode;
	local int		i;

	Canvas = HUD.Canvas;

	if ( PlayerReplicationInfo == None )
	{
		Canvas.DrawText("NO PLAYERREPLICATIONINFO", false);
		out_YPos += out_YL;
		Canvas.SetPos(4,out_YPos);
	}
	else
	{
		PlayerReplicationInfo.DisplayDebug(HUD,out_YL,out_YPos);
	}

	super.DisplayDebug(HUD, out_YL, out_YPos);

	Canvas.SetDrawColor(255,255,255);

	Canvas.DrawText("Health "$Health);
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	if (HUD.ShouldDisplayDebug('AI'))
	{
		Canvas.DrawText("Anchor "$Anchor$" Serpentine Dist "$SerpentineDist$" Time "$SerpentineTime);
		out_YPos += out_YL;
		Canvas.SetPos(4,out_YPos);
	}

	if (HUD.ShouldDisplayDebug('physics'))
	{
		T = "Floor "$Floor$" DesiredSpeed "$DesiredSpeed$" Crouched "$bIsCrouched;
		if ( (OnLadder != None) || (Physics == PHYS_Ladder) )
			T=T$" on ladder "$OnLadder;
		Canvas.DrawText(T);
		out_YPos += out_YL;
		Canvas.SetPos(4,out_YPos);

		T = "Collision Component:" @ CollisionComponent;
		Canvas.DrawText(T);
		out_YPos += out_YL;
		Canvas.SetPos(4,out_YPos);

		T = "bForceMaxAccel:" @ bForceMaxAccel;
		Canvas.DrawText(T);
		out_YPos += out_YL;
		Canvas.SetPos(4,out_YPos);

		if( Mesh != None  )
		{
			T = "RootMotionMode:" @ Mesh.RootMotionMode @ "RootMotionVelocity:" @ Mesh.RootMotionVelocity;
			Canvas.DrawText(T);
			out_YPos += out_YL;
			Canvas.SetPos(4,out_YPos);
		}
	}

	if (HUD.ShouldDisplayDebug('camera'))
	{
		Canvas.DrawText("EyeHeight "$Eyeheight$" BaseEyeHeight "$BaseEyeHeight);
		out_YPos += out_YL;
		Canvas.SetPos(4,out_YPos);
	}

	// Controller
	if ( Controller == None )
	{
		Canvas.SetDrawColor(255,0,0);
		Canvas.DrawText("NO CONTROLLER");
		out_YPos += out_YL;
		Canvas.SetPos(4,out_YPos);
		HUD.PlayerOwner.DisplayDebug(HUD, out_YL, out_YPos);
	}
	else
	{
		Controller.DisplayDebug(HUD, out_YL, out_YPos);
	}

	// Weapon
	if (HUD.ShouldDisplayDebug('weapon'))
	{
		if ( Weapon == None )
		{
			Canvas.SetDrawColor(0,255,0);
			Canvas.DrawText("NO WEAPON");
			out_YPos += out_YL;
			Canvas.SetPos(4, out_YPos);
		}
		else
			Weapon.DisplayDebug(HUD, out_YL, out_YPos);
	}

	if( HUD.ShouldDisplayDebug('animation') )
	{
		if( Mesh != None && Mesh.Animations != None )
		{
			AnimTreeRootNode = AnimTree(Mesh.Animations);
			if( AnimTreeRootNode != None )
			{
				Canvas.DrawText("AnimGroups count:" @ AnimTreeRootNode.AnimGroups.Length);
				out_YPos += out_YL;
				Canvas.SetPos(4,out_YPos);

				for(i=0; i<AnimTreeRootNode.AnimGroups.Length; i++)
				{
					Canvas.DrawText(" GroupName:" @ AnimTreeRootNode.AnimGroups[i].GroupName @ "NodeCount:" @ AnimTreeRootNode.AnimGroups[i].SeqNodes.Length @ "RateScale:" @ AnimTreeRootNode.AnimGroups[i].RateScale);
					out_YPos += out_YL;
					Canvas.SetPos(4,out_YPos);
				}
			}
		}
	}
}

//***************************************
// Interface to Pawn's Controller

/** IsHumanControlled()
return true if controlled by a real live human on the local machine.
On client, only local player's pawn returns true
*/
simulated final native function bool IsHumanControlled();

/** IsLocallyControlled()
return true if controlled by local (not network) player */
simulated final native function bool IsLocallyControlled();

/** IsPlayerPawn()
return true if controlled by a Player (AI or human) on local machine (any controller on server, localclient's pawn on client)
*/
simulated native function bool IsPlayerPawn() const;

// return true if was controlled by a Player (AI or human)
simulated function bool WasPlayerPawn()
{
	return false;
}

// return true if viewing this pawn in first person pov. useful for determining what and where to spawn effects
simulated function bool IsFirstPerson()
{
	local PlayerController PC;

	PC = PlayerController(Controller);
	return ( PC!=None && PC.UsingFirstPersonCamera() );
}

/**
 * Called from PlayerController UpdateRotation() -> ProcessViewRotation() to (pre)process player ViewRotation
 * adds delta rot (player input), applies any limits and post-processing
 * returns the final ViewRotation set on PlayerController
 *
 * @param	DeltaTime, time since last frame
 * @param	ViewRotation, actual PlayerController view rotation
 * @input	out_DeltaRot, delta rotation to be applied on ViewRotation. Represents player's input.
 * @return	processed ViewRotation to be set on PlayerController.
 */
simulated function ProcessViewRotation( float DeltaTime, out rotator out_ViewRotation, out Rotator out_DeltaRot )
{
	// Add Delta Rotation
	out_ViewRotation	+= out_DeltaRot;
	out_DeltaRot		 = rot(0,0,0);

	// Limit Player View Pitch
	if ( PlayerController(Controller) != None )
	{
		out_ViewRotation = PlayerController(Controller).LimitViewRotation( out_ViewRotation, ViewPitchMin, ViewPitchMax );
	}
}

/**
 * returns the point of view of the actor.
 * note that this doesn't mean the camera, but the 'eyes' of the actor.
 * For example, for a Pawn, this would define the eye height location,
 * and view rotation (which is different from the pawn rotation which has a zeroed pitch component).
 * A camera first person view will typically use this view point. Most traces (weapon, AI) will be done from this view point.
 *
 * @output	out_Location, location of view point
 * @output	out_Rotation, view rotation of actor.
 */
simulated event GetActorEyesViewPoint( out vector out_Location, out Rotator out_Rotation )
{
	out_Location = GetPawnViewLocation();
	out_Rotation = GetViewRotation();
}

/** @return the rotation the Pawn is looking
 */
simulated native event rotator GetViewRotation();

/**
 * returns the Eye location of the Pawn.
 *
 * @return	Pawn's eye location
 */
simulated native event vector GetPawnViewLocation();

/**
 * Return world location to start a weapon fire trace from.
 *
 * @return	World location where to start weapon fire traces from
 */
simulated event Vector GetWeaponStartTraceLocation(optional Weapon CurrentWeapon)
{
	local vector	POVLoc;
	local rotator	POVRot;

	// If we have a controller, by default we start tracing from the player's 'eyes' location
	// that is by default Controller.Location for AI, and camera (crosshair) location for human players.
	if ( Controller != None )
	{
		Controller.GetPlayerViewPoint( POVLoc, POVRot );
		return POVLoc;
	}

	// If we have no controller, we simply traces from pawn eyes location
	return GetPawnViewLocation();
}


/**
 * returns base Aim Rotation without any adjustment (no aim error, no autolock, no adhesion.. just clean initial aim rotation!)
 *
 * @return	base Aim rotation.
 */
simulated singular event Rotator GetBaseAimRotation()
{
	local vector	POVLoc;
	local rotator	POVRot;

	// If we have a controller, by default we aim at the player's 'eyes' direction
	// that is by default Controller.Rotation for AI, and camera (crosshair) rotation for human players.
	if( Controller != None && !InFreeCam() )
	{
		Controller.GetPlayerViewPoint(POVLoc, POVRot);
		return POVRot;
	}

	// If we have no controller, we simply use our rotation
	POVRot = Rotation;

	// If our Pitch is 0, then use RemoveViewPitch
	if( POVRot.Pitch == 0 )
	{
		POVRot.Pitch = RemoteViewPitch << 8;
	}

	return POVRot;
}

/** return true if player is viewing this Pawn in FreeCam */
simulated event bool InFreeCam()
{
	local PlayerController	PC;

	PC = PlayerController(Controller);
	return (PC != None && PC.PlayerCamera != None && PC.PlayerCamera.CameraStyle == 'FreeCam');
}

/**
 * Adjusts weapon aiming direction.
 * Gives Pawn a chance to modify its aiming. For example aim error, auto aiming, adhesion, AI help...
 * Requested by weapon prior to firing.
 *
 * @param	W, weapon about to fire
 * @param	StartFireLoc, world location of weapon fire start trace, or projectile spawn loc.
 * @param	BaseAimRot, original aiming rotation without any modifications.
 */
simulated function Rotator GetAdjustedAimFor( Weapon W, vector StartFireLoc )
{
	// If controller doesn't exist or we're a client, get the where the Pawn is aiming at
	if ( Controller == None || Role < Role_Authority )
	{
		return GetBaseAimRotation();
	}

	// otherwise, give a chance to controller to adjust this Aim Rotation
	return Controller.GetAdjustedAimFor( W, StartFireLoc );
}

simulated function SetViewRotation(rotator NewRotation )
{
	if (Controller != None)
	{
		Controller.SetRotation(NewRotation);
	}
	else
	{
		SetRotation(NewRotation);
	}
}

/**
 * PawnCalcCamera is obsolete, replaced by Actor.CalcCamera()
*  rename implementations of PawnCalcCamera to CalcCamera
* @FIXME - remove
 */
simulated function bool PawnCalcCamera( float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV )
{
	return CalcCamera(fDeltaTime, out_CamLoc, out_CamRot, out_FOV);
}

function bool InGodMode()
{
	return ( (Controller != None) && Controller.bGodMode );
}

simulated function bool AffectedByHitEffects()
{
	return (Controller == None || Controller.bAffectedByHitEffects);
}

function bool NearMoveTarget()
{
	if ( (Controller == None) || (Controller.MoveTarget == None) )
		return false;

	return ReachedDestination(Controller.MoveTarget);
}

function Actor GetMoveTarget()
{
	if ( Controller == None )
		return None;

	return Controller.MoveTarget;
}

function SetMoveTarget(Actor NewTarget )
{
	if ( Controller != None )
		Controller.MoveTarget = NewTarget;
}

function bool LineOfSightTo(actor Other)
{
	return ( (Controller != None) && Controller.LineOfSightTo(Other) );
}

/* return a value (typically 0 to 1) adjusting pawn's perceived strength if under some special influence (like berserk)
*/
function float AdjustedStrength()
{
	return 0;
}

function HandlePickup(Inventory Inv)
{
	MakeNoise(0.2);
	if ( Controller != None )
		Controller.HandlePickup(Inv);
}

function ReceiveLocalizedMessage( class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject )
{
	if ( PlayerController(Controller) != None )
		PlayerController(Controller).ReceiveLocalizedMessage( Message, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject );
}

event ClientMessage( coerce string S, optional Name Type )
{
	if ( PlayerController(Controller) != None )
		PlayerController(Controller).ClientMessage( S, Type );
}

//***************************************

function FinishedInterpolation()
{
	DropToGround();
}

function JumpOutOfWater(vector jumpDir)
{
	Falling();
	Velocity = jumpDir * WaterSpeed;
	Acceleration = jumpDir * AccelRate;
	velocity.Z = OutofWaterZ; //set here so physics uses this for remainder of tick
	bUpAndOut = true;
}

/*
Modify velocity called by physics before applying new velocity
for this tick.

Velocity,Acceleration, etc. have been updated by the physics, but location hasn't
*/
simulated event ModifyVelocity(float DeltaTime, vector OldVelocity);

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
	if ( Role == ROLE_Authority )
	{
		Health = -1;
		Died( None, dmgType, Location );
		if ( dmgType == None )
		{
			SetPhysics(PHYS_None);
			SetHidden(True);
			LifeSpan = FMin(LifeSpan, 1.0);
		}
	}
}

simulated singular event OutsideWorldBounds()
{
	// AI pawns on the server just destroy
	if (Role == ROLE_Authority && PlayerController(Controller) == None)
	{
		Destroy();
	}
	else
	{
		// simply destroying the Pawn could cause synchronization issues with the client controlling it
		// so kill it, disable it, and wait a while to give it time to replicate before destroying it
		if (Role == ROLE_Authority)
		{
			KilledBy(self);
		}
		SetPhysics(PHYS_None);
		SetHidden(True);
		LifeSpan = FMin(LifeSpan, 1.0);
    }
}

/**
 * Makes sure a Pawn is not crouching, telling it to stand if necessary.
 */
simulated function UnCrouch()
{
	if( bIsCrouched || bWantsToCrouch )
	{
		ShouldCrouch( false );
	}
}

/**
 * Controller is requesting that pawn crouches.
 * This is not guaranteed as it depends if crouching collision cylinder can fit when Pawn is located.
 *
 * @param	bCrouch		true if Pawn should crouch.
 */
function ShouldCrouch( bool bCrouch )
{
	bWantsToCrouch = bCrouch;
}

/**
 * Event called from native code when Pawn stops crouching.
 * Called on non owned Pawns through bIsCrouched replication.
 * Network: ALL
 *
 * @param	HeightAdjust	height difference in unreal units between default collision height, and actual crouched cylinder height.
 */
simulated event EndCrouch( float HeightAdjust )
{
	EyeHeight -= HeightAdjust;
	OldZ += HeightAdjust;
	SetBaseEyeHeight();
}

/**
 * Event called from native code when Pawn starts crouching.
 * Called on non owned Pawns through bIsCrouched replication.
 * Network: ALL
 *
 * @param	HeightAdjust	height difference in unreal units between default collision height, and actual crouched cylinder height.
 */
simulated event StartCrouch( float HeightAdjust )
{
	EyeHeight += HeightAdjust;
	OldZ -= HeightAdjust;
	SetBaseEyeHeight();
}

function RestartPlayer();
function AddVelocity( vector NewVelocity, vector HitLocation, class<DamageType> damageType, optional TraceHitInfo HitInfo )
{
	if ( bIgnoreForces || (NewVelocity == vect(0,0,0)) )
		return;
	if ( (Physics == PHYS_Walking)
		|| (((Physics == PHYS_Ladder) || (Physics == PHYS_Spider)) && (NewVelocity.Z > Default.JumpZ)) )
		SetPhysics(PHYS_Falling);
	if ( (Velocity.Z > Default.JumpZ) && (NewVelocity.Z > 0) )
		NewVelocity.Z *= 0.5;
	Velocity += NewVelocity;
}

function KilledBy( pawn EventInstigator )
{
	local Controller Killer;

	Health = 0;
	if ( EventInstigator != None )
	{
		Killer = EventInstigator.Controller;
		LastHitBy = None;
	}
	Died( Killer, class'DmgType_Suicided', Location );
}

function TakeFallingDamage()
{
	local float EffectiveSpeed;

	if (Velocity.Z < -0.5 * MaxFallSpeed)
	{
		if ( Role == ROLE_Authority )
		{
			MakeNoise(1.0);
			if (Velocity.Z < -1 * MaxFallSpeed)
			{
				EffectiveSpeed = Velocity.Z;
				if (TouchingWaterVolume())
				{
					EffectiveSpeed += 100;
				}
				if (EffectiveSpeed < -1 * MaxFallSpeed)
				{
					TakeDamage(-100 * (EffectiveSpeed + MaxFallSpeed)/MaxFallSpeed, None, Location, vect(0,0,0), class'DmgType_Fell');
				}
				}
		}
	}
	else if (Velocity.Z < -1.4 * JumpZ)
		MakeNoise(0.5);
	else if ( Velocity.Z < -0.8 * JumpZ )
		MakeNoise(0.2);
}

function Restart();

simulated function ClientReStart()
{
	ZeroMovementVariables();
	SetBaseEyeHeight();
}

function ClientSetLocation( vector NewLocation, rotator NewRotation )
{
	if ( Controller != None )
		Controller.ClientSetLocation(NewLocation, NewRotation);
}

function ClientSetRotation( rotator NewRotation )
{
	if ( Controller != None )
		Controller.ClientSetRotation(NewRotation);
}

simulated function FaceRotation(rotator NewRotation, float DeltaTime)
{
	// Do not update Pawn's rotation depending on controller's ViewRotation if in FreeCam.
	if (!InFreeCam())
	{
		if ( Physics == PHYS_Ladder )
		{
			NewRotation = OnLadder.Walldir;
		}
		else if ( (Physics == PHYS_Walking) || (Physics == PHYS_Falling) )
		{
			NewRotation.Pitch = 0;
		}

		SetRotation(NewRotation);
	}
}

//==============
// Encroachment
event bool EncroachingOn( actor Other )
{
	if ( Other.bWorldGeometry || Other.bBlocksTeleport )
		return true;

	if ( ((Controller == None) || !Controller.bIsPlayer) && (Pawn(Other) != None) )
		return true;

	return false;
}

event EncroachedBy( actor Other )
{
	// Allow encroachment by Vehicles so they can push the pawn out of the way
	if ( Pawn(Other) != None && Vehicle(Other) == None )
		gibbedBy(Other);
}

function gibbedBy(actor Other)
{
	if ( Role < ROLE_Authority )
		return;
	if ( Pawn(Other) != None )
		Died(Pawn(Other).Controller, class'DmgType_Telefragged', Location);
	else
		Died(None, class'DmgType_Telefragged', Location);
}

//Base change - if new base is pawn or decoration, damage based on relative mass and old velocity
// Also, non-players will jump off pawns immediately
function JumpOffPawn()
{
	Velocity += (100 + CylinderComponent.CollisionRadius) * VRand();
	if ( VSize2D(Velocity) > FMax(500.0, GroundSpeed) )
	{
		Velocity = FMax(500.0, GroundSpeed) * Normal(Velocity);
	}
	Velocity.Z = 200 + CylinderComponent.CollisionHeight;
	SetPhysics(PHYS_Falling);
}

/** Called when pawn cylinder embedded in another pawn.  (Collision bug that needs to be fixed).
*/
event StuckOnPawn(Pawn OtherPawn);

/**
  * Event called after actor's base changes.
*/
singular event BaseChange()
{
	local DynamicSMActor Dyn;

	// Pawns can only set base to non-pawns, or pawns which specifically allow it.
	// Otherwise we do some damage and jump off.
	if (Pawn(Base) != None && (DrivenVehicle == None || !DrivenVehicle.IsBasedOn(Base)))
	{
		if( !Pawn(Base).CanBeBaseForPawn(Self) )
		{
			Pawn(Base).CrushedBy(self);
			JumpOffPawn();
		}
	}

	// If it's a KActor, see if we can stand on it.
	Dyn = DynamicSMActor(Base);
	if( Dyn != None && !Dyn.CanBasePawn(self) )

	{
		JumpOffPawn();
	}
}


/**
 * Are we allowing this Pawn to be based on us?
 */
simulated function bool CanBeBaseForPawn(Pawn APawn)
{
	return bCanBeBaseForPawns;
}

/** CrushedBy()
Called for pawns that have bCanBeBaseForPawns=false when another pawn becomes based on them
*/
function CrushedBy(Pawn OtherPawn)
{
	TakeDamage( (1-OtherPawn.Velocity.Z/400)* OtherPawn.Mass/Mass, OtherPawn.Controller,Location, vect(0,0,0) , class'DmgType_Crushed');
}

//=============================================================================

/**
 * Call this function to detach safely pawn from its controller
 *
 * @param	bDestroyController	if true, then destroy controller. (only AI Controllers, not players)
 */
function DetachFromController( optional bool bDestroyController )
{
	local Controller OldController;

	// if we have a controller, notify it we're getting destroyed
	// be careful with bTearOff, we're authority on client! Make sure our controller and pawn match up.
	if ( Controller != None && Controller.Pawn == Self )
	{
		OldController = Controller;
		Controller.PawnDied( Self );
		if ( Controller != None )
		{
			Controller.UnPossess();
		}

		if ( bDestroyController && OldController != None && !OldController.bDeleteMe && !OldController.bIsPlayer )
		{
			OldController.Destroy();
		}
		Controller = None;
	}
}

simulated event Destroyed()
{
	DetachFromController();

	if ( InvManager != None )
		InvManager.Destroy();

	if ( WorldInfo.NetMode == NM_Client )
		return;

	// Clear anchor to avoid checkpoint crash
	SetAnchor( None );

	Weapon = None;

	//debug
	ClearPathStep();

	super.Destroyed();
}

//=============================================================================
//
// Called immediately before gameplay begins.
//
simulated event PreBeginPlay()
{
	// important that this comes before Super so mutators can modify it
	if (HealthMax == 0)
	{
		HealthMax = default.Health;
	}

	Super.PreBeginPlay();

	Instigator = self;
	DesiredRotation = Rotation;
	EyeHeight = BaseEyeHeight;
}

event PostBeginPlay()
{
	super.PostBeginPlay();

	SplashTime = 0;
	SpawnTime = WorldInfo.TimeSeconds;
	EyeHeight	= BaseEyeHeight;

	// automatically add controller to pawns which were placed in level
	// NOTE: pawns spawned during gameplay are not automatically possessed by a controller
	if ( WorldInfo.bStartup && (Health > 0) && !bDontPossess )
	{
		SpawnDefaultController();
	}

	// Spawn Inventory Container
	if (Role == ROLE_Authority && InvManager == None && InventoryManagerClass != None)
	{
		InvManager = Spawn(InventoryManagerClass, Self);
		if ( InvManager == None )
			`log("Warning! Couldn't spawn InventoryManager" @ InventoryManagerClass @ "for" @ Self @ GetHumanReadableName() );
		else
			InvManager.SetupFor( Self );
	}

	//debug
	ClearPathStep();
}


/**
 * Spawn default controller for this Pawn, get possessed by it.
 */
function SpawnDefaultController()
{
	if ( Controller != None )
	{
		`log("SpawnDefaultController" @ Self @ ", Controller != None" @ Controller );
		return;
	}

	if ( ControllerClass != None )
	{
		Controller = Spawn(ControllerClass);
	}

	if ( Controller != None )
	{
		Controller.Possess( Self, false );
	}
}

/**
 * Deletes the current controller if it exists and creates a new one
 * using the specified class.
 * Event called from Kismet.
 *
 * @param		inAction - scripted action that was activated
 */
function OnAssignController(SeqAct_AssignController inAction)
{

	if ( inAction.ControllerClass != None )
	{
		if ( Controller != None )
		{
			DetachFromController( true );
		}

		Controller = Spawn(inAction.ControllerClass);
		Controller.Possess( Self, false );

		// Set class as the default one if pawn is restarted.
		if ( Controller.IsA('AIController') )
		{
			ControllerClass = class<AIController>(Controller.Class);
		}
	}
	else
	{
		`warn("Assign controller w/o a class specified!");
	}
}

/**
 * Iterates through the list of item classes specified in the action
 * and creates instances that are addeed to this Pawn's inventory.
 *
 * @param		inAction - scripted action that was activated
 */
simulated function OnGiveInventory(SeqAct_GiveInventory InAction)
{
	local int Idx;
	local class<Inventory> InvClass;

	if (InAction.bClearExisting)
	{
		InvManager.DiscardInventory();
	}

	if (InAction.InventoryList.Length > 0 )
	{
		for (Idx = 0; Idx < InAction.InventoryList.Length; Idx++)
		{
			InvClass = InAction.InventoryList[idx];
			if (InvClass != None)
			{
				// only create if it doesn't already exist
				if (FindInventoryType(InvClass,FALSE) == None)
				{
					CreateInventory(InvClass);
				}
			}
			else
			{
				InAction.ScriptLog("WARNING: Attempting to give NULL inventory!");
			}
		}
	}
	else
	{
		InAction.ScriptLog("WARNING: Give Inventory without any inventory specified!");
	}
}

function Gasp();

function SetMovementPhysics()
{
	// check for water volume
	if (PhysicsVolume.bWaterVolume)
	{
		SetPhysics(PHYS_Swimming);
	}
	else if (Physics != PHYS_Falling)
	{
		SetPhysics(PHYS_Falling);
	}
}

/* AdjustDamage()
adjust damage based on inventory, other attributes
*/
function AdjustDamage( out int inDamage, out Vector momentum, Controller instigatedBy, Vector hitlocation, class<DamageType> damageType, optional TraceHitInfo HitInfo );

function bool HealDamage(int Amount, Controller Healer, class<DamageType> DamageType)
{
	// not if already dead or already at full
	if (Health > 0 && Health < HealthMax)
	{
		Health = Min(HealthMax, Health + Amount);
		return true;
	}
	else
	{
		return false;
	}
}

/** Take a list of bones passed to TakeRadiusDamageOnBones and remove the ones that don't matter */
function PruneDamagedBoneList( out array<Name> Bones );

/**
 *	Damage radius applied to specific bones on the skeletal mesh
 */
event bool TakeRadiusDamageOnBones
(
 Controller			InstigatedBy,
 float				BaseDamage,
 float				DamageRadius,
class<DamageType>	DamageType,
	float				Momentum,
	vector				HurtOrigin,
	bool				bFullDamage,
	Actor				DamageCauser,
	array<Name>			Bones
	)
{

	local int			Idx;
	local TraceHitInfo	HitInfo;
	local bool			bResult;
	local float			DamageScale, Dist;
	local vector		Dir, BoneLoc;

	PruneDamagedBoneList( Bones );

	for( Idx = 0; Idx < Bones.Length; Idx++ )
	{
		HitInfo.BoneName	 = Bones[Idx];
		HitInfo.HitComponent = Mesh;

		BoneLoc = Mesh.GetBoneLocation(Bones[Idx]);
		Dir		= BoneLoc - HurtOrigin;
		Dist	= VSize(Dir);
		Dir		= Normal(Dir);
		if( bFullDamage )
		{
			DamageScale = 1.f;
		}
		else
		{
			DamageScale = 1.f - Dist/DamageRadius;
		}

		if( DamageScale > 0.f )
		{
			TakeDamage
			(
				DamageScale * BaseDamage,
				InstigatedBy,
				BoneLoc,
				DamageScale * Momentum * Dir,
				DamageType,
				HitInfo,
				DamageCauser
			);
		}

		bResult = TRUE;
	}

	return bResult;
}

/** sends any notifications to anything that needs to know this pawn has taken damage */
function NotifyTakeHit(Controller InstigatedBy, vector HitLocation, int Damage, class<DamageType> DamageType, vector Momentum)
{
	if (Controller != None)
	{
		Controller.NotifyTakeHit(InstigatedBy, HitLocation, Damage, DamageType, Momentum);
	}
}

function controller SetKillInstigator(Controller InstigatedBy, class<DamageType> DamageType)
{
	if ( (InstigatedBy != None) && (InstigatedBy != Controller) )
	{
		return InstigatedBy;
	}
	else if ( DamageType.default.bCausedByWorld && (LastHitBy != None) )
	{
		return LastHitBy;
	}
	return InstigatedBy;
}

event TakeDamage(int Damage, Controller InstigatedBy, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local int actualDamage;
	local PlayerController PC;
	local Controller Killer;

	if ( (Role < ROLE_Authority) || (Health <= 0) )
	{
		return;
	}

	if ( damagetype == None )
	{
		if ( InstigatedBy == None )
			`warn("No damagetype for damage with no instigator");
		else
			`warn("No damagetype for damage by "$instigatedby.pawn$" with weapon "$InstigatedBy.Pawn.Weapon);
		//scripttrace();
		DamageType = class'DamageType';
	}
	Damage = Max(Damage, 0);

	if (Physics == PHYS_None && DrivenVehicle == None)
	{
		SetMovementPhysics();
	}
	if (Physics == PHYS_Walking && damageType.default.bExtraMomentumZ)
	{
		momentum.Z = FMax(momentum.Z, 0.4 * VSize(momentum));
	}
	momentum = momentum/Mass;

	if ( DrivenVehicle != None )
	{
		DrivenVehicle.AdjustDriverDamage( Damage, InstigatedBy, HitLocation, Momentum, DamageType );
	}

	ActualDamage = Damage;
	WorldInfo.Game.ReduceDamage(ActualDamage, self, instigatedBy, HitLocation, Momentum, DamageType);
	AdjustDamage(ActualDamage, Momentum, instigatedBy, HitLocation, DamageType, HitInfo);

	// call Actor's version to handle any SeqEvent_TakeDamage for scripting
	Super.TakeDamage(ActualDamage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);

	Health -= actualDamage;
	if (HitLocation == vect(0,0,0))
	{
		HitLocation = Location;
	}

	if ( Health <= 0 )
	{
		PC = PlayerController(Controller);
		// play force feedback for death
		if (PC != None)
		{
			PC.ClientPlayForceFeedbackWaveform(damageType.default.KilledFFWaveform);
		}
		// pawn died
		Killer = SetKillInstigator(InstigatedBy, DamageType);
		TearOffMomentum = momentum;
		Died(Killer, damageType, HitLocation);
	}
	else
	{
		AddVelocity( momentum, HitLocation, DamageType, HitInfo );
		NotifyTakeHit(InstigatedBy, HitLocation, ActualDamage, DamageType, Momentum);
		if (DrivenVehicle != None)
		{
			DrivenVehicle.NotifyDriverTakeHit(InstigatedBy, HitLocation, actualDamage, DamageType, Momentum);
		}
		if ( instigatedBy != None && instigatedBy != controller )
		{
			LastHitBy = instigatedBy;
		}
	}
	PlayHit(actualDamage,InstigatedBy, hitLocation, damageType, Momentum, HitInfo);
	MakeNoise(1.0);
}

/*
 * Queries the PRI and returns our current team index.
 */
simulated native function byte GetTeamNum();


simulated function TeamInfo GetTeam()
{
	if (Controller != None && Controller.PlayerReplicationInfo != None)
	{
		return Controller.PlayerReplicationInfo.Team;
	}
	else if (PlayerReplicationInfo != None)
	{
		return PlayerReplicationInfo.Team;
	}
	else if (DrivenVehicle != None && DrivenVehicle.PlayerReplicationInfo != None)
	{
		return DrivenVehicle.PlayerReplicationInfo.Team;
	}
	else
	{
		return None;
	}
}

/** Returns true of pawns are on the same team, false otherwise */
simulated event bool IsSameTeam( Pawn Other )
{
	 return ( Other != None &&
		Other.GetTeam() != None &&
		Other.GetTeam() == GetTeam() );
}

/**
 * This pawn has died.
 *
 * @param	Killer			Who killed this pawn
 * @param	DamageType		What killed it
 * @param	HitLocation		Where did the hit occur
 *
 * @returns true if allowed
 */
function bool Died(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{
	local SeqAct_Latent Action;

	// ensure a valid damagetype
	if ( damageType == None )
	{
		damageType = class'DamageType';
	}
	// if already destroyed or level transition is occuring then ignore
	if ( bDeleteMe || WorldInfo.Game == None || WorldInfo.Game.bLevelChange )
	{
		return FALSE;
	}
	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	if ( DamageType.default.bCausedByWorld && (Killer == None || Killer == Controller) && LastHitBy != None )
	{
		Killer = LastHitBy;
	}
	// gameinfo hook to prevent deaths
	// WARNING - don't prevent bot suicides - they suicide when really needed
	if ( WorldInfo.Game.PreventDeath(self, Killer, damageType, HitLocation) )
	{
		Health = max(Health, 1);
		return false;
	}
	Health = Min(0, Health);
	// activate death events
	TriggerEventClass( class'SeqEvent_Death', self );
	// and abort any latent actions
	foreach LatentActions(Action)
	{
		Action.AbortFor(self);
	}
	LatentActions.Length = 0;
	// notify the vehicle we are currently driving
	if ( DrivenVehicle != None )
	{
		Velocity = DrivenVehicle.Velocity;
		DrivenVehicle.DriverDied(DamageType);
	}
	else if ( Weapon != None )
	{
		Weapon.HolderDied();
		ThrowActiveWeapon( DamageType );
	}
	// notify the gameinfo of the death
	if ( Controller != None )
	{
		WorldInfo.Game.Killed(Killer, Controller, self, damageType);
	}
	else
	{
		WorldInfo.Game.Killed(Killer, Controller(Owner), self, damageType);
	}
	DrivenVehicle = None;
	// notify inventory manager
	if ( InvManager != None )
	{
		InvManager.OwnerEvent('died');
		// and destroy
		InvManager.Destroy();
		InvManager = None;
	}
	// push the corpse upward (@fixme - somebody please remove this?)
	Velocity.Z *= 1.3;
	// if this is a human player then force a replication update
	if ( IsHumanControlled() )
	{
		PlayerController(Controller).ForceDeathUpdate();
	}
	PlayDying(DamageType, HitLocation);
	return TRUE;
}

event Falling();

event HitWall(vector HitNormal, actor Wall, PrimitiveComponent WallComp);

event Landed(vector HitNormal, Actor FloorActor)
{
	TakeFallingDamage();
	if ( Health > 0 )
		PlayLanded(Velocity.Z);
	LastHitBy = None;
}

event HeadVolumeChange(PhysicsVolume newHeadVolume)
{
	if ( (WorldInfo.NetMode == NM_Client) || (Controller == None) )
		return;
	if ( HeadVolume.bWaterVolume )
	{
		if (!newHeadVolume.bWaterVolume)
		{
			if ( Controller.bIsPlayer && (BreathTime > 0) && (BreathTime < 8) )
				Gasp();
			BreathTime = -1.0;
		}
	}
	else if ( newHeadVolume.bWaterVolume )
	{
		BreathTime = UnderWaterTime;
	}
}

function bool TouchingWaterVolume()
{
	local PhysicsVolume V;

	ForEach TouchingActors(class'PhysicsVolume',V)
		if ( V.bWaterVolume )
			return true;

	return false;
}

//Pain timer just expired.
//Check what zone I'm in (and which parts are)
//based on that cause damage, and reset BreathTime

function bool IsInPain()
{
	local PhysicsVolume V;

	ForEach TouchingActors(class'PhysicsVolume',V)
		if ( V.bPainCausing && (V.DamagePerSec > 0) )
			return true;
	return false;
}

event BreathTimer()
{
	if ( HeadVolume.bWaterVolume )
	{
		if ( (Health < 0) || (WorldInfo.NetMode == NM_Client) || (DrivenVehicle != None) )
			return;
		TakeDrowningDamage();
		if ( Health > 0 )
			BreathTime = 2.0;
	}
	else
	{
		BreathTime = 0.0;
	}
}

function TakeDrowningDamage();

function bool CheckWaterJump(out vector WallNormal)
{
	local actor HitActor;
	local vector HitLocation, HitNormal, Checkpoint, start, checkNorm, Extent;

	if ( AIController(Controller) != None )
	{
		if ( Controller.InLatentExecution(Controller.LATENT_MOVETOWARD) && (Controller.Movetarget != None)
			&& !Controller.MoveTarget.PhysicsVolume.bWaterVolume )
		{
			CheckPoint = Normal(Controller.MoveTarget.Location - Location);
		}
		else
		{
			Checkpoint = Acceleration;
		}
		Checkpoint.Z = 0.0;
	}
	if ( Checkpoint == vect(0,0,0) )
	{
		Checkpoint = vector(Rotation);
	}
	Checkpoint.Z = 0.0;
	checkNorm = Normal(Checkpoint);
	Checkpoint = Location + 1.2 * CylinderComponent.CollisionRadius * checkNorm;
	Extent = CylinderComponent.CollisionRadius * vect(1,1,0);
	Extent.Z = CylinderComponent.CollisionHeight;
	HitActor = Trace(HitLocation, HitNormal, Checkpoint, Location, true, Extent,,TRACEFLAG_Blocking);
	if ( (HitActor != None) && (Pawn(HitActor) == None) )
	{
		WallNormal = -1 * HitNormal;
		start = Location;
		start.Z += MaxOutOfWaterStepHeight;
		checkPoint = start + 3.2 * CylinderComponent.CollisionRadius * WallNormal;
		HitActor = Trace(HitLocation, HitNormal, Checkpoint, start, true,,,TRACEFLAG_Blocking);
		if ( (HitActor == None) || (HitNormal.Z > 0.7) )
			return true;
	}

	return false;
}

//Player Jumped
function bool DoJump( bool bUpdating )
{
	if (bJumpCapable && !bIsCrouched && !bWantsToCrouch && (Physics == PHYS_Walking || Physics == PHYS_Ladder || Physics == PHYS_Spider))
	{
		if ( Physics == PHYS_Spider )
			Velocity = JumpZ * Floor;
		else if ( Physics == PHYS_Ladder )
			Velocity.Z = 0;
		else if ( bIsWalking )
			Velocity.Z = Default.JumpZ;
		else
			Velocity.Z = JumpZ;
		if (Base != None && !Base.bWorldGeometry && Base.Velocity.Z > 0.f)
		{
			Velocity.Z += Base.Velocity.Z;
		}
		SetPhysics(PHYS_Falling);
		return true;
	}
	return false;
}

function PlayDyingSound();

function PlayHit(float Damage, Controller InstigatedBy, vector HitLocation, class<DamageType> damageType, vector Momentum, TraceHitInfo HitInfo)
{
	if ( (Damage <= 0) && ((Controller == None) || !Controller.bGodMode) )
		return;

	LastPainTime = WorldInfo.TimeSeconds;
}

/** TurnOff()
Freeze pawn - stop sounds, animations, physics, weapon firing
*/
simulated function TurnOff()
{
	if (Role == ROLE_Authority)
	{
		RemoteRole = ROLE_SimulatedProxy;
	}
	if (WorldInfo.NetMode != NM_DedicatedServer && Mesh != None)
	{
		Mesh.bPauseAnims = true;
		if (Physics == PHYS_RigidBody)
		{
			Mesh.PhysicsWeight = 1.0;
			Mesh.bUpdateKinematicBonesFromAnimation = false;
		}
	}
	SetCollision(true,false);
	bNoWeaponFiring = true;
	Velocity = vect(0,0,0);
	SetPhysics(PHYS_None);
	bIgnoreForces = true;
	if (Weapon != None)
	{
		Weapon.StopFire(Weapon.CurrentFireMode);
	}

}

State Dying
{
ignores Bump, HitWall, HeadVolumeChange, PhysicsVolumeChange, Falling, BreathTimer, FellOutOfWorld;

	simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon) {}
	simulated function PlayNextAnimation() {}
	singular event BaseChange() {}
	event Landed(vector HitNormal, Actor FloorActor) {}

	function bool Died(Controller Killer, class<DamageType> damageType, vector HitLocation);

	  simulated singular event OutsideWorldBounds()
	  {
		  SetPhysics(PHYS_None);
		  SetHidden(True);
		  LifeSpan = FMin(LifeSpan, 1.0);
	  }

	event Timer()
	{
		if ( !PlayerCanSeeMe() )
		{
			Destroy();
		}
		else
		{
			SetTimer(2.0, false);
		}
	}

	event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
	{
		SetPhysics(PHYS_Falling);

		if ( (Physics == PHYS_None) && (Momentum.Z < 0) )
			Momentum.Z *= -1;

		Velocity += 3 * momentum/(Mass + 200);

		if ( damagetype == None )
		{
			// `warn("No damagetype for damage by "$instigatedby.pawn$" with weapon "$InstigatedBy.Pawn.Weapon);
			DamageType = class'DamageType';
		}

		Damage *= DamageType.default.GibModifier;
		Health -= Damage;
	}

	/**
	  * Set physics for dying pawn
	  * Always set to falling, unless already a ragdoll
	  */
	function SetDyingPhysics()
	{
		if ( Physics != PHYS_RigidBody )
		{
			SetPhysics(PHYS_Falling);
		}
	}

	event BeginState(Name PreviousStateName)
	{
		local Actor A;
		local array<SequenceEvent> TouchEvents;
		local int i;

		if ( bTearOff && (WorldInfo.NetMode == NM_DedicatedServer) )
			LifeSpan = 2.0;
		else
			SetTimer(5.0, false);

		SetDyingPhysics();

		SetCollision(true, false);

		if ( Controller != None )
		{
			if ( Controller.bIsPlayer )
			{
				DetachFromController();
			}
			else
			{
				Controller.Destroy();
			}
		}

		foreach TouchingActors(class'Actor', A)
		{
			if (A.FindEventsOfClass(class'SeqEvent_Touch', TouchEvents))
			{
				for (i = 0; i < TouchEvents.length; i++)
				{
					SeqEvent_Touch(TouchEvents[i]).NotifyTouchingPawnDied(self);
				}
				// clear array for next iteration
				TouchEvents.length = 0;
			}
		}
		foreach BasedActors(class'Actor', A)
		{
			A.PawnBaseDied();
		}
	}

Begin:
	Sleep(0.2);
	PlayDyingSound();
}

//=============================================================================
// Animation interface for controllers

/* PlayXXX() function called by controller to play transient animation actions
*/
/* PlayDying() is called on server/standalone game when killed
and also on net client when pawn gets bTearOff set to true (and bPlayedDeath is false)
*/
simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	GotoState('Dying');
	bReplicateMovement = false;
	bTearOff = true;
	Velocity += TearOffMomentum;
	SetPhysics(PHYS_Falling);
	bPlayedDeath = true;
}

simulated event TornOff()
{
	// assume dead if bTearOff
	if ( !bPlayedDeath )
	{
		PlayDying(HitDamageType,TakeHitLocation);
	}
}

/**
 * PlayFootStepSound()
 * called by AnimNotify_Footstep
 *
 * FootDown specifies which foot hit
 */
event PlayFootStepSound(int FootDown);


//=============================================================================
// Pawn internal animation functions

// Animation group checks (usually implemented in subclass)

function bool CannotJumpNow()
{
	return false;
}

function PlayLanded(float impactVel);

native function Vehicle GetVehicleBase();

function Suicide()
{
	KilledBy(self);
}

// toss out a weapon
// check before throwing
simulated function bool CanThrowWeapon()
{
	return ( (Weapon != None) && Weapon.CanThrow() );
}

/************************************************************************************
 * Vehicle driving
 ***********************************************************************************/


/**
 * StartDriving() and StopDriving() also called on clients
 * on transitions of DrivenVehicle variable.
 * Network: ALL
 */
simulated event StartDriving(Vehicle V)
{
	StopFiring();
	if ( Health <= 0 )
		return;

	DrivenVehicle = V;
	bForceNetUpdate = TRUE;

	// Move the driver into position, and attach to car.
	ShouldCrouch(false);
	bIgnoreForces = true;
	bCanTeleport = false;
	BreathTime = 0.0;
	V.AttachDriver( Self );
}

/**
 * StartDriving() and StopDriving() also called on clients
 * on transitions of DrivenVehicle variable.
 * Network: ALL
 */
simulated event StopDriving(Vehicle V)
{
	if ( Mesh != None )
	{
		Mesh.SetCullDistance(Default.Mesh.CachedMaxDrawDistance);
		Mesh.SetShadowParent(None);
	}
	bForceNetUpdate = TRUE;
	if (V != None  )
	{
		V.StopFiring();
	}

	if ( Physics == PHYS_RigidBody )
	{
		return;
	}

	DrivenVehicle = None;
	bIgnoreForces = false;
	SetHardAttach(false);
	bCanTeleport = true;
	bCollideWorld = true;

	if ( V != None )
	{
		V.DetachDriver( Self );
	}

	SetCollision(true, true);

	if ( Role == ROLE_Authority )
	{
		if ( PhysicsVolume.bWaterVolume && (Health > 0) )
		{
			SetPhysics(PHYS_Swimming);
		}
		else
		{
			SetPhysics(PHYS_Falling);
		}
		SetBase(None);
		SetHidden(False);
	}
}

//
// Inventory related functions
//

/* AddDefaultInventory:
	Add Pawn default Inventory.
	Called from GameInfo.AddDefaultInventory()
*/
function AddDefaultInventory();

/* epic ===============================================
* ::CreateInventory
*
* Create Inventory Item, adds it to the Pawn's Inventory
* And returns it for post processing.
*
* =====================================================
*/
event final Inventory CreateInventory( class<Inventory> NewInvClass, optional bool bDoNotActivate )
{
	if ( InvManager != None )
		return InvManager.CreateInventory( NewInvClass, bDoNotActivate );

	return None;
}

/* FindInventoryType:
	returns the inventory item of the requested class if it exists in this Pawn's inventory
*/
simulated final function Inventory FindInventoryType(class<Inventory> DesiredClass, optional bool bAllowSubclass)
{
	return (InvManager != None) ? InvManager.FindInventoryType(DesiredClass, bAllowSubclass) : None;
}

/** Hook called from HUD actor. Gives access to HUD and Canvas */
simulated function DrawHUD( HUD H )
{
	if ( InvManager != None )
	{
		InvManager.DrawHUD( H );
	}
}

/**
 * Toss active weapon using default settings (location+velocity).
 *
 * @param DamageType  allows this function to do different behaviors based on the damage type
 */
function ThrowActiveWeapon( optional class<DamageType> DamageType )
{
	if ( Weapon != None )
	{
		TossInventory(Weapon, , DamageType);
	}
}

function OnThrowActiveWeapon(SeqAct_ThrowActiveWeapon Action)
{
	ThrowActiveWeapon();
}

function TossInventory( Inventory Inv, optional vector ForceVelocity, optional class<DamageType> DamageType )
{
	local vector	POVLoc, TossVel;
	local rotator	POVRot;
	local Vector	X,Y,Z;

	if ( ForceVelocity != vect(0,0,0) )
	{
		TossVel = ForceVelocity;
	}
	else
	{
		GetActorEyesViewPoint(POVLoc, POVRot);
		TossVel = Vector(POVRot);
		TossVel = TossVel * ((Velocity Dot TossVel) + 500) + Vect(0,0,200);
	}

	GetAxes(Rotation, X, Y, Z);
	Inv.DropFrom(Location + 0.8 * CylinderComponent.CollisionRadius * X - 0.5 * CylinderComponent.CollisionRadius * Y, TossVel);
}

/* SetActiveWeapon
	Set this weapon as the Pawn's active weapon
*/
simulated function SetActiveWeapon( Weapon NewWeapon )
{
	if ( InvManager != None )
	{
		InvManager.SetCurrentWeapon( NewWeapon );
	}
}


/**
 * Player just changed weapon. Called from InventoryManager::ChangedWeapon().
 * Network: Local Player and Server.
 *
 * @param	OldWeapon	Old weapon held by Pawn.
 * @param	NewWeapon	New weapon held by Pawn.
 */
simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon);

// Cheats - invoked by CheatManager
function bool CheatWalk()
{
	UnderWaterTime = Default.UnderWaterTime;
	SetCollision(true, true);
	SetPhysics(PHYS_Falling);
	bCollideWorld = true;
	SetPushesRigidBodies(Default.bPushesRigidBodies);
	return true;
}

function bool CheatGhost()
{
	UnderWaterTime = -1.0;
	SetCollision(false, false);
	bCollideWorld = false;
	SetPushesRigidBodies(false);
	return true;
}

function bool CheatFly()
{
	UnderWaterTime = Default.UnderWaterTime;
	SetCollision(true, true);
	bCollideWorld = true;
	return true;
}

/**
 * Returns the collision radius of our cylinder
 * collision component.
 *
 * @return	the collision radius of our pawn
 */
simulated function float GetCollisionRadius()
{
	return (CylinderComponent != None) ? CylinderComponent.CollisionRadius : 0.f;
}

/**
 * Returns the collision height of our cylinder
 * collision component.
 *
 * @return	collision height of our pawn
 */
simulated function float GetCollisionHeight()
{
	return (CylinderComponent != None) ? CylinderComponent.CollisionHeight : 0.f;
}

/** @return a vector representing the box around this pawn's cylinder collision component, for use with traces */
simulated final function vector GetCollisionExtent()
{
	local vector Extent;

	Extent = GetCollisionRadius() * vect(1,1,0);
	Extent.Z = GetCollisionHeight();
	return Extent;
}

/**
 * Pawns by nature are not stationary.	Override if you want exact findingds
 */

function bool IsStationary()
{
	return false;
}

event SpawnedByKismet()
{
	// notify controller
	if (Controller != None)
	{
		Controller.SpawnedByKismet();
	}
}


/** Performs actual attachment. Can be subclassed for class specific behaviors. */
function DoKismetAttachment(Actor Attachment, SeqAct_AttachToActor Action)
{
	local bool	bOldCollideActors, bOldBlockActors, bValidBone, bValidSocket;

	// If a bone/socket has been specified, see if it is valid
	if( Mesh != None && Action.BoneName != '' )
	{
		// See if the bone name refers to an existing socket on the skeletal mesh.
		bValidSocket	= (Mesh.GetSocketByName(Action.BoneName) != None);
		bValidBone		= (Mesh.MatchRefBone(Action.BoneName) != INDEX_NONE);

		// Issue a warning if we were expecting to attach to a bone/socket, but it could not be found.
		if( !bValidBone && !bValidSocket )
		{
			`log(WorldInfo.TimeSeconds @ class @ GetFuncName() @ "bone or socket" @ Action.BoneName @ "not found on actor" @ Self @ "with mesh" @ Mesh);
		}
	}

	// Special case for handling relative location/rotation w/ bone or socket
	if( bValidBone || bValidSocket )
	{
		// disable collision, so we can successfully move the attachment
		bOldCollideActors	= Attachment.bCollideActors;
		bOldBlockActors		= Attachment.bBlockActors;
		Attachment.SetCollision(FALSE, FALSE);
		Attachment.SetHardAttach(Action.bHardAttach);

		// Sockets by default move the actor to the socket location.
		// This is not the case for bones!
		// So if we use relative offsets, then first move attachment to bone's location.
		if( bValidBone && !bValidSocket )
		{
			if( Action.bUseRelativeOffset )
			{
				Attachment.SetLocation(Mesh.GetBoneLocation(Action.BoneName));
			}

			if( Action.bUseRelativeRotation )
			{
				Attachment.SetRotation(QuatToRotator(Mesh.GetBoneQuaternion(Action.BoneName)));
			}
		}

		// Attach attachment to base.
		Attachment.SetBase(Self,, Mesh, Action.BoneName);

		if( Action.bUseRelativeRotation )
		{
			Attachment.SetRelativeRotation(Attachment.RelativeRotation + Action.RelativeRotation);
		}

		// if we're using the offset, place attachment relatively to the target
		if( Action.bUseRelativeOffset )
		{
			Attachment.SetRelativeLocation(Attachment.RelativeLocation + Action.RelativeOffset);
		}

		// restore previous collision
		Attachment.SetCollision(bOldCollideActors, bOldBlockActors);
	}
	else
	{
		// otherwise base on location
		Super.DoKismetAttachment(Attachment, Action);
	}
}


/** returns the amount this pawn's damage should be scaled by */
function float GetDamageScaling()
{
	return DamageScaling;
}

/** PoweredUp()
returns true if pawn has game play advantages, as defined by specific game implementation.
*/
function bool PoweredUp()
{
	return (DamageScaling > 1);
}

/** InCombat()
returns true if pawn is currently in combat, as defined by specific game implementation.
*/
function bool InCombat()
{
	return false;
}

function OnSetMaterial(SeqAct_SetMaterial Action)
{
	if (Mesh != None)
	{
		Mesh.SetMaterial( Action.MaterialIndex, Action.NewMaterial );
	}
}

/** Kismet teleport handler, overridden so that updating rotation properly updates our Controller as well */
simulated function OnTeleport(SeqAct_Teleport Action)
{
	local array<Object> objVars;
	local int idx;
	local Actor destActor;
	local Controller C;

	// find the first supplied actor
	Action.GetObjectVars(objVars,"Destination");
	for (idx = 0; idx < objVars.Length && destActor == None; idx++)
	{
		destActor = Actor(objVars[idx]);

		// If its a player variable, teleport to the Pawn not the Controller.
		C = Controller(destActor);
		if(C != None && C.Pawn != None)
		{
			destActor = C.Pawn;
		}
	}
	// and set to that actor's location
	if (destActor != None && SetLocation(destActor.Location))
	{
		PlayTeleportEffect(false, true);
		if (Action.bUpdateRotation)
		{
			SetRotation(destActor.Rotation);
			if (Controller != None)
			{
				Controller.SetRotation(destActor.Rotation);
				Controller.ClientSetRotation(destActor.Rotation);
			}
		}
	}
	else
	{
		`warn("Unable to teleport to"@destActor);
	}

	// Tell controller we teleported (Pass None to avoid recursion)
	if( Controller != None )
	{
		Controller.OnTeleport( None );
	}
}

/**
 * NOTE: using this function for things that are "fired off" immediately at spawn (e.g. res in effect) and never "updated" again
 * this check will always return false.  For effects that are getting updated this code will be able to return true over time due to the
 * LastRenderTime being updated even if the pawn stays in the same location.
 **/
simulated function bool EffectIsRelevant(vector SpawnLocation, bool bForceDedicated, optional float CullDistance )
{
	local PlayerController P;

	if ( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return bForceDedicated;
	}

	if ( (WorldInfo.NetMode == NM_ListenServer) && (WorldInfo.Game.NumPlayers + WorldInfo.Game.NumSpectators > 1) )
	{
		if ( bForceDedicated )
			return true;
		if ( IsHumanControlled() && IsLocallyControlled() )
			return true;
	}
	else if ( IsHumanControlled() )
	{
		return true;
	}

	if ((SpawnLocation != Location) || WorldInfo.TimeSeconds - LastRenderTime < 1.0)
	{
		foreach LocalPlayerControllers(class'PlayerController', P)
		{
			if (P.ViewTarget != None && (P.Pawn == self || CheckMaxEffectDistance(P, SpawnLocation, CullDistance)))
			{
				return true;
			}
		}
	}
	return false;
}

final event MessagePlayer( coerce String Msg )
{
`if(`notdefined(FINAL_RELEASE))
	local PlayerController PC;

	foreach LocalPlayerControllers(class'PlayerController', PC)
	{
		PC.ClientMessage( Msg );
	}
`endif
}

/** moves the camera in or out */
simulated function AdjustCameraScale(bool bMoveCameraIn);

simulated event BecomeViewTarget(PlayerController PC)
{
	if (PhysicsVolume != None)
	{
		PhysicsVolume.NotifyPawnBecameViewTarget(self, PC);
	}
}

/** For AI debugging */
event SoakPause()
{
	local PlayerController PC;

	ForEach WorldInfo.LocalPlayerControllers(class'PlayerController', PC)
	{
		PC.SoakPause(self);
		break;
	}
}

native function ClearConstraints();
native function AddPathConstraint( PathConstraint Constraint );
native function AddGoalEvaluator( PathGoalEvaluator Evaluator );

/**
 * Path shaping creation functions...
 * these functions by default will just new the class, but this offers a handy
 * interface to override for to do things like pool the constraints
 */
function PathConstraint CreatePathConstraint( class<PathConstraint> ConstraintClass )
{
	return new(self) ConstraintClass;
}
function PathGoalEvaluator CreatePathGoalEvaluator( class<PathGoalEvaluator> GoalEvalClass )
{
	return new(self) GoalEvalClass;
}

native function IncrementPathStep( int Cnt, Canvas C );
native function IncrementPathChild( int Cnt, Canvas C );
native function DrawPathStep( Canvas C );
native function	ClearPathStep();

simulated function ZeroMovementVariables()
{
	Velocity = vect(0,0,0);
	Acceleration = vect(0,0,0);
}


/** Kismet Action to add/remove FaceFXAnimSets. */
simulated function OnAddRemoveFaceFXAnimSet( SeqAct_AddRemoveFaceFXAnimSet InAction )
{
	local int AnimSetIdx;

	if( ( Mesh != none )
		&& ( Mesh.SkeletalMesh != none )
		&& ( Mesh.SkeletalMesh.FaceFXAsset != none )
		)
	{
		for( AnimSetIdx = 0; AnimSetIdx < InAction.FaceFXAnimSets.Length; ++AnimSetIdx )
		{
			// Add FaceFXAnimSets
			if( InAction.InputLinks[0].bHasImpulse )
			{
				Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet( InAction.FaceFXAnimSets[AnimSetIdx] );
				//`log( "Add FaceFXAnimSets" @ InAction.FaceFXAnimSets[AnimSetIdx] );
			}
			// Remove FaceFXAnimSets
			else if( InAction.InputLinks[1].bHasImpulse )
			{
				Mesh.SkeletalMesh.FaceFXAsset.UnmountFaceFXAnimSet( InAction.FaceFXAnimSets[AnimSetIdx] );
				//`log( "Remove FaceFXAnimSets" @ InAction.FaceFXAnimSets[AnimSetIdx] );
			}
		}
	}
}

simulated function SetCinematicMode( bool bInCinematicMode );

defaultproperties
{
	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	// Pawns often manipulate physics components so need to be done pre-async
	TickGroup=TG_PreAsyncWork

	InventoryManagerClass=class'InventoryManager'
	ControllerClass=class'AIController'

	// Flags
	bCanBeDamaged=true
	bCanCrouch=false
	bCanFly=false
	bCanJump=true
	bCanSwim=false
	bCanTeleport=true
	bCanWalk=true
	bJumpCapable=true
	bProjTarget=true
	bSimulateGravity=true
	bShouldBaseAtStartup=true

	// Locomotion
	LandMovementState=PlayerWalking
	WaterMovementState=PlayerSwimming

	AccelRate=+02048.000000
	DesiredSpeed=+00001.000000
	MaxDesiredSpeed=+00001.000000
	MaxFallSpeed=+1200.0
	AIMaxFallSpeedFactor=1.0
	NonPreferredVehiclePathMultiplier=1.0

	AirSpeed=+00600.000000
	GroundSpeed=+00600.000000
	JumpZ=+00420.000000
	OutofWaterZ=+420.0
	LadderSpeed=+200.0
	WaterSpeed=+00300.000000

	AirControl=+0.05

	CrouchedPct=+0.5
	WalkingPct=+0.5

	// Sound
	bLOSHearing=true
	HearingThreshold=+2800.0
	SoundDampening=+00001.000000
	noise1time=-00010.000000
	noise2time=-00010.000000

	// Physics
	AvgPhysicsTime=+00000.100000
	bPushesRigidBodies=false
	RBPushRadius=10.0
	RBPushStrength=50.0

	// FOV / Sight
	ViewPitchMin=-16384
	ViewPitchMax=16383
	RotationRate=(Pitch=20000,Yaw=20000,Roll=20000)
	MaxPitchLimit=3072

	SightRadius=+05000.000000

	// Network
	RemoteRole=ROLE_SimulatedProxy
	bUpdateSimulatedPosition=true
	bStasis=false

	// GamePlay
	bCanUse=true
	DamageScaling=+00001.000000
	Health=100

	// Collision
	BaseEyeHeight=+00064.000000
	EyeHeight=+00054.000000

	CrouchHeight=+40.0
	CrouchRadius=+34.0

	MaxStepHeight=35.0
	MaxJumpHeight=96.0
	WalkableFloorZ=0.7		   // 0.7 ~= 45 degree angle for floor
	MaxOutOfWaterStepHeight=40.0
	AllowedYawError=2000
	Mass=+00100.000000

	bCollideActors=true
	bCollideWorld=true
	bBlockActors=true

	Begin Object Class=CylinderComponent Name=CollisionCylinder
		CollisionRadius=+0034.000000
		CollisionHeight=+0078.000000
		BlockNonZeroExtent=true
		BlockZeroExtent=true
		BlockActors=true
		CollideActors=true
	End Object
	CollisionComponent=CollisionCylinder
	CylinderComponent=CollisionCylinder
	Components.Add(CollisionCylinder)

	Begin Object Class=ArrowComponent Name=Arrow
		ArrowColor=(R=150,G=200,B=255)
		bTreatAsASprite=True
	End Object
	Components.Add(Arrow)

	VehicleCheckRadius=150
}
