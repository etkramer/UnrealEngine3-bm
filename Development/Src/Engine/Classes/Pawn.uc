class Pawn extends Actor
    abstract
    native
    nativereplication
    config(Game)
    placeable;

enum EPathSearchType
{
    PST_Default,                    // 0
    PST_Breadth,                    // 1
    PST_NewBestPathTo,              // 2
    PST_Constraint,                 // 3
    PST_MAX                         // 4
};

var const float MaxStepHeight;
var const float MaxJumpHeight;
var const float WalkableFloorZ;
var repnotify Controller Controller;
var const Pawn NextPawn;
var float NetRelevancyTime;
var PlayerController LastRealViewer;
var Actor LastViewer;
var bool bUpAndOut;
var bool bIsWalking;
var bool bWantsToCrouch;
var const bool bIsCrouched;
var const bool bTryToUncrouch;
var() bool bCanCrouch;
var bool bCrouchCollisionCheck;
var bool bCrawler;
var const bool bReducedSpeed;
var bool bJumpCapable;
var bool bCanJump;
var bool bCanWalk;
var bool bCanSwim;
var bool bCanFly;
var bool bCanClimbLadders;
var bool bCanStrafe;
var bool bAvoidLedges;
var bool bStopAtLedges;
var const bool bSimulateGravity;
var bool bIgnoreForces;
var bool bCanWalkOffLedges;
var bool bCanBeBaseForPawns;
var const bool bSimGravityDisabled;
var bool bDirectHitWall;
var const bool bPushesRigidBodies;
var bool bForceFloorCheck;
var bool bForceKeepAnchor;
var bool bRootMotionOverridesFallingXY;
var config bool bCanMantle;
var config bool bCanClimbUp;
var bool bCanClimbCeilings;
var config bool bCanSwatTurn;
var config bool bCanLeap;
var config bool bCanCoverSlip;
var globalconfig bool bDisplayPathErrors;
var bool bIsFemale;
var bool bCanPickupInventory;
var bool bAmbientCreature;
var(AI) bool bLOSHearing;
var(AI) bool bMuffledHearing;
var(AI) bool bDontPossess;
var bool bAutoFire;
var bool bRollToDesired;
var bool bStationary;
var bool bCachedRelevant;
var bool bSpecialHUD;
var bool bNoWeaponFiring;
var bool bCanUse;
var bool bModifyReachSpecCost;
var bool bModifyNavPointDest;
var bool bPathfindsAsVehicle;
var bool bIsRoaming;
var bool bForceMaxAnchorChecks;
var bool bRunPhysicsWithNoController;
var bool bForceMaxAccel;
var bool bForceRMVelocity;
var bool bForceRegularVelocity;
var bool bPlayedDeath;
var bool bCanTraverse;
var bool bUseSimplePhysWalking;
var bool bUseComplexStepUpCode;
var bool bIsBatman;
var const float UncrouchTime;
var float CrouchHeight;
var float CrouchRadius;
var const int FullHeight;
var float NonPreferredVehiclePathMultiplier;
var Pawn.EPathSearchType PathSearchType;
var const byte RemoteViewPitch;
var repnotify byte FlashCount;
var repnotify byte FiringMode;
var PathConstraint PathConstraintList;
var PathGoalEvaluator PathGoalList;
var float DesiredSpeed;
var float MaxDesiredSpeed;
var(AI) float HearingThreshold;
var(AI) float Alertness;
var(AI) float SightRadius;
var(AI) float PeripheralVision;
var const float AvgPhysicsTime;
var float Mass;
var float Buoyancy;
var float MeleeRange;
var const NavigationPoint Anchor;
var const NavigationPoint LastAnchor;
var float FindAnchorFailedTime;
var float LastValidAnchorTime;
var float DestinationOffset;
var float NextPathRadius;
var Vector SerpentineDir;
var float SerpentineDist;
var float SerpentineTime;
var float SpawnTime;
var int MaxPitchLimit;
var int MaxPathLength;
var float GroundSpeed;
var float WaterSpeed;
var float AirSpeed;
var float LadderSpeed;
var float AccelRate;
var float JumpZ;
var float OutofWaterZ;
var float MaxOutOfWaterStepHeight;
var float AirControl;
var float WalkingPct;
var float CrouchedPct;
var float MaxFallSpeed;
var float AIMaxFallSpeedFactor;
var(Camera) float BaseEyeHeight;
var(Camera) float EyeHeight;
var Vector Floor;
var float SplashTime;
var float OldZ;
var transient PhysicsVolume HeadVolume;
var() int Health;
var() int HealthMax;
var float BreathTime;
var float UnderWaterTime;
var float LastPainTime;
var Vector RMVelocity;
var const Vector noise1spot;
var const float noise1time;
var const Pawn noise1other;
var const float noise1loudness;
var const Vector noise2spot;
var const float noise2time;
var const Pawn noise2other;
var const float noise2loudness;
var float SoundDampening;
var float DamageScaling;
var const localized string MenuName;
var class<AIController> ControllerClass;
var repnotify PlayerReplicationInfo PlayerReplicationInfo;
var LadderVolume OnLadder;
var name LandMovementState;
var name WaterMovementState;
var PlayerStart LastStartSpot;
var float LastStartTime;
var Vector TakeHitLocation;
var class<DamageType> HitDamageType;
var Vector TearOffMomentum;
var() export editinline SkeletalMeshComponent Mesh;
var export editinline CylinderComponent CylinderComponent;
var() float RBPushRadius;
var() float RBPushStrength;
var repnotify Vehicle DrivenVehicle;
var float AlwaysRelevantDistanceSquared;
var() float VehicleCheckRadius;
var Controller LastHitBy;
var() float ViewPitchMin;
var() float ViewPitchMax;
var int AllowedYawError;
var class<InventoryManager> InventoryManagerClass;
var repnotify InventoryManager InvManager;
var() Weapon Weapon;
var repnotify Vector FlashLocation;
var Vector LastFiringFlashLocation;
var int ShotCount;
var export editinline PrimitiveComponent PreRagdollCollisionComponent;
var RB_BodyInstance PhysicsPushBody;
var int FailedLandingCount;
var Vector walkFailPoint;
var Actor LinkedCullPawn;

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

// Export UPawn::execSetBasedPosition(FFrame&, void* const)
native static final function SetBasedPosition(out BasedPosition BP, Vector pos, optional Actor ForcedBase);

// Export UPawn::execGetBasedPosition(FFrame&, void* const)
native static final function Vector GetBasedPosition(BasedPosition BP);

simulated event ReplicatedEvent(name VarName)
{
    super.ReplicatedEvent(VarName);
    // End:0x2C
    if(VarName == 'FlashCount')
    {
        FlashCountUpdated(true);        
    }
    else
    {
        // End:0x4D
        if(VarName == 'FlashLocation')
        {
            FlashLocationUpdated(true);            
        }
        else
        {
            // End:0x6E
            if(VarName == 'FiringMode')
            {
                FiringModeUpdated(true);                
            }
            else
            {
                // End:0x99
                if(VarName == 'DrivenVehicle')
                {
                    // End:0x96
                    if(DrivenVehicle != none)
                    {
                        NotifyTeamChanged();
                    }                    
                }
                else
                {
                    // End:0xB9
                    if(VarName == 'PlayerReplicationInfo')
                    {
                        NotifyTeamChanged();                        
                    }
                    else
                    {
                        // End:0x14A
                        if(VarName == 'Controller')
                        {
                            // End:0x14A
                            if((Controller != none) && Controller.Pawn == none)
                            {
                                Controller.Pawn = self;
                                // End:0x14A
                                if((PlayerController(Controller) != none) && PlayerController(Controller).ViewTarget == Controller)
                                {
                                    PlayerController(Controller).SetViewTarget(self);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //return;    
}

// Export UPawn::execIsAliveAndWell(FFrame&, void* const)
native final simulated function bool IsAliveAndWell();

// Export UPawn::execAdjustDestination(FFrame&, void* const)
native final function Vector AdjustDestination(Actor GoalActor, optional Vector Dest);

// Export UPawn::execValidAnchor(FFrame&, void* const)
native final function bool ValidAnchor();

// Export UPawn::execSuggestJumpVelocity(FFrame&, void* const)
native function bool SuggestJumpVelocity(out Vector JumpVelocity, Vector Destination, Vector Start);

// Export UPawn::execIsValidTargetFor(FFrame&, void* const)
native function bool IsValidTargetFor(const Controller C);

// Export UPawn::execIsValidEnemyTargetFor(FFrame&, void* const)
native function bool IsValidEnemyTargetFor(const PlayerReplicationInfo PRI, bool bNoPRIisEnemy);

// Export UPawn::execIsInvisible(FFrame&, void* const)
native function bool IsInvisible();

// Export UPawn::execSetRemoteViewPitch(FFrame&, void* const)
native final function SetRemoteViewPitch(int NewRemoteViewPitch);

// Export UPawn::execSetAnchor(FFrame&, void* const)
native function SetAnchor(NavigationPoint NewAnchor);

// Export UPawn::execGetBestAnchor(FFrame&, void* const)
native function NavigationPoint GetBestAnchor(Actor TestActor, Vector TestLocation, bool bStartPoint, bool bOnlyCheckVisible, out float out_Dist);

// Export UPawn::execReachedDestination(FFrame&, void* const)
native function bool ReachedDestination(Actor Goal);

// Export UPawn::execReachedPoint(FFrame&, void* const)
native function bool ReachedPoint(Vector Point, Actor NewAnchor);

// Export UPawn::execForceCrouch(FFrame&, void* const)
native function ForceCrouch();

// Export UPawn::execSetPushesRigidBodies(FFrame&, void* const)
native function SetPushesRigidBodies(bool NewPush);

// Export UPawn::execReachedDesiredRotation(FFrame&, void* const)
native final function bool ReachedDesiredRotation();

// Export UPawn::execGetBoundingCylinder(FFrame&, void* const)
native function GetBoundingCylinder(out float CollisionRadius, out float CollisionHeight);

function int SpecialCostForPath(ReachSpec Path)
{
    return NavigationPoint(Path.End.Actor).Cost;
    //return ReturnValue;    
}

simulated function bool IsValidEnemy()
{
    return true;
    //return ReturnValue;    
}

// Export UPawn::execInitRagdoll(FFrame&, void* const)
native function bool InitRagdoll();

// Export UPawn::execTermRagdoll(FFrame&, void* const)
native function bool TermRagdoll();

function bool SpecialMoveTo(NavigationPoint Start, NavigationPoint End, Actor Next)
{
    //return ReturnValue;    
}

simulated function SetBaseEyeheight()
{
    // End:0x19
    if(!bIsCrouched)
    {
        BaseEyeHeight = default.BaseEyeHeight;        
    }
    else
    {
        BaseEyeHeight = FMin(0.8000000 * CrouchHeight, CrouchHeight - float(10));
    }
    //return;    
}

function PlayerChangedTeam()
{
    Died(none, Class'DamageType', Location);
    //return;    
}

function Reset()
{
    // End:0x31
    if((Controller == none) || Controller.bIsPlayer)
    {
        DetachFromController();
        Destroy();        
    }
    else
    {
        super.Reset();
    }
    //return;    
}

function bool StopFiring()
{
    // End:0x2E
    if(Weapon != none)
    {
        Weapon.StopFire(Weapon.CurrentFireMode);
    }
    return true;
    //return ReturnValue;    
}

simulated function StartFire(byte FireModeNum)
{
    // End:0x0B
    if(bNoWeaponFiring)
    {
        return;
    }
    // End:0x2F
    if(InvManager != none)
    {
        InvManager.StartFire(FireModeNum);
    }
    //return;    
}

simulated function StopFire(byte FireModeNum)
{
    // End:0x24
    if(InvManager != none)
    {
        InvManager.StopFire(FireModeNum);
    }
    //return;    
}

simulated function SetFiringMode(byte FiringModeNum)
{
    // End:0x31
    if(FiringModeNum != FiringMode)
    {
        FiringMode = FiringModeNum;
        bForceNetUpdate = true;
        FiringModeUpdated(false);
    }
    //return;    
}

simulated function FiringModeUpdated(bool bViaReplication)
{
    // End:0x2A
    if(Weapon != none)
    {
        Weapon.FireModeUpdated(FiringMode, bViaReplication);
    }
    //return;    
}

simulated function IncrementFlashCount(Weapon Who, byte FireModeNum)
{
    bForceNetUpdate = true;
    FlashCount++;
    // End:0x28
    if(FlashCount == 0)
    {
        FlashCount += 2;
    }
    SetFiringMode(FireModeNum);
    FlashCountUpdated(false);
    //return;    
}

simulated function ClearFlashCount(Weapon Who)
{
    // End:0x2B
    if(FlashCount != 0)
    {
        bForceNetUpdate = true;
        FlashCount = 0;
        FlashCountUpdated(false);
    }
    //return;    
}

function SetFlashLocation(Weapon Who, byte FireModeNum, Vector NewLoc)
{
    // End:0x23
    if(NewLoc == LastFiringFlashLocation)
    {
        NewLoc += vect(0.0000000, 0.0000000, 1.0000000);
    }
    // End:0x4D
    if(NewLoc == vect(0.0000000, 0.0000000, 0.0000000))
    {
        NewLoc = vect(0.0000000, 0.0000000, 1.0000000);
    }
    bForceNetUpdate = true;
    FlashLocation = NewLoc;
    LastFiringFlashLocation = NewLoc;
    SetFiringMode(FireModeNum);
    FlashLocationUpdated(false);
    //return;    
}

function ClearFlashLocation(Weapon Who)
{
    // End:0x33
    if(!IsZero(FlashLocation))
    {
        bForceNetUpdate = true;
        FlashLocation = vect(0.0000000, 0.0000000, 0.0000000);
        FlashLocationUpdated(false);
    }
    //return;    
}

simulated function FlashCountUpdated(bool bViaReplication)
{
    // End:0x24
    if(FlashCount > 0)
    {
        WeaponFired(bViaReplication);        
    }
    else
    {
        WeaponStoppedFiring(bViaReplication);
    }
    //return;    
}

simulated function FlashLocationUpdated(bool bViaReplication)
{
    // End:0x25
    if(!IsZero(FlashLocation))
    {
        WeaponFired(bViaReplication, FlashLocation);        
    }
    else
    {
        WeaponStoppedFiring(bViaReplication);
    }
    //return;    
}

simulated function WeaponFired(bool bViaReplication, optional Vector HitLocation)
{
    ShotCount++;
    // End:0x31
    if(Weapon != none)
    {
        Weapon.PlayFireEffects(FiringMode, HitLocation);
    }
    //return;    
}

simulated function WeaponStoppedFiring(bool bViaReplication)
{
    ShotCount = 0;
    // End:0x2B
    if(Weapon != none)
    {
        Weapon.StopFireEffects(FiringMode);
    }
    //return;    
}

function bool BotFire(bool bFinished)
{
    StartFire(ChooseFireMode());
    return true;
    //return ReturnValue;    
}

function byte ChooseFireMode()
{
    return 0;
    //return ReturnValue;    
}

function bool CanAttack(Actor Other)
{
    // End:0x0D
    if(Weapon == none)
    {
        return false;
    }
    return Weapon.CanAttack(Other);
    //return ReturnValue;    
}

function bool TooCloseToAttack(Actor Other)
{
    return false;
    //return ReturnValue;    
}

function bool FireOnRelease()
{
    // End:0x20
    if(Weapon != none)
    {
        return Weapon.FireOnRelease();
    }
    return false;
    //return ReturnValue;    
}

function bool HasRangedAttack()
{
    return Weapon != none;
    //return ReturnValue;    
}

function bool IsFiring()
{
    // End:0x20
    if(Weapon != none)
    {
        return Weapon.IsFiring();
    }
    return false;
    //return ReturnValue;    
}

function bool NeedToTurn(Vector targ)
{
    local Vector LookDir, AimDir;

    LookDir = Vector(Rotation);
    LookDir.Z = 0.0000000;
    LookDir = Normal(LookDir);
    AimDir = targ - Location;
    AimDir.Z = 0.0000000;
    AimDir = Normal(AimDir);
    return (LookDir Dot AimDir) < 0.9300000;
    //return ReturnValue;    
}

simulated function string GetHumanReadableName()
{
    // End:0x1B
    if(PlayerReplicationInfo != none)
    {
        return PlayerReplicationInfo.PlayerName;
    }
    return MenuName;
    //return ReturnValue;    
}

function PlayTeleportEffect(bool bOut, bool bSound)
{
    MakeNoise(1.0000000);
    //return;    
}

simulated function NotifyTeamChanged()
{
    //return;    
}

function PossessedBy(Controller C, bool bVehicleTransition)
{
    Controller = C;
    // End:0x35
    if(C.PlayerReplicationInfo != none)
    {
        PlayerReplicationInfo = C.PlayerReplicationInfo;
    }
    UpdateControllerOnPossess(bVehicleTransition);
    SetOwner(Controller);
    EyeHeight = BaseEyeHeight;
    // End:0xB6
    if(C.IsA('PlayerController'))
    {
        // End:0x92
        if(WorldInfo.NetMode != NM_Standalone)
        {
            RemoteRole = ROLE_AutonomousProxy;
        }
        // End:0xB3
        if(Weapon != none)
        {
            Weapon.ClientWeaponSet(false);
        }        
    }
    else
    {
        RemoteRole = default.RemoteRole;
        // End:0xE6
        if(Weapon != none)
        {
            Weapon.AIController = AIController(C);
        }
    }
    //return;    
}

function UpdateControllerOnPossess(bool bVehicleTransition)
{
    // End:0x1D
    if(!bVehicleTransition)
    {
        Controller.SetRotation(Rotation);
    }
    //return;    
}

function UnPossessed()
{
    LogInternalAI(("" $ string(self)) $ " Unpossessed!");
    PlayerReplicationInfo = none;
    SetOwner(none);
    Controller = none;
    //return;    
}

simulated function name GetDefaultCameraMode(PlayerController RequestedBy)
{
    // End:0x55
    if(((RequestedBy != none) && RequestedBy.PlayerCamera != none) && RequestedBy.PlayerCamera.CameraStyle == 'Fixed')
    {
        return 'Fixed';
    }
    return 'FirstPerson';
    //return ReturnValue;    
}

function DropToGround()
{
    bCollideWorld = true;
    // End:0x3C
    if(Health > 0)
    {
        SetCollision(true, true);
        SetPhysics(2);
        // End:0x3C
        if(IsHumanControlled())
        {
            Controller.GotoState(LandMovementState);
        }
    }
    //return;    
}

function bool CanGrabLadder()
{
    return ((bCanClimbLadders && Controller != none) && Physics != 9) && (Physics != 2) || Abs(Velocity.Z) <= JumpZ;
    //return ReturnValue;    
}

function bool RecommendLongRangedAttack()
{
    return false;
    //return ReturnValue;    
}

function float RangedAttackTime()
{
    return 0.0000000;
    //return ReturnValue;    
}

event SetWalking(bool bNewIsWalking)
{
    // End:0x1E
    if(bNewIsWalking != bIsWalking)
    {
        bIsWalking = bNewIsWalking;
    }
    //return;    
}

simulated function bool CanSplash()
{
    // End:0x78
    if((((WorldInfo.TimeSeconds - SplashTime) > 0.1500000) && (Physics == 2) || Physics == 4) && Abs(Velocity.Z) > float(100))
    {
        SplashTime = WorldInfo.TimeSeconds;
        return true;
    }
    return false;
    //return ReturnValue;    
}

function EndClimbLadder(LadderVolume OldLadder)
{
    // End:0x1F
    if(Controller != none)
    {
        Controller.EndClimbLadder();
    }
    // End:0x35
    if(Physics == 9)
    {
        SetPhysics(2);
    }
    //return;    
}

function ClimbLadder(LadderVolume L)
{
    OnLadder = L;
    SetRotation(OnLadder.WallDir);
    SetPhysics(9);
    // End:0x44
    if(IsHumanControlled())
    {
        Controller.GotoState('PlayerClimbing');
    }
    //return;    
}

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
    local string T;
    local Canvas Canvas;
    local AnimTree AnimTreeRootNode;
    local int I;

    Canvas = HUD.Canvas;
    // End:0x76
    if(PlayerReplicationInfo == none)
    {
        Canvas.DrawText("NO PLAYERREPLICATIONINFO", false);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);        
    }
    else
    {
        PlayerReplicationInfo.DisplayDebug(HUD, out_YL, out_YPos);
    }
    super.DisplayDebug(HUD, out_YL, out_YPos);
    Canvas.SetDrawColor(255, 255, 255);
    Canvas.DrawText("Health " $ string(Health));
    out_YPos += out_YL;
    Canvas.SetPos(4.0000000, out_YPos);
    // End:0x1AC
    if(HUD.ShouldDisplayDebug('AI'))
    {
        Canvas.DrawText((((("Anchor " $ string(Anchor)) $ " Serpentine Dist ") $ string(SerpentineDist)) $ " Time ") $ string(SerpentineTime));
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
    }
    // End:0x3F3
    if(HUD.ShouldDisplayDebug('Physics'))
    {
        T = (((("Floor " $ string(Floor)) $ " DesiredSpeed ") $ string(DesiredSpeed)) $ " Crouched ") $ string(bIsCrouched);
        // End:0x256
        if((OnLadder != none) || Physics == 9)
        {
            T = (T $ " on ladder ") $ string(OnLadder);
        }
        Canvas.DrawText(T);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        T = "Collision Component:" @ string(CollisionComponent);
        Canvas.DrawText(T);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        T = "bForceMaxAccel:" @ string(bForceMaxAccel);
        Canvas.DrawText(T);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        // End:0x3F3
        if(Mesh != none)
        {
            T = (("RootMotionMode:" @ string(Mesh.RootMotionMode)) @ "RootMotionVelocity:") @ string(Mesh.RootMotionVelocity);
            Canvas.DrawText(T);
            out_YPos += out_YL;
            Canvas.SetPos(4.0000000, out_YPos);
        }
    }
    // End:0x47D
    if(HUD.ShouldDisplayDebug('Camera'))
    {
        Canvas.DrawText((("EyeHeight " $ string(EyeHeight)) $ " BaseEyeHeight ") $ string(BaseEyeHeight));
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
    }
    // End:0x517
    if(Controller == none)
    {
        Canvas.SetDrawColor(255, 0, 0);
        Canvas.DrawText("NO CONTROLLER");
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        HUD.PlayerOwner.DisplayDebug(HUD, out_YL, out_YPos);        
    }
    else
    {
        Controller.DisplayDebug(HUD, out_YL, out_YPos);
    }
    // End:0x5E6
    if(HUD.ShouldDisplayDebug('Weapon'))
    {
        // End:0x5C3
        if(Weapon == none)
        {
            Canvas.SetDrawColor(0, 255, 0);
            Canvas.DrawText("NO WEAPON");
            out_YPos += out_YL;
            Canvas.SetPos(4.0000000, out_YPos);            
        }
        else
        {
            Weapon.DisplayDebug(HUD, out_YL, out_YPos);
        }
    }
    // End:0x7A7
    if(HUD.ShouldDisplayDebug('Animation'))
    {
        // End:0x7A7
        if((Mesh != none) && Mesh.Animations != none)
        {
            AnimTreeRootNode = AnimTree(Mesh.Animations);
            // End:0x7A7
            if(AnimTreeRootNode != none)
            {
                Canvas.DrawText("AnimGroups count:" @ string(AnimTreeRootNode.AnimGroups.Length));
                out_YPos += out_YL;
                Canvas.SetPos(4.0000000, out_YPos);
                I = 0;
                J0x6B4:

                // End:0x7A7 [Loop If]
                if(I < AnimTreeRootNode.AnimGroups.Length)
                {
                    Canvas.DrawText(((((" GroupName:" @ string(AnimTreeRootNode.AnimGroups[I].GroupName)) @ "NodeCount:") @ string(AnimTreeRootNode.AnimGroups[I].SeqNodes.Length)) @ "RateScale:") @ string(AnimTreeRootNode.AnimGroups[I].RateScale));
                    out_YPos += out_YL;
                    Canvas.SetPos(4.0000000, out_YPos);
                    I++;
                    // [Loop Continue]
                    goto J0x6B4;
                }
            }
        }
    }
    //return;    
}

// Export UPawn::execIsHumanControlled(FFrame&, void* const)
native final simulated function bool IsHumanControlled();

// Export UPawn::execIsLocallyControlled(FFrame&, void* const)
native final simulated function bool IsLocallyControlled();

// Export UPawn::execIsPlayerPawn(FFrame&, void* const)
native simulated function bool IsPlayerPawn();

simulated function bool WasPlayerPawn()
{
    return false;
    //return ReturnValue;    
}

simulated function bool IsFirstPerson()
{
    local PlayerController PC;

    PC = PlayerController(Controller);
    return (PC != none) && PC.UsingFirstPersonCamera();
    //return ReturnValue;    
}

simulated function ProcessViewRotation(float DeltaTime, out Rotator out_ViewRotation, out Rotator out_DeltaRot)
{
    out_ViewRotation += out_DeltaRot;
    out_DeltaRot = rot(0, 0, 0);
    // End:0x5E
    if(PlayerController(Controller) != none)
    {
        out_ViewRotation = PlayerController(Controller).LimitViewRotation(out_ViewRotation, ViewPitchMin, ViewPitchMax);
    }
    //return;    
}

simulated event GetActorEyesViewPoint(out Vector out_Location, out Rotator out_Rotation)
{
    out_Location = GetPawnViewLocation();
    out_Rotation = GetViewRotation();
    //return;    
}

// Export UPawn::execGetViewRotation(FFrame&, void* const)
native simulated event Rotator GetViewRotation();

// Export UPawn::execGetPawnViewLocation(FFrame&, void* const)
native simulated event Vector GetPawnViewLocation();

simulated function Vector GetPawnViewLocLocal()
{
    return vect(0.0000000, 0.0000000, 1.0000000) * (BaseEyeHeight + CylinderComponent.CollisionHeight);
    //return ReturnValue;    
}

simulated event Vector GetWeaponStartTraceLocation(optional Weapon CurrentWeapon)
{
    local Vector POVLoc;
    local Rotator POVRot;

    // End:0x30
    if(Controller != none)
    {
        Controller.GetPlayerViewPoint(POVLoc, POVRot);
        return POVLoc;
    }
    return GetPawnViewLocation();
    //return ReturnValue;    
}

singular simulated event Rotator GetBaseAimRotation()
{
    local Vector POVLoc;
    local Rotator POVRot;

    // End:0x40
    if((Controller != none) && !InFreeCam())
    {
        Controller.GetPlayerViewPoint(POVLoc, POVRot);
        return POVRot;
    }
    POVRot = Rotation;
    // End:0x7F
    if(POVRot.Pitch == 0)
    {
        POVRot.Pitch = RemoteViewPitch << 8;
    }
    return POVRot;
    //return ReturnValue;    
}

simulated event bool InFreeCam()
{
    local PlayerController PC;

    PC = PlayerController(Controller);
    return ((PC != none) && PC.PlayerCamera != none) && PC.PlayerCamera.CameraStyle == 'FreeCam';
    //return ReturnValue;    
}

simulated function Rotator GetAdjustedAimFor(Weapon W, Vector StartFireLoc)
{
    // End:0x28
    if((Controller == none) || Role < ROLE_Authority)
    {
        return GetBaseAimRotation();
    }
    return Controller.GetAdjustedAimFor(W, StartFireLoc);
    //return ReturnValue;    
}

simulated function SetViewRotation(Rotator NewRotation)
{
    // End:0x20
    if(Controller != none)
    {
        Controller.SetRotation(NewRotation);        
    }
    else
    {
        SetRotation(NewRotation);
    }
    //return;    
}

simulated function bool PawnCalcCamera(float fDeltaTime, out Vector out_CamLoc, out Rotator out_CamRot, out float out_FOV)
{
    return CalcCamera(fDeltaTime, out_CamLoc, out_CamRot, out_FOV);
    //return ReturnValue;    
}

function bool InGodMode()
{
    return (Controller != none) && Controller.bGodMode;
    //return ReturnValue;    
}

simulated function bool AffectedByHitEffects()
{
    return (Controller == none) || Controller.bAffectedByHitEffects;
    //return ReturnValue;    
}

function bool NearMoveTarget()
{
    // End:0x24
    if((Controller == none) || Controller.MoveTarget == none)
    {
        return false;
    }
    return ReachedDestination(Controller.MoveTarget);
    //return ReturnValue;    
}

function Actor GetMoveTarget()
{
    // End:0x0D
    if(Controller == none)
    {
        return none;
    }
    return Controller.MoveTarget;
    //return ReturnValue;    
}

function SetMoveTarget(Actor NewTarget)
{
    // End:0x20
    if(Controller != none)
    {
        Controller.MoveTarget = NewTarget;
    }
    //return;    
}

function bool LineOfSightTo(Actor Other)
{
    return (Controller != none) && Controller.LineOfSightTo(Other);
    //return ReturnValue;    
}

function float AdjustedStrength()
{
    return 0.0000000;
    //return ReturnValue;    
}

function HandlePickup(Inventory Inv)
{
    MakeNoise(0.2000000);
    // End:0x2D
    if(Controller != none)
    {
        Controller.HandlePickup(Inv);
    }
    //return;    
}

function ReceiveLocalizedMessage(class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject)
{
    // End:0x46
    if(PlayerController(Controller) != none)
    {
        PlayerController(Controller).ReceiveLocalizedMessage(Message, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);
    }
    //return;    
}

event ClientMessage(coerce string S, optional name Type)
{
    // End:0x35
    if(PlayerController(Controller) != none)
    {
        PlayerController(Controller).ClientMessage(S, Type);
    }
    //return;    
}

function FinishedInterpolation()
{
    DropToGround();
    //return;    
}

function JumpOutOfWater(Vector jumpDir)
{
    Falling();
    Velocity = jumpDir * WaterSpeed;
    Acceleration = jumpDir * AccelRate;
    Velocity.Z = OutofWaterZ;
    bUpAndOut = true;
    //return;    
}

simulated event ModifyVelocity(float DeltaTime, Vector OldVelocity)
{
    //return;    
}

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
    // End:0x5E
    if(Role == ROLE_Authority)
    {
        Health = -1;
        Died(none, dmgType, Location);
        // End:0x5E
        if(dmgType == none)
        {
            SetPhysics(0);
            SetHidden(true);
            LifeSpan = FMin(LifeSpan, 1.0000000);
        }
    }
    //return;    
}

singular simulated event OutsideWorldBounds()
{
    // End:0x28
    if((Role == ROLE_Authority) && PlayerController(Controller) == none)
    {
        Destroy();        
    }
    else
    {
        // End:0x43
        if(Role == ROLE_Authority)
        {
            KilledBy(self);
        }
        SetPhysics(PHYS_None);
        SetHidden(true);
        LifeSpan = FMin(LifeSpan, 1.0000000);
    }
    //return;    
}

simulated function UnCrouch()
{
    // End:0x1F
    if(bIsCrouched || bWantsToCrouch)
    {
        ShouldCrouch(false);
    }
    //return;    
}

function ShouldCrouch(bool bCrouch)
{
    bWantsToCrouch = bCrouch;
    //return;    
}

simulated event EndCrouch(float HeightAdjust)
{
    EyeHeight -= HeightAdjust;
    OldZ += HeightAdjust;
    SetBaseEyeheight();
    //return;    
}

simulated event StartCrouch(float HeightAdjust)
{
    EyeHeight += HeightAdjust;
    OldZ -= HeightAdjust;
    SetBaseEyeheight();
    //return;    
}

function RestartPlayer()
{
    //return;    
}

function AddVelocity(Vector NewVelocity, Vector HitLocation, class<DamageType> DamageType, optional TraceHitInfo HitInfo)
{
    // End:0x25
    if(bIgnoreForces || NewVelocity == vect(0.0000000, 0.0000000, 0.0000000))
    {
        return;
    }
    // End:0x7B
    if((Physics == 1) || ((Physics == 9) || Physics == 8) && NewVelocity.Z > default.JumpZ)
    {
        SetPhysics(2);
    }
    // End:0xC6
    if((Velocity.Z > default.JumpZ) && NewVelocity.Z > float(0))
    {
        NewVelocity.Z *= 0.5000000;
    }
    Velocity += NewVelocity;
    //return;    
}

function KilledBy(Pawn EventInstigator)
{
    local Controller Killer;

    Health = 0;
    // End:0x2E
    if(EventInstigator != none)
    {
        Killer = EventInstigator.Controller;
        LastHitBy = none;
    }
    Died(Killer, Class'DmgType_Suicided', Location);
    //return;    
}

function TakeFallingDamage()
{
    local float EffectiveSpeed;

    // End:0xE8
    if(Velocity.Z < (-0.5000000 * MaxFallSpeed))
    {
        // End:0xE5
        if(Role == ROLE_Authority)
        {
            MakeNoise(1.0000000);
            // End:0xE5
            if(Velocity.Z < (float(-1) * MaxFallSpeed))
            {
                EffectiveSpeed = Velocity.Z;
                // End:0x8B
                if(TouchingWaterVolume())
                {
                    EffectiveSpeed += float(100);
                }
                // End:0xE5
                if(EffectiveSpeed < (float(-1) * MaxFallSpeed))
                {
                    TakeDamage(int((float(-100) * (EffectiveSpeed + MaxFallSpeed)) / MaxFallSpeed), none, Location, vect(0.0000000, 0.0000000, 0.0000000), Class'DmgType_Fell');
                }
            }
        }        
    }
    else
    {
        // End:0x115
        if(Velocity.Z < (-1.4000000 * JumpZ))
        {
            MakeNoise(0.5000000);            
        }
        else
        {
            // End:0x13F
            if(Velocity.Z < (-0.8000000 * JumpZ))
            {
                MakeNoise(0.2000000);
            }
        }
    }
    //return;    
}

function Restart()
{
    //return;    
}

simulated function ClientRestart()
{
    ZeroMovementVariables();
    SetBaseEyeheight();
    //return;    
}

function ClientSetLocation(Vector NewLocation, Rotator NewRotation)
{
    // End:0x29
    if(Controller != none)
    {
        Controller.ClientSetLocation(NewLocation, NewRotation);
    }
    //return;    
}

function ClientSetRotation(Rotator NewRotation)
{
    // End:0x25
    if(Controller != none)
    {
        Controller.ClientSetRotation(NewRotation);
    }
    //return;    
}

simulated function FaceRotation(Rotator NewRotation, float DeltaTime)
{
    // End:0x73
    if(!InFreeCam())
    {
        // End:0x37
        if(Physics == 9)
        {
            NewRotation = OnLadder.WallDir;            
        }
        else
        {
            // End:0x6B
            if((Physics == 1) || Physics == 2)
            {
                NewRotation.Pitch = 0;
            }
        }
        SetRotation(NewRotation);
    }
    //return;    
}

event bool EncroachingOn(Actor Other)
{
    // End:0x2A
    if(Other.bWorldGeometry || Other.bBlocksTeleport)
    {
        return true;
    }
    // End:0x60
    if(((Controller == none) || !Controller.bIsPlayer) && Pawn(Other) != none)
    {
        return true;
    }
    return false;
    //return ReturnValue;    
}

event EncroachedBy(Actor Other)
{
    // End:0x31
    if((Pawn(Other) != none) && Vehicle(Other) == none)
    {
        gibbedBy(Other);
    }
    //return;    
}

function gibbedBy(Actor Other)
{
    // End:0x12
    if(Role < ROLE_Authority)
    {
        return;
    }
    // End:0x4D
    if(Pawn(Other) != none)
    {
        Died(Pawn(Other).Controller, Class'DmgType_Telefragged', Location);        
    }
    else
    {
        Died(none, Class'DmgType_Telefragged', Location);
    }
    //return;    
}

function JumpOffPawn()
{
    Velocity += ((float(100) + CylinderComponent.CollisionRadius) * VRand());
    // End:0x57
    if(VSize2D(Velocity) > FMax(500.0000000, GroundSpeed))
    {
        Velocity = FMax(500.0000000, GroundSpeed) * Normal(Velocity);
    }
    Velocity.Z = 200.0000000 + CylinderComponent.CollisionHeight;
    SetPhysics(2);
    //return;    
}

event StuckOnPawn(Pawn OtherPawn)
{
    //return;    
}

singular event BaseChange()
{
    local DynamicSMActor Dyn;

    // End:0x7C
    if((Pawn(Base) != none) && (DrivenVehicle == none) || !DrivenVehicle.IsBasedOn(Base))
    {
        // End:0x7C
        if(!Pawn(Base).CanBeBaseForPawn(self))
        {
            Pawn(Base).CrushedBy(self);
            JumpOffPawn();
        }
    }
    Dyn = DynamicSMActor(Base);
    // End:0xBD
    if((Dyn != none) && !Dyn.CanBasePawn(self))
    {
        JumpOffPawn();
    }
    //return;    
}

simulated function bool CanBeBaseForPawn(Pawn aPawn)
{
    return bCanBeBaseForPawns;
    //return ReturnValue;    
}

function CrushedBy(Pawn OtherPawn)
{
    TakeDamage(int(((float(1) - (OtherPawn.Velocity.Z / float(400))) * OtherPawn.Mass) / Mass), OtherPawn.Controller, Location, vect(0.0000000, 0.0000000, 0.0000000), Class'DmgType_Crushed');
    //return;    
}

function DetachFromController(optional bool bDestroyController)
{
    local Controller OldController;

    // End:0xBA
    if((Controller != none) && Controller.Pawn == self)
    {
        OldController = Controller;
        Controller.PawnDied(self);
        // End:0x62
        if(Controller != none)
        {
            Controller.UnPossess();
        }
        // End:0xB3
        if(((bDestroyController && OldController != none) && !OldController.bDeleteMe) && !OldController.bIsPlayer)
        {
            OldController.Destroy();
        }
        Controller = none;
    }
    //return;    
}

simulated event Destroyed()
{
    DetachFromController();
    // End:0x23
    if(InvManager != none)
    {
        InvManager.Destroy();
    }
    // End:0x3F
    if(WorldInfo.NetMode == NM_Client)
    {
        return;
    }
    SetAnchor(none);
    Weapon = none;
    ClearPathStep();
    super.Destroyed();
    //return;    
}

simulated event PreBeginPlay()
{
    // End:0x16
    if(HealthMax == 0)
    {
        HealthMax = default.Health;
    }
    super.PreBeginPlay();
    Instigator = self;
    DesiredRotation = Rotation;
    EyeHeight = BaseEyeHeight;
    //return;    
}

event PostBeginPlay()
{
    super.PostBeginPlay();
    SplashTime = 0.0000000;
    SpawnTime = WorldInfo.TimeSeconds;
    EyeHeight = BaseEyeHeight;
    // End:0x68
    if((WorldInfo.bStartup && Health > 0) && !bDontPossess)
    {
        SpawnDefaultController();
    }
    // End:0x11A
    if(((Role == ROLE_Authority) && InvManager == none) && InventoryManagerClass != none)
    {
        InvManager = Spawn(InventoryManagerClass, self);
        // End:0x105
        if(InvManager == none)
        {
            LogInternal(((("Warning! Couldn't spawn InventoryManager" @ string(InventoryManagerClass)) @ "for") @ string(self)) @ (GetHumanReadableName()));            
        }
        else
        {
            InvManager.SetupFor(self);
        }
    }
    ClearPathStep();
    //return;    
}

function SpawnDefaultController()
{
    local Controller TempController;

    // End:0x4E
    if(Controller != none)
    {
        LogInternal((("SpawnDefaultController" @ string(self)) @ ", Controller != None") @ string(Controller));
        return;
    }
    // End:0x70
    if(ControllerClass != none)
    {
        TempController = Spawn(ControllerClass);
    }
    // End:0x9C
    if(TempController != none)
    {
        TempController.Possess(self, false);
        Controller = TempController;
    }
    //return;    
}

function OnAssignController(SeqAct_AssignController inAction)
{
    // End:0x97
    if(inAction.ControllerClass != none)
    {
        // End:0x2B
        if(Controller != none)
        {
            DetachFromController(true);
        }
        Controller = Spawn(inAction.ControllerClass);
        Controller.Possess(self, false);
        // End:0x94
        if(Controller.IsA('AIController'))
        {
            ControllerClass = class<AIController>(Controller.Class);
        }        
    }
    else
    {
        WarnInternal("Assign controller w/o a class specified!");
    }
    //return;    
}

simulated function OnGiveInventory(SeqAct_GiveInventory inAction)
{
    local int Idx;
    local class<Inventory> InvClass;

    // End:0x27
    if(inAction.bClearExisting)
    {
        InvManager.DiscardInventory();
    }
    // End:0xF0
    if(inAction.InventoryList.Length > 0)
    {
        Idx = 0;
        J0x44:

        // End:0xED [Loop If]
        if(Idx < inAction.InventoryList.Length)
        {
            InvClass = inAction.InventoryList[Idx];
            // End:0xA5
            if(InvClass != none)
            {
                // End:0xA2
                if(FindInventoryType(InvClass, false) == none)
                {
                    CreateInventory(InvClass);
                }
                // [Explicit Continue]
                goto J0xE3;
            }
            inAction.ScriptLog("WARNING: Attempting to give NULL inventory!");
            J0xE3:

            Idx++;
            // [Loop Continue]
            goto J0x44;
        }        
    }
    else
    {
        inAction.ScriptLog("WARNING: Give Inventory without any inventory specified!");
    }
    //return;    
}

function Gasp()
{
    //return;    
}

function SetMovementPhysics()
{
    // End:0x1C
    if(PhysicsVolume.bWaterVolume)
    {
        SetPhysics(3);        
    }
    else
    {
        // End:0x32
        if(Physics != 2)
        {
            SetPhysics(2);
        }
    }
    //return;    
}

function AdjustDamage(out int inDamage, out Vector Momentum, Controller InstigatedBy, Vector HitLocation, class<DamageType> DamageType, optional TraceHitInfo HitInfo)
{
    //return;    
}

function bool HealDamage(int Amount, Controller Healer, class<DamageType> DamageType)
{
    // End:0x3A
    if((Health > 0) && Health < HealthMax)
    {
        Health = Min(HealthMax, Health + Amount);
        return true;        
    }
    else
    {
        return false;
    }
    //return ReturnValue;    
}

function PruneDamagedBoneList(out array<name> Bones)
{
    //return;    
}

event bool TakeRadiusDamageOnBones(Controller InstigatedBy, float BaseDamage, float DamageRadius, class<DamageType> DamageType, float Momentum, Vector HurtOrigin, bool bFullDamage, Actor DamageCauser, array<name> Bones)
{
    local int Idx;
    local TraceHitInfo HitInfo;
    local bool bResult;
    local float DamageScale, Dist;
    local Vector Dir, BoneLoc;

    PruneDamagedBoneList(Bones);
    Idx = 0;
    J0x16:

    // End:0x13B [Loop If]
    if(Idx < Bones.Length)
    {
        HitInfo.BoneName = Bones[Idx];
        HitInfo.HitComponent = Mesh;
        BoneLoc = Mesh.GetBoneLocation(Bones[Idx]);
        Dir = BoneLoc - HurtOrigin;
        Dist = VSize(Dir);
        Dir = Normal(Dir);
        // End:0xBD
        if(bFullDamage)
        {
            DamageScale = 1.0000000;            
        }
        else
        {
            DamageScale = 1.0000000 - (Dist / DamageRadius);
        }
        // End:0x129
        if(DamageScale > 0.0000000)
        {
            TakeDamage(int(DamageScale * BaseDamage), InstigatedBy, BoneLoc, (DamageScale * Momentum) * Dir, DamageType, HitInfo, DamageCauser);
        }
        bResult = true;
        Idx++;
        // [Loop Continue]
        goto J0x16;
    }
    return bResult;
    //return ReturnValue;    
}

function NotifyTakeHit(Controller InstigatedBy, Vector HitLocation, int Damage, class<DamageType> DamageType, Vector Momentum)
{
    // End:0x38
    if(Controller != none)
    {
        Controller.NotifyTakeHit(InstigatedBy, HitLocation, Damage, DamageType, Momentum);
    }
    //return;    
}

function Controller SetKillInstigator(Controller InstigatedBy, class<DamageType> DamageType)
{
    // End:0x25
    if((InstigatedBy != none) && InstigatedBy != Controller)
    {
        return InstigatedBy;        
    }
    else
    {
        // End:0x4B
        if(DamageType.default.bCausedByWorld && LastHitBy != none)
        {
            return LastHitBy;
        }
    }
    return InstigatedBy;
    //return ReturnValue;    
}

event TakeDamage(int Damage, Controller InstigatedBy, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
    local int actualDamage;
    local PlayerController PC;
    local Controller Killer;

    // End:0x21
    if((Role < ROLE_Authority) || Health <= 0)
    {
        return;
    }
    // End:0xD5
    if(DamageType == none)
    {
        // End:0x69
        if(InstigatedBy == none)
        {
            WarnInternal("No damagetype for damage with no instigator");            
        }
        else
        {
            WarnInternal((("No damagetype for damage by " $ string(InstigatedBy.Pawn)) $ " with weapon ") $ string(InstigatedBy.Pawn.Weapon));
        }
        DamageType = Class'DamageType';
    }
    Damage = Max(Damage, 0);
    // End:0x139
    if((Physics == 1) && DamageType.default.bExtraMomentumZ)
    {
        Momentum.Z = FMax(Momentum.Z, 0.4000000 * VSize(Momentum));
    }
    Momentum = Momentum / Mass;
    actualDamage = Damage;
    WorldInfo.Game.ReduceDamage(actualDamage, self, InstigatedBy, HitLocation, Momentum, DamageType);
    AdjustDamage(actualDamage, Momentum, InstigatedBy, HitLocation, DamageType, HitInfo);
    super.TakeDamage(actualDamage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
    Health -= actualDamage;
    // End:0x20D
    if(HitLocation == vect(0.0000000, 0.0000000, 0.0000000))
    {
        HitLocation = Location;
    }
    // End:0x297
    if(Health <= 0)
    {
        PC = PlayerController(Controller);
        // End:0x256
        if(PC != none)
        {
            PC.ClientPlayForceFeedbackWaveform(DamageType.default.KilledFFWaveform);
        }
        Killer = SetKillInstigator(InstigatedBy, DamageType);
        TearOffMomentum = Momentum;
        Died(Killer, DamageType, HitLocation);        
    }
    else
    {
        NotifyTakeHit(InstigatedBy, HitLocation, actualDamage, DamageType, Momentum);
        // End:0x2C5
        if(DrivenVehicle != none)
        {
        }
        // End:0x2EC
        if((InstigatedBy != none) && InstigatedBy != Controller)
        {
            LastHitBy = InstigatedBy;
        }
    }
    PlayHit(float(actualDamage), InstigatedBy, HitLocation, DamageType, Momentum, HitInfo);
    MakeNoise(1.0000000);
    //return;    
}

// Export UPawn::execGetTeamNum(FFrame&, void* const)
native simulated function byte GetTeamNum();

simulated function TeamInfo GetTeam()
{
    // End:0x3F
    if((Controller != none) && Controller.PlayerReplicationInfo != none)
    {
        return Controller.PlayerReplicationInfo.Team;        
    }
    else
    {
        // End:0x5D
        if(PlayerReplicationInfo != none)
        {
            return PlayerReplicationInfo.Team;            
        }
        else
        {
            // End:0x9C
            if((DrivenVehicle != none) && DrivenVehicle.PlayerReplicationInfo != none)
            {
                return DrivenVehicle.PlayerReplicationInfo.Team;                
            }
            else
            {
                return none;
            }
        }
    }
    //return ReturnValue;    
}

simulated event bool IsSameTeam(Pawn Other)
{
    return ((Other != none) && Other.GetTeam() != none) && Other.GetTeam() == (GetTeam());
    //return ReturnValue;    
}

function bool Died(Controller Killer, class<DamageType> DamageType, Vector HitLocation)
{
    local SeqAct_Latent Action;

    // End:0x16
    if(DamageType == none)
    {
        DamageType = Class'DamageType';
    }
    // End:0x57
    if((bDeleteMe || WorldInfo.Game == none) || WorldInfo.Game.bLevelChange)
    {
        return false;
    }
    // End:0xA0
    if((DamageType.default.bCausedByWorld && (Killer == none) || Killer == Controller) && LastHitBy != none)
    {
        Killer = LastHitBy;
    }
    // End:0xE1
    if(WorldInfo.Game.PreventDeath(self, Killer, DamageType, HitLocation))
    {
        Health = Max(Health, 1);
        return false;
    }
    Health = Min(0, Health);
    TriggerEventClass(Class'SeqEvent_Death', self);
    // End:0x127
    foreach LatentActions(Action)
    {
        Action.AbortFor(self);        
    }    
    LatentActions.Length = 0;
    // End:0x13E
    if(DrivenVehicle != none)
    {        
    }
    else
    {
        // End:0x16C
        if(Weapon != none)
        {
            Weapon.HolderDied();
            ThrowActiveWeapon(DamageType);
        }
    }
    // End:0x1A8
    if(Controller != none)
    {
        WorldInfo.Game.Killed(Killer, Controller, self, DamageType);        
    }
    else
    {
        WorldInfo.Game.Killed(Killer, Controller(Owner), self, DamageType);
    }
    DrivenVehicle = none;
    // End:0x21E
    if(InvManager != none)
    {
        InvManager.OwnerEvent('Died');
        InvManager.Destroy();
        InvManager = none;
    }
    Velocity.Z *= 1.3000000;
    // End:0x257
    if(IsHumanControlled())
    {
        PlayerController(Controller).ForceDeathUpdate();
    }
    PlayDying(DamageType, HitLocation);
    return true;
    //return ReturnValue;    
}

event Falling()
{
    //return;    
}

event HitWall(Vector HitNormal, Actor Wall, PrimitiveComponent WallComp)
{
    //return;    
}

event AnimationTriggerCallback(name TagName, array<string> Params, AnimSet TagAnimSet, float Time)
{
    LogInternal("Pawn::AnimationTriggerCallback" @ string(TagName));
    //return;    
}

event Landed(Vector HitNormal, Actor FloorActor)
{
    TakeFallingDamage();
    // End:0x2F
    if(Health > 0)
    {
        PlayLanded(Velocity.Z);
    }
    LastHitBy = none;
    //return;    
}

event HeadVolumeChange(PhysicsVolume newHeadVolume)
{
    // End:0x29
    if((WorldInfo.NetMode == NM_Client) || Controller == none)
    {
        return;
    }
    // End:0x9B
    if(HeadVolume.bWaterVolume)
    {
        // End:0x98
        if(!newHeadVolume.bWaterVolume)
        {
            // End:0x8D
            if((Controller.bIsPlayer && BreathTime > float(0)) && BreathTime < float(8))
            {
                Gasp();
            }
            BreathTime = -1.0000000;
        }        
    }
    else
    {
        // End:0xB9
        if(newHeadVolume.bWaterVolume)
        {
            BreathTime = UnderWaterTime;
        }
    }
    //return;    
}

function bool TouchingWaterVolume()
{
    local PhysicsVolume V;

    // End:0x27
    foreach TouchingActors(Class'PhysicsVolume', V)
    {
        // End:0x26
        if(V.bWaterVolume)
        {            
            return true;
        }        
    }    
    return false;
    //return ReturnValue;    
}

function bool IsInPain()
{
    local PhysicsVolume V;

    // End:0x40
    foreach TouchingActors(Class'PhysicsVolume', V)
    {
        // End:0x3F
        if(V.bPainCausing && V.DamagePerSec > float(0))
        {            
            return true;
        }        
    }    
    return false;
    //return ReturnValue;    
}

event BreathTimer()
{
    // End:0x6C
    if(HeadVolume.bWaterVolume)
    {
        // End:0x49
        if(((Health < 0) || WorldInfo.NetMode == NM_Client) || DrivenVehicle != none)
        {
            return;
        }
        TakeDrowningDamage();
        // End:0x69
        if(Health > 0)
        {
            BreathTime = 2.0000000;
        }        
    }
    else
    {
        BreathTime = 0.0000000;
    }
    //return;    
}

function TakeDrowningDamage()
{
    //return;    
}

function bool CheckWaterJump(out Vector WallNormal)
{
    local Actor HitActor;
    local Vector HitLocation, HitNormal, Checkpoint, Start, checkNorm, Extent;

    // End:0xC0
    if(AIController(Controller) != none)
    {
        // End:0x9F
        if((Controller.InLatentExecution(Controller.LATENT_MOVETOWARD) && Controller.MoveTarget != none) && !Controller.MoveTarget.PhysicsVolume.bWaterVolume)
        {
            Checkpoint = Normal(Controller.MoveTarget.Location - Location);            
        }
        else
        {
            Checkpoint = Acceleration;
        }
        Checkpoint.Z = 0.0000000;
    }
    // End:0xE4
    if(Checkpoint == vect(0.0000000, 0.0000000, 0.0000000))
    {
        Checkpoint = Vector(Rotation);
    }
    Checkpoint.Z = 0.0000000;
    checkNorm = Normal(Checkpoint);
    Checkpoint = Location + ((1.2000000 * CylinderComponent.CollisionRadius) * checkNorm);
    Extent = CylinderComponent.CollisionRadius * vect(1.0000000, 1.0000000, 0.0000000);
    Extent.Z = CylinderComponent.CollisionHeight;
    HitActor = Trace(HitLocation, HitNormal, Checkpoint, Location, true, Extent,, 8);
    // End:0x263
    if((HitActor != none) && Pawn(HitActor) == none)
    {
        WallNormal = float(-1) * HitNormal;
        Start = Location;
        Start.Z += MaxOutOfWaterStepHeight;
        Checkpoint = Start + ((3.2000000 * CylinderComponent.CollisionRadius) * WallNormal);
        HitActor = Trace(HitLocation, HitNormal, Checkpoint, Start, true,,, 8);
        // End:0x263
        if((HitActor == none) || HitNormal.Z > 0.7000000)
        {
            return true;
        }
    }
    return false;
    //return ReturnValue;    
}

function bool DoJump(bool bUpdating)
{
    // End:0x15B
    if(((bJumpCapable && !bIsCrouched) && !bWantsToCrouch) && ((Physics == 1) || Physics == 9) || Physics == 8)
    {
        // End:0x7E
        if(Physics == 8)
        {
            Velocity = JumpZ * Floor;            
        }
        else
        {
            // End:0xA7
            if(Physics == 9)
            {
                Velocity.Z = 0.0000000;                
            }
            else
            {
                // End:0xC9
                if(bIsWalking)
                {
                    Velocity.Z = default.JumpZ;                    
                }
                else
                {
                    Velocity.Z = JumpZ;
                }
            }
        }
        // End:0x153
        if(((Base != none) && !Base.bWorldGeometry) && Base.Velocity.Z > 0.0000000)
        {
            Velocity.Z += Base.Velocity.Z;
        }
        SetPhysics(2);
        return true;
    }
    return false;
    //return ReturnValue;    
}

function PlayDyingSound()
{
    //return;    
}

function PlayHit(float Damage, Controller InstigatedBy, Vector HitLocation, class<DamageType> DamageType, Vector Momentum, TraceHitInfo HitInfo)
{
    // End:0x33
    if((Damage <= float(0)) && (Controller == none) || !Controller.bGodMode)
    {
        return;
    }
    LastPainTime = WorldInfo.TimeSeconds;
    //return;    
}

simulated function TurnOff()
{
    // End:0x18
    if(Role == ROLE_Authority)
    {
        RemoteRole = ROLE_SimulatedProxy;
    }
    // End:0x88
    if((WorldInfo.NetMode != NM_DedicatedServer) && Mesh != none)
    {
        Mesh.bPauseAnims = true;
        // End:0x88
        if(Physics == 10)
        {
            Mesh.PhysicsWeight = 1.0000000;
            Mesh.bUpdateKinematicBonesFromAnimation = false;
        }
    }
    SetCollision(true, false);
    bNoWeaponFiring = true;
    Velocity = vect(0.0000000, 0.0000000, 0.0000000);
    SetPhysics(0);
    bIgnoreForces = true;
    // End:0xE5
    if(Weapon != none)
    {
        Weapon.StopFire(Weapon.CurrentFireMode);
    }
    //return;    
}

simulated function PlayDying(class<DamageType> DamageType, Vector HitLoc)
{
    GotoState('Dying');
    bReplicateMovement = false;
    bTearOff = true;
    Velocity += TearOffMomentum;
    bPlayedDeath = true;
    //return;    
}

simulated event TornOff()
{
    // End:0x1F
    if(!bPlayedDeath)
    {
        PlayDying(HitDamageType, TakeHitLocation);
    }
    //return;    
}

event PlayFootStepSound(int FootDown)
{
    //return;    
}

event PlayFoleySound(int Foley)
{
    //return;    
}

function bool CannotJumpNow()
{
    return false;
    //return ReturnValue;    
}

function PlayLanded(float ImpactVel)
{
    //return;    
}

// Export UPawn::execGetVehicleBase(FFrame&, void* const)
native function Vehicle GetVehicleBase();

function Suicide()
{
    KilledBy(self);
    //return;    
}

simulated function bool CanThrowWeapon()
{
    return (Weapon != none) && Weapon.CanThrow();
    //return ReturnValue;    
}

simulated event StartDriving(Vehicle V)
{
    StopFiring();
    // End:0x17
    if(Health <= 0)
    {
        return;
    }
    DrivenVehicle = V;
    bForceNetUpdate = true;
    ShouldCrouch(false);
    bIgnoreForces = true;
    bCanTeleport = false;
    BreathTime = 0.0000000;
    //return;    
}

simulated event StopDriving(Vehicle V)
{
    // End:0x3B
    if(Mesh != none)
    {
        Mesh.SetCullDistance(default.Mesh.CachedMaxDrawDistance);
        Mesh.SetShadowParent(none);
    }
    bForceNetUpdate = true;
    // End:0x62
    if(V != none)
    {
        V.StopFiring();
    }
    // End:0x74
    if(Physics == 10)
    {
        return;
    }
    DrivenVehicle = none;
    bIgnoreForces = false;
    SetHardAttach(false);
    bCanTeleport = true;
    bCollideWorld = true;
    SetCollision(true, true);
    // End:0xF1
    if(Role == ROLE_Authority)
    {
        // End:0xD9
        if(PhysicsVolume.bWaterVolume && Health > 0)
        {
            SetPhysics(3);            
        }
        else
        {
            SetPhysics(2);
        }
        SetBase(none);
        SetHidden(false);
    }
    //return;    
}

function AddDefaultInventory()
{
    //return;    
}

final event Inventory CreateInventory(class<Inventory> NewInvClass, optional bool bDoNotActivate)
{
    // End:0x2C
    if(InvManager != none)
    {
        return InvManager.CreateInventory(NewInvClass, bDoNotActivate);
    }
    return none;
    //return ReturnValue;    
}

final simulated function Inventory FindInventoryType(class<Inventory> DesiredClass, optional bool bAllowSubclass)
{
    return ((InvManager != none) ? InvManager.FindInventoryType(DesiredClass, bAllowSubclass) : none);
    //return ReturnValue;    
}

simulated function DrawHUD(HUD H)
{
    // End:0x24
    if(InvManager != none)
    {
        InvManager.DrawHUD(H);
    }
    //return;    
}

function ThrowActiveWeapon(optional class<DamageType> DamageType)
{
    // End:0x21
    if(Weapon != none)
    {
        TossInventory(Weapon,, DamageType);
    }
    //return;    
}

function OnThrowActiveWeapon(SeqAct_ThrowActiveWeapon Action)
{
    ThrowActiveWeapon();
    //return;    
}

function TossInventory(Inventory Inv, optional Vector ForceVelocity, optional class<DamageType> DamageType)
{
    local Vector POVLoc, TossVel;
    local Rotator POVRot;
    local Vector X, Y, Z;

    // End:0x27
    if(ForceVelocity != vect(0.0000000, 0.0000000, 0.0000000))
    {
        TossVel = ForceVelocity;        
    }
    else
    {
        GetActorEyesViewPoint(POVLoc, POVRot);
        TossVel = Vector(POVRot);
        TossVel = (TossVel * ((Velocity Dot TossVel) + float(500))) + vect(0.0000000, 0.0000000, 200.0000000);
    }
    GetAxes(Rotation, X, Y, Z);
    Inv.DropFrom((Location + ((0.8000000 * CylinderComponent.CollisionRadius) * X)) - ((0.5000000 * CylinderComponent.CollisionRadius) * Y), TossVel);
    //return;    
}

simulated function SetActiveWeapon(Weapon NewWeapon)
{
    // End:0x26
    if(InvManager != none)
    {
        InvManager.SetCurrentWeapon(NewWeapon);
    }
    //return;    
}

simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon)
{
    //return;    
}

function bool CheatWalk()
{
    UnderWaterTime = default.UnderWaterTime;
    SetCollision(true, true);
    SetPhysics(2);
    bCollideWorld = true;
    SetPushesRigidBodies(default.bPushesRigidBodies);
    return true;
    //return ReturnValue;    
}

function bool CheatGhost()
{
    UnderWaterTime = -1.0000000;
    SetCollision(false, false);
    bCollideWorld = false;
    SetPushesRigidBodies(false);
    return true;
    //return ReturnValue;    
}

function bool CheatFly()
{
    UnderWaterTime = default.UnderWaterTime;
    SetCollision(true, true);
    bCollideWorld = true;
    return true;
    //return ReturnValue;    
}

simulated function float GetCollisionRadius()
{
    return ((CylinderComponent != none) ? CylinderComponent.CollisionRadius : 0.0000000);
    //return ReturnValue;    
}

simulated function float GetCollisionHeight()
{
    return ((CylinderComponent != none) ? CylinderComponent.CollisionHeight : 0.0000000);
    //return ReturnValue;    
}

final simulated function Vector GetCollisionExtent()
{
    local Vector Extent;

    Extent = (GetCollisionRadius()) * vect(1.0000000, 1.0000000, 0.0000000);
    Extent.Z = GetCollisionHeight();
    return Extent;
    //return ReturnValue;    
}

function bool IsStationary()
{
    return false;
    //return ReturnValue;    
}

event SpawnedByKismet()
{
    // End:0x1F
    if(Controller != none)
    {
        Controller.SpawnedByKismet();
    }
    //return;    
}

function DoKismetAttachment(Actor Attachment, SeqAct_AttachToActor Action)
{
    local bool bOldCollideActors, bOldBlockActors, bValidBone, bValidSocket;

    // End:0x115
    if((Mesh != none) && Action.BoneName != 'None')
    {
        bValidSocket = Mesh.GetSocketByName(Action.BoneName) != none;
        bValidBone = Mesh.MatchRefBone(Action.BoneName) != -1;
        // End:0x115
        if(!bValidBone && !bValidSocket)
        {
            LogInternal((((((((string(WorldInfo.TimeSeconds) @ string(Class)) @ string(GetFuncName())) @ "bone or socket") @ string(Action.BoneName)) @ "not found on actor") @ string(self)) @ "with mesh") @ string(Mesh));
        }
    }
    // End:0x2EA
    if(bValidBone || bValidSocket)
    {
        bOldCollideActors = Attachment.bCollideActors;
        bOldBlockActors = Attachment.bBlockActors;
        Attachment.SetCollision(false, false);
        Attachment.SetHardAttach(Action.bHardAttach);
        // End:0x223
        if(bValidBone && !bValidSocket)
        {
            // End:0x1DD
            if(Action.bUseRelativeOffset)
            {
                Attachment.SetLocation(Mesh.GetBoneLocation(Action.BoneName));
            }
            // End:0x223
            if(Action.bUseRelativeRotation)
            {
                Attachment.SetRotation(QuatToRotator(Mesh.GetBoneQuaternion(Action.BoneName)));
            }
        }
        Attachment.SetBase(self,, Mesh, Action.BoneName);
        // End:0x28A
        if(Action.bUseRelativeRotation)
        {
            Attachment.SetRelativeRotation(Attachment.RelativeRotation + Action.RelativeRotation);
        }
        // End:0x2CD
        if(Action.bUseRelativeOffset)
        {
            Attachment.SetRelativeLocation(Attachment.RelativeLocation + Action.RelativeOffset);
        }
        Attachment.SetCollision(bOldCollideActors, bOldBlockActors);        
    }
    else
    {
        super.DoKismetAttachment(Attachment, Action);
    }
    //return;    
}

function float GetDamageScaling()
{
    return DamageScaling;
    //return ReturnValue;    
}

function bool PoweredUp()
{
    return DamageScaling > float(1);
    //return ReturnValue;    
}

function bool InCombat()
{
    return false;
    //return ReturnValue;    
}

function OnSetMaterial(SeqAct_SetMaterial Action)
{
    // End:0x3D
    if(Mesh != none)
    {
        Mesh.SetMaterial(Action.MaterialIndex, Action.NewMaterial);
    }
    //return;    
}

function OnSetMaterialInstance(RSeqAct_SetMaterialInstance Action)
{
    // End:0x3D
    if(Mesh != none)
    {
        Mesh.SetMaterial(Action.MaterialIndex, Action.NewMaterial);
    }
    //return;    
}

event EmitOnTeleport()
{
    OnTeleport(none);
    //return;    
}

simulated function OnTeleport(SeqAct_Teleport Action)
{
    local bool UpdateRot;
    local Vector Loc;
    local Rotator Rot;

    UpdateRot = Action.GetDestination(Loc, Rot);
    // End:0x87
    if(SetLocation(Loc))
    {
        PlayTeleportEffect(false, true);
        // End:0x84
        if(UpdateRot)
        {
            SetRotation(Rot);
            // End:0x84
            if(Controller != none)
            {
                Controller.SetRotation(Rot);
                Controller.ClientSetRotation(Rot);
            }
        }        
    }
    else
    {
        WarnInternal("Unable to teleport to" @ string(Loc));
    }
    // End:0xC9
    if(Controller != none)
    {
        Controller.OnTeleport(none);
    }
    //return;    
}

simulated function bool EffectIsRelevant(Vector SpawnLocation, bool bForceDedicated, optional float CullDistance)
{
    local PlayerController P;

    // End:0x22
    if(WorldInfo.NetMode == NM_DedicatedServer)
    {
        return bForceDedicated;
    }
    // End:0x9C
    if((WorldInfo.NetMode == NM_ListenServer) && (WorldInfo.Game.NumPlayers + WorldInfo.Game.NumSpectators) > 1)
    {
        // End:0x83
        if(bForceDedicated)
        {
            return true;
        }
        // End:0x99
        if(IsHumanControlled() && IsLocallyControlled())
        {
            return true;
        }        
    }
    else
    {
        // End:0xA7
        if(IsHumanControlled())
        {
            return true;
        }
    }
    // End:0x13A
    if((SpawnLocation != Location) || (WorldInfo.TimeSeconds - LastRenderTime) < 1.0000000)
    {
        // End:0x139
        foreach LocalPlayerControllers(Class'PlayerController', P)
        {
            // End:0x138
            if((P.ViewTarget != none) && (P.Pawn == self) || CheckMaxEffectDistance(P, SpawnLocation, CullDistance))
            {                
                return true;
            }            
        }        
    }
    return false;
    //return ReturnValue;    
}

event bool IsInLoadedVisibleWorld()
{
    return true;
    //return ReturnValue;    
}

final event MessagePlayer(coerce string msg)
{
    local PlayerController PC;

    // End:0x2F
    foreach LocalPlayerControllers(Class'PlayerController', PC)
    {
        PC.ClientMessage(msg);        
    }    
    //return;    
}

simulated function AdjustCameraScale(bool bMoveCameraIn)
{
    //return;    
}

simulated event BecomeViewTarget(PlayerController PC)
{
    // End:0x25
    if(PhysicsVolume != none)
    {
        PhysicsVolume.NotifyPawnBecameViewTarget(self, PC);
    }
    //return;    
}

event SoakPause()
{
    local PlayerController PC;

    // End:0x36
    foreach WorldInfo.LocalPlayerControllers(Class'PlayerController', PC)
    {
        PC.SoakPause(self);
        // End:0x36
        break;        
    }    
    //return;    
}

// Export UPawn::execClearConstraints(FFrame&, void* const)
native function ClearConstraints();

// Export UPawn::execAddPathConstraint(FFrame&, void* const)
native function AddPathConstraint(PathConstraint Constraint);

// Export UPawn::execAddGoalEvaluator(FFrame&, void* const)
native function AddGoalEvaluator(PathGoalEvaluator Evaluator);

function PathConstraint CreatePathConstraint(class<PathConstraint> ConstraintClass)
{
    return new (self) ConstraintClass;
    //return ReturnValue;    
}

function PathGoalEvaluator CreatePathGoalEvaluator(class<PathGoalEvaluator> GoalEvalClass)
{
    return new (self) GoalEvalClass;
    //return ReturnValue;    
}

// Export UPawn::execIncrementPathStep(FFrame&, void* const)
native function IncrementPathStep(int Cnt, Canvas C);

// Export UPawn::execIncrementPathChild(FFrame&, void* const)
native function IncrementPathChild(int Cnt, Canvas C);

// Export UPawn::execDrawPathStep(FFrame&, void* const)
native function DrawPathStep(Canvas C);

// Export UPawn::execClearPathStep(FFrame&, void* const)
native function ClearPathStep();

simulated function ZeroMovementVariables()
{
    Velocity = vect(0.0000000, 0.0000000, 0.0000000);
    Acceleration = vect(0.0000000, 0.0000000, 0.0000000);
    //return;    
}

simulated function SetCinematicMode(bool bInCinematicMode)
{
    //return;    
}

event PreventedWalkingOverLedge()
{
    //return;    
}

state Dying
{
    // ignores BreathTimer, FellOutOfWorld, PlayWeaponSwitch, PlayNextAnimation, BaseChange, Landed, 
	//     Died;

    ignores BreathTimer, FellOutOfWorld, PlayWeaponSwitch, BaseChange, Landed,
        Died;

    singular simulated event OutsideWorldBounds()
    {
        SetPhysics(0);
        SetHidden(true);
        LifeSpan = FMin(LifeSpan, 1.0000000);
        //return;        
    }

    event Timer()
    {
        // End:0x0E
        if(!PlayerCanSeeMe())
        {
            Destroy();            
        }
        else
        {
            SetTimer(2.0000000, false);
        }
        //return;        
    }

    event TakeDamage(int Damage, Controller EventInstigator, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
    {
        SetPhysics(2);
        // End:0x4B
        if((Physics == 0) && Momentum.Z < float(0))
        {
            Momentum.Z *= float(-1);
        }
        Velocity += ((float(3) * Momentum) / (Mass + float(200)));
        // End:0x80
        if(DamageType == none)
        {
            DamageType = Class'DamageType';
        }
        Damage *= DamageType.default.GibModifier;
        Health -= Damage;
        //return;        
    }

    function SetDyingPhysics()
    {
        // End:0x2F
        if(Physics != 10)
        {
            // End:0x29
            if(Physics == 4)
            {
                SetPhysics(2);                
            }
            else
            {
                SetPhysics(1);
            }
        }
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        local Actor A;
        local array<SequenceEvent> TouchEvents;
        local int I;

        // End:0x33
        if(bTearOff && WorldInfo.NetMode == NM_DedicatedServer)
        {
            LifeSpan = 2.0000000;            
        }
        else
        {
            SetTimer(5.0000000, false);
        }
        SetDyingPhysics();
        SetCollision(true, false);
        // End:0x8E
        if(Controller != none)
        {
            // End:0x7A
            if(Controller.bIsPlayer)
            {
                DetachFromController();                
            }
            else
            {
                Controller.Destroy();
                Controller = none;
            }
        }
        // End:0x106
        foreach TouchingActors(Class'Actor', A)
        {
            // End:0x105
            if(A.FindEventsOfClass(Class'SeqEvent_Touch', TouchEvents))
            {
                I = 0;
                J0xC3:

                // End:0xFD [Loop If]
                if(I < TouchEvents.Length)
                {
                    SeqEvent_Touch(TouchEvents[I]).NotifyTouchingPawnDied(self);
                    I++;
                    // [Loop Continue]
                    goto J0xC3;
                }
                TouchEvents.Length = 0;
            }            
        }        
        // End:0x12C
        foreach BasedActors(Class'Actor', A)
        {
            A.PawnBaseDied();            
        }        
        //return;        
    }
Begin:

    Sleep(0.2000000);
    PlayDyingSound();
    stop;        
}

defaultproperties
{
    MaxStepHeight=35.0000000
    MaxJumpHeight=96.0000000
    WalkableFloorZ=0.7000000
    bCrouchCollisionCheck=true
    bJumpCapable=true
    bCanJump=true
    bCanWalk=true
    bSimulateGravity=true
    bLOSHearing=true
    bCanUse=true
    CrouchHeight=40.0000000
    CrouchRadius=34.0000000
    NonPreferredVehiclePathMultiplier=1.0000000
    DesiredSpeed=1.0000000
    MaxDesiredSpeed=1.0000000
    HearingThreshold=2800.0000000
    SightRadius=5000.0000000
    AvgPhysicsTime=0.1000000
    Mass=100.0000000
    MaxPitchLimit=3072
    MaxPathLength=-1
    GroundSpeed=600.0000000
    WaterSpeed=300.0000000
    AirSpeed=600.0000000
    LadderSpeed=200.0000000
    AccelRate=2048.0000000
    JumpZ=420.0000000
    OutofWaterZ=420.0000000
    MaxOutOfWaterStepHeight=40.0000000
    AirControl=0.0500000
    WalkingPct=0.5000000
    CrouchedPct=0.5000000
    MaxFallSpeed=1200.0000000
    AIMaxFallSpeedFactor=1.0000000
    BaseEyeHeight=64.0000000
    EyeHeight=54.0000000
    Health=100
    noise1time=-10.0000000
    noise2time=-10.0000000
    SoundDampening=1.0000000
    DamageScaling=1.0000000
    ControllerClass=Class'AIController'
    LandMovementState="PlayerWalking"
    WaterMovementState="PlayerSwimming"
    // Reference: CylinderComponent'Default__Pawn.CollisionCylinder'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'CollisionCylinder'
    begin object name="CollisionCylinder" class=Class'CylinderComponent'
        CollisionHeight=78.0000000
        CollisionRadius=34.0000000
        CollideActors=true
        BlockActors=true
    end object
    CylinderComponent=CollisionCylinder
    RBPushRadius=10.0000000
    RBPushStrength=50.0000000
    VehicleCheckRadius=150.0000000
    ViewPitchMin=-16384.0000000
    ViewPitchMax=16383.0000000
    AllowedYawError=2000
    InventoryManagerClass=Class'InventoryManager'
    bUpdateSimulatedPosition=true
    bCanBeDamaged=true
    bShouldBaseAtStartup=true
    bCanTeleport=true
    bCollideActors=true
    bCollideWorld=true
    bBlockActors=true
    bProjTarget=true
    Components[0]=none
    Components[1]=CollisionCylinder
    Components[2]=none
    RemoteRole=ROLE_SimulatedProxy
    CollisionComponent=CollisionCylinder
    RotationRate=(Pitch=20000,Yaw=20000,Roll=20000)
}
