/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_RockWormBase extends GearPawn
	native(Pawn)
	config(Pawn);


// temporary until we get a skeletal mesh
var RockWorm_TailSegment TailSegmentList;
var RockWorm_TailSegment LastTailSegment;
const NUM_TAILSEGMENTS = 15;
const COVER_TO_SEGMENTS_RATIO = 2;

/** offset from the 'head' of the rockworm to the first tail segment behind it **/
var() vector InitialSegmentOffset;

/** offset from each segment to the cover associated with it (this is inverted and used for both left/right) **/
var() vector CoverToSegmentOffset;

/** Link for left side cover chain **/
var transient CoverLink_Dynamic LeftCover;

/** Link for right side cover chain **/
var transient CoverLink_Dynamic RightCover;

/** Position the head tail segment was last time we updated the cover **/
var transient vector LastCoverUpdatePos;

/** a list of reachspecs the rockworm is currently blocking **/
var transient array<ReachSpec> ReachSpecsImBlocking;

/** the minimum dot to consider a pathnode reachable (to restrict the rockworm from doing 180 degree turns, etc..) **/
var() float MinDotReachable;

/** ground speed to use when 'angry' (e.g. after someon shot me in the bum) **/
var config float AngryGroundSpeed;
var config float AngryGroundSpeedDuration;

var class<RockWorm_TailSegment> TailSegmentClass;

var transient Pawn ChompVictim;

var name RearPivotBoneName;

/** sound that plays when the bloodmount does his melee attack */
var SoundCue AttackSound;

/** the time it takes the velocity of one segment to reach the velocity of the next segment (locations are set, so this only effects stuff like animation that looks at animation directly) */
var float TailVelocityDelay;

/** used to clamp rotation relative to the tail (so we can't rotate into the tail) */
var int MinAngleToTail;

/** cached ref to scout */
var transient scout Scout;

replication
{
	if(ROLE==ROLE_Authority)
		TailSegmentList,LastTailSegment,LeftCover,RightCover;
}

cpptext
{
	
	virtual UBOOL IgnoreBlockingBy( const AActor *Other ) const;

	// tail related functions
	void UpdateTail(FLOAT DeltaTime);
	void AdjustCoverSlotPositionAlongTail(ACoverLink* Link, const FVector& LinkLocation, ARockWorm_TailSegment* TailSegment, INT SlotIdx);
	void SetCoverSlotLocFromOffset(ACoverLink* Link, const FVector& LinkLocation, INT SlotIdx, const FVector& Offset, AActor* Parent, FName ParentBoneName = NAME_None);
	void UpdateCoverSlots();
	virtual void TickAuthoritative( FLOAT DeltaSeconds);
	virtual void TickSimulated( FLOAT DeltaSeconds);

	// overidden to invalidate actors outside our reachable angle
	virtual int actorReachable(AActor *Other, UBOOL bKnowVisible=0, UBOOL bNoAnchorCheck=0);

	// overidden to disallow anchors behind us (in the tail)
	virtual UBOOL	IsValidAnchor( ANavigationPoint* AnchorCandidate );

	UBOOL IsActorBlockedByTail( AActor* Actor );

	// overidden to disallow rotating into the tail
	virtual void physicsRotation(FLOAT deltaTime, FVector OldVelocity);
	// overidden to return when we hit our rotation clamps
	virtual UBOOL ReachedDesiredRotation();


}
native simulated function UpdateBlockedReachSpecs();

native function vector GetParentAttachPos(Actor Parent, Name ParentBoneName='');

native private function AddSlotToSlotSpecs();

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if(ROLE == ROLE_Authority)
	{
		SpawnTailSegments();
		LastCoverUpdatePos = TailSegmentList.Location;
		UpdateBlockedReachSpecs();
		AddDefaultInventory();
	}
}

/**
* Overidden to make him invulnerable!
*
* @see Actor::TakeDamage
*/
function TakeDamage
(
 int					Damage,
 Controller			InstigatedBy,
 Vector				HitLocation,
 Vector				Momentum,
class<DamageType>	DamageType,
	optional	TraceHitInfo		HitInfo,
	optional	Actor				DamageCauser
	)
{
	local class<GearDamageType> GDT;
	
	GDT = class<GearDamageType>(DamageType);
	// play any associated fx/sounds
	DoDamageEffects(Damage, (InstigatedBy != None) ? InstigatedBy.Pawn : None, HitLocation, GDT, Momentum, HitInfo);

	// allow the damage type to add any extra fx/etc
	GDT.static.HandleDamagedPawn(self, (InstigatedBy != None) ? InstigatedBy.Pawn : None, Damage, Momentum);

	Damage = 0;

	Super.TakeDamage(Damage,InstigatedBy,HitLocation,Momentum,DamageType,HitInfo,DamageCauser);
}


function SpawnTailSegments()
{
	local int i;
	local RockWorm_TailSegment CurSegment;
	local Actor PreviousSegment;

	PreviousSegment = self;

	for(i=0;i<NUM_TAILSEGMENTS;i++)
	{
		CurSegment = Spawn(TailSegmentClass);
		CurSegment.WormOwner=self;
		CurSegment.SetupMesh(i+1==NUM_TAILSEGMENTS);		
		if(TailSegmentList == none)
		{
			TailSegmentList = CurSegment;
		}

		CurSegment.AttachToChain(PreviousSegment);

		PreviousSegment = CurSegment;
	}

	LastTailSegment = CurSegment;

	CreateCoverForTail();
}
function CreateCoverForTail()
{
	local RockWorm_TailSegment CurrentSegment;
	local Vector Offset;
	local vector SlotLoc;
	local CoverSlotMarker_Rockworm NewMarker;
	local vector ParentAttachPos;
	local int leftSideIndex;
	local int Count;

	// create left first
	LeftCover = CreateCoverLink(vect(0,0,0));
	// add slots for left side
	CurrentSegment = TailSegmentList;
	Offset = CoverToSegmentOffset;
	Offset.Y *= -1.0f;
	Count=0;
	while(CurrentSegment != none)
	{
		if(Count++ % COVER_TO_SEGMENTS_RATIO == 0)
		{
			ParentAttachPos = GetParentAttachPos(CurrentSegment);
			SlotLoc = ParentAttachPos + (Offset >> CurrentSegment.Rotation);
			ParentAttachPos.Z = SlotLoc.Z;
			CreateCoverSlotMarker(CoverLink_Spawnable(LeftCover),SlotLoc,rotator(ParentAttachPos - SlotLoc),CurrentSegment);
		}
		CurrentSegment = CurrentSegment.NextSegment;
	}

	// add slots for right side (using list generated for left, so we maintain left->right order
	RightCover = CreateCoverLink(vect(0,0,0));	
	CurrentSegment = LastTailSegment;
	leftSideIndex = LeftCover.Slots.length-1;
	Count = 0;
	while(CurrentSegment != none)
	{
		if(Count++ % COVER_TO_SEGMENTS_RATIO == 0)
		{
			ParentAttachPos = GetParentAttachPos(CurrentSegment);
			SlotLoc = ParentAttachPos + (CoverToSegmentOffset >> CurrentSegment.Rotation);
			ParentAttachPos.Z = SlotLoc.Z;
			NewMarker = CreateCoverSlotMarker(CoverLink_Spawnable(RightCover),SlotLoc,rotator(ParentAttachPos - SlotLoc),CurrentSegment);
			// set up mantle shit
			// right to left
			RightCover.Slots[NewMarker.OwningSlot.SlotIdx].MantleTarget.Actor = LeftCover;
			RightCover.Slots[NewMarker.OwningSlot.SlotIdx].MantleTarget.SlotIdx = leftSideIndex;
			// left to right
			LeftCover.Slots[leftSideIndex].MantleTarget.Actor = RightCover;
			LeftCover.Slots[leftSideIndex].MantleTarget.SlotIdx = NewMarker.OwningSlot.SlotIdx;

			NewMarker.InitializeDynamicMantleSpec(class'MantleReachSpec_Rockworm');
			CoverSlotMarker_Rockworm(LeftCover.Slots[leftSideIndex].SlotMarker).InitializeDynamicMantleSpec(class'MantleReachSpec_Rockworm');
			UpdateMarker(LeftCover,leftSideIndex);
			UpdateMarker(RightCover,NewMarker.OwningSlot.SlotIdx);
			leftSideIndex--;

		}
		CurrentSegment = CurrentSegment.PrevSegment;
		
	}

	AddSlotToSlotSpecs();
}


function CoverSlotMarker_Rockworm CreateCoverSlotMarker(CoverLink_Spawnable ParentLink,  vector SlotLocation, rotator SlotRotation, Actor AttachParent, optional Name AttachParentBoneName)
{
	local CoverSlotMarker_Rockworm NewMarker;
	local CoverSlot Slot;
	local int SlotIdx;



	// create and add a slot
	Slot.ForceCoverType = CT_MidLevel;
	Slot.CoverType = CT_MidLevel;
	Slot.bLeanLeft = FALSE;
	Slot.bLeanRight = FALSE;
	Slot.bCanPopUp = TRUE;
	Slot.bCanMantle = TRUE;
	Slot.bAllowMantle = TRUE;
	Slot.bCanCoverSlip_Left = FALSE;
	Slot.bCanCoverSlip_Right = FALSE;
	Slot.bCanSwatTurn_Left = FALSE;
	Slot.bCanSwatTurn_Right = FALSE;
	Slot.bEnabled = TRUE;
	Slot.LocationOffset = (SlotLocation - ParentLink.Location) << ParentLink.Rotation;
	Slot.RotationOffset = Normalize(SlotRotation - ParentLink.Rotation);

	SlotIdx = ParentLink.Slots.length;
	ParentLink.Slots[SlotIdx] = Slot;
	// setup a marker
	NewMarker = Spawn(class'CoverSlotMarker_Rockworm',,,SlotLocation,SlotRotation,,TRUE);
	if(NewMarker == none)
	{
		`log(GetFuncName()@"NewMarker was NONE!");
	}
	ParentLink.Slots[SlotIdx].SlotMarker = NewMarker;
	NewMarker.OwningSlot.Link = ParentLink;
	NewMarker.OwningSlot.SlotIdx = SlotIdx;
	NewMarker.SetCoverInfo(ParentLink,SlotIdx,Slot);
	NewMarker.SetMaxPathSize(class'GearPawn'.default.CylinderComponent.CollisionRadius,class'GearPawn'.default.CylinderComponent.CollisionHeight);	
	ParentLink.UpdateCoverSlot(SlotIdx);

	return NewMarker;
}

function CoverLink_Spawnable CreateCoverLink(vector SpawnLocOffset)
{
	local CoverLink_Spawnable NewLink;

	local vector SpawnLoc;
	local vector LocWithoutZOffset;

	SpawnLoc = Location + SpawnLocOffset;
	LocWithoutZOffset = SpawnLoc;
	LocWithoutZOffset.z = Location.z;

	// pointing toward the center
	NewLink = Spawn(class'CoverLink_Spawnable',,,SpawnLoc,Rotator(Location - LocWithoutZOffset),,TRUE);

	// add the link the navigation network so we can take cover on it later
	NewLink.bAddToPathNetwork = TRUE;
	NewLink.UpdateCoverLink();
	NewLink.Slots.Length = 0;
	//NewLink.bDebug = true;
	NewLink.bBlocked=true;
	return NewLink;
}

simulated event Destroyed()
{
	local RockWorm_TailSegment CurSegment,NextSeg;

	// clean up the tail
	CurSegment = TailSegmentList;
	while(CurSegment!=none)
	{
		NextSeg = CurSegment.NextSegment;
		CurSegment.Destroy();
		CurSegment=NextSeg;
	}

	LeftCover.Destroy();
	RightCover.Destroy();

	Scout = none;

	Super.Destroyed();
}


event UpdateMarker(CoverLink_Dynamic Link, int SlotIdx)
{
	local CoverSlot Slot;
	local CoverSlotMarker_Rockworm Marker;

	Slot = Link.Slots[SlotIdx];
	Marker = CoverSlotMarker_Rockworm(Slot.SlotMarker);
	Marker.SetCoverInfo(Link,SlotIdx,Slot);
	Marker.OctreeUpdate();
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

function OuchYouShotMyBum()
{
	GroundSpeed = AngryGroundSpeed;
	SetTimer( AngryGroundSpeedDuration,false,nameof(ImNotAngryAnyMore) );
}

function ImNotAngryAnyMore()
{
	GroundSpeed = DefaultGroundSpeed;
}


function PlayAttackSound()
{
	PlaySound(AttackSound);
}

function PlaySecondAttackSound();

function DoChompDamage()
{
	local GearWeap_RockWormMelee	Wpn;

	if( IsDoingSpecialMove(GSM_SwipeAttack) )
	{
		PlayAttackSound();
		SetTimer( 0.25f,FALSE,nameof(PlaySecondAttackSound) );
		Wpn = GearWeap_RockWormMelee(Weapon);
		//`log(GetFuncName()@Wpn@Weapon);
		if( Wpn != None )
		{
			Wpn.DoMeleeDamage( ChompVictim,ChompVictim.Location, 1.f );
		}
	}
}

// fruit sound stubs
function MovingToFruit(Rockworm_FruitBase Fruit);
function ArrivedAtFruit(Rockworm_FruitBase Fruit);

defaultproperties
{
	InitialSegmentOffset=(x=-100,z=-10)
	CoverToSegmentOffset=(x=0.f,y=110.f,z=58.f)
	//bCanStrafe=FALSE
	ControllerClass=class'GearGame.GearAI_RockWorm'


	Begin Object Name=CollisionCylinder
		CollisionHeight=+0024.000000
	End Object
	bIgnoreEncroachers=true

	Begin Object Class=CylinderComponent Name=CollisionCylinder2
		CollisionHeight=+0048.000000
		CollisionRadius=+0065.000000	
		BlockNonZeroExtent=true
		BlockZeroExtent=true
		BlockActors=false
		CollideActors=true
		Translation=(x=120)
		AlwaysCheckCollision=true
	End Object
	Components.Add(CollisionCylinder2)
	MinDotReachable=-1.f

	TailSegmentClass=class'RockWorm_TailSegment'
	bCanDBNO=false

	SpecialMoveClasses(GSM_SwipeAttack)		=class'GSM_Rockworm_Headchomp'
	SpecialMoveClasses(SM_PushObject)		=class'GSM_Rockworm_EatThatFruit'

	DefaultInventory(0)=class'GearWeap_RockWormMelee'
	TailVelocityDelay=0.1f

	TurningRadius=128.f
	MinAngleToTail=12740
}
