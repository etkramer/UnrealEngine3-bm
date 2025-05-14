class Controller extends Actor
    abstract
    native
    nativereplication
    notplaceable;

const LATENT_MOVETOWARD = 503;

struct native BasedPosition
{
    var() Actor Base;
    var() Vector Position;
    var Vector CachedBaseLocation;
    var Rotator CachedBaseRotation;
    var Vector CachedTransPosition;

    structcpptext
	{
		FBasedPosition()
		{
			Base = NULL;
			Position = FVector(0,0,0);
		}
		explicit FBasedPosition( class AActor *InBase, FVector& InPosition )
		{
			Set( InBase, InPosition );
		}
		// Retrieve world location of this position
		FORCEINLINE FVector operator*()
		{
			if( Base != NULL )
			{
				// If base hasn't changed location/rotation use cached transformed position
				if( CachedBaseLocation != Base->Location ||
					CachedBaseRotation != Base->Rotation )
				{
					CachedBaseLocation	= Base->Location;
					CachedBaseRotation	= Base->Rotation;
					CachedTransPosition = Base->Location + FRotationMatrix(Base->Rotation).TransformFVector(Position);
				}

				return CachedTransPosition;
			}
			return Position;
		}

		void Set( class AActor* InBase, FVector& InPosition )
		{
			if( InPosition.IsNearlyZero() )
			{
				Base = NULL;
				Position = FVector(0,0,0);
				return;
			}

			Base = (InBase && !InBase->bStatic) ? InBase : NULL;
			if( Base != NULL )
			{
				Position = FRotationMatrix(Base->Rotation).InverseTransformFVectorNoScale( InPosition - Base->Location );

				CachedBaseLocation	= Base->Location;
				CachedBaseRotation	= Base->Rotation;
				CachedTransPosition = InPosition;
			}
			else
			{
				Position = InPosition;
			}
		}
	}

    structdefaultproperties
    {
        Base=none
        Position=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        CachedBaseLocation=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        CachedBaseRotation=(Pitch=0,Yaw=0,Roll=0)
        CachedTransPosition=(X=0.0000000,Y=0.0000000,Z=0.0000000)
    }
};

struct native VisiblePortalInfo
{
    var Actor Source;
    var Actor Destination;

	structcpptext
	{
		FVisiblePortalInfo()
		{}
		FVisiblePortalInfo(EEventParm)
		{
			appMemzero(this, sizeof(FVisiblePortalInfo));
		}
		FVisiblePortalInfo(AActor* InSource, AActor* InDest)
		: Source(InSource), Destination(InDest)
		{}

		UBOOL operator==(const FVisiblePortalInfo& Other)
		{
			return Other.Source == Source && Other.Destination == Destination;
		}
	}

    structdefaultproperties
    {
        Source=none
        Destination=none
    }
};

var Pawn Pawn;
var repnotify PlayerReplicationInfo PlayerReplicationInfo;
var const int PlayerNum;
var private const Controller NextController;
var bool bIsPlayer;
var bool bGodMode;
var bool bAffectedByHitEffects;
var bool bSoaking;
var bool bSlowerZAcquire;
var bool bForceStrafe;
var bool bNotifyPostLanded;
var bool bNotifyApex;
var bool bAdvancedTactics;
var bool bCanDoSpecial;
var bool bAdjusting;
var bool bPreparingMove;
var const bool bLOSflag;
var bool bUsePlayerHearing;
var bool bNotifyFallingHitWall;
var bool bForceDesiredRotation;
var bool bPreciseDestination;
var bool bSeeFriendly;
var bool bUsingPathLanes;
var input byte bFire;
var float MinHitWall;
var float MoveTimer;
var Actor MoveTarget;
var BasedPosition DestinationPosition;
var BasedPosition FocalPosition;
var Actor Focus;
var Actor GoalList[4];
var BasedPosition AdjustPosition;
var NavigationPoint StartSpot;
var array<NavigationPoint> RouteCache;
var ReachSpec CurrentPath;
var ReachSpec NextRoutePath;
var Vector CurrentPathDir;
var Actor RouteGoal;
var float RouteDist;
var float LastRouteFind;
var InterpActor PendingMover;
var Actor FailedMoveTarget;
var int MoveFailureCount;
var float GroundPitchTime;
var Vector ViewX;
var Vector ViewY;
var Vector ViewZ;
var Pawn ShotTarget;
var const Actor LastFailedReach;
var const float FailedReachTime;
var const Vector FailedReachLocation;
var float SightCounter;
var float SightCounterInterval;
var float RespawnPredictionTime;
var float InUseNodeCostMultiplier;
var int HighJumpNodeCostModifier;
var Pawn Enemy;
var deprecated Actor Target;
var array<VisiblePortalInfo> VisiblePortals;
var float LaneOffset;
var const Rotator OldBasedRotation;

cpptext
{
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );
	virtual void Spawned();

	virtual UBOOL IsPlayerOwned()
	{
	return IsPlayerOwner();
	}

	virtual UBOOL IsPlayerOwner()
	{
		return bIsPlayer;
	}
	virtual AController* GetAController() { return this; }

	// Seeing and hearing checks
	virtual UBOOL CanHear(const FVector& NoiseLoc, FLOAT Loudness, AActor *Other);
	virtual void ShowSelf();
	virtual UBOOL ShouldCheckVisibilityOf(AController* C);
	virtual DWORD SeePawn(APawn *Other, UBOOL bMaySkipChecks = TRUE);
	virtual DWORD LineOfSightTo(const AActor* Other, INT bUseLOSFlag=0, const FVector* chkLocation = NULL, UBOOL bTryAlternateTargetLoc = FALSE);
	void CheckEnemyVisible();
	virtual void HearNoise(AActor* NoiseMaker, FLOAT Loudness, FName NoiseType);

	AActor* HandleSpecial(AActor *bestPath);
	virtual INT AcceptNearbyPath(AActor* goal);
	virtual UReachSpec* PrepareForMove( ANavigationPoint *NavGoal, UReachSpec* Path );
	UReachSpec* GetNextRoutePath( ANavigationPoint *NavGoal );
	virtual void AdjustFromWall(FVector HitNormal, AActor* HitActor);
	void SetRouteCache( ANavigationPoint *EndPath, FLOAT StartDist, FLOAT EndDist );
	AActor* FindPath(const FVector& Point, AActor* Goal, UBOOL bWeightDetours, INT MaxPathLength, UBOOL bReturnPartial);
	/** given the passed in goal for pathfinding, set bTransientEndPoint on all NavigationPoints that are acceptable
	 * destinations on the path network
	 * @param EndAnchor the Anchor for the goal on the navigation network
	 * @param Goal the goal actor we're pathfinding toward (may be NULL)
	 * @param GoalLocation the goal world location we're pathfinding toward
	 */
	virtual void MarkEndPoints(ANavigationPoint* EndAnchor, AActor* Goal, const FVector& GoalLocation);
	/** gives the Controller a chance to pre-empt pathfinding with its own result (if a cached path is still valid, for example)
	 * called just before navigation network traversal, after Anchor determination and NavigationPoint transient properties are set up
	 * only called when using the 'FindEndPoint' node evaluator
	 * @param EndAnchor - Anchor for Goal on the path network
	 * @param Goal - Destination Actor we're trying to path to (may be NULL)
	 * @param GoalLocation - the goal world location we're pathfinding toward
	 * @param bWeightDetours - whether we should consider short detours for pickups and such
	 * @param BestWeight - weighting value for best node on path - if this function returns true, findPathToward() will return this value
	 * @return whether the normal pathfinding should be skipped
	 */
	virtual UBOOL OverridePathTo(ANavigationPoint* EndAnchor, AActor* Goal, const FVector& GoalLocation, UBOOL bWeightDetours, FLOAT& BestWeight)
	{
		return FALSE;
	}
	AActor* SetPath(INT bInitialPath=1);
	virtual UBOOL LocalPlayerController();
	virtual UBOOL WantsLedgeCheck();
	virtual UBOOL StopAtLedge();
	virtual void PrePollMove()
	{}
	virtual void PostPollMove()
	{}
	virtual AActor* GetViewTarget();
	virtual void UpdateEnemyInfo(APawn* AcquiredEnemy) {};
	virtual void JumpOverWall(FVector WallNormal);
	virtual void UpdatePawnRotation();
	virtual UBOOL ForceReached(ANavigationPoint *Nav, const FVector& TestPosition);
	virtual FRotator SetRotationRate(FLOAT deltaTime);
	virtual FVector DesiredDirection();
	/** activates path lanes for this Controller's current movement and adjusts its destination accordingly
	 * @param DesiredLaneOffset the offset from the center of the Controller's CurrentPath that is desired
	 * 				the Controller sets its LaneOffset as close as it can get to it without
	 *				allowing any part of the Pawn's cylinder outside of the CurrentPath
	 */
	void SetPathLane(FLOAT InPathOffset);
	virtual void FailMove();

	// falling physics AI hooks
	virtual void PreAirSteering(FLOAT DeltaTime) {};
	virtual void PostAirSteering(FLOAT DeltaTime) {};
	virtual void PostPhysFalling(FLOAT DeltaTime) {};
	virtual void PostPhysWalking(FLOAT DeltaTime) {};
	virtual void PostPhysSpider(FLOAT DeltaTime) {};
	virtual UBOOL AirControlFromWall(float DeltaTime, FVector& RealAcceleration) { return FALSE; };
	virtual void NotifyJumpApex();

	virtual void PostBeginPlay();
	virtual void PostScriptDestroyed();

	virtual void ClearCrossLevelPaths(ULevel *Level);

	// Natives.
	DECLARE_FUNCTION(execPollWaitForLanding);
	DECLARE_FUNCTION(execPollMoveTo);
	virtual DECLARE_FUNCTION(execPollMoveToward);
	DECLARE_FUNCTION(execPollFinishRotation);

	virtual UBOOL ShouldOffsetCorners() { return TRUE; }
	virtual UBOOL ShouldUsePathLanes() { return TRUE; }
	virtual UBOOL ShouldIgnoreNavigationBlockingFor(const AActor* Other){ return !Other->bBlocksNavigation; }
}

// Export UController::execIsLocalPlayerController(FFrame&, void* const)
native function bool IsLocalPlayerController();

// Export UController::execRouteCache_Empty(FFrame&, void* const)
native function RouteCache_Empty();

// Export UController::execRouteCache_AddItem(FFrame&, void* const)
native function RouteCache_AddItem(NavigationPoint Nav);

// Export UController::execRouteCache_InsertItem(FFrame&, void* const)
native function RouteCache_InsertItem(NavigationPoint Nav, optional int Idx = 0);

// Export UController::execRouteCache_RemoveItem(FFrame&, void* const)
native function RouteCache_RemoveItem(NavigationPoint Nav);

// Export UController::execRouteCache_RemoveIndex(FFrame&, void* const)
native function RouteCache_RemoveIndex(int InIndex, optional int Count = 1);

// Export UController::execSetBasedPosition(FFrame&, void* const)
native static final function SetBasedPosition(out BasedPosition BP, Vector pos, optional Actor ForcedBase);

// Export UController::execGetBasedPosition(FFrame&, void* const)
native static final function Vector GetBasedPosition(BasedPosition BP);

// Export UController::execSetFocalPoint(FFrame&, void* const)
native final function SetFocalPoint(Vector FP, optional bool bOffsetFromBase);

// Export UController::execGetFocalPoint(FFrame&, void* const)
native final function Vector GetFocalPoint();

// Export UController::execSetDestinationPosition(FFrame&, void* const)
native final function SetDestinationPosition(Vector Dest, optional bool bOffsetFromBase);

// Export UController::execGetDestinationPosition(FFrame&, void* const)
native final function Vector GetDestinationPosition();

// Export UController::execSetAdjustLocation(FFrame&, void* const)
native final function SetAdjustLocation(Vector NewLoc, bool bAdjust, optional bool bOffsetFromBase);

// Export UController::execGetAdjustLocation(FFrame&, void* const)
native final function Vector GetAdjustLocation();

event PostBeginPlay()
{
    super.PostBeginPlay();
    // End:0x42
    if((!bDeleteMe && bIsPlayer) && WorldInfo.NetMode != NM_Client)
    {
        InitPlayerReplicationInfo();
    }
    SightCounter = SightCounterInterval * FRand();
    //return;    
}

function Reset()
{
    super.Reset();
    Enemy = none;
    StartSpot = none;
    bAdjusting = false;
    bPreparingMove = false;
    MoveTimer = -1.0000000;
    MoveTarget = none;
    CurrentPath = none;
    RouteGoal = none;
    //return;    
}

reliable client simulated function ClientSetLocation(Vector NewLocation, Rotator NewRotation)
{
    SetRotation(NewRotation);
    // End:0xFE
    if(Pawn != none)
    {
        // End:0xC8
        if((Rotation.Pitch > Pawn.MaxPitchLimit) && Rotation.Pitch < (65536 - Pawn.MaxPitchLimit))
        {
            // End:0xA1
            if(Rotation.Pitch < 32768)
            {
                NewRotation.Pitch = Pawn.MaxPitchLimit;                
            }
            else
            {
                NewRotation.Pitch = 65536 - Pawn.MaxPitchLimit;
            }
        }
        NewRotation.Roll = 0;
        Pawn.SetRotation(NewRotation);
        Pawn.SetLocation(NewLocation);
    }
    //return;    
}

reliable client simulated function ClientSetRotation(Rotator NewRotation, optional bool bResetCamera)
{
    SetRotation(NewRotation);
    // End:0x4A
    if(Pawn != none)
    {
        NewRotation.Pitch = 0;
        NewRotation.Roll = 0;
        Pawn.SetRotation(NewRotation);
    }
    //return;    
}

simulated event ReplicatedEvent(name VarName)
{
    // End:0x36
    if(VarName == 'PlayerReplicationInfo')
    {
        // End:0x33
        if(PlayerReplicationInfo != none)
        {
            PlayerReplicationInfo.ClientInitialize(self);
        }        
    }
    else
    {
        super.ReplicatedEvent(VarName);
    }
    //return;    
}

function OnPossess(SeqAct_Possess inAction)
{
    local Pawn OldPawn;
    local Vehicle V;

    V = Vehicle(Pawn);
    // End:0xB6
    if(inAction.PawnToPossess != none)
    {
        V = Vehicle(inAction.PawnToPossess);
        // End:0x5A
        if((Pawn != none) && V != none)
        {            
        }
        else
        {
            OldPawn = Pawn;
            UnPossess();
            Possess(inAction.PawnToPossess, false);
            // End:0xB6
            if(inAction.bKillOldPawn && OldPawn != none)
            {
                OldPawn.Destroy();
            }
        }
    }
    //return;    
}

event Possess(Pawn inPawn, bool bVehicleTransition)
{
    // End:0x33
    if(inPawn.Controller != none)
    {
        inPawn.Controller.UnPossess();
    }
    inPawn.PossessedBy(self, bVehicleTransition);
    Pawn = inPawn;
    // End:0x6E
    if(PlayerReplicationInfo != none)
    {
        UpdateSex();
    }
    SetFocalPoint(Pawn.Location + (float(512) * Vector(Pawn.Rotation)), true);
    Restart(bVehicleTransition);
    // End:0xD0
    if(Pawn.Weapon == none)
    {
        ClientSwitchToBestWeapon();
    }
    //return;    
}

function UpdateSex()
{
    //return;    
}

event UnPossess()
{
    LogInternalAI((("POSSESSLOG - " $ string(self)) $ " leaving pawn ") $ string(Pawn));
    // End:0x5C
    if(Pawn != none)
    {
        Pawn.UnPossessed();
        Pawn = none;
    }
    //return;    
}

function PawnDied(Pawn inPawn)
{
    local int Idx;

    // End:0x11
    if(inPawn != Pawn)
    {
        return;
    }
    LogInternalAI((("POSSESSLOG - " $ string(self)) $ " death of pawn ") $ string(Pawn));
    TriggerEventClass(Class'SeqEvent_Death', self);
    Idx = 0;
    J0x62:

    // End:0xA8 [Loop If]
    if(Idx < LatentActions.Length)
    {
        // End:0x9E
        if(LatentActions[Idx] != none)
        {
            LatentActions[Idx].AbortFor(self);
        }
        Idx++;
        // [Loop Continue]
        goto J0x62;
    }
    LatentActions.Length = 0;
    // End:0xE1
    if(Pawn != none)
    {
        SetLocation(Pawn.Location);
        Pawn.UnPossessed();
    }
    // End:0x10A
    if(bIsPlayer)
    {
        // End:0x107
        if(!GamePlayEndedState())
        {
            GotoState('Dead');
        }        
    }
    else
    {
        Destroy();
    }
    Pawn = none;
    //return;    
}

function bool GamePlayEndedState()
{
    return false;
    //return ReturnValue;    
}

event NotifyPostLanded()
{
    //return;    
}

event Destroyed()
{
    // End:0xAF
    if(Role == ROLE_Authority)
    {
        // End:0x4F
        if(bIsPlayer && WorldInfo.Game != none)
        {
            WorldInfo.Game.Logout(self);
        }
        // End:0xAF
        if(PlayerReplicationInfo != none)
        {
            // End:0xA5
            if(!PlayerReplicationInfo.bOnlySpectator && PlayerReplicationInfo.Team != none)
            {
                PlayerReplicationInfo.Team.RemoveFromTeam(self);
            }
            CleanupPRI();
        }
    }
    super.Destroyed();
    //return;    
}

function CleanupPRI()
{
    PlayerReplicationInfo.Destroy();
    PlayerReplicationInfo = none;
    //return;    
}

function Restart(bool bVehicleTransition)
{
    Pawn.Restart();
    // End:0x26
    if(!bVehicleTransition)
    {
        Enemy = none;
    }
    // End:0x67
    if((bVehicleTransition == false) && Pawn.InvManager != none)
    {
        Pawn.InvManager.UpdateController();
    }
    //return;    
}

// Export UController::execBeyondFogDistance(FFrame&, void* const)
native final function bool BeyondFogDistance(Vector ViewPoint, Vector OtherPoint);

function EnemyJustTeleported()
{
    LineOfSightTo(Enemy);
    //return;    
}

function NotifyTakeHit(Controller InstigatedBy, Vector HitLocation, int Damage, class<DamageType> DamageType, Vector Momentum)
{
    //return;    
}

function InitPlayerReplicationInfo()
{
    PlayerReplicationInfo = Spawn(WorldInfo.Game.PlayerReplicationInfoClass, self,, vect(0.0000000, 0.0000000, 0.0000000), rot(0, 0, 0));
    // End:0x7C
    if(PlayerReplicationInfo.PlayerName == "")
    {
        PlayerReplicationInfo.SetPlayerName(Class'GameInfo'.default.DefaultPlayerName);
    }
    //return;    
}

// Export UController::execGetTeamNum(FFrame&, void* const)
native simulated function byte GetTeamNum();

reliable server function ServerRestartPlayer()
{
    // End:0x31
    if((WorldInfo.NetMode != NM_Client) && Pawn != none)
    {
        ServerGivePawn();
    }
    //return;    
}

function ServerGivePawn()
{
    //return;    
}

function SetCharacter(string inCharacter)
{
    //return;    
}

function GameHasEnded(optional Actor EndGameFocus, optional bool bIsWinner)
{
    GotoState('RoundEnded');
    //return;    
}

function NotifyKilled(Controller Killer, Controller Killed, Pawn KilledPawn)
{
    // End:0x2C
    if(Pawn != none)
    {
        Pawn.TriggerEventClass(Class'SeqEvent_SeeDeath', KilledPawn);
    }
    // End:0x42
    if(Enemy == KilledPawn)
    {
        Enemy = none;
    }
    //return;    
}

function NotifyProjLanded(Projectile Proj)
{
    // End:0x39
    if((Proj != none) && Pawn != none)
    {
        Pawn.TriggerEventClass(Class'SeqEvent_ProjectileLanded', Proj);
    }
    //return;    
}

function WarnProjExplode(Projectile Proj)
{
    //return;    
}

event float RatePickup(Actor PickupHolder, class<Inventory> inPickup)
{
    //return ReturnValue;    
}

function bool FireWeaponAt(Actor inActor)
{
    //return ReturnValue;    
}

event StopFiring()
{
    bFire = 0;
    // End:0x27
    if(Pawn != none)
    {
        Pawn.StopFiring();
    }
    //return;    
}

function RoundHasEnded(optional Actor EndRoundFocus)
{
    GotoState('RoundEnded');
    //return;    
}

function HandlePickup(Inventory Inv)
{
    //return;    
}

function Rotator GetAdjustedAimFor(Weapon W, Vector StartFireLoc)
{
    // End:0x20
    if(Pawn != none)
    {
        return Pawn.GetBaseAimRotation();
    }
    return Rotation;
    //return ReturnValue;    
}

function InstantWarnTarget(Actor InTarget, Weapon FiredWeapon, Vector FireDir)
{
    local Pawn P;

    P = Pawn(InTarget);
    // End:0x5F
    if((P != none) && P.Controller != none)
    {
        P.Controller.ReceiveWarning(Pawn, -1.0000000, FireDir);
    }
    //return;    
}

function ReceiveWarning(Pawn shooter, float projSpeed, Vector FireDir)
{
    //return;    
}

function ReceiveProjectileWarning(Projectile Proj)
{
    //return;    
}

exec function SwitchToBestWeapon(optional bool bForceNewWeapon)
{
    // End:0x25
    if((Pawn == none) || Pawn.InvManager == none)
    {
        return;
    }
    Pawn.InvManager.SwitchToBestWeapon(bForceNewWeapon);
    //return;    
}

reliable client simulated function ClientSwitchToBestWeapon(optional bool bForceNewWeapon)
{
    SwitchToBestWeapon(bForceNewWeapon);
    //return;    
}

reliable client simulated function ClientSetWeapon(class<Weapon> WeaponClass)
{
    local Inventory Inv;

    // End:0x0D
    if(Pawn == none)
    {
        return;
    }
    Inv = Pawn.FindInventoryType(WeaponClass);
    // End:0x57
    if(Weapon(Inv) != none)
    {
        Pawn.SetActiveWeapon(Weapon(Inv));
    }
    //return;    
}

function NotifyChangedWeapon(Weapon PrevWeapon, Weapon NewWeapon)
{
    //return;    
}

// Export UController::execLineOfSightTo(FFrame&, void* const)
native(514) noexport final function bool LineOfSightTo(Actor Other, optional Vector chkLocation, optional bool bTryAlternateTargetLoc);

// Export UController::execCanSee(FFrame&, void* const)
native(533) final function bool CanSee(Pawn Other);

// Export UController::execCanSeeByPoints(FFrame&, void* const)
native(537) final function bool CanSeeByPoints(Vector ViewLocation, Vector TestLocation, Rotator ViewRotation);

// Export UController::execPickTarget(FFrame&, void* const)
native(531) final function Pawn PickTarget(class<Pawn> TargetClass, out float bestAim, out float bestDist, Vector FireDir, Vector projStart, float MaxRange);

event HearNoise(float Loudness, Actor NoiseMaker, optional name NoiseType)
{
    //return;    
}

event SeePlayer(Pawn Seen)
{
    //return;    
}

event SeeMonster(Pawn Seen)
{
    //return;    
}

event EnemyNotVisible()
{
    //return;    
}

// Export UController::execMoveTo(FFrame&, void* const)
native(500) noexport final latent function MoveTo(Vector NewDestination, optional Actor ViewFocus, optional bool bShouldWalk = ((Pawn != none) ? Pawn.bIsWalking : false));

// Export UController::execMoveToward(FFrame&, void* const)
native(502) noexport final latent function MoveToward(Actor NewTarget, optional Actor ViewFocus, optional float DestinationOffset, optional bool bUseStrafing, optional bool bShouldWalk = ((Pawn != none) ? Pawn.bIsWalking : false));

event SetupSpecialPathAbilities()
{
    //return;    
}

// Export UController::execFinishRotation(FFrame&, void* const)
native(508) final latent function FinishRotation();

// Export UController::execFindPathTo(FFrame&, void* const)
native(518) final function Actor FindPathTo(Vector aPoint, optional int MaxPathLength, optional bool bReturnPartial);

// Export UController::execFindPathToward(FFrame&, void* const)
native(517) final function Actor FindPathToward(Actor anActor, optional bool bWeightDetours, optional int MaxPathLength, optional bool bReturnPartial);

// Export UController::execFindPathTowardNearest(FFrame&, void* const)
native final function Actor FindPathTowardNearest(class<NavigationPoint> GoalClass, optional bool bWeightDetours, optional int MaxPathLength, optional bool bReturnPartial);

// Export UController::execFindRandomDest(FFrame&, void* const)
native(525) final function NavigationPoint FindRandomDest();

// Export UController::execFindPathToIntercept(FFrame&, void* const)
native final function Actor FindPathToIntercept(Pawn P, Actor InRouteGoal, optional bool bWeightDetours, optional int MaxPathLength, optional bool bReturnPartial);

// Export UController::execPointReachable(FFrame&, void* const)
native(521) final function bool PointReachable(Vector aPoint);

// Export UController::execActorReachable(FFrame&, void* const)
native(520) final function bool ActorReachable(Actor anActor);

event MoveUnreachable(Vector AttemptedDest, Actor AttemptedTarget)
{
    //return;    
}

// Export UController::execPickWallAdjust(FFrame&, void* const)
native(526) final function bool PickWallAdjust(Vector HitNormal);

// Export UController::execWaitForLanding(FFrame&, void* const)
native(527) noexport final latent function WaitForLanding(optional float waitDuration);

event LongFall()
{
    //return;    
}

// Export UController::execEndClimbLadder(FFrame&, void* const)
native function EndClimbLadder();

event MayFall()
{
    //return;    
}

event bool AllowDetourTo(NavigationPoint N)
{
    return true;
    //return ReturnValue;    
}

function WaitForMover(InterpActor M)
{
    PendingMover = M;
    M.bMonitorMover = true;
    bPreparingMove = true;
    Pawn.Acceleration = vect(0.0000000, 0.0000000, 0.0000000);
    //return;    
}

event bool MoverFinished()
{
    // End:0x5B
    if(((Pawn == none) || PendingMover.MyMarker == none) || PendingMover.MyMarker.ProceedWithMove(Pawn))
    {
        PendingMover = none;
        bPreparingMove = false;
        return true;
    }
    return false;
    //return ReturnValue;    
}

function UnderLift(LiftCenter Lift)
{
    //return;    
}

event bool HandlePathObstruction(Actor BlockedBy)
{
    //return ReturnValue;    
}

simulated event GetPlayerViewPoint(out Vector out_Location, out Rotator out_Rotation)
{
    out_Location = Location;
    out_Rotation = Rotation;
    //return;    
}

simulated event GetActorEyesViewPoint(out Vector out_Location, out Rotator out_Rotation)
{
    // End:0x2C
    if(Pawn != none)
    {
        Pawn.GetActorEyesViewPoint(out_Location, out_Rotation);        
    }
    else
    {
        out_Location = Location;
        out_Rotation = Rotation;
    }
    //return;    
}

simulated function bool IsAimingAt(Actor ATarget, float Epsilon)
{
    local Vector Loc;
    local Rotator Rot;

    GetPlayerViewPoint(Loc, Rot);
    return (Normal(ATarget.Location - Loc) Dot Vector(Rot)) >= Epsilon;
    //return ReturnValue;    
}

simulated function bool LandingShake()
{
    return false;
    //return ReturnValue;    
}

event NotifyPhysicsVolumeChange(PhysicsVolume NewVolume)
{
    //return;    
}

event bool NotifyHeadVolumeChange(PhysicsVolume NewVolume)
{
    //return ReturnValue;    
}

event bool NotifyLanded(Vector HitNormal, Actor FloorActor)
{
    //return ReturnValue;    
}

event bool NotifyHitWall(Vector HitNormal, Actor Wall)
{
    //return ReturnValue;    
}

event NotifyFallingHitWall(Vector HitNormal, Actor Wall)
{
    //return;    
}

event bool NotifyBump(Actor Other, Vector HitNormal)
{
    //return ReturnValue;    
}

event NotifyJumpApex()
{
    //return;    
}

event NotifyMissedJump()
{
    //return;    
}

// Export UController::execInLatentExecution(FFrame&, void* const)
native final function bool InLatentExecution(int LatentActionNumber);

// Export UController::execStopLatentExecution(FFrame&, void* const)
native final function StopLatentExecution();

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
    local Canvas Canvas;

    Canvas = HUD.Canvas;
    // End:0xBB
    if(Pawn == none)
    {
        // End:0x5B
        if(PlayerReplicationInfo == none)
        {
            Canvas.DrawText("NO PLAYERREPLICATIONINFO", false);            
        }
        else
        {
            PlayerReplicationInfo.DisplayDebug(HUD, out_YL, out_YPos);
        }
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        super.DisplayDebug(HUD, out_YL, out_YPos);
        return;
    }
    Canvas.SetDrawColor(255, 0, 0);
    Canvas.DrawText((("CONTROLLER " $ (GetItemName(string(self)))) $ " Pawn ") $ (GetItemName(string(Pawn))));
    out_YPos += out_YL;
    Canvas.SetPos(4.0000000, out_YPos);
    Canvas.DrawText(" bPreciseDestination:" @ string(bPreciseDestination));
    out_YPos += out_YL;
    Canvas.SetPos(4.0000000, out_YPos);
    // End:0x271
    if(HUD.ShouldDisplayDebug('AI'))
    {
        // End:0x215
        if(Enemy != none)
        {
            Canvas.DrawText((("     STATE: " $ string(GetStateName())) $ " Enemy ") $ Enemy.GetHumanReadableName(), false);            
        }
        else
        {
            Canvas.DrawText(("     STATE: " $ string(GetStateName())) $ " NO Enemy ", false);
        }
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
    }
    //return;    
}

simulated function string GetHumanReadableName()
{
    // End:0x1E
    if(PlayerReplicationInfo != none)
    {
        return PlayerReplicationInfo.PlayerName;        
    }
    else
    {
        return GetItemName(string(self));
    }
    //return ReturnValue;    
}

function bool IsDead()
{
    //return ReturnValue;    
}

simulated function OnMakeNoise(SeqAct_MakeNoise Action)
{
    // End:0x30
    if(Pawn != none)
    {
        Pawn.MakeNoise(Action.Loudness, 'ScriptNoise');
    }
    //return;    
}

simulated function OnTeleport(SeqAct_Teleport Action)
{
    // End:0x3D
    if(Action != none)
    {
        // End:0x32
        if(Pawn != none)
        {
            Pawn.OnTeleport(Action);            
        }
        else
        {
            super.OnTeleport(Action);
        }
    }
    //return;    
}

function OnToggleGodMode(SeqAct_ToggleGodMode inAction)
{
    // End:0x2B
    if(inAction.InputLinks[0].bHasImpulse)
    {
        bGodMode = true;        
    }
    else
    {
        // End:0x56
        if(inAction.InputLinks[1].bHasImpulse)
        {
            bGodMode = false;            
        }
        else
        {
            bGodMode = !bGodMode;
        }
    }
    //return;    
}

function OnToggleAffectedByHitEffects(SeqAct_ToggleAffectedByHitEffects inAction)
{
    // End:0x2B
    if(inAction.InputLinks[0].bHasImpulse)
    {
        bAffectedByHitEffects = true;        
    }
    else
    {
        // End:0x56
        if(inAction.InputLinks[1].bHasImpulse)
        {
            bAffectedByHitEffects = false;            
        }
        else
        {
            bAffectedByHitEffects = !bAffectedByHitEffects;
        }
    }
    //return;    
}

simulated function NotifyCoverDisabled(CoverLink Link, int SlotIdx, optional bool bAdjacentIdx)
{
    //return;    
}

simulated event NotifyCoverAdjusted()
{
    //return;    
}

simulated function bool NotifyCoverClaimViolation(Controller NewClaim, CoverLink Link, int SlotIdx)
{
    //return ReturnValue;    
}

simulated function OnCauseDamage(SeqAct_CauseDamage Action)
{
    // End:0x24
    if(Pawn != none)
    {
        Pawn.OnCauseDamage(Action);
    }
    //return;    
}

function NotifyAddInventory(Inventory NewItem)
{
    //return;    
}

simulated function OnToggleHidden(SeqAct_ToggleHidden Action)
{
    // End:0x24
    if(Pawn != none)
    {
        Pawn.OnToggleHidden(Action);
    }
    //return;    
}

function Controller GetKillerController()
{
    return self;
    //return ReturnValue;    
}

event bool IsSpectating()
{
    return false;
    //return ReturnValue;    
}

event bool IsInCombat()
{
    //return ReturnValue;    
}

function Actor GetRouteGoalAfter(int RouteIdx)
{
    // End:0x22
    if((RouteIdx + 1) < RouteCache.Length)
    {
        return RouteCache[RouteIdx + 1];
    }
    return RouteGoal;
    //return ReturnValue;    
}

event CurrentLevelUnloaded()
{
    //return;    
}

event RemoveFromAIManager()
{
    //return;    
}

function SendMessage(PlayerReplicationInfo Recipient, name MessageType, float Wait, optional class<DamageType> DamageType)
{
    //return;    
}

function ReadyForLift()
{
    //return;    
}

state Dead
{
    ignores KilledBy;

    function bool IsDead()
    {
        return true;
        //return ReturnValue;        
    }

    function PawnDied(Pawn P)
    {
        // End:0x36
        if(WorldInfo.NetMode != NM_Client)
        {
            WarnInternal(string(self) @ "Pawndied while dead");
        }
        //return;        
    }

    reliable server function ServerRestartPlayer()
    {
        // End:0x1C
        if(WorldInfo.NetMode == NM_Client)
        {
            return;
        }
        // End:0x31
        if(Pawn != none)
        {
            UnPossess();
        }
        WorldInfo.Game.RestartPlayer(self);
        //return;        
    }
    stop;    
}

state RoundEnded
{
    ignores KilledBy, TakeDamage, ReceiveWarning;

    function bool GamePlayEndedState()
    {
        return true;
        //return ReturnValue;        
    }

    event BeginState(name PreviousStateName)
    {
        // End:0x4F
        if(Pawn != none)
        {
            Pawn.TurnOff();
            StopFiring();
            // End:0x4F
            if(!bIsPlayer)
            {
                Pawn.UnPossessed();
                Pawn = none;
            }
        }
        // End:0x5D
        if(!bIsPlayer)
        {
            Destroy();
        }
        //return;        
    }
    stop;    
}

defaultproperties
{
    bAffectedByHitEffects=true
    bSlowerZAcquire=true
    MinHitWall=-1.0000000
    SightCounterInterval=0.2000000
    bHidden=true
    bOnlyRelevantToOwner=true
    bHiddenEd=true
    Components[0]=none
    RotationRate=(Pitch=30000,Yaw=30000,Roll=2048)
}
