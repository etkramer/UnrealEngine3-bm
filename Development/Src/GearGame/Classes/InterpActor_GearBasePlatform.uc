/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_GearBasePlatform extends InterpActor
	native
	notplaceable;

/** List of navigation points attached to this platform */
var array<NavigationPoint>	AttachedNavList;
/** List of pawns attached to this platform */
var array<GearPawn>			AttachedPawnList;
/** Enforce clamped base */
var() bool bClampedBaseEnabled;

/** Don't let riders change base, even to another platform. */
var() bool bAlwaysConfineToClampedBase;

/** Amount of damage to do to a non clamped pawn when Platform runs into them */
var() int	DamageToNonClampedPawnOnCollision;
/** Damage type to do when colliding */
var() class<GearDamageType>	DamageTypeOnCollision;

var() bool bDoComplexCameraCollision;

/** keep pawns from moving at all while based on this */
var bool bDisallowPawnMovement;

cpptext
{
	virtual UBOOL InStasis();
};

simulated function PostBeginPlay()
{
	local NavigationPoint Nav;
	local int Idx;

	Super.PostBeginPlay();

	foreach BasedActors( class'NavigationPoint', Nav )
	{
		// don't use coverslot markers because they're close to walls :)
		// don't use pickup factories because they aren't necessarily safe
		if(CoverSlotMarker(Nav) != none || GearPickupFactory(Nav) != none)
		{
			continue;
		}

		Idx = AttachedNavList.Find(Nav);
		if( Idx < 0 )
		{
			AttachedNavList[AttachedNavList.Length] = Nav;
		}
	}
}

native function FixBasedPawn( GearPawn GP );

/** Overridden to hide the wheels, too. */
simulated function OnToggleHidden(SeqAct_ToggleHidden Action)
{
	local Turret AttachedTurret;

	Super.OnToggleHidden(Action);

	// apply to any turrets as well
	foreach BasedActors(class'Turret', AttachedTurret)
	{
		AttachedTurret.SetHidden(bHidden);
	}
}

function ApplyCheckpointRecord(const out InterpActor.CheckpointRecord Record)
{
	local Turret AttachedTurret;

	Super.ApplyCheckpointRecord(Record);

	foreach BasedActors(class'Turret', AttachedTurret)
	{
		AttachedTurret.SetHidden(bHidden);
	}
}

simulated event InterpolationStarted(SeqAct_Interp InterpAction)
{
	Super.InterpolationStarted(InterpAction);

	if (Role == ROLE_Authority)
	{
		SetTimer(1.f, TRUE, nameof(ForceUpdateReplication));
		SetTimer(1.f, TRUE, nameof(CheckBasedPawns));
	}
}

simulated event InterpolationFinished(SeqAct_Interp InterpAction)
{
	Super.InterpolationFinished(InterpAction);

	if (Role == ROLE_Authority)
	{
		ClearTimer(nameof(ForceUpdateReplication));
		ClearTimer(nameof(CheckBasedPawns));
	}
}

/** looks for matinees controlling this mover and forces their ReplicatedActor to update
 * this is needed because some of the platform matinees are very long and due to all the crazy effects and such
 * tend to go out of sync, especially when the game hitches
 */
function ForceUpdateReplication()
{
	local int i;
	local SeqAct_Interp InterpAction;

	for (i = 0; i < LatentActions.length; i++)
	{
		InterpAction = SeqAct_Interp(LatentActions[i]);
		if (InterpAction != None && InterpAction.ReplicatedActor != None)
		{
			InterpAction.ReplicatedActor.Update();
		}
	}
}

event Attach(Actor Other)
{
	local GearPawn P;
	local int Idx;

	P = GearPawn(Other);
	if( P != None )
	{
		if( bClampedBaseEnabled )
		{
			if (bDisallowPawnMovement || !P.ConfineToClampedBase() || P.bJustTeleported || P.Physics == PHYS_RigidBody)
			{
//				`log( self@GetFuncName()@Other@"Assign clamped base" );

				P.ClampedBase = self;

				Idx = AttachedPawnList.Find(GearPawn(Other));
				if( Idx < 0 )
				{
					AttachedPawnList[AttachedPawnList.Length] = GearPawn(Other);
				}

				if( AttachedPawnList.Length > 0 && !IsTimerActive(nameof(CheckBasedPawns)) )
				{
					SetTimer(1.f, TRUE, nameof(CheckBasedPawns));
				}
			}
			else
			{
//				`log( self@GetFuncName()@"reject attachee"@Other );

				return;
			}
		}
	}
	else
	if( NavigationPoint(Other) != None && CoverSlotMarker(Other) == none && GearPickupFactory(Other) == none)
	{
		Idx = AttachedNavList.Find(Other);
		if( Idx < 0 )
		{
			AttachedNavList[AttachedNavList.Length] = NavigationPoint(Other);
		}
	}

	Super.Attach(Other);
}

native function bool IsStillOnClampedBase(GearPawn GP, vector PawnExtent);
function CheckBasedPawns()
{
	local int PawnIdx;
	local GearPawn GP;

	if( bClampedBaseEnabled )
	{
		// For each attached pawn
		for( PawnIdx = 0; PawnIdx < AttachedPawnList.Length; PawnIdx++ )
		{
			GP = AttachedPawnList[PawnIdx];

			//debug
			`log( self@GetFuncName()@GP@GP.ClampedBase@GP.SpecialMove, bDebug );

			if( GP == None || (GP.ClampedBase != None && GP.ClampedBase != self))
			{
				AttachedPawnList.Remove( PawnIdx--, 1 );
				continue;
			}

			if( !GP.IsDoingASpecialMove() && !(!GP.IsDBNO() && GP.Health <=0))
			{
				if(!IsStillOnClampedBase(GP,GP.GetCollisionExtent()))
				{
					//DebugFreezeGame();
					FixBasedPawn( GP );
					if (bDisallowPawnMovement)
					{
						GP.SetPhysics(PHYS_None);
						GP.bCollideWorld = false;
						GP.SetCollision(GP.bCollideActors, GP.bBlockActors, true);
					}
				}
			}
		}
	}

	if( AttachedPawnList.Length == 0 )
	{
		ClearTimer( nameof(CheckBasedPawns) );
	}
}

function DoKismetAttachment(Actor Attachment, SeqAct_AttachToActor Action)
{
	if (bDisallowPawnMovement && GearPawn(Attachment) != None)
	{
		Attachment.SetPhysics(PHYS_None);
		Attachment.bCollideWorld = false;
		Attachment.SetCollision(Attachment.bCollideActors, Attachment.bBlockActors, true);
	}

	Super.DoKismetAttachment(Attachment, Action);
}

function OnToggle( SeqAct_Toggle inAction )
{
	`log( self@GetFuncName()@inAction );

	if( inAction.InputLinks[0].bHasImpulse )
	{
		SetEnabled( TRUE );
	}
	else
	if( inAction.InputLinks[1].bHasImpulse )
	{
		SetEnabled( FALSE );
	}
	else
	{
		SetEnabled( !bClampedBaseEnabled );
	}
}

function SetEnabled( bool inbEnabled )
{
	local GearPawn GP;

	bClampedBaseEnabled = inbEnabled;

	foreach WorldInfo.AllPawns( class'GearPawn', GP )
	{
		if( bClampedBaseEnabled )
		{
			if( GP.Base == self )
			{
				Attach( GP );
			}
		}
		else
		{
			if( GP.ClampedBase == self )
			{
//				`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "Pawn:" @ GP @ "clampedbase cleared" );

				GP.ClampedBase = None;
			}
		}
	}
}

event RanInto( Actor Other )
{
	local GearPawn GP;
	if( DamageToNonClampedPawnOnCollision > 0 )
	{
		GP = GearPawn(Other);
		if( GP != None && GP.Base != None && GP.ClampedBase != self )
		{
			GP.TakeDamage( DamageToNonClampedPawnOnCollision, None, GP.Location, Velocity, DamageTypeOnCollision,, self );
		}
	}
}


defaultproperties
{
	bClampedBaseEnabled=TRUE
	bDoComplexCameraCollision=TRUE
}
