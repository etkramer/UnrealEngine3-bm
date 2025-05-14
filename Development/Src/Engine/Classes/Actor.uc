class Actor extends Object
    abstract
    native
    nativereplication
    DependsOn(AnimNode);

const TRACEFLAG_Bullet = 1;
const TRACEFLAG_PhysicsVolumes = 2;
const TRACEFLAG_SkipMovers = 4;
const TRACEFLAG_Blocking = 8;
const TRACEFLAG_CheckZeroExtent = 16;
const TRACEFLAG_IgnorePawns = 32;
const TRACEFLAG_Destructibles = 64;
const REP_RBLOCATION_ERROR_TOLERANCE_SQ = 16.0f;
const MINFLOORZ = 0.7;
const ACTORMAXSTEPHEIGHT = 35.0;
const RBSTATE_LINVELSCALE = 10.0;
const RBSTATE_ANGVELSCALE = 1000.0;
const RB_None = 0x00;
const RB_NeedsUpdate = 0x01;
const RB_Sleeping = 0x02;

enum EPhysics
{
    PHYS_None,                      // 0
    PHYS_Walking,                   // 1
    PHYS_Falling,                   // 2
    PHYS_Swimming,                  // 3
    PHYS_Flying,                    // 4
    PHYS_Rotating,                  // 5
    PHYS_Projectile,                // 6
    PHYS_Interpolating,             // 7
    PHYS_Spider,                    // 8
    PHYS_Ladder,                    // 9
    PHYS_RigidBody,                 // 10
    PHYS_SoftBody,                  // 11
    PHYS_Floating,                  // 12
    PHYS_Unused,                    // 13
    PHYS_MAX                        // 14
};

enum EMoveDir
{
    MD_Stationary,                  // 0
    MD_Forward,                     // 1
    MD_Backward,                    // 2
    MD_Left,                        // 3
    MD_Right,                       // 4
    MD_Up,                          // 5
    MD_Down,                        // 6
    MD_MAX                          // 7
};

enum ENetRole
{
    ROLE_None,                      // 0
    ROLE_SimulatedProxy,            // 1
    ROLE_AutonomousProxy,           // 2
    ROLE_Authority,                 // 3
    ROLE_MAX                        // 4
};

enum ECollisionType
{
    COLLIDE_CustomDefault,          // 0
    COLLIDE_NoCollision,            // 1
    COLLIDE_BlockAll,               // 2
    COLLIDE_BlockWeapons,           // 3
    COLLIDE_TouchAll,               // 4
    COLLIDE_TouchWeapons,           // 5
    COLLIDE_BlockAllButWeapons,     // 6
    COLLIDE_TouchAllButWeapons,     // 7
    COLLIDE_BlockWeaponsKickable,   // 8
    COLLIDE_MAX                     // 9
};

enum ETravelType
{
    TRAVEL_Absolute,                // 0
    TRAVEL_Partial,                 // 1
    TRAVEL_Relative,                // 2
    TRAVEL_MAX                      // 3
};

enum EDoubleClickDir
{
    DCLICK_None,                    // 0
    DCLICK_Left,                    // 1
    DCLICK_Right,                   // 2
    DCLICK_Forward,                 // 3
    DCLICK_Back,                    // 4
    DCLICK_Active,                  // 5
    DCLICK_Done,                    // 6
    DCLICK_MAX                      // 7
};

struct native Thought
{
    var string Text;
    var byte Red;
    var byte Green;
    var byte Blue;
    var byte Alpha;

    structdefaultproperties
    {
        Text="Empty Thought"
        Red=255
        Green=255
        Blue=255
        Alpha=255
    }
};

struct native TimerData
{
    var bool bLoop;
    var name FuncName;
    var float Rate;
    var float Count;
    var Object TimerObj;
    var bool bPaused;

    structdefaultproperties
    {
        bLoop=false
        FuncName="None"
        Rate=0.0000000
        Count=0.0000000
        TimerObj=none
        bPaused=false
    }
};

struct native transient TraceHitInfo
{
    var init Material Material;
    var init PhysicalMaterial PhysMaterial;
    var init int Item;
    var init int LevelIndex;
    var init name BoneName;
    var init export editinline PrimitiveComponent HitComponent;

    structdefaultproperties
    {
        Material=none
        PhysMaterial=none
        Item=0
        LevelIndex=0
        BoneName="None"
        HitComponent=none
    }
};

struct native transient ImpactInfo
{
    var init Actor HitActor;
    var init Vector HitLocation;
    var init Vector HitNormal;
    var init Vector RayDir;
    var init Vector StartTrace;
    var init TraceHitInfo HitInfo;

    structdefaultproperties
    {
        HitActor=none
        HitLocation=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        HitNormal=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        RayDir=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        StartTrace=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        HitInfo=(Material=none,PhysMaterial=none,Item=0,LevelIndex=0,BoneName="None",HitComponent=none)
    }
};

struct native transient AnimSlotInfo
{
    var init name SlotName;
    var init array<float> ChannelWeights;

    structdefaultproperties
    {
        SlotName="None"
        ChannelWeights=none
    }
};

struct native transient AnimSlotDesc
{
    var init name SlotName;
    var init int NumChannels;

    structdefaultproperties
    {
        SlotName="None"
        NumChannels=0
    }
};

struct native transient PhysContactModificationData
{
    var init int ChangeFlags;
    var init native const Pointer PhysShape0;
    var init native const Pointer PhysShape1;
    var init Actor Actor0;
    var init Actor Actor1;
    var init int PhysFeatureIndex0;
    var init int physFeatureIndex1;
    var init native Pointer PhysData;

    structdefaultproperties
    {
        ChangeFlags=0
        Actor0=none
        Actor1=none
        PhysFeatureIndex0=0
        physFeatureIndex1=0
    }
};

struct RigidBodyState
{
    var Vector Position;
    var Quat Quaternion;
    var Vector LinVel;
    var Vector AngVel;
    var byte bNewData;

    structdefaultproperties
    {
        Position=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        Quaternion=(X=0.0000000,Y=0.0000000,Z=0.0000000,W=0.0000000)
        LinVel=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        AngVel=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        bNewData=0
    }
};

struct RigidBodyContactInfo
{
    var Vector ContactPosition;
    var Vector ContactNormal;
    var float ContactPenetration;
    var Vector ContactVelocity[2];
    var PhysicalMaterial PhysMaterial[2];

    structdefaultproperties
    {
        ContactPosition=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        ContactNormal=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        ContactPenetration=0.0000000
        ContactVelocity[0]=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        ContactVelocity[1]=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        PhysMaterial[0]=none
        PhysMaterial[1]=none
    }
};

struct CollisionImpactData
{
    var array<RigidBodyContactInfo> ContactInfos;
    var Vector TotalNormalForceVector;
    var Vector TotalFrictionForceVector;

    structdefaultproperties
    {
        ContactInfos=none
        TotalNormalForceVector=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        TotalFrictionForceVector=(X=0.0000000,Y=0.0000000,Z=0.0000000)
    }
};

struct native ReplicatedHitImpulse
{
    var Vector AppliedImpulse;
    var Vector HitLocation;
    var name BoneName;
    var byte ImpulseCount;
    var bool bRadialImpulse;

    structdefaultproperties
    {
        AppliedImpulse=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        HitLocation=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        BoneName="None"
        ImpulseCount=0
        bRadialImpulse=false
    }
};

struct native PhysEffectInfo
{
    var() float MinEffectSpeed;
    var() float MaxEffectSpeed;
    var() float ReFireDelay;
    var() ParticleSystem Effect;
    var() SoundCue Sound;
    var() export editinline RB_ForceComponent Force;

    structdefaultproperties
    {
        MinEffectSpeed=0.0000000
        MaxEffectSpeed=0.0000000
        ReFireDelay=0.0000000
        Effect=none
        Sound=none
        Force=none
    }
};

struct native InvestigationData
{
    var() init string InvestigationInfoTitle;
    var() init string InvestigationInfo;
    var() SoundCue BatmanThought;
    var() name GlobalFlagCheck;
    var() bool bInvertFlag;
    var() bool bWarningFlag;

    structdefaultproperties
    {
        InvestigationInfoTitle=""
        InvestigationInfo=""
        BatmanThought=none
        GlobalFlagCheck="None"
        bInvertFlag=false
        bWarningFlag=false
    }
};

struct native immutablewhencooked ActorReference
{
    var() Actor Actor;
    var() const editconst Guid Guid;

    structcpptext
	{
		FActorReference()
		{
			Actor = NULL;
		}
		FActorReference(EEventParm)
		{
			appMemzero(this, sizeof(FActorReference));
		}
		explicit FActorReference(class AActor *InActor, FGuid &InGuid)
		{
			Actor = InActor;
			Guid = InGuid;
		}
		// overload various operators to make the reference struct as transparent as possible
		FORCEINLINE AActor* operator*()
		{
			return Actor;
		}
		FORCEINLINE AActor* operator->()
		{
			return Actor;
		}
		/** Slow version of deref that will use GUID if Actor is NULL */
		AActor* operator~();
		FORCEINLINE FActorReference* operator=(AActor* TargetActor)
		{
			Actor = TargetActor;
			return this;
		}
		FORCEINLINE UBOOL operator==(const FActorReference &Ref) const
		{
			return (Ref != NULL && (Ref.Actor == Actor));
		}
		FORCEINLINE UBOOL operator!=(const FActorReference &Ref) const
		{
			return (Ref == NULL || (Ref.Actor != Actor));
		}
		FORCEINLINE UBOOL operator==(AActor *TestActor) const
		{
			return (Actor == TestActor);
		}
		FORCEINLINE UBOOL operator!=(AActor *TestActor) const
		{
			return (Actor != TestActor);
		}
		FORCEINLINE operator AActor*()
		{
			return Actor;
		}
		FORCEINLINE operator UBOOL()
		{
			return (Actor != NULL);
		}
		FORCEINLINE UBOOL operator!()
		{
			return (Actor == NULL);
		}
		FORCEINLINE class ANavigationPoint* Nav()
		{
			return ((class ANavigationPoint*)Actor);
		}
	}

    structdefaultproperties
    {
        Actor=none
        Guid=(A=0,B=0,C=0,D=0)
    }
};

struct native immutablewhencooked NavReference
{
    var() NavigationPoint Nav;
    var() const editconst Guid Guid;

    structdefaultproperties
    {
        Nav=none
        Guid=(A=0,B=0,C=0,D=0)
    }
};

cpptext
{
	// Used to adjust box used for collision in overlap checks which are performed at a location other than the actor's current location.
	static FVector OverlapAdjust;

	// Constructors.
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();

	// UObject interface.
	virtual INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	void ProcessEvent( UFunction* Function, void* Parms, void* Result=NULL );
	void ProcessState( FLOAT DeltaSeconds );
	UBOOL ProcessRemoteFunction( UFunction* Function, void* Parms, FFrame* Stack );
	void ProcessDemoRecFunction( UFunction* Function, void* Parms, FFrame* Stack );
	void InitExecution();
	virtual void PreEditChange(UProperty* PropertyThatWillChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PreSave();
	virtual void PostLoad();
	void NetDirty(UProperty* property);

	// AActor interface.
	virtual APawn* GetPlayerPawn() const {return NULL;}
	virtual UBOOL IsPlayerPawn() const {return false;}
	virtual UBOOL IgnoreBlockingBy( const AActor *Other) const;
	UBOOL IsOwnedBy( const AActor *TestOwner ) const;
	UBOOL IsBlockedBy( const AActor* Other, const UPrimitiveComponent* Primitive ) const;
	UBOOL IsBasedOn( const AActor *Other ) const;

	/**
	 * Utility for finding the PrefabInstance that 'owns' this actor.
	 * If the actor is not part of a prefab instance, returns NULL.
	 * If the actor _is_ a PrefabInstance, return itself.
	 */
	class APrefabInstance* FindOwningPrefabInstance() const;

	/**
	 * @return		TRUE if the actor is in the named group, FALSE otherwise.
	 */
	UBOOL IsInGroup(const TCHAR* GroupName) const;

	/**
	 * Parses the actor's group string into a list of group names (strings).
	 * @param		OutGroups		[out] Receives the list of group names.
	 */
	void GetGroups(TArray<FString>& OutGroups) const;

	AActor* GetBase() const;

	/**
	 * Called by ApplyDeltaToActor to perform an actor class-specific operation based on widget manipulation.
	 * The default implementation is simply to translate the actor's location.
	 */
	virtual void EditorApplyTranslation(const FVector& DeltaTranslation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);

	/**
	 * Called by ApplyDeltaToActor to perform an actor class-specific operation based on widget manipulation.
	 * The default implementation is simply to modify the actor's rotation.
	 */
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);

	/**
	 * Called by ApplyDeltaToActor to perform an actor class-specific operation based on widget manipulation.
	 * The default implementation is simply to modify the actor's draw scale.
	 */
	virtual void EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);

	void EditorUpdateBase();
	void EditorUpdateAttachedActors(const TArray<AActor*>& IgnoreActors);

	// Editor specific
	UBOOL IsHiddenEd() const;
	virtual UBOOL IsSelected() const
	{
		return (UObject::IsSelected() && !bDeleteMe);
	}

	virtual FLOAT GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, APlayerController* Viewer, UActorChannel* InChannel, FLOAT Time, UBOOL bLowBandwidth);
	/** ticks the actor
	 * @return TRUE if the actor was ticked, FALSE if it was aborted (e.g. because it's in stasis)
	 */
	virtual UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );
	/* AActor::InStasis()
	 * Called from AActor::Tick() if Actor->bStasis==true
	 * @return true if this actor ands its components can safely not be ticked.
	 */
	virtual UBOOL InStasis();
	/**
	 * bFinished is FALSE while the actor is being continually moved, and becomes TRUE on the last call.
	 * This can be used to defer computationally intensive calculations to the final PostEditMove call of
	 * eg a drag operation.
	 */
	virtual void PostEditMove(UBOOL bFinished);
	virtual void PostRename();
	virtual void Spawned();
	/** sets CollisionType to a default value based on the current collision settings of this Actor and its CollisionComponent */
	void SetDefaultCollisionType();
	/** sets collision flags based on the current CollisionType */
	void SetCollisionFromCollisionType();
	virtual void PreNetReceive();
	virtual void PostNetReceive();
	virtual void PostNetReceiveLocation();
	virtual void PostNetReceiveBase(AActor* NewBase);

	// Rendering info.

	virtual FMatrix LocalToWorld() const
	{
#if 0
		FTranslationMatrix	LToW		( -PrePivot					);
		FScaleMatrix		TempScale	( DrawScale3D * DrawScale	);
		FRotationMatrix		TempRot		( Rotation					);
		FTranslationMatrix	TempTrans	( Location					);
		LToW *= TempScale;
		LToW *= TempRot;
		LToW *= TempTrans;
		return LToW;
#else
		FMatrix Result;

		const FLOAT	SR = GMath.SinTab(Rotation.Roll),
				    SP = GMath.SinTab(Rotation.Pitch),
					SY = GMath.SinTab(Rotation.Yaw),
					CR = GMath.CosTab(Rotation.Roll),
					CP = GMath.CosTab(Rotation.Pitch),
					CY = GMath.CosTab(Rotation.Yaw);

		const FLOAT	LX = Location.X,
				    LY = Location.Y,
					LZ = Location.Z,
					PX = PrePivot.X,
					PY = PrePivot.Y,
					PZ = PrePivot.Z;

		const FLOAT	DX = DrawScale3D.X * DrawScale,
			        DY = DrawScale3D.Y * DrawScale,
					DZ = DrawScale3D.Z * DrawScale;

		Result.M[0][0] = CP * CY * DX;
		Result.M[0][1] = CP * DX * SY;
		Result.M[0][2] = DX * SP;
		Result.M[0][3] = 0.f;

		Result.M[1][0] = DY * ( CY * SP * SR - CR * SY );
		Result.M[1][1] = DY * ( CR * CY + SP * SR * SY );
		Result.M[1][2] = -CP * DY * SR;
		Result.M[1][3] = 0.f;

		Result.M[2][0] = -DZ * ( CR * CY * SP + SR * SY );
		Result.M[2][1] =  DZ * ( CY * SR - CR * SP * SY );
		Result.M[2][2] = CP * CR * DZ;
		Result.M[2][3] = 0.f;

		Result.M[3][0] = LX - CP * CY * DX * PX + CR * CY * DZ * PZ * SP - CY * DY * PY * SP * SR + CR * DY * PY * SY + DZ * PZ * SR * SY;
		Result.M[3][1] = LY - (CR * CY * DY * PY + CY * DZ * PZ * SR + CP * DX * PX * SY - CR * DZ * PZ * SP * SY + DY * PY * SP * SR * SY);
		Result.M[3][2] = LZ - (CP * CR * DZ * PZ + DX * PX * SP - CP * DY * PY * SR);
		Result.M[3][3] = 1.f;

		return Result;
#endif
	}
	virtual FMatrix WorldToLocal() const
	{
		return	FTranslationMatrix(-Location) *
				FInverseRotationMatrix(Rotation) *
				FScaleMatrix(FVector( 1.f / DrawScale3D.X, 1.f / DrawScale3D.Y, 1.f / DrawScale3D.Z) / DrawScale) *
				FTranslationMatrix(PrePivot);
	}

	/** Returns the size of the extent to use when moving the object through the world */
	FVector GetCylinderExtent() const;

	AActor* GetTopOwner();
	virtual UBOOL IsPendingKill() const
	{
		return bDeleteMe || HasAnyFlags(RF_PendingKill);
	}
	/** Fast check to see if an actor is alive by not being virtual */
	FORCEINLINE UBOOL ActorIsPendingKill(void) const
	{
		return bDeleteMe || HasAnyFlags(RF_PendingKill);
	}
	virtual void PostScriptDestroyed() {} // C++ notification that the script Destroyed() function has been called.

	// AActor collision functions.
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
	UBOOL IsOverlapping( AActor *Other, FCheckResult* Hit=NULL, UPrimitiveComponent* OtherPrimitiveComponent=NULL, UPrimitiveComponent* MyPrimitiveComponent=NULL );

	FBox GetComponentsBoundingBox(UBOOL bNonColliding=0) const;

	/**
	 * This will check to see if the Actor is still in the world.  It will check things like
	 * the KillZ, SoftKillZ, outside world bounds, etc. and handle the situation.
	 **/
	void CheckStillInWorld();

	// AActor general functions.
	void UnTouchActors();
	void FindTouchingActors();
	void BeginTouch(AActor *Other, UPrimitiveComponent* OtherComp, const FVector &HitLocation, const FVector &HitNormal, UPrimitiveComponent* MyComp=NULL);
	void EndTouch(AActor *Other, UBOOL NoNotifySelf);
	UBOOL IsBrush()       const;
	UBOOL IsStaticBrush() const;
	UBOOL IsVolumeBrush() const;
	UBOOL IsEncroacher() const;

	UBOOL FindInterpMoveTrack(class UInterpTrackMove** MoveTrack, class UInterpTrackInstMove** MoveTrackInst, class USeqAct_Interp** OutSeq);

	/**
	 * Returns True if an actor cannot move or be destroyed during gameplay, and can thus cast and receive static shadowing.
	 */
	UBOOL HasStaticShadowing() const { return bStatic || (bNoDelete && !bMovable); }

	/**
	 * Sets the hard attach flag by first handling the case of already being
	 * based upon another actor
	 *
	 * @param bNewHardAttach the new hard attach setting
	 */
	virtual void SetHardAttach(UBOOL bNewHardAttach);

	virtual void NotifyBump(AActor *Other, UPrimitiveComponent* OtherComp, const FVector &HitNormal);
	/** notification when actor has bumped against the level */
	virtual void NotifyBumpLevel(const FVector &HitLocation, const FVector &HitNormal);

	void SetCollision( UBOOL bNewCollideActors, UBOOL bNewBlockActors, UBOOL bNewIgnoreEncroachers );
	virtual void SetBase(AActor *NewBase, FVector NewFloor = FVector(0,0,1), int bNotifyActor=1, USkeletalMeshComponent* SkelComp=NULL, FName AttachName=NAME_None );
	void UpdateTimers(FLOAT DeltaSeconds);
	virtual void TickAuthoritative( FLOAT DeltaSeconds );
	virtual void TickSimulated( FLOAT DeltaSeconds );
	virtual void TickSpecial( FLOAT DeltaSeconds );
	virtual UBOOL PlayerControlled();
	virtual UBOOL IsNetRelevantFor(APlayerController* RealViewer, AActor* Viewer, const FVector& SrcLocation);
	virtual UBOOL DelayScriptReplication(FLOAT LastFullUpdateTime) { return false; }

	/** returns true if this actor should be considered relevancy owner for ReplicatedActor, which has bOnlyRelevantToOwner=true
	*/
	virtual UBOOL IsRelevancyOwnerFor(AActor* ReplicatedActor, AActor* ActorOwner);

	/** returns whether this Actor should be considered relevant because it is visible through
	 * the other side of any portals RealViewer can see
	 */
	UBOOL IsRelevantThroughPortals(APlayerController* RealViewer);

	// Level functions
	virtual void SetZone( UBOOL bTest, UBOOL bForceRefresh );
	virtual void SetVolumes();
	virtual void SetVolumes(const TArray<class AVolume*>& Volumes);
	virtual void PreBeginPlay();
	virtual void PostBeginPlay();

	/*
	 * Play a sound.  Creates an AudioComponent only if the sound is determined to be audible, and replicates the sound to clients based on optional flags
	 *
	 * @param	SoundLocation	the location to play the sound; if not specified, uses the actor's location.
	 */
	void PlaySound(class USoundCue* InSoundCue, UBOOL bNotReplicated = FALSE, UBOOL bNoRepToOwner = FALSE, UBOOL bStopWhenOwnerDestroyed = FALSE, FVector* SoundLocation = NULL, UBOOL bNoRepToRelevant = FALSE);

	// Physics functions.
	virtual void setPhysics(BYTE NewPhysics, AActor *NewFloor = NULL, FVector NewFloorV = FVector(0,0,1) );
	virtual void performPhysics(FLOAT DeltaSeconds);
	virtual void physProjectile(FLOAT deltaTime, INT Iterations);
	virtual void BoundProjectileVelocity();
	virtual void processHitWall(FCheckResult const& Hit, FLOAT TimeSlice=0.f);
	virtual void processLanded(FVector const& HitNormal, AActor *HitActor, FLOAT remainingTime, INT Iterations);
	virtual void physFalling(FLOAT deltaTime, INT Iterations);
	virtual void physWalking(FLOAT deltaTime, INT Iterations);
	virtual void physicsRotation(FLOAT deltaTime);
	inline void TwoWallAdjust(const FVector &DesiredDir, FVector &Delta, const FVector &HitNormal, const FVector &OldHitNormal, FLOAT HitTime)
	{
		if ((OldHitNormal | HitNormal) <= 0.f) //90 or less corner, so use cross product for dir
		{
			FVector NewDir = (HitNormal ^ OldHitNormal);
			NewDir = NewDir.SafeNormal();
			Delta = (Delta | NewDir) * (1.f - HitTime) * NewDir;
			if ((DesiredDir | Delta) < 0.f)
				Delta = -1.f * Delta;
		}
		else //adjust to new wall
		{
			Delta = (Delta - HitNormal * (Delta | HitNormal)) * (1.f - HitTime);
			if ((Delta | DesiredDir) <= 0.f)
				Delta = FVector(0.f,0.f,0.f);
		}
	}
	UBOOL moveSmooth(FVector const& Delta);
	virtual FRotator FindSlopeRotation(FVector const& FloorNormal, FRotator const& NewRotation);
	void UpdateRelativeRotation();
	virtual void GetNetBuoyancy(FLOAT &NetBuoyancy, FLOAT &NetFluidFriction);
	virtual void SmoothHitWall(FVector const& HitNormal, AActor *HitActor);
	virtual void stepUp(const FVector& GravDir, const FVector& DesiredDir, const FVector& Delta, FCheckResult &Hit);
	virtual UBOOL ShrinkCollision(AActor *HitActor, UPrimitiveComponent* HitComponent, const FVector &StartLocation);
	virtual void GrowCollision() {};
	virtual UBOOL MoveWithInterpMoveTrack(UInterpTrackMove* MoveTrack, UInterpTrackInstMove* MoveInst, FLOAT CurTime, FLOAT DeltaTime);
	virtual void AdjustInterpTrackMove(FVector& Pos, FRotator& Rot, FLOAT DeltaTime) {}
	virtual void physInterpolating(FLOAT DeltaTime);
	virtual void PushedBy(AActor* Other) {};
	virtual void UpdateBasedRotation(FRotator &FinalRotation, const FRotator& ReducedRotation) {};
	virtual void ReverseBasedRotation() {};

	/** Utility to add extra forces necessary for rigid-body gravity and damping to the collision component. */
	void AddRBGravAndDamping();

	virtual void physRigidBody(FLOAT DeltaTime);
	virtual void physSoftBody(FLOAT DeltaTime);

	virtual void InitRBPhys();
	virtual void TermRBPhys(FRBPhysScene* Scene);

	/**
	* Used by the cooker to pre cache the convex data for static meshes within a given actor.
	* This data is stored with the level.
	* @param Level - The level the cache is in
	* @param TriByteCount - running total of memory usage for per-tri collision cache
	* @param TriMeshCount - running count of per-tri collision cache
	* @param HullByteCount - running total of memory usage for hull cache
	* @param HullCount - running count of hull cache
	*/
	virtual void BuildPhysStaticMeshCache(ULevel* Level,
										  INT& TriByteCount, INT& TriMeshCount, INT& HullByteCount, INT& HullCount);

	void ApplyNewRBState(const FRigidBodyState& NewState, FLOAT* AngErrorAccumulator, FVector& OutDeltaPos);
	UBOOL GetCurrentRBState(FRigidBodyState& OutState);

	/**
	 *	Event called when this Actor is involved in a rigid body collision.
	 *	bNotifyRigidBodyCollision must be true on the physics PrimitiveComponent within this Actor for this event to be called.
	 *	This base class implementation fires off the RigidBodyCollision Kismet event if attached.
	 */
	virtual void OnRigidBodyCollision(const FRigidBodyCollisionInfo& MyInfo, const FRigidBodyCollisionInfo& OtherInfo, const FCollisionImpactData& RigidCollisionData);

	/** Update information used to detect overlaps between this actor and physics objects, used for 'pushing' things */
	virtual void UpdatePushBody() {};

#if WITH_NOVODEX
	virtual void ModifyNxActorDesc(NxActorDesc& ActorDesc,UPrimitiveComponent* PrimComp, const class NxGroupsMask& GroupsMask, UINT MatIndex) {}
	virtual void PostInitRigidBody(NxActor* nActor, NxActorDesc& ActorDesc, UPrimitiveComponent* PrimComp) {}
	virtual void PreTermRigidBody(NxActor* nActor) {}
	virtual void SyncActorToRBPhysics();
	void SyncActorToClothPhysics();
#endif // WITH_NOVODEX

	// AnimControl Matinee Track support

	/** Used to provide information on the slots that this Actor provides for animation to Matinee. */
	virtual void GetAnimControlSlotDesc(TArray<struct FAnimSlotDesc>& OutSlotDescs) {}

	/**
	 *	Called by Matinee when we open it to start controlling animation on this Actor.
	 *	Is also called again when the GroupAnimSets array changes in Matinee, so must support multiple calls.
	 */
	virtual void PreviewBeginAnimControl(TArray<class UAnimSet*>& InAnimSets) {}

	/** Called each frame by Matinee to update the desired sequence by name and position within it. */
	virtual void PreviewSetAnimPosition(FName SlotName, INT ChannelIndex, FName InAnimSeqName, FLOAT InPosition, UBOOL bLooping) {}

	/** Called each frame by Matinee to update the desired animation channel weights for this Actor. */
	virtual void PreviewSetAnimWeights(TArray<FAnimSlotInfo>& SlotInfos) {}

	/** Called by Matinee when we close it after we have been controlling animation on this Actor. */
	virtual void PreviewFinishAnimControl() {}

	/** Function used to control FaceFX animation in the editor (Matinee). */
	virtual void PreviewUpdateFaceFX(UBOOL bForceAnim, const FString& GroupName, const FString& SeqName, FLOAT InPosition) {}

	/** Used by Matinee playback to start a FaceFX animation playing. */
	virtual void PreviewActorPlayFaceFX(const FString& GroupName, const FString& SeqName, USoundCue* InSoundCue) {}

	/** Used by Matinee to stop current FaceFX animation playing. */
	virtual void PreviewActorStopFaceFX() {}

	/** Used in Matinee to get the AudioComponent we should play facial animation audio on. */
	virtual UAudioComponent* PreviewGetFaceFXAudioComponent() { return NULL; }

	/** Get the UFaceFXAsset that is currently being used by this Actor when playing facial animations. */
	virtual class UFaceFXAsset* PreviewGetActorFaceFXAsset() { return NULL; }

	/** Called each frame by Matinee to update the weight of a particular MorphNodeWeight. */
	virtual void PreviewSetMorphWeight(FName MorphNodeName, FLOAT MorphWeight) {}

	/** Called each frame by Matinee to update the scaling on a SkelControl. */
	virtual void PreviewSetSkelControlScale(FName SkelControlName, FLOAT Scale) {}

	// AI functions.
	int TestCanSeeMe(class APlayerController *Viewer);
	virtual INT AddMyMarker(AActor *S) { return 0; };
	virtual void ClearMarker() {};
	virtual AActor* AssociatedLevelGeometry();
	virtual UBOOL HasAssociatedLevelGeometry(AActor *Other);
	UBOOL SuggestTossVelocity(FVector* TossVelocity, const FVector& Dest, const FVector& Start, FLOAT TossSpeed, FLOAT BaseTossZ, FLOAT DesiredZPct, const FVector& CollisionSize, FLOAT TerminalVelocity, FLOAT OverrideGravityZ = 0.f);
	virtual UBOOL ReachedBy(APawn* P, const FVector& TestPosition, const FVector& Dest);
	virtual UBOOL TouchReachSucceeded(APawn *P, const FVector &TestPosition);
	virtual UBOOL BlockedByVehicle();

	// Special editor behavior
	AActor* GetHitActor();
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();
	virtual void CheckForDeprecated();

	// path creation
	virtual void PrePath() {};
	virtual void PostPath() {};

	/** tells this Actor to set its collision for the path building state
	 * for normally colliding Actors that AI should path through (e.g. doors) or vice versa
	 * @param bNowPathBuilding - whether we are now building paths
	 */
	virtual void SetCollisionForPathBuilding(UBOOL bNowPathBuilding);

	/**
	 * Return whether this actor is a builder brush or not.
	 *
	 * @return TRUE if this actor is a builder brush, FALSE otherwise
	 */
	virtual UBOOL IsABuilderBrush() const { return FALSE; }

	/**
	 * Return whether this actor is the current builder brush or not
	 *
	 * @return TRUE if htis actor is the current builder brush, FALSE otherwise
	 */
	virtual UBOOL IsCurrentBuilderBrush() const { return FALSE; }

	virtual UBOOL IsABrush() const {return FALSE;}
	virtual UBOOL IsAVolume() const {return FALSE;}
	virtual UBOOL IsAFluidSurface() const {return FALSE;}

	virtual APlayerController* GetAPlayerController() { return NULL; }
	virtual AController* GetAController() { return NULL; }
	virtual APawn* GetAPawn() { return NULL; }
	virtual const APawn* GetAPawn() const { return NULL; }
	virtual class AVehicle* GetAVehicle() { return NULL; }
	virtual AVolume* GetAVolume() { return NULL; }
	virtual class AFluidSurfaceActor* GetAFluidSurface() { return NULL; }
	virtual class AProjectile* GetAProjectile() { return NULL; }
	virtual const class AProjectile* GetAProjectile() const { return NULL; }
	virtual class APortalTeleporter* GetAPortalTeleporter() { return NULL; };

	virtual APlayerController* GetTopPlayerController()
	{
		AActor* TopActor = GetTopOwner();
		return (TopActor ? TopActor->GetAPlayerController() : NULL);
	}

	/**
	 * Verifies that neither this actor nor any of its components are RF_Unreachable and therefore pending
	 * deletion via the GC.
	 *
	 * @return TRUE if no unreachable actors are referenced, FALSE otherwise
	 */
	virtual UBOOL VerifyNoUnreachableReferences();

	virtual void ClearComponents();
	void ConditionalUpdateComponents(UBOOL bCollisionUpdate = FALSE);
protected:
	virtual void UpdateComponentsInternal(UBOOL bCollisionUpdate = FALSE);
public:

	/**
	 * Flags all components as dirty if in the editor, and then calls UpdateComponents().
	 *
	 * @param	bCollisionUpdate	[opt] As per UpdateComponents; defaults to FALSE.
	 * @param	bTransformOnly		[opt] TRUE to update only the component transforms, FALSE to update the entire component.
	 */
	virtual void ConditionalForceUpdateComponents(UBOOL bCollisionUpdate = FALSE,UBOOL bTransformOnly = TRUE);

	/**
	 * Flags all components as dirty so that they will be guaranteed an update from
	 * AActor::Tick(), and also be conditionally reattached by AActor::ConditionalUpdateComponents().
	 * @param	bTransformOnly	- True if only the transform has changed.
	 */
	void MarkComponentsAsDirty(UBOOL bTransformOnly = TRUE);

	/**
	 * Works through the component arrays marking entries as pending kill so references to them
	 * will be NULL'ed.
	 *
	 * @param	bAllowComponentOverride		Whether to allow component to override marking the setting
	 */
	virtual void MarkComponentsAsPendingKill( UBOOL bAllowComponentOverride = FALSE );

	void InvalidateLightingCache();

	virtual UBOOL ActorLineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags);

	// Natives.
	DECLARE_FUNCTION(execPollSleep);
	DECLARE_FUNCTION(execPollFinishAnim);

	// Matinee
	void GetInterpFloatPropertyNames(TArray<FName> &outNames);
	void GetInterpVectorPropertyNames(TArray<FName> &outNames);
	void GetInterpColorPropertyNames(TArray<FName> &outNames);
	FLOAT* GetInterpFloatPropertyRef(FName inName);
	FVector* GetInterpVectorPropertyRef(FName inName);
	FColor* GetInterpColorPropertyRef(FName inName);

	/**
	 * Returns TRUE if this actor is contained by TestLevel.
	 * @todo seamless: update once Actor->Outer != Level
	 */
	UBOOL IsInLevel(const ULevel *TestLevel) const;
	/** Return the ULevel that this Actor is part of. */
	ULevel* GetLevel() const;

	/**
	 * Determine whether this actor is referenced by its level's GameSequence.
	 *
	 * @param	pReferencer		if specified, will be set to the SequenceObject that is referencing this actor.
	 *
	 * @return TRUE if this actor is referenced by kismet.
	 */
	UBOOL IsReferencedByKismet( class USequenceObject** pReferencer=NULL ) const;

	/**
	 *	Do anything needed to clear out cross level references; Called from ULevel::PreSave
	 */
	virtual void ClearCrossLevelReferences();

	/**
	 * Called when a level is loaded/unloaded, to get a list of all the crosslevel
	 * paths that need to be fixed up.
	 */
	virtual void GetActorReferences(TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel) {}

	/** Returns ptr to GUID object for this actor.  Override in child classes that actually have a GUID */
	virtual FGuid* GetGuid() { return NULL; }

	/*
	 * Route finding notifications (sent to target)
	 */
	virtual class ANavigationPoint* SpecifyEndAnchor(APawn* RouteFinder) { return NULL; }
	virtual UBOOL AnchorNeedNotBeReachable();
	virtual void NotifyAnchorFindingResult(ANavigationPoint* EndAnchor, APawn* RouteFinder) {}
	virtual UBOOL ShouldHideActor(FVector const& CameraLocation) { return FALSE; }
	/** @return whether this Actor has exactly one attached colliding component (directly or indirectly)
	 *  and that component is its CollisionComponent
	 */
	UBOOL HasSingleCollidingComponent();
	/** Called each from while the Matinee action is running, to set the animation weights for the actor. */
	virtual void SetAnimWeights( const TArray<struct FAnimSlotInfo>& SlotInfos );
	/** called when this Actor was moved because its Base moved, but after that move the Actor was
	 * encroaching on its Base
	 * @param EncroachedBase - the Actor we encroached (Base will be temporarily NULL when this function is called)
	 * @param OverlapHit - result from the overlap check that determined this Actor was encroaching
	 * @return whether the encroachment was resolved (i.e, this Actor is no longer encroaching its base)
	 */
	virtual UBOOL ResolveAttachedMoveEncroachment(AActor* EncroachedBase, const FCheckResult& OverlapHit)
	{
	 	return FALSE;
	}
}

var(Advanced) bool bLoadIfPhysXLevel0;
var(Advanced) bool bLoadIfPhysXLevel1;
var(Advanced) bool bLoadIfPhysXLevel2;
var const bool bStatic;
var(Display) const bool bHidden;
var const bool bNoDelete;
var const bool bDeleteMe;
var const transient bool bTicked;
var const bool bOnlyOwnerSee;
var bool bStasis;
var const bool bExtraStasis;
var bool bWorldGeometry;
var bool bIgnoreRigidBodyPawns;
var bool bOrientOnSlope;
var const bool bIgnoreEncroachers;
var bool bPushedByEncroachers;
var bool bDestroyedByInterpActor;
var const bool bRouteBeginPlayEvenIfStatic;
var const bool bIsMoving;
var bool bAlwaysEncroachCheck;
var bool bHasAlternateTargetLocation;
var(Collision) bool bCanStepUpOn;
var const bool bNetTemporary;
var const bool bOnlyRelevantToOwner;
var transient bool bNetDirty;
var bool bAlwaysRelevant;
var bool bReplicateInstigator;
var bool bReplicateMovement;
var bool bSkipActorPropertyReplication;
var bool bUpdateSimulatedPosition;
var bool bTearOff;
var bool bOnlyDirtyReplication;
var(Physics) bool bAllowFluidSurfaceInteraction;
var transient bool bDemoRecording;
var bool bDemoOwner;
var bool bForceDemoRelevant;
var const bool bNetInitialRotation;
var bool bReplicateRigidBodyLocation;
var bool bKillDuringLevelTransition;
var const bool bExchangedRoles;
var(Advanced) bool bConsiderAllStaticMeshComponentsForStreaming;
var(Debug) bool bDebug;
var bool bPostRenderIfNotVisible;
var transient bool bForceNetUpdate;
var bool bAutomaticPerformPhysics;
var(Attachment) const bool bHardAttach;
var(Attachment) bool bSnapAttach;
var(Attachment) bool bIgnoreBaseRotation;
var(Attachment) bool bShadowParented;
var bool bCanBeAdheredTo;
var bool bCanBeFrictionedTo;
var bool bHurtEntry;
var bool bGameRelevant;
var const bool bMovable;
var bool bDestroyInPainVolume;
var bool bCanBeDamaged;
var bool bShouldBaseAtStartup;
var bool bPendingDelete;
var bool bCanTeleport;
var const bool bAlwaysTick;
var(Navigation) bool bBlocksNavigation;
var(Collision) const transient bool BlockRigidBody;
var bool bCollideWhenPlacing;
var const bool bCollideActors;
var bool bCollideWorld;
var(Collision) bool bCollideComplex;
var bool bBlockActors;
var bool bProjTarget;
var bool bBlocksTeleport;
var bool bForceZeroExtentCollision;
var bool bPlayerMovementCheck;
var bool bIgnoreDynamic;
var(Collision) bool bNoEncroachCheck;
var(Collision) bool bPhysRigidBodyOutOfWorldCheck;
var const transient bool bComponentOutsideWorld;
var bool bBounce;
var const bool bJustTeleported;
var const bool bNetInitial;
var const bool bNetOwner;
var(Advanced) const bool bHiddenEd;
var(Advanced) const bool bHiddenEdGroup;
var const bool bHiddenEdCustom;
var(Advanced) bool bEdShouldSnap;
var const transient bool bTempEditor;
var(Collision) bool bPathColliding;
var transient bool bPathTemp;
var bool bScriptInitialized;
var(Advanced) bool bLockLocation;
var const bool bForceAllowKismetModification;
var bool bIsPointOfInterest;
var const bool bDonePostBeginPlay;
var(Collision) bool bBatmanCanClimb;
var(Gadget) bool bValidLineLauncherTarget;
var(Gadget) bool bValidGelTarget;
var bool bCurrentInvestigateHightlighted;
var bool CachedInvestigateSightCheck;
var private const export editinline array<export editinline ActorComponent> Components;
var private const export editinline transient array<export editinline ActorComponent> AllComponents;
var(Movement) const Vector Location;
var(Movement) const Rotator Rotation;
var(Display) interp const float DrawScale;
var(Display) interp const Vector DrawScale3D;
var(Display) const Vector PrePivot;
var private native const RenderCommandFence DetachFence;
var float CustomTimeDilation;
var(Movement) const Actor.EPhysics Physics;
var Actor.ENetRole RemoteRole;
var Actor.ENetRole Role;
var(Collision) const transient Actor.ECollisionType CollisionType;
var transient Actor.ECollisionType ReplicatedCollisionType;
var const Object.ETickingGroup TickGroup;
var byte FramesTillInvestigateSightCheck;
var const Actor Owner;
var(Attachment) const Actor Base;
var const array<TimerData> Timers;
var Pawn Instigator;
var const transient WorldInfo WorldInfo;
var float LifeSpan;
var const float CreationTime;
var transient float LastRenderTime;
var(Object) name Tag;
var name InitialState;
var(Object) name Group;
var const transient array<Actor> Touching;
var const transient array<Actor> Children;
var const float LatentFloat;
var const AnimNodeSequence LatentSeqNode;
var const transient PhysicsVolume PhysicsVolume;
var Vector Velocity;
var Vector Acceleration;
var const transient Vector AngularVelocity;
var(Attachment) export editinline SkeletalMeshComponent BaseSkelComponent;
var(Attachment) name BaseBoneName;
var const array<Actor> Attached;
var const Vector RelativeLocation;
var const Rotator RelativeRotation;
var(Collision) editconst export editinline PrimitiveComponent CollisionComponent;
var native int OverlapTag;
var(Movement) Rotator RotationRate;
var(Movement) Rotator DesiredRotation;
var Actor PendingTouch;
var const array< class<SequenceEvent> > SupportedEvents;
var const array<SequenceEvent> GeneratedEvents;
var array<SeqAct_Latent> LatentActions;
var(Investigate) float InvestigationMaxDistance;
var(Investigate) array<InvestigationData> InvestigationDataArray;

// Export UActor::execForceUpdateComponents(FFrame&, void* const)
native function ForceUpdateComponents(optional bool bCollisionUpdate = false, optional bool bTransformOnly = true);

// Export UActor::execSetExtraStasis(FFrame&, void* const)
native function SetExtraStasis(bool NewValue);

// Export UActor::execConsoleCommand(FFrame&, void* const)
native function string ConsoleCommand(string Command, optional bool bWriteToLog = true);

// Export UActor::execSleep(FFrame&, void* const)
native(256) final latent function Sleep(float Seconds);

// Export UActor::execFinishAnim(FFrame&, void* const)
native(261) final latent function FinishAnim(AnimNodeSequence SeqNode);

// Export UActor::execSetCollision(FFrame&, void* const)
native(262) final function SetCollision(optional bool bNewColActors, optional bool bNewBlockActors, optional bool bNewIgnoreEncroachers);

// Export UActor::execSetCollisionSize(FFrame&, void* const)
native(283) final function SetCollisionSize(float NewRadius, float NewHeight);

// Export UActor::execSetCollisionType(FFrame&, void* const)
native final function SetCollisionType(Actor.ECollisionType NewCollisionType);

// Export UActor::execSetDrawScale(FFrame&, void* const)
native final function SetDrawScale(float NewScale);

// Export UActor::execSetDrawScale3D(FFrame&, void* const)
native final function SetDrawScale3D(Vector NewScale3D);

// Export UActor::execMove(FFrame&, void* const)
native(266) final function bool Move(Vector Delta);

// Export UActor::execSetLocation(FFrame&, void* const)
native(267) final function bool SetLocation(Vector NewLocation);

// Export UActor::execSetRotation(FFrame&, void* const)
native(299) final function bool SetRotation(Rotator NewRotation);

// Export UActor::execMovingWhichWay(FFrame&, void* const)
native function Actor.EMoveDir MovingWhichWay(out float Amount);

// Export UActor::execSetLocationForTest(FFrame&, void* const)
native final function bool SetLocationForTest(Vector NewLocation, bool bNoCheck);

// Export UActor::execSetZone(FFrame&, void* const)
native final function SetZone(bool bForceRefresh);

// Export UActor::execSetRelativeRotation(FFrame&, void* const)
native final function bool SetRelativeRotation(Rotator NewRotation);

// Export UActor::execSetRelativeLocation(FFrame&, void* const)
native final function bool SetRelativeLocation(Vector NewLocation);

// Export UActor::execSetHardAttach(FFrame&, void* const)
native final function SetHardAttach(optional bool bNewHardAttach);

// Export UActor::execfixedTurn(FFrame&, void* const)
native final function int fixedTurn(int Current, int Desired, int DeltaRate);

// Export UActor::execMoveSmooth(FFrame&, void* const)
native(3969) final function bool MoveSmooth(Vector Delta);

// Export UActor::execAutonomousPhysics(FFrame&, void* const)
native(3971) final function AutonomousPhysics(float DeltaSeconds);

// Export UActor::execGetTerminalVelocity(FFrame&, void* const)
native function float GetTerminalVelocity();

// Export UActor::execGetZoneVelocity(FFrame&, void* const)
native function Vector GetZoneVelocity();

// Export UActor::execSetBase(FFrame&, void* const)
native(298) final function SetBase(Actor NewBase, optional Vector NewFloor, optional SkeletalMeshComponent SkelComp, optional name AttachName);

// Export UActor::execSetOwner(FFrame&, void* const)
native(272) final function SetOwner(Actor NewOwner);

// Export UActor::execFindBase(FFrame&, void* const)
native function FindBase();

// Export UActor::execIsBasedOn(FFrame&, void* const)
native final function bool IsBasedOn(Actor TestActor);

// Export UActor::execGetBaseMost(FFrame&, void* const)
native function Actor GetBaseMost();

// Export UActor::execIsOwnedBy(FFrame&, void* const)
native final function bool IsOwnedBy(Actor TestActor);

simulated event ReplicatedEvent(name VarName)
{
    //return;    
}

simulated event ReplicatedDataBinding(name VarName)
{
    //return;    
}

// Export UActor::execSetForcedInitialReplicatedProperty(FFrame&, void* const)
native final function SetForcedInitialReplicatedProperty(Property PropToReplicate, bool bAdd);

// Export UActor::execFlushPersistentDebugLines(FFrame&, void* const)
native static final function FlushPersistentDebugLines();

// Export UActor::execDrawDebugLine(FFrame&, void* const)
native static final function DrawDebugLine(Vector LineStart, Vector LineEnd, byte R, byte G, byte B, optional bool bPersistentLines);

// Export UActor::execDrawDebugBox(FFrame&, void* const)
native static final function DrawDebugBox(Vector Center, Vector Extent, byte R, byte G, byte B, optional bool bPersistentLines);

// Export UActor::execDrawDebugCoordinateSystem(FFrame&, void* const)
native static final function DrawDebugCoordinateSystem(Vector AxisLoc, Rotator AxisRot, float Scale, optional bool bPersistentLines);

// Export UActor::execDrawDebugSphere(FFrame&, void* const)
native static final function DrawDebugSphere(Vector Center, float Radius, int Segments, byte R, byte G, byte B, optional bool bPersistentLines);

// Export UActor::execDrawDebugCylinder(FFrame&, void* const)
native static final function DrawDebugCylinder(Vector Start, Vector End, float Radius, int Segments, byte R, byte G, byte B, optional bool bPersistentLines);

// Export UActor::execDrawDebugCone(FFrame&, void* const)
native static final function DrawDebugCone(Vector Origin, Vector Direction, float Length, float AngleWidth, float AngleHeight, int NumSides, Color DrawColor, optional bool bPersistentLines);

// Export UActor::execChartData(FFrame&, void* const)
native final function ChartData(string DataName, float DataValue);

// Export UActor::execSetHidden(FFrame&, void* const)
native function SetHidden(bool bNewHidden);

// Export UActor::execSetOnlyOwnerSee(FFrame&, void* const)
native final function SetOnlyOwnerSee(bool bNewOnlyOwnerSee);

// Export UActor::execSetPhysics(FFrame&, void* const)
native(3970) final function SetPhysics(Actor.EPhysics newPhysics, optional bool WakePhysics = true);

// Export UActor::execClock(FFrame&, void* const)
native final function Clock(out float Time);

// Export UActor::execUnClock(FFrame&, void* const)
native final function UnClock(out float Time);

// Export UActor::execAttachComponent(FFrame&, void* const)
native final function AttachComponent(ActorComponent NewComponent);

// Export UActor::execDetachComponent(FFrame&, void* const)
native final function DetachComponent(ActorComponent ExComponent);

// Export UActor::execReattachComponent(FFrame&, void* const)
native final function ReattachComponent(ActorComponent ComponentToReattach);

// Export UActor::execSetTickGroup(FFrame&, void* const)
native final function SetTickGroup(Object.ETickingGroup NewTickGroup);

event Destroyed()
{
    //return;    
}

event GainedChild(Actor Other)
{
    //return;    
}

event LostChild(Actor Other)
{
    //return;    
}

event Tick(float DeltaTime)
{
    //return;    
}

event Timer()
{
    //return;    
}

event HitWall(Vector HitNormal, Actor Wall, PrimitiveComponent WallComp)
{
    //return;    
}

event Falling()
{
    //return;    
}

event Landed(Vector HitNormal, Actor FloorActor)
{
    //return;    
}

event PhysicsVolumeChange(PhysicsVolume NewVolume)
{
    //return;    
}

event Touch(Actor Other, PrimitiveComponent OtherComp, Vector HitLocation, Vector HitNormal)
{
    //return;    
}

event PostTouch(Actor Other)
{
    //return;    
}

event UnTouch(Actor Other)
{
    //return;    
}

event Bump(Actor Other, PrimitiveComponent OtherComp, Vector HitNormal)
{
    //return;    
}

event BaseChange()
{
    //return;    
}

event Attach(Actor Other)
{
    //return;    
}

event Detach(Actor Other)
{
    //return;    
}

event Actor SpecialHandling(Pawn Other)
{
    //return ReturnValue;    
}

event CollisionChanged()
{
    //return;    
}

event bool EncroachingOn(Actor Other)
{
    //return ReturnValue;    
}

event EncroachedBy(Actor Other)
{
    //return;    
}

event RanInto(Actor Other)
{
    //return;    
}

event AnimationTriggerCallback(name TagName, array<string> Params, AnimSet TagAnimSet, float Time)
{
    LogInternal("Actor::AnimationTriggerCallback" @ string(TagName));
    //return;    
}

simulated event FaceFXAudioStartedCallback(AudioComponent AC)
{
    //return;    
}

// Export UActor::execClampRotation(FFrame&, void* const)
native final simulated function bool ClampRotation(out Rotator out_Rot, Rotator rBase, Rotator rUpperLimits, Rotator rLowerLimits);

simulated event bool OverRotated(out Rotator out_Desired, out Rotator out_Actual)
{
    //return ReturnValue;    
}

function bool UsedBy(Pawn User)
{
    return TriggerEventClass(Class'SeqEvent_Used', User, -1);
    //return ReturnValue;    
}

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
    SetPhysics(0);
    SetHidden(true);
    SetCollision(false, false);
    Destroy();
    //return;    
}

simulated event OutsideWorldBounds()
{
    Destroy();
    //return;    
}

simulated function VolumeBasedDestroy(PhysicsVolume PV)
{
    Destroy();
    //return;    
}

// Export UActor::execTrace(FFrame&, void* const)
native(277) final function Actor Trace(out Vector HitLocation, out Vector HitNormal, Vector TraceEnd, optional Vector TraceStart, optional bool bTraceActors, optional Vector Extent, optional out TraceHitInfo HitInfo, optional int ExtraTraceFlags);

// Export UActor::execTraceComponent(FFrame&, void* const)
native final function bool TraceComponent(out Vector HitLocation, out Vector HitNormal, PrimitiveComponent InComponent, Vector TraceEnd, optional Vector TraceStart, optional Vector Extent, optional out TraceHitInfo HitInfo);

// Export UActor::execPointCheckComponent(FFrame&, void* const)
native final function bool PointCheckComponent(PrimitiveComponent InComponent, Vector PointLocation, Vector PointExtent);

// Export UActor::execFastTrace(FFrame&, void* const)
native(548) final function bool FastTrace(Vector TraceEnd, optional Vector TraceStart, optional Vector BoxExtent, optional bool bTraceBullet);

// Export UActor::execTraceAllPhysicsAssetInteractions(FFrame&, void* const)
native final function bool TraceAllPhysicsAssetInteractions(SkeletalMeshComponent SkelMeshComp, Vector EndTrace, Vector StartTrace, out array<ImpactInfo> out_Hits, optional Vector Extent);

// Export UActor::execFindSpot(FFrame&, void* const)
native final function bool FindSpot(Vector BoxExtent, out Vector SpotLocation, optional bool DontEaryOut = false);

// Export UActor::execContainsPoint(FFrame&, void* const)
native final function bool ContainsPoint(Vector Spot);

// Export UActor::execIsOverlapping(FFrame&, void* const)
native final function bool IsOverlapping(Actor A);

// Export UActor::execGetComponentsBoundingBox(FFrame&, void* const)
native final function GetComponentsBoundingBox(out Box ActorBox);

// Export UActor::execGetBoundingCylinder(FFrame&, void* const)
native function GetBoundingCylinder(out float CollisionRadius, out float CollisionHeight);

// Export UActor::execSpawn(FFrame&, void* const)
native noexport final function coerce actor Spawn
(
	class<actor>      SpawnClass,
	optional actor	  SpawnOwner,
	optional name     SpawnTag,
	optional vector   SpawnLocation,
	optional rotator  SpawnRotation,
	optional Actor    ActorTemplate,
	optional bool	  bNoCollisionFail
);

// Export UActor::execDestroy(FFrame&, void* const)
native(279) final function bool Destroy();

event TornOff()
{
    //return;    
}

// Export UActor::execSetTimer(FFrame&, void* const)
native(280) final function SetTimer(float InRate, optional bool inbLoop, optional name inTimerFunc = 'Timer', optional Object inObj);

// Export UActor::execClearTimer(FFrame&, void* const)
native final function ClearTimer(optional name inTimerFunc = 'Timer', optional Object inObj);

// Export UActor::execPauseTimer(FFrame&, void* const)
native final function PauseTimer(bool bPause, optional name inTimerFunc = 'Timer', optional Object inObj);

// Export UActor::execIsTimerActive(FFrame&, void* const)
native final function bool IsTimerActive(optional name inTimerFunc = 'Timer', optional Object inObj);

// Export UActor::execGetTimerCount(FFrame&, void* const)
native final function float GetTimerCount(optional name inTimerFunc = 'Timer', optional Object inObj);

// Export UActor::execGetTimerRate(FFrame&, void* const)
native final function float GetTimerRate(optional name TimerFuncName = 'Timer', optional Object inObj);

final simulated function float GetRemainingTimeForTimer(optional name TimerFuncName = 'Timer', optional Object inObj)
{
    local float Count, Rate;

    Rate = GetTimerRate(TimerFuncName, inObj);
    // End:0x56
    if(Rate != -1.0000000)
    {
        Count = GetTimerCount(TimerFuncName, inObj);
        return Rate - Count;
    }
    return -1.0000000;
    //return ReturnValue;    
}

// Export UActor::execCreateAudioComponent(FFrame&, void* const)
native final function AudioComponent CreateAudioComponent(SoundCue InSoundCue, optional bool bPlay, optional bool bStopWhenOwnerDestroyed, optional bool bUseLocation, optional Vector SourceLocation, optional bool bAttachToSelf = true);

// Export UActor::execPlaySound(FFrame&, void* const)
native final function PlaySound(SoundCue InSoundCue, optional bool bNotReplicated, optional bool bNoRepToOwner, optional bool bStopWhenOwnerDestroyed, optional Vector SoundLocation, optional bool bNoRepToRelevant);

// Export UActor::execEnableEffect(FFrame&, void* const)
native final function EnableEffect(name effectToEnable, optional bool Enable = true);

// Export UActor::execEffectEnabled(FFrame&, void* const)
native final function bool EffectEnabled(name effectToEnable);

// Export UActor::execEffectValue(FFrame&, void* const)
native final function EffectValue(name effectToEnable, float Value);

// Export UActor::execEffectSoundCue(FFrame&, void* const)
native final function EffectSoundCue(name effectToEnable, SoundCue InSoundCue);

// Export UActor::execEffectReverb(FFrame&, void* const)
native final function EffectReverb(name effectToEnable, FMODReverb InReverb);

// Export UActor::execSetMixBin(FFrame&, void* const)
native final function SetMixBin(MixBin mixToStart, float Time);

// Export UActor::execSetRoomMixBin(FFrame&, void* const)
native final function SetRoomMixBin(MixBin mixToStart);

// Export UActor::execSetMixBinMix(FFrame&, void* const)
native final function SetMixBinMix(MixBin mixToSet, float Value);

// Export UActor::execSetMasterMixBin(FFrame&, void* const)
native final function SetMasterMixBin(MixBin mixToStart);

// Export UActor::execMakeNoise(FFrame&, void* const)
native(512) final function MakeNoise(float Loudness, optional name NoiseType);

// Export UActor::execPlayerCanSeeMe(FFrame&, void* const)
native(532) final function bool PlayerCanSeeMe();

// Export UActor::execSuggestTossVelocity(FFrame&, void* const)
native final function bool SuggestTossVelocity(out Vector TossVelocity, Vector Destination, Vector Start, float TossSpeed, optional float BaseTossZ, optional float DesiredZPct, optional Vector CollisionSize, optional float TerminalVelocity, optional float OverrideGravityZ);

// Export UActor::execGetDestination(FFrame&, void* const)
native final function Vector GetDestination(Controller C);

function bool PreTeleport(Teleporter InTeleporter)
{
    //return ReturnValue;    
}

function PostTeleport(Teleporter OutTeleporter)
{
    //return;    
}

// Export UActor::execGetURLMap(FFrame&, void* const)
native(547) final function string GetURLMap();

// Export UActor::execAllActors(FFrame&, void* const)
native(304) final iterator function AllActors(class<Actor> BaseClass, out Actor Actor);

// Export UActor::execDynamicActors(FFrame&, void* const)
native(313) final iterator function DynamicActors(class<Actor> BaseClass, out Actor Actor);

// Export UActor::execChildActors(FFrame&, void* const)
native(305) final iterator function ChildActors(class<Actor> BaseClass, out Actor Actor);

// Export UActor::execBasedActors(FFrame&, void* const)
native(306) final iterator function BasedActors(class<Actor> BaseClass, out Actor Actor);

// Export UActor::execTouchingActors(FFrame&, void* const)
native(307) final iterator function TouchingActors(class<Actor> BaseClass, out Actor Actor);

// Export UActor::execTraceActors(FFrame&, void* const)
native(309) final iterator function TraceActors(class<Actor> BaseClass, out Actor Actor, out Vector HitLoc, out Vector HitNorm, Vector End, optional Vector Start, optional Vector Extent, optional out TraceHitInfo HitInfo, optional int ExtraTraceFlags);

// Export UActor::execVisibleActors(FFrame&, void* const)
native(311) final iterator function VisibleActors(class<Actor> BaseClass, out Actor Actor, optional float Radius, optional Vector Loc);

// Export UActor::execVisibleCollidingActors(FFrame&, void* const)
native(312) final iterator function VisibleCollidingActors(class<Actor> BaseClass, out Actor Actor, float Radius, optional Vector Loc, optional bool bIgnoreHidden, optional Vector Extent, optional bool bTraceActors);

// Export UActor::execCollidingActors(FFrame&, void* const)
native(321) final iterator function CollidingActors(class<Actor> BaseClass, out Actor Actor, float Radius, optional Vector Loc, optional bool bUseOverlapCheck);

// Export UActor::execOverlappingActors(FFrame&, void* const)
native final iterator function OverlappingActors(class<Actor> BaseClass, out Actor out_Actor, float Radius, optional Vector Loc, optional bool bIgnoreHidden);

// Export UActor::execComponentList(FFrame&, void* const)
native final iterator function ComponentList(Class BaseClass, out ActorComponent out_Component);

// Export UActor::execAllOwnedComponents(FFrame&, void* const)
native final iterator function AllOwnedComponents(class<Component> BaseClass, out ActorComponent OutComponent);

// Export UActor::execLocalPlayerControllers(FFrame&, void* const)
native final iterator function LocalPlayerControllers(class<PlayerController> BaseClass, out PlayerController PC);

final function bool FindActorsOfClass(class<Actor> ActorClass, out array<Actor> out_Actors)
{
    local Actor TestActor;

    out_Actors.Length = 0;
    // End:0x2B
    foreach AllActors(ActorClass, TestActor)
    {
        out_Actors[out_Actors.Length] = TestActor;        
    }    
    return out_Actors.Length > 0;
    //return ReturnValue;    
}

event PreBeginPlay()
{
    // End:0x66
    if(bStatic && !(bLoadIfPhysXLevel0 && bLoadIfPhysXLevel1) && bLoadIfPhysXLevel2)
    {
        // End:0x63
        if(!WorldInfo.Game.CheckRelevance(self))
        {
            SetHidden(true);
            SetCollisionType(1);
        }        
    }
    else
    {
        // End:0xFC
        if((!(bLoadIfPhysXLevel0 && bLoadIfPhysXLevel1) && bLoadIfPhysXLevel2 || (!bGameRelevant && !bStatic) && WorldInfo.NetMode != NM_Client) && !WorldInfo.Game.CheckRelevance(self))
        {
            // End:0xF9
            if(bNoDelete)
            {
                ShutDown();                
            }
            else
            {
                Destroy();
            }
        }
    }
    //return;    
}

event BroadcastLocalizedMessage(class<LocalMessage> InMessageClass, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject)
{
    WorldInfo.Game.BroadcastLocalized(self, InMessageClass, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);
    //return;    
}

event BroadcastLocalizedTeamMessage(int TeamIndex, class<LocalMessage> InMessageClass, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject)
{
    WorldInfo.Game.BroadcastLocalizedTeam(TeamIndex, self, InMessageClass, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);
    //return;    
}

event PostBeginPlay()
{
    //return;    
}

simulated event SetInitialState()
{
    bScriptInitialized = true;
    // End:0x28
    if(InitialState != 'None')
    {
        GotoState(InitialState);        
    }
    else
    {
        GotoState('Auto');
    }
    //return;    
}

simulated event ConstraintBrokenNotify(Actor ConOwner, RB_ConstraintSetup ConSetup, RB_ConstraintInstance ConInstance)
{
    //return;    
}

simulated event NotifySkelControlBeyondLimit(SkelControlLookAt LookAt)
{
    //return;    
}

simulated function bool StopsProjectile(Projectile P)
{
    return bProjTarget || bBlockActors;
    //return ReturnValue;    
}

simulated function bool HurtRadius(float BaseDamage, float DamageRadius, class<DamageType> DamageType, float Momentum, Vector HurtOrigin, optional Actor IgnoredActor, optional Controller InstigatedByController = ((Instigator != none) ? Instigator.Controller : none), optional bool bDoFullDamage)
{
    local Actor Victim;
    local bool bCausedDamage;

    // End:0x2E
    if(bHurtEntry)
    {
        return false;
    }
    bHurtEntry = true;
    bCausedDamage = false;
    // End:0x111
    foreach VisibleCollidingActors(Class'Actor', Victim, DamageRadius, HurtOrigin)
    {
        // End:0x110
        if(((!Victim.bWorldGeometry && Victim != self) && Victim != IgnoredActor) && Victim.bProjTarget || NavigationPoint(Victim) == none)
        {
            Victim.TakeRadiusDamage(InstigatedByController, BaseDamage, DamageRadius, DamageType, Momentum, HurtOrigin, bDoFullDamage, self);
            bCausedDamage = bCausedDamage || Victim.bProjTarget;
        }        
    }    
    bHurtEntry = false;
    return bCausedDamage;
    //return ReturnValue;    
}

function KilledBy(Pawn EventInstigator)
{
    //return;    
}

event TakeDamage(int DamageAmount, Controller EventInstigator, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
    local int Idx;
    local SeqEvent_TakeDamage dmgEvent;

    Idx = 0;
    J0x09:

    // End:0x6D [Loop If]
    if(Idx < GeneratedEvents.Length)
    {
        dmgEvent = SeqEvent_TakeDamage(GeneratedEvents[Idx]);
        // End:0x63
        if(dmgEvent != none)
        {
            dmgEvent.HandleDamage(self, EventInstigator, DamageType, DamageAmount, HitLocation);
        }
        Idx++;
        // [Loop Continue]
        goto J0x09;
    }
    //return;    
}

function bool HealDamage(int Amount, Controller Healer, class<DamageType> DamageType)
{
    //return ReturnValue;    
}

simulated function TakeRadiusDamage(Controller InstigatedBy, float BaseDamage, float DamageRadius, class<DamageType> DamageType, float Momentum, Vector HurtOrigin, bool bFullDamage, Actor DamageCauser)
{
    local float ColRadius, ColHeight, DamageScale, Dist;
    local Vector Dir;

    GetBoundingCylinder(ColRadius, ColHeight);
    Dir = Location - HurtOrigin;
    Dist = VSize(Dir);
    Dir = Normal(Dir);
    // End:0x57
    if(bFullDamage)
    {
        DamageScale = 1.0000000;        
    }
    else
    {
        Dist = FClamp(Dist - ColRadius, 0.0000000, DamageRadius);
        DamageScale = 1.0000000 - (Dist / DamageRadius);
    }
    // End:0xF9
    if(DamageScale > 0.0000000)
    {
        TakeDamage(int(DamageScale * BaseDamage), InstigatedBy, Location - ((0.5000000 * (ColHeight + ColRadius)) * Dir), (DamageScale * Momentum) * Dir, DamageType,, DamageCauser);
    }
    //return;    
}

final simulated function CheckHitInfo(out TraceHitInfo HitInfo, PrimitiveComponent FallBackComponent, Vector Dir, out Vector out_HitLocation)
{
    local Vector out_NewHitLocation, out_HitNormal, TraceEnd, TraceStart;
    local TraceHitInfo newHitInfo;

    // End:0x3D
    if((SkeletalMeshComponent(HitInfo.HitComponent) != none) && HitInfo.BoneName != 'None')
    {
        return;
    }
    // End:0x98
    if((HitInfo.HitComponent == none) || (SkeletalMeshComponent(HitInfo.HitComponent) == none) && SkeletalMeshComponent(FallBackComponent) != none)
    {
        HitInfo.HitComponent = FallBackComponent;
    }
    // End:0x1C1
    if((SkeletalMeshComponent(HitInfo.HitComponent) != none) && HitInfo.BoneName == 'None')
    {
        // End:0xEB
        if(IsZero(Dir))
        {
            Dir = Vector(Rotation);
        }
        // End:0x101
        if(IsZero(out_HitLocation))
        {
            out_HitLocation = Location;
        }
        TraceStart = out_HitLocation - (float(128) * Normal(Dir));
        TraceEnd = out_HitLocation + (float(128) * Normal(Dir));
        // End:0x1C1
        if(TraceComponent(out_NewHitLocation, out_HitNormal, HitInfo.HitComponent, TraceEnd, TraceStart, vect(0.0000000, 0.0000000, 0.0000000), newHitInfo))
        {
            HitInfo.BoneName = newHitInfo.BoneName;
            HitInfo.PhysMaterial = newHitInfo.PhysMaterial;
            out_HitLocation = out_NewHitLocation;
        }
    }
    //return;    
}

// Export UActor::execGetGravityZ(FFrame&, void* const)
native function float GetGravityZ();

event DebugFreezeGame(optional Actor ActorToLookAt)
{
    local PlayerController PC;

    ScriptTrace();
    // End:0x6A
    foreach LocalPlayerControllers(Class'PlayerController', PC)
    {        
        PC.ConsoleCommand("PlayersOnly");
        // End:0x66
        if(ActorToLookAt != none)
        {
            PC.SetViewTarget(ActorToLookAt);
        }        
        return;        
    }    
    //return;    
}

function bool CheckForErrors()
{
    //return ReturnValue;    
}

event BecomeViewTarget(PlayerController PC)
{
    //return;    
}

event EndViewTarget(PlayerController PC)
{
    //return;    
}

simulated function bool CalcCamera(float fDeltaTime, out Vector out_CamLoc, out Rotator out_CamRot, out float out_FOV)
{
    local Vector HitNormal;
    local float Radius, Height;

    GetBoundingCylinder(Radius, Height);
    // End:0x6F
    if(Trace(out_CamLoc, HitNormal, Location - ((Vector(out_CamRot) * Radius) * float(20)), Location, false) == none)
    {
        out_CamLoc = Location - ((Vector(out_CamRot) * Radius) * float(20));        
    }
    else
    {
        out_CamLoc = Location + (Height * Vector(Rotation));
    }
    return false;
    //return ReturnValue;    
}

simulated function string GetItemName(string FullName)
{
    local int pos;

    pos = InStr(FullName, ".");
    J0x11:

    // End:0x52 [Loop If]
    if(pos != -1)
    {
        FullName = Right(FullName, (Len(FullName) - pos) - 1);
        pos = InStr(FullName, ".");
        // [Loop Continue]
        goto J0x11;
    }
    return FullName;
    //return ReturnValue;    
}

simulated function string GetHumanReadableName()
{
    return GetItemName(string(Class));
    //return ReturnValue;    
}

static function ReplaceText(out string Text, string Replace, string With)
{
    local int I;
    local string Input;

    Input = Text;
    Text = "";
    I = InStr(Input, Replace);
    J0x26:

    // End:0x87 [Loop If]
    if(I != -1)
    {
        Text = (Text $ Left(Input, I)) $ With;
        Input = Mid(Input, I + Len(Replace));
        I = InStr(Input, Replace);
        // [Loop Continue]
        goto J0x26;
    }
    Text = Text $ Input;
    //return;    
}

static function string GetLocalString(optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2)
{
    return "";
    //return ReturnValue;    
}

function MatchStarting()
{
    //return;    
}

function SetGRI(GameReplicationInfo GRI)
{
    //return;    
}

function string GetDebugName()
{
    return GetItemName(string(self));
    //return ReturnValue;    
}

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
    local string T;
    local Actor A;
    local float MyRadius, MyHeight;
    local Canvas Canvas;

    Canvas = HUD.Canvas;
    Canvas.SetPos(4.0000000, out_YPos);
    Canvas.SetDrawColor(255, 0, 0);
    T = GetDebugName();
    // End:0x8A
    if(bDeleteMe)
    {
        T = T $ " DELETED (bDeleteMe == true)";
    }
    // End:0xD4
    if(T != "")
    {
        Canvas.DrawText(T, false);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
    }
    Canvas.SetDrawColor(255, 255, 255);
    // End:0x1D0
    if(HUD.ShouldDisplayDebug('net'))
    {
        // End:0x1D0
        if(WorldInfo.NetMode != NM_Standalone)
        {
            T = (((("ROLE:" @ string(Role)) @ "RemoteRole:") @ string(RemoteRole)) @ "NetMode:") @ string(WorldInfo.NetMode);
            // End:0x192
            if(bTearOff)
            {
                T = T @ "Tear Off";
            }
            Canvas.DrawText(T, false);
            out_YPos += out_YL;
            Canvas.SetPos(4.0000000, out_YPos);
        }
    }
    Canvas.DrawText((("Location:" @ string(Location)) @ "Rotation:") @ string(Rotation), false);
    out_YPos += out_YL;
    Canvas.SetPos(4.0000000, out_YPos);
    // End:0x4A9
    if(HUD.ShouldDisplayDebug('Physics'))
    {
        T = (((((("Physics" @ (GetPhysicsName())) @ "in physicsvolume") @ (GetItemName(string(PhysicsVolume)))) @ "on base") @ (GetItemName(string(Base)))) @ "gravity") @ string(GetGravityZ());
        // End:0x2F2
        if(bBounce)
        {
            T = T $ " - will bounce";
        }
        Canvas.DrawText(T, false);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        Canvas.DrawText((((((((("bHardAttach:" @ string(bHardAttach)) @ "RelativeLoc:") @ string(RelativeLocation)) @ "RelativeRot:") @ string(RelativeRotation)) @ "SkelComp:") @ string(BaseSkelComponent)) @ "Bone:") @ string(BaseBoneName), false);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        Canvas.DrawText((((("Velocity:" @ string(Velocity)) @ "Speed:") @ string(VSize(Velocity))) @ "Speed2D:") @ string(VSize2D(Velocity)), false);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        Canvas.DrawText("Acceleration:" @ string(Acceleration), false);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
    }
    // End:0x6F1
    if(HUD.ShouldDisplayDebug('Collision'))
    {
        Canvas.DrawColor.B = 0;
        GetBoundingCylinder(MyRadius, MyHeight);
        Canvas.DrawText((("Collision Radius:" @ string(MyRadius)) @ "Height:") @ string(MyHeight));
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        Canvas.DrawText((((("Collides with Actors:" @ string(bCollideActors)) @ " world:") @ string(bCollideWorld)) @ "proj. target:") @ string(bProjTarget));
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        Canvas.DrawText("Blocks Actors:" @ string(bBlockActors));
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
        T = "Touching ";
        // End:0x685
        foreach TouchingActors(Class'Actor', A)
        {
            T = (T $ (GetItemName(string(A)))) $ " ";            
        }        
        // End:0x6B3
        if(T == "Touching ")
        {
            T = "Touching nothing";
        }
        Canvas.DrawText(T, false);
        out_YPos += out_YL;
        Canvas.SetPos(4.0000000, out_YPos);
    }
    Canvas.DrawColor.B = 255;
    Canvas.DrawText(" STATE:" @ string(GetStateName()), false);
    out_YPos += out_YL;
    Canvas.SetPos(4.0000000, out_YPos);
    Canvas.DrawText(((" Instigator:" @ (GetItemName(string(Instigator)))) @ "Owner:") @ (GetItemName(string(Owner))));
    out_YPos += out_YL;
    Canvas.SetPos(4.0000000, out_YPos);
    //return;    
}

event string GetPhysicsName()
{
    switch(Physics)
    {
        // End:0x17
        case 0:
            return "None";
            // End:0xFC
            break;
        // End:0x29
        case 1:
            return "Walking";
            // End:0xFC
            break;
        // End:0x3B
        case 2:
            return "Falling";
            // End:0xFC
            break;
        // End:0x4E
        case 3:
            return "Swimming";
            // End:0xFC
            break;
        // End:0x5F
        case 4:
            return "Flying";
            // End:0xFC
            break;
        // End:0x72
        case 5:
            return "Rotating";
            // End:0xFC
            break;
        // End:0x87
        case 6:
            return "Projectile";
            // End:0xFC
            break;
        // End:0x9F
        case 7:
            return "Interpolating";
            // End:0xFC
            break;
        // End:0xB0
        case 8:
            return "Spider";
            // End:0xFC
            break;
        // End:0xC1
        case 9:
            return "Ladder";
            // End:0xFC
            break;
        // End:0xD5
        case 10:
            return "RigidBody";
            // End:0xFC
            break;
        // End:0xE6
        case 13:
            return "Unused";
            // End:0xFC
            break;
        // End:0xF9
        case 12:
            return "Floating";
            // End:0xFC
            break;
        // End:0xFFFF
        default:
            break;
    }
    return "Unknown";
    //return ReturnValue;    
}

simulated event ModifyHearSoundComponent(AudioComponent AC)
{
    //return;    
}

simulated event AudioComponent GetFaceFXAudioComponent()
{
    return none;
    //return ReturnValue;    
}

event Reset()
{
    //return;    
}

function bool IsInVolume(Volume aVolume)
{
    local Volume V;

    // End:0x23
    foreach TouchingActors(Class'Volume', V)
    {
        // End:0x22
        if(V == aVolume)
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

function PlayTeleportEffect(bool bOut, bool bSound)
{
    //return;    
}

simulated function bool CanSplash()
{
    return false;
    //return ReturnValue;    
}

simulated function bool EffectedByReverb()
{
    return false;
    //return ReturnValue;    
}

simulated function ApplyFluidSurfaceImpact(FluidSurfaceActor Fluid, Vector HitLocation)
{
    local float Radius, Height, AdjustedVelocity;

    // End:0x88
    if(bAllowFluidSurfaceInteraction)
    {
        AdjustedVelocity = 0.0100000 * Abs(Velocity.Z);
        GetBoundingCylinder(Radius, Height);
        Fluid.FluidComponent.ApplyForce(HitLocation, AdjustedVelocity * Fluid.FluidComponent.ForceImpact, Radius * 0.3000000, true);
    }
    //return;    
}

simulated function bool CheckMaxEffectDistance(PlayerController P, Vector SpawnLocation, optional float CullDistance)
{
    local float Dist;

    // End:0x18
    if(P.ViewTarget == none)
    {
        return true;
    }
    // End:0x81
    if((Vector(P.Rotation) Dot (SpawnLocation - P.ViewTarget.Location)) < 0.0000000)
    {
        return VSize(P.ViewTarget.Location - SpawnLocation) < float(1600);
    }
    Dist = VSize(SpawnLocation - P.ViewTarget.Location);
    // End:0xDC
    if((CullDistance > 0.0000000) && CullDistance < (Dist * P.LODDistanceFactor))
    {
        return false;
    }
    return !P.BeyondFogDistance(P.ViewTarget.Location, SpawnLocation);
    //return ReturnValue;    
}

simulated function bool EffectIsRelevant(Vector SpawnLocation, bool bForceDedicated, optional float CullDistance)
{
    local PlayerController P;
    local bool bResult;

    // End:0x22
    if(WorldInfo.NetMode == NM_DedicatedServer)
    {
        return bForceDedicated;
    }
    // End:0xA2
    if((WorldInfo.NetMode == NM_ListenServer) && WorldInfo.Game.NumPlayers > 1)
    {
        // End:0x68
        if(bForceDedicated)
        {
            return true;
        }
        // End:0x9F
        if(((Instigator != none) && Instigator.IsHumanControlled()) && Instigator.IsLocallyControlled())
        {
            return true;
        }        
    }
    else
    {
        // End:0xC4
        if((Instigator != none) && Instigator.IsHumanControlled())
        {
            return true;
        }
    }
    // End:0xFA
    if(SpawnLocation == Location)
    {
        bResult = (WorldInfo.TimeSeconds - LastRenderTime) < 0.5000000;        
    }
    else
    {
        // End:0x139
        if((Instigator != none) && (WorldInfo.TimeSeconds - Instigator.LastRenderTime) < 1.0000000)
        {
            bResult = true;
        }
    }
    // End:0x1C3
    if(bResult)
    {
        bResult = false;
        // End:0x1C2
        foreach LocalPlayerControllers(Class'PlayerController', P)
        {
            // End:0x1C1
            if(P.ViewTarget != none)
            {
                // End:0x19E
                if((P.Pawn == Instigator) && Instigator != none)
                {                    
                    return true;
                    // End:0x1C1
                    continue;
                }
                bResult = CheckMaxEffectDistance(P, SpawnLocation, CullDistance);
                // End:0x1C2
                break;
            }            
        }        
    }
    return bResult;
    //return ReturnValue;    
}

final simulated function float TimeSince(float Time)
{
    return WorldInfo.TimeSeconds - Time;
    //return ReturnValue;    
}

simulated function bool TriggerEventClass(class<SequenceEvent> InEventClass, Actor InInstigator, optional int ActivateIndex = -1, optional bool bTest, optional out array<SequenceEvent> ActivatedEvents)
{
    local array<int> ActivateIndices;

    // End:0x23
    if(ActivateIndex >= 0)
    {
        ActivateIndices[0] = ActivateIndex;
    }
    return ActivateEventClass(InEventClass, InInstigator, GeneratedEvents, ActivateIndices, bTest, ActivatedEvents);
    //return ReturnValue;    
}

final simulated function bool ActivateEventClass(class<SequenceEvent> InClass, Actor InInstigator, const out array<SequenceEvent> EventList, const optional out array<int> ActivateIndices, optional bool bTest, optional out array<SequenceEvent> ActivatedEvents)
{
    local SequenceEvent Evt;

    ActivatedEvents.Length = 0;
    // End:0x69
    foreach EventList(Evt)
    {
        // End:0x68
        if(ClassIsChildOf(Evt.Class, InClass) && Evt.CheckActivate(self, InInstigator, bTest, ActivateIndices))
        {
            ActivatedEvents.AddItem(Evt);
        }        
    }    
    return ActivatedEvents.Length > 0;
    //return ReturnValue;    
}

final simulated function bool FindEventsOfClass(class<SequenceEvent> EventClass, optional out array<SequenceEvent> out_EventList, optional bool bIncludeDisabled)
{
    local SequenceEvent Evt;
    local bool bFoundEvent;

    // End:0xAA
    foreach GeneratedEvents(Evt)
    {
        // End:0xA9
        if((((Evt != none) && Evt.bEnabled || bIncludeDisabled) && ClassIsChildOf(Evt.Class, EventClass)) && (Evt.MaxTriggerCount == 0) || Evt.MaxTriggerCount > Evt.TriggerCount)
        {
            out_EventList.AddItem(Evt);
            bFoundEvent = true;
        }        
    }    
    return bFoundEvent;
    //return ReturnValue;    
}

final simulated function ClearLatentAction(class<SeqAct_Latent> actionClass, optional bool bAborted, optional SeqAct_Latent exceptionAction)
{
    local int Idx;

    Idx = 0;
    J0x09:

    // End:0xAE [Loop If]
    if(Idx < LatentActions.Length)
    {
        // End:0x3B
        if(LatentActions[Idx] == none)
        {
            LatentActions.Remove(Idx--, 1);
            // [Explicit Continue]
            goto J0xA4;
        }
        // End:0xA4
        if(ClassIsChildOf(LatentActions[Idx].Class, actionClass) && LatentActions[Idx] != exceptionAction)
        {
            // End:0x96
            if(bAborted)
            {
                LatentActions[Idx].AbortFor(self);
            }
            LatentActions.Remove(Idx--, 1);
        }
        J0xA4:

        Idx++;
        // [Loop Continue]
        goto J0x09;
    }
    //return;    
}

simulated function OnDestroy(SeqAct_Destroy Action)
{
    local int AttachIdx, IgnoreIdx;
    local Actor A;

    // End:0xCB
    if(Action.bDestroyBasedActors)
    {
        AttachIdx = 0;
        J0x1A:

        // End:0xCB [Loop If]
        if(AttachIdx < Attached.Length)
        {
            A = Attached[AttachIdx];
            IgnoreIdx = 0;
            J0x42:

            // End:0x9A [Loop If]
            if(IgnoreIdx < Action.IgnoreBasedClasses.Length)
            {
                // End:0x90
                if(ClassIsChildOf(A.Class, Action.IgnoreBasedClasses[IgnoreIdx]))
                {
                    A = none;
                    // [Explicit Break]
                    goto J0x9A;
                }
                IgnoreIdx++;
                // [Loop Continue]
                goto J0x42;
            }
            J0x9A:

            // End:0xA8
            if(A == none)
            {
                // [Explicit Continue]
                goto J0xC1;
            }
            A.OnDestroy(Action);
            J0xC1:

            AttachIdx++;
            // [Loop Continue]
            goto J0x1A;
        }
    }
    // End:0xF3
    if(bNoDelete || Role < ROLE_Authority)
    {
        ShutDown();        
    }
    else
    {
        // End:0x101
        if(!bDeleteMe)
        {
            Destroy();
        }
    }
    //return;    
}

event ForceNetRelevant()
{
    //return;    
}

// Export UActor::execSetNetUpdateTime(FFrame&, void* const)
native final function SetNetUpdateTime(float NewUpdateTime);

simulated event ShutDown()
{
    SetPhysics(0);
    SetCollision(false, false);
    // End:0x28
    if(CollisionComponent != none)
    {
        CollisionComponent.SetBlockRigidBody(false);
    }
    SetHidden(true);
    bStasis = true;
    //return;    
}

simulated function OnCauseDamage(SeqAct_CauseDamage Action)
{
    local Controller InstigatorController;
    local Pawn InstigatorPawn;

    InstigatorController = Controller(Action.Instigator);
    // End:0x5F
    if(InstigatorController == none)
    {
        InstigatorPawn = Pawn(Action.Instigator);
        // End:0x5F
        if(InstigatorPawn != none)
        {
            InstigatorController = InstigatorPawn.Controller;
        }
    }
    TakeDamage(int(Action.DamageAmount), InstigatorController, Location, Vector(Rotation) * -Action.Momentum, Action.DamageType);
    //return;    
}

// Export UActor::execPrestreamTextures(FFrame&, void* const)
native function PrestreamTextures(float Seconds, bool bEnableStreaming);

function OnHealDamage(SeqAct_HealDamage Action)
{
    local Controller InstigatorController;
    local Pawn InstigatorPawn;

    InstigatorController = Controller(Action.Instigator);
    // End:0x5F
    if(InstigatorController == none)
    {
        InstigatorPawn = Pawn(Action.Instigator);
        // End:0x5F
        if(InstigatorPawn != none)
        {
            InstigatorController = InstigatorPawn.Controller;
        }
    }
    HealDamage(Action.HealAmount, InstigatorController, Action.DamageType);
    //return;    
}

simulated function OnTeleport(SeqAct_Teleport Action)
{
    local bool UpdateRot;
    local Vector Loc;
    local Rotator Rot;

    UpdateRot = Action.GetDestination(Loc, Rot);
    // End:0x6A
    if(SetLocation(Loc))
    {
        PlayTeleportEffect(false, true);
        // End:0x4D
        if(UpdateRot)
        {
            SetRotation(Rot);
        }
        ForceNetRelevant();
        bUpdateSimulatedPosition = true;
        bNetDirty = true;        
    }
    else
    {
        WarnInternal("Unable to teleport to" @ string(Loc));
    }
    //return;    
}

simulated function OnSetBlockRigidBody(SeqAct_SetBlockRigidBody Action)
{
    // End:0x70
    if(CollisionComponent != none)
    {
        // End:0x3F
        if(Action.InputLinks[0].bHasImpulse)
        {
            CollisionComponent.SetBlockRigidBody(true);            
        }
        else
        {
            // End:0x70
            if(Action.InputLinks[1].bHasImpulse)
            {
                CollisionComponent.SetBlockRigidBody(false);
            }
        }
    }
    //return;    
}

simulated function OnSetPhysics(SeqAct_SetPhysics Action)
{
    ForceNetRelevant();
    SetPhysics(Action.newPhysics, Action.bWakePhysics);
    // End:0x80
    if(RemoteRole != ROLE_None)
    {
        // End:0x65
        if(Physics != 0)
        {
            bUpdateSimulatedPosition = true;
            // End:0x65
            if(bOnlyDirtyReplication)
            {
                bNetDirty = true;
            }
        }
        SetForcedInitialReplicatedProperty(ByteProperty'Actor.Physics', Physics == default.Physics);
    }
    //return;    
}

function OnChangeCollision(SeqAct_ChangeCollision Action)
{
    // End:0x7E
    if(Action.ObjInstanceVersion < Action.GetObjClassVersion())
    {
        SetCollision(Action.bCollideActors, Action.bBlockActors, Action.bIgnoreEncroachers);
        CollisionComponent.SetBlockRigidBody(Action.bBlockActors);        
    }
    else
    {
        SetCollisionType(Action.CollisionType);
    }
    ForceNetRelevant();
    // End:0xDF
    if(RemoteRole != ROLE_None)
    {
        SetForcedInitialReplicatedProperty(BoolProperty'Actor.bCollideActors', bCollideActors == default.bCollideActors);
        SetForcedInitialReplicatedProperty(BoolProperty'Actor.bBlockActors', bBlockActors == default.bBlockActors);
    }
    //return;    
}

simulated function OnToggleHidden(SeqAct_ToggleHidden Action)
{
    local int AttachIdx, IgnoreIdx;
    local Actor A;

    // End:0xCB
    if(Action.bToggleBasedActors)
    {
        AttachIdx = 0;
        J0x1A:

        // End:0xCB [Loop If]
        if(AttachIdx < Attached.Length)
        {
            A = Attached[AttachIdx];
            IgnoreIdx = 0;
            J0x42:

            // End:0x9A [Loop If]
            if(IgnoreIdx < Action.IgnoreBasedClasses.Length)
            {
                // End:0x90
                if(ClassIsChildOf(A.Class, Action.IgnoreBasedClasses[IgnoreIdx]))
                {
                    A = none;
                    // [Explicit Break]
                    goto J0x9A;
                }
                IgnoreIdx++;
                // [Loop Continue]
                goto J0x42;
            }
            J0x9A:

            // End:0xA8
            if(A == none)
            {
                // [Explicit Continue]
                goto J0xC1;
            }
            A.OnToggleHidden(Action);
            J0xC1:

            AttachIdx++;
            // [Loop Continue]
            goto J0x1A;
        }
    }
    // End:0xF9
    if(Action.InputLinks[0].bHasImpulse)
    {
        SetHidden(true);        
    }
    else
    {
        // End:0x127
        if(Action.InputLinks[1].bHasImpulse)
        {
            SetHidden(false);            
        }
        else
        {
            SetHidden(!bHidden);
        }
    }
    ForceNetRelevant();
    // End:0x16C
    if(RemoteRole != ROLE_None)
    {
        SetForcedInitialReplicatedProperty(BoolProperty'Actor.bHidden', bHidden == default.bHidden);
    }
    // End:0x1CD
    if(Action.bToggleCollision)
    {
        SetCollision(!bHidden);
        CollisionComponent.SetBlockRigidBody(!bHidden);
        // End:0x1CD
        if(RemoteRole != ROLE_None)
        {
            SetForcedInitialReplicatedProperty(BoolProperty'Actor.bCollideActors', bCollideActors == default.bCollideActors);
        }
    }
    //return;    
}

function OnAttachToActor(SeqAct_AttachToActor Action)
{
    local int Idx;
    local Actor Attachment;
    local Controller C;
    local array<Object> ObjVars;

    Action.GetObjectVars(ObjVars, "Attachment");
    Idx = 0;
    J0x28:

    // End:0x17D [Loop If]
    if((Idx < ObjVars.Length) && Attachment == none)
    {
        Attachment = Actor(ObjVars[Idx]);
        C = Controller(Attachment);
        // End:0xA2
        if((C != none) && C.Pawn != none)
        {
            Attachment = C.Pawn;
        }
        // End:0x173
        if(Attachment != none)
        {
            // End:0xD4
            if(Action.bDetach)
            {
                Attachment.SetBase(none);
                // [Explicit Continue]
                goto J0x173;
            }
            C = Controller(self);
            // End:0x12D
            if((C != none) && C.Pawn != none)
            {
                C.Pawn.DoKismetAttachment(Attachment, Action);                
            }
            else
            {
                DoKismetAttachment(Attachment, Action);
            }
            // End:0x173
            if(Action.bUseTickGroup)
            {
                Attachment.SetTickGroup(Action.TickGroup);
            }
        }
        J0x173:

        Idx++;
        // [Loop Continue]
        goto J0x28;
    }
    //return;    
}

function DoKismetAttachment(Actor Attachment, SeqAct_AttachToActor Action)
{
    local bool bOldCollideActors, bOldBlockActors;
    local Vector X, Y, Z;

    Attachment.bIgnoreBaseRotation = Action.bIgnoreBaseRotation;
    Attachment.SetBase(none);
    Attachment.SetHardAttach(Action.bHardAttach);
    // End:0x1AD
    if(Action.bUseRelativeOffset || Action.bUseRelativeRotation)
    {
        bOldCollideActors = Attachment.bCollideActors;
        bOldBlockActors = Attachment.bBlockActors;
        Attachment.SetCollision(false, false);
        // End:0xEF
        if(Action.bUseRelativeRotation)
        {
            Attachment.SetRotation(Rotation + Action.RelativeRotation);
        }
        // End:0x193
        if(Action.bUseRelativeOffset)
        {
            GetAxes(Rotation, X, Y, Z);
            Attachment.SetLocation(((Location + (Action.RelativeOffset.X * X)) + (Action.RelativeOffset.Y * Y)) + (Action.RelativeOffset.Z * Z));
        }
        Attachment.SetCollision(bOldCollideActors, bOldBlockActors);
    }
    Attachment.SetBase(self);
    Attachment.ForceNetRelevant();
    Attachment.bNetDirty = true;
    // End:0x292
    if(Attachment.RemoteRole != ROLE_None && Attachment.bStatic || Attachment.bNoDelete)
    {
        Attachment.SetForcedInitialReplicatedProperty(StructProperty'Actor.RelativeLocation', Attachment.RelativeLocation == Attachment.default.RelativeLocation);
        Attachment.SetForcedInitialReplicatedProperty(StructProperty'Actor.RelativeRotation', Attachment.RelativeRotation == Attachment.default.RelativeRotation);
    }
    //return;    
}

simulated function OnMakeNoise(SeqAct_MakeNoise Action)
{
    MakeNoise(Action.Loudness, 'ScriptNoise');
    //return;    
}

event OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
    //return;    
}

event OnAnimPlay(AnimNodeSequence SeqNode)
{
    //return;    
}

event BeginAnimControl(array<AnimSet> InAnimSets)
{
    //return;    
}

event SetAnimPosition(name SlotName, int ChannelIndex, name InAnimSeqName, float InPosition, bool bFireNotifies, bool bLooping)
{
    //return;    
}

event SetAnimDirectorBoneName(name BoneName)
{
    //return;    
}

event FinishAnimControl()
{
    //return;    
}

event bool PlayActorFaceFXAnim(FaceFXAnimSet AnimSet, string GroupName, string SeqName, SoundCue SoundCueToPlay)
{
    //return ReturnValue;    
}

event StopActorFaceFXAnim()
{
    //return;    
}

event SetFaceFXRegister(string RegisterName, float Value, byte RegOp)
{
    //return;    
}

event SetMorphWeight(name MorphNodeName, float MorphWeight)
{
    //return;    
}

event SetSkelControlScale(name SkelControlName, float Scale)
{
    //return;    
}

simulated function bool IsActorPlayingFaceFXAnim()
{
    return false;
    //return ReturnValue;    
}

simulated function bool CanActorPlayFaceFXAnim()
{
    return true;
    //return ReturnValue;    
}

event FaceFXAsset GetActorFaceFXAsset()
{
    //return ReturnValue;    
}

function bool IsStationary()
{
    return true;
    //return ReturnValue;    
}

simulated event GetActorEyesViewPoint(out Vector out_Location, out Rotator out_Rotation)
{
    out_Location = Location;
    out_Rotation = Rotation;
    //return;    
}

// Export UActor::execIsPlayerOwned(FFrame&, void* const)
native simulated function bool IsPlayerOwned();

function PawnBaseDied()
{
    //return;    
}

// Export UActor::execGetTeamNum(FFrame&, void* const)
native simulated function byte GetTeamNum();

simulated event byte ScriptGetTeamNum()
{
    return 255;
    //return ReturnValue;    
}

simulated function string GetLocationStringFor(PlayerReplicationInfo PRI)
{
    return "";
    //return ReturnValue;    
}

simulated function NotifyLocalPlayerTeamReceived()
{
    //return;    
}

simulated function FindGoodEndView(PlayerController PC, out Rotator GoodRotation)
{
    GoodRotation = PC.Rotation;
    //return;    
}

// Export UActor::execGetTargetLocation(FFrame&, void* const)
native simulated function Vector GetTargetLocation(optional Actor RequestedBy, optional bool bRequestAlternateLoc);

// Export UActor::execGetFOVCheckLocation(FFrame&, void* const)
native simulated function Vector GetFOVCheckLocation();

event SpawnedByKismet()
{
    //return;    
}

function PickedUpBy(Pawn P)
{
    //return;    
}

simulated event InterpolationStarted(SeqAct_Interp InterpAction)
{
    //return;    
}

simulated event InterpolationFinished(SeqAct_Interp InterpAction)
{
    //return;    
}

simulated event InterpolationChanged(SeqAct_Interp InterpAction)
{
    //return;    
}

event RigidBodyCollision(PrimitiveComponent HitComponent, PrimitiveComponent OtherComponent, const out CollisionImpactData RigidCollisionData, int ContactIndex, float Speed, int Index0, int Index1)
{
    //return;    
}

event OnRanOver(SVehicle Vehicle, PrimitiveComponent RunOverComponent, int WheelIndex)
{
    //return;    
}

// Export UActor::execSetHUDLocation(FFrame&, void* const)
native simulated function SetHUDLocation(Vector NewHUDLocation);

// Export UActor::execNativePostRenderFor(FFrame&, void* const)
native simulated function NativePostRenderFor(PlayerController PC, Canvas Canvas, Vector CameraPosition, Vector CameraDir);

simulated event PostRenderFor(PlayerController PC, Canvas Canvas, Vector CameraPosition, Vector CameraDir)
{
    //return;    
}

simulated event RootMotionModeChanged(SkeletalMeshComponent SkelComp)
{
    //return;    
}

simulated event RootMotionExtracted(SkeletalMeshComponent SkelComp, out BoneAtom ExtractedRootMotionDelta)
{
    //return;    
}

event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
    //return;    
}

function OnToggleIsInteresting(SeqAct_Toggle ToggleAction)
{
    // End:0x28
    if(ToggleAction.InputLinks[0].bHasImpulse)
    {
        bIsPointOfInterest = true;
    }
    // End:0x50
    if(ToggleAction.InputLinks[1].bHasImpulse)
    {
        bIsPointOfInterest = false;
    }
    // End:0x80
    if(ToggleAction.InputLinks[2].bHasImpulse)
    {
        bIsPointOfInterest = !bIsPointOfInterest;
    }
    //return;    
}

event PostRestreamed()
{
    //return;    
}

event PreStreamOut()
{
    //return;    
}

function GetSoundInfo(out array<Thought> ThoughtList)
{
    //return;    
}

event bool LinkToActor(Actor LinkTarget)
{
    return false;
    //return ReturnValue;    
}

event bool UnlinkToActor(Actor LinkTarget)
{
    return false;
    //return ReturnValue;    
}

// Export UActor::execGetPackageGuid(FFrame&, void* const)
native static final function Guid GetPackageGuid(name PackageName);

simulated event OnRigidBodySpringOverextension(RB_BodyInstance BodyInstance)
{
    //return;    
}

// Export UActor::execIsInPersistentLevel(FFrame&, void* const)
native final function bool IsInPersistentLevel();

simulated function GetAimFrictionExtent(out float Width, out float Height, out Vector Center)
{
    // End:0x20
    if(bCanBeFrictionedTo)
    {
        GetBoundingCylinder(Width, Height);        
    }
    else
    {
        Width = 0.0000000;
        Height = 0.0000000;
    }
    Center = Location;
    //return;    
}

simulated function GetAimAdhesionExtent(out float Width, out float Height, out Vector Center)
{
    // End:0x20
    if(bCanBeAdheredTo)
    {
        GetBoundingCylinder(Width, Height);        
    }
    else
    {
        Width = 0.0000000;
        Height = 0.0000000;
    }
    Center = Location;
    //return;    
}

simulated function OnForceMaterialMipsResident(SeqAct_ForceMaterialMipsResident Action)
{
    local PlayerController PC;
    local int MaterialIdx;

    MaterialIdx = 0;
    J0x07:

    // End:0x82 [Loop If]
    if(MaterialIdx < Action.ForceMaterials.Length)
    {
        // End:0x77
        foreach WorldInfo.AllControllers(Class'PlayerController', PC)
        {
            PC.ClientSetForceMipLevelsToBeResident(Action.ForceMaterials[MaterialIdx], Action.ForceDuration);            
        }        
        MaterialIdx++;
        // [Loop Continue]
        goto J0x07;
    }
    //return;    
}

event PlayParticleEffect(const AnimNotify_PlayParticleEffect AnimNotifyData)
{
    //return;    
}

// Export UActor::execSupportsKismetModification(FFrame&, void* const)
native final function bool SupportsKismetModification(SequenceOp AskingOp, out string Reason);

simulated event AnimTreeUpdated(SkeletalMeshComponent SkelMesh)
{
    //return;    
}

simulated event PostDemoRewind()
{
    //return;    
}

function SetInvestigationArray(array<InvestigationData> NewInfo)
{
    InvestigationDataArray = NewInfo;
    //return;    
}

event SetInvestigateHighlighted(MaterialInstanceConstant highMat, bool On)
{
    //return;    
}

defaultproperties
{
    bLoadIfPhysXLevel0=true
    bLoadIfPhysXLevel1=true
    bLoadIfPhysXLevel2=true
    bPushedByEncroachers=true
    bRouteBeginPlayEvenIfStatic=true
    bCanStepUpOn=true
    bReplicateMovement=true
    bAllowFluidSurfaceInteraction=true
    bAutomaticPerformPhysics=true
    bMovable=true
    bJustTeleported=true
    bBatmanCanClimb=true
    bValidLineLauncherTarget=true
    bValidGelTarget=true
    DrawScale=1.0000000
    DrawScale3D=(X=1.0000000,Y=1.0000000,Z=1.0000000)
    CustomTimeDilation=1.0000000
    Role=ROLE_Authority
    ReplicatedCollisionType=None
    FramesTillInvestigateSightCheck=255
    SupportedEvents[0]=Class'SeqEvent_Touch'
    SupportedEvents[1]=Class'SeqEvent_Destroyed'
    SupportedEvents[2]=Class'SeqEvent_TakeDamage'
}
