/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *	Used for AI that can interact with/use cover
 */
class GearAI_Cover extends GearAI
	native(AI)
	dependson(GearPawn)
	config(AI);

/** GoW global macros */

const TIMER_ADJUSTTOSLOT_LIMIT = 5.f;

cpptext
{
	virtual void ClearCrossLevelPaths(ULevel *Level);

	DECLARE_FUNCTION(execPollAdjustToSlot);
	DECLARE_FUNCTION(execPollMoveToward);
}

enum ECoverSearchType
{
	SearchType_Towards,
	SearchType_Away,
	SearchType_Near,
	SearchType_Ambush,
	SearchType_AlongPath,
};

/** Currently claimed cover */
var			CoverInfo		Cover;
/** Cover we are moving to */
var			CoverInfo		CoverGoal;
/** Last claimed cover */
var			CoverInfo		LastCover;
/** Time that we entered most recent cover */
var			float			EnterCoverTime;

/** Min amount of time to stay in uncompromised cover */
var	config	float			MinStayAtCoverTime;
/** Action that will be taken after transition anim is complete */
var			FireLinkItem	PendingFireLinkItem;
var			ECoverType		OldCoverType;
/** No ranged weapons so ignore fire links in cover eval */
var			bool			bIgnoreFireLinks;
/** Last time we attempted to run to cover */
var			float			LastRun2CoverTime;
/** Flag set to indicate we should start a run 2 cover */
var			bool			bShouldRun2Cover;
/** Last time we tried to fire from cover */
var			float			LastFireFromCoverTime;
/** last time we changed cover (called AICmd_MoveToCover with something different than current cover) */
var float LastChangeCoverTime;
/** Ideal/Min/Max distance to search for cover when flooding the network */
var			float			DesiredCoverMoveDist,
							MinCoverMoveDist,
							MaxCoverMoveDist;
/** Failsafe time used to prevent getting stuck in AdjustToSlot */
var			float			AdjustToSlotTime;
/** Used for cover searching via FindCover */
var transient bool			bSkipMinCoverDistCheck,
							bSkipMaxCoverDistCheck;
/** Marker AI is running to */
var transient CoverSlotMarker Run2CoverMarker;
/** Next time this AI is allowed to peek */
var			float			NextAllowPeekTime;


/** Last time we played a meat shield reaction **/
var float LastMeatShieldReactionTime;
/** Minimum amount of time in between meat shield reactions **/
var() config float MeatShield_Reaction_MinTimeInterval;
/** Chance that we will react to a given meat shield event **/
var() config float MeatShield_Reaction_Chance;
/** Time after noticing meat shield to play the reaction **/
var() config float MeatShield_Reaction_Delay;
var GearPawn MeatShield_Reaction_Target;

/** for SP only - inventory the LD has given the AI the OK to pick up (AI ignores all other pickups) */
var array< class<GearWeapon> > SPItemsToConsider;

/** default AtCover goal evaluater profiles */
var instanced Goal_AtCover AtCover_Toward;
var instanced Goal_AtCover AtCover_Away;
var instanced Goal_AtCover AtCover_Near;
var instanced Goal_AtCover AtCover_Squad;
var instanced Goal_AtCover AtCover_Ambush;
var instanced Goal_AtCover AtCover_AlongPath;

/** list of cover links this AI is allowed to use for cover (if blank all cover is fair game) */
var array<CoverLink> AllowedCoverLinks;

/** template for instancing stopping power reaction - should not be attached itself */
var AIReactCond_StoppingPowerThreshold StoppingPowerReactionTemplate;

// next time we setcovertype, don't pushoutofcover (will be reset once we successfully leave cover)
var bool bLeaveCoverFast;

// forces log output (even in FINAL_RELEASE) use with caution!
native final function ForceLog(coerce string s);

event Possess(Pawn NewPawn, bool bVehicleTransition)
{
	local AIReactCond_StoppingPowerThreshold ReactCond;

	Super.Possess( NewPawn, bVehicleTransition );

	//`AILog(self@MyGearPawn@MyGearPawn.bAllowStoppingPower);
	//`log(self@MyGearPawn@MyGearPawn.bAllowStoppingPower);

	if( MyGearPawn != None )
	{
		// Only let AI slide half the distance to cover
		MyGearPawn.Run2CoverMaxDist	= MyGearPawn.default.Run2CoverMaxDist * 0.5f;


		ReactCond = AIReactCond_StoppingPowerThreshold(ReactionManager.FindReactionByType(class'AIReactCond_StoppingPowerThreshold', false));

		// if our pawn supports stopping power, push on a stopping power threshold reaction so we evade when we get too much stopping power
		if(MyGearPawn.bAllowStoppingPower && ReactCond == none)
		{
			`AILog("My pawn supports stopping power, pushing stopping power evade reaction");
			ReactCond = new(self) class'AIReactCond_StoppingPowerThreshold'(StoppingPowerReactionTemplate);
			ReactCond.bAlwaysNotify = true;
			ReactCond.Initialize();
		}
		else if(ReactCond != none) // otherwise we just possessed a pawn that doesn't support it, and we still have a reaction so clean it up
		{
			ReactCond.UnsubscribeAll();
			ReactCond = none;
		}
	}
}


function PawnDied(Pawn InPawn)
{
	if( InPawn == Pawn )
	{
		// remove any cover claims
		UnClaimCover();
	}

	Super.PawnDied( InPawn );
}

function bool StepAsideFor(Pawn ChkPawn)
{
	if( HasValidCover() && IsAtCover() )
	{
		if( MyGearPawn.CurrentLink != none && MyGearPawn.CurrentLink.bDynamicCover )
		{
			`AILog( "- in dynamic cover (ie derrick)... change cover" );

			InvalidateCover();
			return Super.StepAsideFor( ChkPawn );
		}
		else
		{
			`AILog("- in cover, ignoring StepAsideFor request");
			return FALSE;
		}
	}

	return Super.StepAsideFor( ChkPawn );
}


/*
 *	Choose cover in available cover zones
 */
event bool MoveToCombatZone()
{
	SetAcquireCover( ACT_Immediate, 'MoveToCombatZone' );
	return Super.MoveToCombatZone();
}

event SetCurrentCombatZone( CombatZone NewCZ )
{
	local Actor		AmbushTarget;
	local CoverInfo	CovInfo;

	//debug
	`AILog( GetFuncName()@NewCZ );

	Super.SetCurrentCombatZone( NewCZ );

	if( CurrentCombatZone == None )
		return;

	//debug
	`AILog( "..."@CurrentCombatZone.ZoneType@CurrentCombatZone.AmbushTargets.length );

	if( CurrentCombatZone.ZoneType == CZT_Ambush )
	{
		if( CurrentCombatZone.AmbushTargets.Length < 0 || (MoveAction != none && !MoveAction.bInterruptable))
			return;

		AmbushTarget = CurrentCombatZone.AmbushTargets[Rand(CurrentCombatZone.AmbushTargets.Length)];
		if( EvaluateCover( SearchType_Ambush, AmbushTarget, CovInfo ) )
		{
			SetCombatMood( AICM_Ambush );
			SetCoverGoal( CovInfo, TRUE );
		}
	}
}

function FireFromCover()
{
	class'AICmd_Attack_FireFromCover'.static.FireFromCover( self );
}

function bool PeekFromCover( optional bool bLookForEnemy = TRUE, optional float PeekTime=0.f )
{
	if (WorldInfo.TimeSeconds > NextAllowPeekTime && MyGearPawn != None && !MyGearPawn.IsCarryingAHeavyWeapon())
	{
		return class'AICmd_Attack_PeekFromCover'.static.PeekFromCover( self, bLookForEnemy, PeekTime );
	}
	bFailedToFireFromCover = TRUE;
	return FALSE;
}

function bool HasAnyEnemiesRequiringCover()
{
	local Pawn P;
	if( Squad != None )
	{
		foreach Squad.AllEnemies( class'Pawn', P )
		{
			if( !P.IsA( 'GearPawn_LocustNemaslug' ) )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

protected function FireAtForcedTarget()
{
	Focus = FireTarget;
	StartFiring();
	if (MyGearPawn != None && MyGearPawn.IsInCover())
	{
		FireFromCover();
	}
	else
	{
		FireFromOpen();
	}
}

protected function StopForceFiring()
{
	SelectTarget();
	StopFiring();
	AbortCommand(none,class'AICmd_Attack_FireFromCover');
}

protected native function bool IsLookingAtWall();

// try to face something that's not the wall
function IdleDontFaceWalls()
{
	local Actor SquadLeaderPos;
	local bool bFoundFaceTarget;
	local GearPC Player;

	if( Focus != None && NavigationPoint(Focus) == None )
	{
		`AILog( GetFuncName()@"... already looking at"@Focus );
		return;
	}

	// early out if we're not looking at a wall
	if((Pawn.Base != None && ClassIsChildOf( Pawn.Base.Class, class'InterpActor_GearBasePlatform' )) || !IsLookingAtWall())
	{
		`AILog(GetFuncName()@"...Wasn't looking at wall, bailing");
		return;
	}

	// if we have a squad position, face it
	if(Squad != none)
	{
		// sometiems look at the player anyway, otherwise look at squad position
		if(FRand() < 0.5f && Squad.Leader != self || Squad.SquadRoute != none)
		{
			// face squad position
			SquadLeaderPos = GetSquadLeaderPosition();
			if(SquadLeaderPos != none)
			{
				`AILog("Facing Squad Position");
				Focus=SquadLeaderPos;
				bFoundFaceTarget=true;
			}
		}
		// if we're in a player's squad, face the player
		else if(Squad.bPlayerSquad)
		{
			foreach Squad.AllMembers(class'GearPC',Player)
			{
				if(VSizeSq(Player.Pawn.Location - Pawn.Location) < 1024.f*1024.f)
				{
					`AIlog("Facing player in squad:"@Player);
					Focus = Player.Pawn;
					bFoundFaceTarget=true;
					break;
				}
			}
		}
	}


	// if we still didn't find anything, look for a player in the area
	if(!bFoundFaceTarget)
	{
		foreach LocalPlayerControllers(class'GearPC', Player)
		{
			`AIlog("Fallback, facing player:"@Player);
			Focus=Player.Pawn;
			bFoundFaceTarget=true;
			break;
		}

	}
}

///////////////////////////////////////////////
//////////////// ACTION STATES ////////////////
///////////////////////////////////////////////

state Action_Idle
{
Begin:
	//debug
	`AILog( "BEGIN TAG"@TetherActor, 'State' );


	// Select our weapon
	if( SelectWeapon() )
	{
		`AILog("Selected weapon");
		Sleep( 0.25f );
	}

	CheckReviveOrExecute(1200.0); // player might get downed by scripted stuff while out of combat, or combat just recently ended

	// check for a tether
	if( (Pawn != None) && !IsWithinTether( Pawn.Location ) )
	{
		//debug
		`AILog( "Not within tether, moving" );

		if(MoveAction != none && MoveAction.TetherDistance == 0)
		{
			OnAIMove(MoveAction);
		}
		else
		{
			// Move to tether
			SetTether( TetherActor, DesiredTetherDistance, bDynamicTether, TetherPersistDuration );
		}
	}
	else
	// If we have enemies
	if( HasAnyEnemies() )
	{
		// Try to notice one which will put us into combat
		CheckNoticedEnemy();
		Sleep(1.f);
	}
	else
	if( IsSquadLeader() && Squad.SquadRoute != None )
	{
		// if this isn't the player squad, just run straight to the end of the route
		`AILog("I'm the squad leader! Squad.bPlayerSquad: "$Squad.bPlayerSquad@Squad.GetSquadLeaderPosition());
		if(!Squad.bPlayerSquad)
		{
			SetRouteMoveGoal( Squad.SquadRoute );
		}
		else if(ShouldMoveToSquad()) // otherwise we need to move up with the player, so call getsquadleaderposition
		{
			SetMoveGoal(Squad.GetSquadLeaderPosition());
			if(bReachedMoveGoal)
			{
				LastSquadLeaderPosition = MoveGoal.Location;
			}
		}
		else
		{
			sleep(2.f);
		}
	}
	else
	// otherwise if we have no enemies but do have a squad
	if ( ShouldMoveToSquad() )
	{
MoveToSquad:
		//debug
		`AILog("Moving to squad...");

		// then move to the squad
		MoveToSquadPosition();

		// If no tether actor to clear
		if( bFailedToMoveToEnemy )
		{
			// Sleep to prevent recursive craziness
			Sleep( 0.25f );
		}

		// If we shouldn't keep the tether
		if( !bKeepTether )
		{
			// Clear the tether so that we always update to a new one
			TetherActor = None;
		}
		bKeepTether = FALSE;

		// check to see if we should immediately move again
		if (!HasAnyEnemies() &&
			ShouldMoveToSquad())
		{
			//debug
			`AILog("Continuing move to squad");
			Sleep(0.1f);
			Goto('MoveToSquad');
		}
		else
		{
			`AILog("Not continuing move to squad?"@HasAnyEnemies()@ShouldMoveToSquad());
		}
		Sleep(0.25f);
	}
	else
	{
		// check to see if we should get into cover because of our squad leader
		if (TetherActor == none && IsAlert() &&
			(!HasValidCover() || !IsAtCover()))
		{
			//debug
			`AILog("Moving to cover because of alertness or squad leader being in cover"@IsAlert()@HasValidCover()@IsAtCover());

			ClearMovementInfo();
			MoveToNearestCoverIdle();
			if (!bReachedMoveGoal)
			{
				`AILog("No nearby cover or failed to reach cover, sleeping");
				Sleep(1.f);
			}
		}
		else
		if( IsAlert() &&
			IsAtCover() )
		{
			PeekFromCover();
			if( bFailedToFireFromCover )
			{
				Sleep( 0.5f );
			}
		}
		else if (bIdleMoveToNearestEnemy)
		{
			MoveToNearestEnemy();
			Sleep(1.0);
		}
		else
		{
			//debug
			`AILog("Shouldn't move to squad or cover, sleeping"@TetherActor@MoveAction);

			Sleep(1.f);
		}
	}
	IdleDontFaceWalls();
	Goto('Begin');
}

//////////////////////////////////////////////////
//////////////// SUBACTION STATES ////////////////
//////////////////////////////////////////////////
/**
 * Checks to see if we can run to cover, if TestMoveTarget is a slot marker within range basically.
 */
final event bool CanRun2Cover()
{
	local CoverSlotMarker	Marker;

	Marker = CoverSlotMarker(MoveTarget);

	if(!IsAlert() || !IsInCombat())
	{
		return FALSE;
	}

	// If Pawn cannot do the run2cover, then don't go any further
	if (!MyGearPawn.bCanDoRun2Cover || Marker == None || MyGearPawn.IsAKidnapper() || MyGearPawn.IsCarryingShield() || MyGearPawn.IsCarryingAHeavyWeapon())
	{
		return FALSE;
	}

	// if it is our covergoal or current cover, and
	// if not already in cover, and
	// it is within range, and
	// is roughly in front of us, and
	// we're roughly facing it
	if ((Marker.OwningSlot == CoverGoal || Marker.OwningSlot == Cover) &&
		!MyGearPawn.IsInCover() &&
		VSize(Pawn.Location - Marker.Location) <= MyGearPawn.Run2CoverMaxDist &&
		Normal(Marker.Location - Pawn.Location) dot vector(Marker.GetSlotRotation()) > 0.3f &&
		vector(Pawn.Rotation) dot vector(Marker.GetSlotRotation()) > 0.3f)
	{
		return TRUE;
	}
	return FALSE;
}

function NotifyFlankedByEnemy( ESpecialMove SM )
{
	class'AICmd_React_Flanked'.static.Flanked( self, SM );
}

function HandleFlankReaction()
{
}

final function bool ShouldReactToMeatShield(GearPawn MSKidnapper)
{
	return (WorldInfo.TimeSeconds - LastMeatShieldReactionTime > MeatShield_Reaction_MinTimeInterval && FRand() < MeatShield_Reaction_Chance);
}

function NotifyEnemyUsingMeatShield(GearPawn MSKidnapper)
{
	`AILog(GetFuncName()@Enemy);
	if( ShouldReactToMeatShield(MSKidnapper) )
	{
		MeatShield_Reaction_Target = MSKidnapper;
		SetTimer( MeatShield_Reaction_Delay, false, nameof(PlayMeatShieldReaction) );
	}
}

final function PlayMeatShieldReaction()
{
	// make sure we should still react
	if(WorldInfo.TimeSeconds - LastMeatShieldReactionTime < MeatShield_Reaction_MinTimeInterval ||
	   !MeatShield_Reaction_Target.IsAKidnapper() ||
	   !IsPawnVisibleViaTrace(MeatShield_Reaction_Target)
	   )
	{
		return;
	}

	// tell our squad we're reacting to this MS
	if(Squad != none)
	{
		Squad.NotifyReactingToMeatShield( MyGearPawn, MeatShield_Reaction_Target );
	}
	else
	{
		LastMeatShieldReactionTime = WorldInfo.TimeSeconds;
	}

	class'AICmd_ReactMeatshield'.static.React(self, MeatShield_Reaction_Target);
}

function NotifyKnockDownStart()
{
	class'AICmd_React_KnockDown'.static.InitCommand(self);
}

/////////////////////////////////////////////////
//////////////// COVER FUNCTIONS ////////////////
/////////////////////////////////////////////////

function ClearMovementInfo( optional bool bSkipCombatZones = TRUE)
{
	Super.ClearMovementInfo( bSkipCombatZones );

	ClearCoverGoal();
}

function ClearCoverGoal()
{
	CoverGoal.Link		= None;
	CoverGoal.SlotIdx	= -1;
}

/**
 * Sets a cover goal, pathing to it if necessary.
 */
final function SetCoverGoal( CoverInfo NewCoverGoal, optional bool bIsValidCache )
{
	//debug
	`AILog( GetFuncName()@NewCoverGoal.Link@NewCoverGoal.SlotIdx, 'Cover' );

	class'AICmd_MoveToCover'.static.MoveToCover( self, NewCoverGoal, bIsValidCache, MoveIsInterruptable() );
}

/**
* Handle scripted AI movement, figures out what the right thing to do is
* based on the move targets.
*/
function OnAIMove(SeqAct_AIMove Action)
{
	// always clear cover link restrictions on a new set tether
	AllowedCoverLinks.length=0;

	Super.OnAIMove(Action);
}

function bool SetTether( Actor NewTetherActor, optional float NewTetherDistance, optional bool NewbDynamicTether, optional float NewTetherPersistDuration, optional bool bInteruptable, optional bool bIsValidCache )
{
	local CombatZone	CZ;
	local CoverInfo		Info;
	local int			Count;
	local float			OldTetherDistance;
	local CoverLink		Link;
	local int			Idx;
	local GearPawn_RockwormBase RW;
	local float			CurDist;
	local float			BestDist;

	`AILog( GetFuncName()@NewTetherActor@NewTetherDistance@NewbDynamicTether@NewTetherPersistDuration@bInteruptable@bIsValidCache);

	// Don't allow tether to controllers, instead move to a node in the combat zone
	CZ = CombatZone(NewTetherActor);
	if( CZ != None )
	{
		NewTetherActor = None;

		// If my anchor is already in the combat zone, stay tethered to it
		if( MyGearPawn.Anchor != None &&
			MyGearPawn.GetCombatZoneForNav( MyGearPawn.Anchor ) == CZ )
		{
			//debug
			`AILog( "Anchor"@MyGearPawn.Anchor@"already in combat zone" );

			NewTetherActor = MyGearPawn.Anchor;
		}
		else
		{
			//debug
			`AILog( "Search for cover in comat zone"@CZ@CZ.GetHumanReadableName() );

			// Otherwise, search for cover closest to me and move to it
			OldTetherDistance = TetherDistance;
			TetherDistance = 4096.f;
			if( EvaluateCover( SearchType_Near, MyGearPawn, Info ) )
			{
				NewTetherActor = Info.Link.GetSlotMarker( Info.SlotIdx );

				//debug
				`AILog( "Found cover at"@Info.Link@Info.SlotIdx@NewTetherActor );
			}
			else
			{
				// Randomly select a navigation point inside the combat zone
				while( NewTetherActor == None && Count++ < 100 )
				{
					NewTetherActor = CZ.MyNavRefs[Rand(CZ.MyNavRefs.Length)].Actor;
				}

				//debug
				`AILog( GetFuncName()@"Randomly selected a nav point"@NewTetherActor@"from CZ"@CZ@CZ.GetHumanReadableName() );
			}
			TetherDistance = OldTetherDistance;
		}

		if( NewTetherActor == None )
		{
			//debug
			`AILog( GetFuncName()@"FAILED to handle CZ"@CZ@CZ.GetHumanReadableName() );
		}
	}
	else if(MoveAction != none)
	{
		Link = CoverLink(NewTetherActor);

		if(Link == none)
		{
			RW = GearPawn_RockwormBase(NewTetherActor);
			// if we've just been told to run to a rockworm, do some funny business and swap out the link for one of the worm's links
			if(RW != none)
			{
				if(VSizeSq(RW.LeftCover.Location - MyGearPawn.Location) < VSizeSq(RW.RightCover.Location - MyGearPawn.Location))
				{
					Link = RW.LeftCover;
				}
				else
				{
					Link = RW.RightCover;
				}
			}
		}
		if(Link != none)
		{
			// if we've been given a specific index to run to
			if(MoveAction.DestinationSlotIndex > -1)
			{
				Info.Link = Link;
				Info.SlotIdx = MoveAction.DestinationSlotIndex;
				//`AILog("Calling MoveToCover: "@Info.Link@Info.SlotIdx);
				return class'AICmd_MoveToCover'.static.MoveToCover(self,Info,,bInteruptable);
			}
			// if tether dist is 0, treat this like an order to go to that link, and use any of its coverslots
			else if(NewTetherDistance==0)
			{
				BestDist=-1;
				// find a valid slot to use
				for(Idx=0;Idx<Link.Slots.length;Idx++)
				{
					if(Link.IsValidClaim(Pawn,Idx))
					{
						// find the closest spot to the rockworm's head so it'll last longest
						if(RW != none)
						{
							CurDist = VSizeSq(Link.GetSlotLocation(Idx) - RW.Location);
						}
						else
						{
							CurDist = VSizeSq(Link.GetSlotLocation(Idx) - Pawn.Location);
						}


						if(CurDist < BestDist || BestDist < 0)
						{
							Info.Link = Link;
							Info.SlotIdx = Idx;
							BestDist = CurDist;
						}
					}
				}
				if(Info.Link != none)
				{
					return class'AICmd_MoveToCover'.static.MoveToCover(self,Info,,bInteruptable,TRUE);
				}
				`AILog("Could not find unclaimed slot on "$Link$" running to link instead =/");
			}
		}
	}


	return Super.SetTether( NewTetherActor, NewTetherDistance, NewbDynamicTether, NewTetherPersistDuration, bInteruptable, bIsValidCache );
}

/**
 * Returns TRUE if there is an enemy closer than our current cover.
 */
final function bool HasEnemyCloserThanCover()
{
	local float EnemyDist;
	if (HasValidCover() && !IsAtCover())
	{
		EnemyDist = GetNearEnemyDistance(Pawn.Location);
		return (EnemyDist * EnemyDist < VSizeSq(Pawn.Location - Cover.Link.GetSlotLocation(Cover.SlotIdx)));
	}
	return FALSE;
}

/** will return TRUE if we are already in a cover slot from one of our restricted cover links */
function bool AtSlotFromRestrictedLink()
{
	local int i;
	if(IsAtCover()  && AllowedCoverLinks.length > 0)
	{
		for(i=0;i<AllowedCoverLinks.length;i++)
		{
			if(Cover.Link == AllowedCoverLinks[i])
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

function bool HasRestrictedCoverLinks()
{
	return (AllowedCoverLinks.length > 0);
}


/**
 * Returns TRUE if the specified cover is valid.
 */
final native function bool IsValidCover(const out CoverInfo TestCover);

/**
 * Invalidates our current cover.
 */
function InvalidateCover( optional bool bUnclaimOnly, optional bool bFast )
{
	//debug
	`AILog( GetFuncName()@bUnclaimOnly );

	UnClaimCover();
	if( !bUnclaimOnly )
	{
		bLeaveCoverFast=bFast;
		ResetCoverType();
		ClearCoverGoal();
	}
	else
	{
		// Otherwise, just reset our cover action
		ResetCoverAction( TRUE );
	}

}

/**
 * Returns TRUE if we have a valid cover claim.
 */
final native function bool HasValidCover();

/**
 * Returns TRUE if we are currently at our claimed cover.
 */
event bool IsAtCover()
{
	return bReachedCover;
}

/**
 * Claims the specified cover, unclaiming previous if it was valid.
 */
final function bool ClaimCover( CoverInfo NewCover )
{
	local bool bResult;

	if( IsValidCover( NewCover ) )
	{
		bReachedCover = FALSE;
		// If we have cover to unclaim and we're not repeating a claim
		if( Cover.Link != None &&
			(Cover.Link != NewCover.Link || Cover.SlotIdx != NewCover.SlotIdx))
		{
			// Clear the last cover
			UnClaimCover();
		}

		if( MyGearPawn != None )
		{
			// Claim
			bResult = MyGearPawn.ClaimCover( NewCover.Link, NewCover.SlotIdx );
		}

		// If claimed and this is our goal - keep link
		if( bResult )
		{
			//debug
			`AILog( GetFuncName()@NewCover.Link@NewCover.SlotIdx@bResult );

			Cover = NewCover;
		}
		else
		{
			//debug
			`AILog( "Failed to claim cover - existing claim:"@NewCover.Link.Slots[NewCover.SlotIdx].SlotOwner );
		}
	}
	else
	{
		//debug
		`AILog( "Invalid cover:"@NewCover.Link@NewCover.SlotIdx );
	}

	return bResult;
}

/**
 * Revokes current cover claim.
 */
final function UnClaimCover()
{
	if( Cover.Link != None )
	{
		//debug
		`AILog( GetFuncName()@Cover.Link@Cover.SlotIdx );

		// save the last cover
		LastCover = Cover;
		// release the claim
		if( MyGearPawn != None )
		{
			MyGearPawn.UnClaimCover( Cover.Link, Cover.SlotIdx, TRUE );
		}
	}
	// clear the cover
	Cover.Link = None;
	Cover.SlotIdx = -1;
	bReachedCover = FALSE;
}

final function CoverSlotMarker GetCoverSlotMarker()
{
	if( Cover.Link != None )
	{
		return Cover.Link.GetSlotMarker( Cover.SlotIdx );
	}
	return None;
}

protected function bool EvaluateCoverInternal
(
 ECoverSearchType Type,
 CombatZone TestWithinCombatZone,
 Actor GoalActor,
 out CoverInfo out_Cover,
 optional CoverSlotMarker InBestMarker,
 optional Goal_AtCover GoalEvaluator,
 optional float MaxDistFromGoal
 )
{
	local CoverSlotMarker Marker;
	local bool bResult;


	//local float Time;
	//Clock(Time);

	// we can't use cover when we're holding a shield
	if (MyGearPawn != None && (MyGearPawn.IsAKidnapper() || MyGearPawn.IsCarryingShield() || MyGearPawn.CarriedCrate != None))
	{
		return false;
	}

	//debug
	`AILog( GetFuncName()@Type@GoalActor@InBestMarker@GoalEvaluator@Pawn.Anchor@TestWithinCombatZone );

	// don't do this if we're searching along a path because we don't necessarily want a node in our combat zone
	if(TestWithinCombatZone != none && Type != SearchType_AlongPath)
	{
		// if we're trying to reacha  specific combat zone, once we're in it don't leave!
		class'Path_WithinCombatZone'.static.WithinCombatZone( Pawn, 10000000, TestWithinCombatZone);
		class'Path_TowardPoint'.static.TowardPoint(Pawn, TestWithinCombatZone.ZoneCenter);
	}
	else
	{
		// otherwise just add a penalty for leaving a combat zone
		class'Path_WithinCombatZone'.static.WithinCombatZone( Pawn,512 );
	}

	AddBasePathConstraints();

	// if we've been given a max dist from our goal, enforce it
	if(MaxDistFromGoal > 0)
	{
		`AILog("Not traversing outside max distance: "@MaxDistFromGoal);
		class'Path_WithinDistanceEnvelope'.static.StayWithinEnvelopeToLoc(Pawn,GoalActor.Location,MaxDistFromGoal,0.f,FALSE,,TRUE);
	}



	if( Type == SearchType_AlongPath )
	{
		//debug
		`AILog( GetFuncName()@"Searching... ALONG PATH!" );

		class'Path_AlongLine'.static.AlongLine( Pawn, Normal( GoalActor.Location - Pawn.Location ) );
		class'Path_WithinTraversalDist'.static.DontExceedMaxDist(Pawn, Max(VSize(GoalActor.location-Pawn.Location)*0.5f,512),FALSE);
		if( GoalEvaluator == None )
		{
			GoalEvaluator = AtCover_AlongPath;
			if (CovGoal_ProtectedByLocation(GoalEvaluator.CoverGoalConstraints[0]) != None)
			{
				CovGoal_ProtectedByLocation(GoalEvaluator.CoverGoalConstraints[0]).ThreatLocation = GoalActor.Location;
			}
		}
	}
	else
	if( Type == SearchType_Towards )
	{
		//debug
		`AILog(GetFuncName()@"Searching... TOWARDS!");
		class'Path_AlongLine'.static.AlongLine( Pawn, Normal( GoalActor.Location - Pawn.Location ) );

		if(GoalEvaluator == none)
		{
			GoalEvaluator = AtCover_Toward;
		}
	}
	else
	if( Type == SearchType_Away )
	{
		//debug
		`AILog(GetFuncName()@"Searching... AWAY!");
		class'Path_AlongLine'.static.AlongLine( Pawn, -Normal( GoalActor.Location - Pawn.Location ) );
		if(GoalEvaluator == none)
		{
			GoalEvaluator = AtCover_Away;
		}
	}
	else
	if( Type == SearchType_Near )
	{
		//debug
		`AILog(GetFuncName()@"Searching... NEAR!");
		// don't try and move toward a goal actor if we have a combatzone to test.. we should be moving toward
		// the closest combat zone already
		if(TestWithinCombatZone == none)
		{
			class'Path_TowardGoal'.static.TowardGoal( Pawn, GoalActor );
		}
		if(GoalEvaluator == none)
		{
			GoalEvaluator = AtCover_Near;
		}

	}
	else
	if( Type == SearchType_Ambush )
	{
		//debug
		`AILog(GetFuncName()@"Searching... AMBUSH!");
		if( GoalEvaluator == None )
		{
			GoalEvaluator = AtCover_Ambush;
			if (CovGoal_ProtectedByLocation(GoalEvaluator.CoverGoalConstraints[0]) != None)
			{
				CovGoal_ProtectedByLocation(GoalEvaluator.CoverGoalConstraints[0]).ThreatLocation = GoalActor.Location;
			}
		}
	}

	// if we have been restricted to a set of coverlinks then push the allowed coverlinks constraint
	if(AllowedCoverLinks.length > 0 && Type != SearchType_AlongPath)
	{
		`AILog("Adding withinallowedcoverlinks since we have some in our list.");
		GoalEvaluator.CoverGoalConstraints[GoalEvaluator.CoverGoalConstraints.length] = new(self) class'CovGoal_WithinAllowedCoverLinks';
	}

	GoalEvaluator.Init(self,GoalActor);

	// rate the best so we can save off the 'best' scores for comparison against new ones
	if(InBestMarker != none)
	{
		//@note: initial weight here assumes marker is anchor or at least directly reachable
		GoalEvaluator.RateSlotMarker( InBestMarker, Pawn, VSize(InBestMarker.Location - Pawn.Location) +
			InBestMarker.FearCost + InBestMarker.TransientCost + InBestMarker.ExtraCost );
	}

	//UnClock(Time);
	//ForceLog(GetFuncName()@" BEFORE findpath toward took "$Time);
	if( FindPathToward( GoalActor ) != None )
	{
		Marker = CoverSlotMarker(RouteCache[RouteCache.Length-1]);
		out_Cover = Marker.OwningSlot;
		bResult = TRUE;
	}

	// if we added a coverlink limit, remove it now so it doesn't stick around after the fact
	if(AllowedCoverLinks.length > 0 && Type != SearchType_AlongPath)
	{
		GoalEvaluator.CoverGoalConstraints.Remove(GoalEvaluator.CoverGoalConstraints.length-1,1);
	}

	// then we need to clear constraints because it's possible FindPathToward didn't (this does need to be here, please don't remove it! ;) )
	Pawn.ClearConstraints();


	return bResult;
}

function bool EvaluateCover
(
	ECoverSearchType Type,
	Actor GoalActor,
	out CoverInfo out_Cover,
	optional CoverSlotMarker InBestMarker,
	optional Goal_AtCover GoalEvaluator,
	optional float MaxDistFromGoal
)
{
	local int i;
	local CombatZone TestingZone;
	local CombatZone ClosestZoneToGoal;
	local float CurDistSq;
	local float BestDistSq;
	`AILog(GetFuncName()@Type@GoalActor@InBestMarker@GoalEvaluator);
	if(Type == SearchType_Near)
	{
		// find out if our goal is inside a combat zone
		if(GoalActor != none && CombatZonelist.length > 0)
		{
			BestDistSq=-1.f;
			for(i=0;i<CombatZonelist.length;i++)
			{
				if(CombatZoneList[i].Encompasses(GoalActor))
				{
					TestingZone = CombatZoneList[i];
					break;
				}
				else
				{
					// if we haven't found a zone that encompasses the goal, then keep track of the closest one so
					// we to got he combat zone closest to our goal
					CurDistSq = VSizeSq(CombatZonelist[i].ZoneCenter - GoalActor.Location);
					if(CurDistSq < BestDistSq || BestDistSq < 0)
					{
						ClosestZoneToGoal = CombatZonelist[i];
						BestDistSq=CurDistSq;
					}
				}
			}

			// if we didnt' find a zone that encompasses the goal, use the closest one
			TestingZone = ClosestZoneToGoal;
		}
		return EvaluateCoverInternal(Type,TestingZone,GoalActor,out_Cover,InBestMarker,GoalEvaluator,MaxDistFromGoal);
	}
	else if(Type == SearchType_AlongPath)
	{
		// if we're testing along a path then don't do combat zone magic, cuz we want one near our path!
		return EvaluateCoverInternal(Type,none,GoalActor,out_Cover,InBestMarker,GoalEvaluator);
	}
	else
	{
		// otherwise we need to try pathing to each combat zone one at a time, so loop through them all
		if(CombatZoneList.length > 0)
		{
			for(i=0;i<CombatZoneList.length;i++)
			{
				// if we found one in this node, bail!
				if(EvaluateCoverInternal(Type,CombatZoneList[i],GoalActor,out_Cover,InBestMarker,GoalEvaluator,MaxDistFromGoal))
				{
					return true;
				}
			}
		}
		else
		{
			return EvaluateCoverInternal(Type,none,GoalActor,out_Cover,InBestMarker,GoalEvaluator,MaxDistFromGoal);
		}

	}

	return false;
}

function bool IgnoreTimeTransitions()
{
	if( MyGearPawn != None &&
		MyGearPawn.Weapon != None &&
		MyGearPawn.Weapon.IsA( 'GearWeap_GrenadeBase' ) &&
		IsLeaning() )
	{
		return TRUE;
	}

	return Super.IgnoreTimeTransitions();
}

/**
 *	Figure out if AI should blind fire
 */
final event bool ShouldBlindFire()
{
	local float	  Chance;
	local int	  Idx, Num, Tickets;

	// If pawn can't blind fire - fail
	if( MyGearPawn == None || !MyGearPawn.bCanBlindFire )
	{
		return FALSE;
	}

	// if our difficulty level doesn't allow it, don't blindfire
	if ( (!WorldInfo.GRI.IsMultiplayerGame() || WorldInfo.GRI.IsCoopMultiplayerGame()) &&
		MyGearPawn.GetTeamNum() == TEAM_LOCUST &&
		!class'DifficultySettings'.static.GetLowestPlayerDifficultyLevel(WorldInfo).default.bAllowLocustBlindFire )
	{
		return FALSE;
	}


	// No blind firing grenades
	if( MyGearPawn.Weapon.IsA( 'GearWeap_GrenadeBase' ) )
	{
		return FALSE;
	}

	// Figure out how many fire tickets are taken
	Num = MyGearPawn.FireTickets.Length;
	for( Idx = 0; Idx < Num; Idx++ )
	{
		if( MyGearPawn.FireTickets[Idx] != None )
		{
			Tickets++;
		}
	}

	// Chance to blind fire is base value + tickets
	Chance = MyGearPawn.BlindFirePct + (Tickets * MyGearPawn.BlindFirePct);
	if( FRand() > Chance )
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * Updates our pawn's covertype to match current cover.
 */
final function bool SetCoverType()
{
	if( MyGearPawn != None &&
		Cover.Link != None &&
		Cover.SlotIdx >= 0 &&
		Cover.SlotIdx < Cover.Link.Slots.Length )
	{
		//debug
		`AILog(GetFuncName()@Cover.Link@Cover.SlotIdx);

		if( Cover.Link.Slots.Length <= 1 )
		{
			MyGearPawn.SetCoverInfo( Cover.Link, Cover.SlotIdx, Cover.SlotIdx, Cover.SlotIdx, 0.f, TRUE );
		}
		else if( Cover.SlotIdx == Cover.Link.Slots.Length - 1 )
		{
			MyGearPawn.SetCoverInfo( Cover.Link, Cover.SlotIdx, Cover.SlotIdx - 1, Cover.SlotIdx, 1.f, TRUE );
		}
		else
		{
			MyGearPawn.SetCoverInfo( Cover.Link, Cover.SlotIdx, Cover.SlotIdx, Cover.SlotIdx + 1, 0.f, TRUE );
		}
		return TRUE;
	}
	else
	{
		//debug
		`AILog( "Bad SetCoverType call"@Cover.Link@Cover.SlotIdx, 'Cover' );
	}

	return FALSE;
}

/**
 * Resets our cover type.
 */
final function ResetCoverType()
{
	local Rotator Rot;
	local Vector  Loc;

	`AILog(GetFuncname()@MyGearPawn@MyGearPawn.CurrentLink);
	if( MyGearPawn != None )
	{
		if(MyGearPawn.CurrentLink != None )
		{
			if( IsLeaning() )
			{
				GetPlayerViewPoint( Loc, Rot );
			}
			else
			{
				Rot = MyGearPawn.CurrentLink.GetSlotRotation(MyGearPawn.CurrentSlotIdx);
			}

			MyGearPawn.SetRotation( Rot );
			Focus = None;
			SetFocalPoint( MyGearPawn.Location + Vector(MyGearPawn.Rotation) * 512.f, TRUE );

		}

		MyGearPawn.LeaveCover();
	}

}

final function bool SetCoverAction( ECoverAction Action )
{
	local bool bShouldTarget;

	//debug
	`AILog( GetFuncName()@Action );

	if( MyGearPawn != None )
	{
		bShouldTarget = (Action == CA_PopUp || MyGearPawn.Weapon.IsA('GearWeap_GrenadeBase') || MyGearPawn.IsCarryingAHeavyWeapon());
		MyGearPawn.SetTargetingMode( bShouldTarget );
		MyGearPawn.SetCoverAction( Action );
		MyGearPawn.SetWeaponAlert( 5.f );
		return TRUE;
	}

	return FALSE;
}

final function ResetCoverAction( optional bool bResetCrouch )
{
	if( MyGearPawn != None && MyGearPawn.CoverAction != CA_Default )
	{
		MyGearPawn.SetTargetingMode( FALSE );
		MyGearPawn.SetCoverAction(CA_Default);
		if( bResetCrouch )
		{
			MyGearPawn.ShouldCrouch( FALSE );
		}
	}

	StopFiring();
	PendingFireLinkItem.SrcAction = CA_Default;
	PendingFireLinkItem.SrcType	  = CT_None;
}

final function bool IsLeaning()
{
	return MyGearPawn != None && MyGearPawn.IsLeaning();
}

final function bool IsPeeking()
{
	return MyGearPawn != None && MyGearPawn.IsPeeking(MyGearPawn.CoverAction);
}

event bool ShouldDo360Aiming( Vector TargLoc )
{
	local Rotator	DeltaRot;
	local vector2d  NewAimOffsetPct;

	// heavy weapons can't do 360 aiming
	if (MyGearPawn != None && MyGearPawn.IsCarryingAHeavyWeapon())
	{
		return false;
	}

	// find out delta angle between pawn's rotation and crosshair direction
	DeltaRot			= Normalize(Rotator(TargLoc - Pawn.Location) - Pawn.Rotation);
	NewAimOffsetPct.X	= NormalizeRotAxis(DeltaRot.Yaw)	/ 16384.f;
	NewAimOffsetPct.Y	= NormalizeRotAxis(DeltaRot.Pitch)	/ 16384.f;

	// Target is not in front of cover
	//@fixme - this doesn't work in all cases, only if the player is off the edge of the current
	// slot, or entirely behind us.  need to improve this a bit more, as right now the AI will
	// think they can't fire from cover when they could more than likely pop up and still shoot
	if( Abs(NewAimOffsetPct.X) > 0.6f )
	{
		return TRUE;
	}

	return FALSE;
}

final native function bool GetCoverAction( out CoverInfo ChkCover, optional Actor ChkTarget, optional bool bAnyAction, optional bool bTest, optional Name DebugTag );

final function bool GetPeekCoverAction( optional ECoverAction Pending )
{
	local array<ECoverAction> Actions;
	local ECoverAction Action;

	// Don't peek during 360 aiming
	if( MyGearPawn == None || MyGearPawn.bDoing360Aiming )
	{
		return FALSE;
	}

	if( CombatMood == AICM_Ambush )
	{
		return FALSE;
	}

	if( Pending != CA_Default )
	{
		switch( Pending )
		{
			case CA_LeanLeft:	PendingFireLinkItem.SrcAction = CA_PeekLeft;	break;
			case CA_LeanRight:	PendingFireLinkItem.SrcAction = CA_PeekRight;	break;
			case CA_PopUp:		PendingFireLinkItem.SrcAction = CA_PeekUp;		break;
		}
		PendingFireLinkItem.SrcType = CT_None;

		return TRUE;
	}

	MyGearPawn.CurrentLink.GetSlotActions( MyGearPawn.CurrentSlotIdx, Actions );

	// If we have available actions
	if( Actions.Length > 0 )
	{
		// Randomly choose an action
		Action = Actions[Rand(Actions.Length)];

		//debug
		`AILog( "New peek cover action is:"@Action@MyGearPawn.CurrentLink@MyGearPawn.CurrentSlotIdx );

		PendingFireLinkItem.SrcAction = Action;
		PendingFireLinkItem.SrcType	  = CT_None;
		return TRUE;
	}

	//debug
	`AILog( "Failed to get peek cover action"@MyGearPawn.CurrentLink@MyGearPawn.CurrentSlotIdx );

	return FALSE;
}

final function bool ShouldWaitForTransition( ECoverAction Action )
{
	local bool	  bSwitch;
	local bool	  bWantsToBeMirrored;

	if( MyGearPawn != None )
	{
		// If not mirrored and should be - switch
		if( Action == CA_LeanLeft	||
			Action == CA_BlindLeft	||
			Action == CA_PeekLeft	)
		{
			bSwitch = TRUE;
			bWantsToBeMirrored = TRUE;
		}
		else
		// Otherwise, if mirrored and should not be - switch
		if( Action == CA_LeanRight	||
			Action == CA_BlindRight ||
			Action == CA_PeekRight  )
		{
			bSwitch = TRUE;
			bWantsToBeMirrored = FALSE;
		}

		// If should switch
		if( bSwitch )
		{
			// Switch mirrored side and return TRUE to wait for transition
			MyGearPawn.SetMirroredSide( bWantsToBeMirrored );
			return TRUE;
		}
	}

	return FALSE;
}

/** Move pawn towards current slot */
native final noexport latent function AdjustToSlot( CoverSlotMarker TargetMarker );

/**
* Called when cover is reached, save the information for later use.
*/
function NotifyReachedCover()
{
	bReachedCover = TRUE;
	bFailedToFireFromCover = false;
}

/**
 * Called when AI reaches a slot while transitioning between them
 */
final function NotifyReachedCoverSlot( int SlotIdx )
{
	if( MyGearPawn == None || MyGearPawn.TargetSlotMarker == None )
	{
		return;
	}

	//debug
	`AILog( "NotifyReachedCoverSlot -- Link"@MyGearPawn.CurrentLink$"/"$Cover.Link@"Reached"@MyGearPawn.CurrentSlotIdx@"Target"@MyGearPawn.TargetSlotMarker.OwningSlot.SlotIdx@"Dir"@MyGearPawn.CurrentSlotDirection@"Pct"@MyGearPawn.CurrentSlotPct, 'Cover' );

	if( SlotIdx != MyGearPawn.TargetSlotMarker.OwningSlot.SlotIdx &&
		Cover.Link == MyGearPawn.CurrentLink )
	{
		if( !Cover.Link.bClaimAllSlots )
		{
			// Otherwise, if we aren't supposed to have all slots claimed
			// Release the old one before recognizing the new one
			Cover.Link.UnClaim( MyGearPawn, SlotIdx, FALSE );
		}
	}
}

simulated function bool NotifyCoverClaimViolation( Controller NewClaim, CoverLink Link, int SlotIdx )
{
	//debug
	`AILog( GetFuncName()@NewClaim@Link@SlotIdx );

	if( NewClaim == self )
	{
		return TRUE;
	}

	// If claim violation is our cover destination
	if( Cover.Link == Link &&
		(Cover.Link.bClaimAllSlots ||
			Cover.SlotIdx == SlotIdx) )
	{
		// Force release of cover, even if we don't claim new
		InvalidateCover();

		ClearMovementInfo();

		// Try to find cover nearby until we can reevaluate
		if( HasAnyEnemies() )
		{
			MoveToNearestCoverEnemy();
		}
		else
		{
			MoveToNearestCoverIdle();
		}
	}
	else
	// Otherwise, moving through this slot... give chance to recompute path
	if( CommandList != None )
	{
		return CommandList.NotifyCoverClaimViolation( NewClaim, Link, SlotIdx );
	}

	return FALSE;
}

simulated event NotifyCoverAdjusted()
{
	if( MyGearPawn != None)
	{
		MyGearPawn.SetCurrentCoverType();
	}
}

simulated function NotifyCoverDisabled( CoverLink Link, int SlotIdx, optional bool bAdjacentIdx )
{
	`AILog(GetFuncname()@Link@SlotIdx@bAdjacentIdx@TetherActor@MoveAction@((MoveAction != None) ? string(MoveAction.TetherDistance) : "0"));
	if( !bAdjacentIdx )
	{
		InvalidateCover();

		if(MoveAction != none && MoveAction.TetherDistance == 0)
		{
			`AILog("Our cover was disabled, but we have a 0 tether.. go back to tether!");
			OnAIMove(MoveAction);
		}
		else
		{
			 `AILog("tried to run back to tether"@TetherActor@"but claim failed.. removing tether actor!");
			 TetherActor=none;
		}
	}
}

final function MoveToNearestCoverIdle()
{
	local CoverSlotMarker Marker;
	local CoverInfo CovToLeader;

	bReachedMoveGoal = FALSE;
	bReachedCover = FALSE;

	// use the cover we're standing on if possible
	Marker = CoverSlotMarker(Pawn.Anchor);
	if (Marker != None &&
		Marker.OwningSlot.Link.IsValidClaim(MyGearPawn,Marker.OwningSlot.SlotIdx) &&
		IsCoverWithinCombatZone(Marker.OwningSlot) )
	{
		`AILog("Using current marker anchor:"@Marker.OwningSlot.Link@Marker.OwningSlot.SlotIdx);
		SetCoverGoal(Marker.OwningSlot);
		return;
	}

	//debug
	`AILog( GetFuncName()@Squad );

	bReachedMoveGoal = FALSE;
	if( Squad != None )
	{
		TetherDistance = 512.f;
		if( EvaluateCover( SearchType_Near, GetSquadPosition(), CovToLeader,, AtCover_Squad, TetherDistance ) )
		{
			SetCoverGoal( CovToLeader );
			return;
		}
	}

	//debug
	`AILog("Fallback looking for cover near current location");

	if( EvaluateCover( SearchType_Near, Pawn, CovToLeader,, AtCover_Squad ) )
	{
		SetCoverGoal( CovToLeader );
		return;
	}

	//debug
	`AILog( "Failed to find idle cover" );
}

final function MoveToNearestCoverEnemy()
{
	local CoverInfo			CovToEnemy;
	local bool				bTowardsGoal;
	local float				NearEnemyDist;
	local CoverSlotMarker	Marker;

	// we can't use cover if we have a shield
	if (MyGearPawn != None && (MyGearPawn.IsAKidnapper() || MyGearPawn.IsCarryingShield()))
	{
		`AILog(GetFuncName() @ "ignored because I have a shield");
		return;
	}

	//debug
	`AILog(GetFuncName()@Pawn.Anchor);

	if( !HasValidCover() && (!HasValidEnemy() || LineOfSightTo(Enemy)) )
	{
		//debug
		`AILog("Looking at potential anchor cover..");

		// use the cover we're standing on if possible
		Marker = CoverSlotMarker(Pawn.Anchor);
		if (Marker != None &&
			Marker.OwningSlot.Link.IsValidClaim(MyGearPawn,Marker.OwningSlot.SlotIdx) &&
			!IsCoverExposedToAnEnemy(Marker.OwningSlot) &&
			 IsCoverWithinCombatZone(Marker.OwningSlot))
		{
			//debug
			`AILog("Using current marker anchor:"@Marker.OwningSlot.Link@Marker.OwningSlot.SlotIdx);

			SetCoverGoal(Marker.OwningSlot);
			return;
		}
	}
	NearEnemyDist = GetNearEnemyDistance(Pawn.Location);
	// search for cover either towards/away based on closest enemy distance
	bTowardsGoal = SearchForCoverTowardsEnemy( NearEnemyDist );
	`AILog("Searching"@(bTowardsGoal?"towards":"away from")@"enemy first");
	if (EvaluateCover(bTowardsGoal ? SearchType_Towards : SearchType_Away,Enemy,CovToEnemy))
	{
		`AILog("Found cover towards/away enemy:"@CovToEnemy.Link@CovToEnemy.SlotIdx@"["@CovToEnemy.Link.Slots[CovToEnemy.SlotIdx].SlotMarker@"]");
		SetCoverGoal( CovToEnemy, TRUE );
		return;
	}
	else
	// try the opposite if the enemy is close enough
	if (!HasValidCover() &&
		NearEnemyDist < EnemyDistance_Long &&
		EvaluateCover(bTowardsGoal ? SearchType_Away : SearchType_Towards,Enemy,CovToEnemy))
	{
		`AILog("Found fallback cover towards/away enemy:"@CovToEnemy.Link@CovToEnemy.SlotIdx@"["@CovToEnemy.Link.Slots[CovToEnemy.SlotIdx].SlotMarker@"]");
		SetCoverGoal( CovToEnemy, TRUE );
		return;
	}
	else
	{
		if (HasValidCover() && !IsCoverExposedToAnEnemy(Cover))
		{
			SetAcquireCover( ACT_None );
			if (!IsAtCover())
			{
				`AILog("Failed to find cover, using existing");
				SetCoverGoal(Cover);
				return;
			}
			else
			{
				`AILog("Failed to find new cover, keeping existing");
			}
		}
		else
		{
			`AILog("Failed to find cover, don't have valid or it's exposed - dunno what to do, halp!");
		}
	}
	return;
}

final function bool SearchForCoverTowardsEnemy( float NearEnemyDist )
{
	return (CombatMood == AICM_Aggressive || (CombatMood != AICM_Passive && NearEnemyDist > EnemyDistance_Short));
}

final function bool ShouldFireFromCover()
{
	if( CombatMood == AICM_Passive ||
		CombatMood == AICM_Ambush )
	{
		if( HasEnemyWithinDistance(EnemyDistance_Short) || IsCoverExposedToAnEnemy(Cover) )
		{
			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_DamagedEnemyHeavy,Pawn);
			`AILog("Firing in passive because of enemy within distance?"@HasEnemyWithinDistance(EnemyDistance_Short)@"or exposed?"@IsCoverExposedToAnEnemy(Cover));
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

event bool SetCombatMood(ECombatMood NewMood,optional Pawn MoodInstigator)
{
	//debug
	`AILog(GetFuncName()@NewMood@"old:"@CombatMood);

	CombatMood = NewMood;
	SetupEnemyDistances();
	// if now passive
	if( CombatMood == AICM_Passive )
	{
		if( ShouldFireFromCover() )
		{
			return FALSE;
		}
		else
		{
			// then go ahead and stop immediately
			StopFiring();

			AbortCommand( None, class'AICmd_Attack_FireFromOpen'  );
			AbortCommand( None, class'AICmd_Attack_FireFromCover' );

			return TRUE;
		}
	}
	else
	if (CombatMood == AICM_Aggressive)
	{
		if (GetHealthPercentage() > 0.5f)
		{
			// reset any cover properties so that the AI re-evaluates soon
			if (HasAnyEnemies())
			{
				SetAcquireCover( ACT_DesireBetter );
			}
			return TRUE;
		}
		// not attacking, crazy mofo!
		return FALSE;
	}
	else
	if (CombatMood == AICM_Normal)
	{
		// try to make the AI move now
		if (MoveAction == None &&
			(!HasAnyEnemies() || CombatZoneList.Length == 0) &&
			!MyGearPawn.IsDBNO() &&
			!IsDead() &&
			!IsInState('WaitingForEvent'))
		{
			if (GetSquadLeader() != self && ShouldMoveToSquad())
			{
				MoveToSquadPosition();
			}
			return TRUE;
		}
		else
		{
			// can't move right now
			return FALSE;
		}
	}
	else if(CombatMood == AICM_Ambush)
	{
		AbortCombatCommands();
	}
	// unknown order?
	return FALSE;
}

/** called by AIReactCond_DmgLeaveCover when we've taken enough damage that we should leave cover */
function InvalidateCoverFromDamage(Actor Damager)
{
	Cover.Link.SetInvalidUntil(Cover.SlotIdx, WorldInfo.TimeSeconds + 2.0f);
	if(CanEvade(true,0.125f))
	{
		DoEvade( GetBestEvadeDir( MyGearPawn.Location, Pawn(Damager) ), TRUE );
	}
}

/////////////////////////////////////////////////
//////////////// TURRET FUNCTIONS ///////////////
/////////////////////////////////////////////////

/**
 * Handler for scripted use of turrets.  Push into turret state.
 */
final function OnAIUseTurret( SeqAct_AIUseTurret Action )
{
	local AIReactCond_CanReturnToTroika ReturnReact;
	`AILog(GetFuncName()@Action@Action.Turret);

	if( !UseTurret( Action.Turret, Action.bIgnoreFlank, Action.bTeleportToTurret ) )
	{
		bIgnoreFlank	= default.bIgnoreFlank;
		DefaultCommand	= default.DefaultCommand;
		ReturnReact = AIReactCond_CanReturnToTroika(ReactionManager.FindReactionByType(class'AIReactCond_CanReturnToTroika',true));
		if(ReturnReact != none)
		{
			ReturnReact.UnsubscribeAll();
		}
		ReturnReact=none;
	}
}

final function OnPossessTurret( SetAct_PossessTurret inAction )
{
	local AIReactCond_CanReturnToTroika ReturnReact;
	`AILog(GetFuncName()@inAction@inAction.Turret);

	if( !UseTurret( inAction.Turret, TRUE, TRUE ) )
	{
		bIgnoreFlank	= default.bIgnoreFlank;
		DefaultCommand	= default.DefaultCommand;
		ReturnReact = AIReactCond_CanReturnToTroika(ReactionManager.FindReactionByType(class'AIReactCond_CanReturnToTroika',true));
		if(ReturnReact != none)
		{
			ReturnReact.UnsubscribeAll();
		}
		ReturnReact=none;
	}
}

final event bool UseTurret( Turret inTurret, bool inbIgnoreFlank, optional bool inbTeleportToTurret )
{
	local GearPawn OldPawn;

	OldPawn = MyGearPawn;
	if( inTurret != None &&
		inTurret.ClaimTurret( self ) )
	{
		bIgnoreFlank	 = inbIgnoreFlank;
		LastTurret		 = CurrentTurret;

		if( inbTeleportToTurret &&
			Pawn.SetLocation( CurrentTurret.GetEntryLocation() ) &&
			Pawn.SetRotation( CurrentTurret.Rotation ) &&
			CurrentTurret.TryToDrive(Pawn) )
		{
			OldPawn.bCanDBNO = FALSE;
			OldPawn.SetBase( CurrentTurret.Base );
			bReachedTurret = TRUE;
		}
		else
		{
			bReachedTurret = FALSE;
		}

		BeginCombatCommand( class'AICmd_Base_TroikaGunner', "Put on turret", inbTeleportToTurret );
		return TRUE;
	}
	return FALSE;
}

final function StopUsingTurret(GearTurret Turr)
{
	local AIReactCond_CanReturnToTroika ReactCond;

	`AILog(GetFuncName()@Turr);
	// abort turret ai
	AbortCommand(none,class'AICmd_Base_TroikaGunner');

	// if we're being told to stop using a turret entirely, kill the re-engage reaction
	ReactCond = AIReactCond_CanReturnToTroika(ReactionManager.FindReactionByType(class'AIReactCond_CanReturnToTroika', false));
	if(ReactCond != none)
	{
		ReactCond.UnsubscribeAll();
	}
}

final function bool CanTurretFireAt( Actor TestActor, optional Turret TestTurret )
{
	local GearPawn TargetWP;

	if(TestTurret == none)
	{
		TestTurret = CurrentTurret;
	}

	if( TestActor != None )
	{
		// check to see if it's a target protected by cover
		TargetWP = GearPawn(TestActor);
		if (TargetWP != None &&
			TargetWP.IsProtectedByCover(Normal(TargetWP.Location - TestTurret.GetPhysicalFireStartLoc(vect(0,0,0))),TRUE))
		{
			return FALSE;
		}

		// check to see if the target is outside our rotation clamps
		if(!GearTurret(TestTurret).IsWithinRotationClamps(TestActor.Location))
		{
			return FALSE;
		}

		// check to see if we have los
		return	(CanFireAt( TestActor, TestTurret.GetPhysicalFireStartLoc( vect(0,25,0)  ) ) ||
				 CanFireAt( TestActor, TestTurret.GetPhysicalFireStartLoc( vect(0,-25,0) ) ));
	}

	return FALSE;
}

final function TargetInsideFireArc()
{
	//debug
	`AILog( GetFuncName() );

	BeginCombatCommand( class'AICmd_Melee_Forced', "Target got inside turret arc" );
}

final function NotifyTurretRotationClamped()
{
	//debug
	`AILog( GetFuncName()@CommandList );
	ReactionManager.NudgeChannel(none,'TurretRotationClamped');

}


/////////////////////////////////////////////////
//////////////// DEBUG FUNCTIONS ////////////////
/////////////////////////////////////////////////

simulated function DrawDebug(GearHUD_Base HUD, Name Category)
{
	local int i;
	local int j;

	Super.DrawDebug( HUD, Category );

	if( bDrawAIDebug )
	{
		if(Category == 'Default')
		{
			if( HasValidCover() )
			{
				// PINK
				DrawDebugLine( Pawn.Location, Cover.Link.GetSlotLocation(Cover.SlotIdx) + vect(0,0,15), 255, 128, 255 );
			}

			if( MyGearPawn != None )
			{
				if( MyGearPawn.IsInCover() )
				{
					// TEAL
					DrawDebugLine( MyGearPawn.Location, MyGearPawn.CurrentLink.GetSlotLocation(MyGearPawn.CurrentSlotIdx) - vect(0,0,15), 0, 255, 255 );
				}
			}
			if( CurrentTurret != None )
			{
				DrawDebugLine(CurrentTurret.GetPhysicalFireStartLoc( vect(0,0,0) ) + vector(CurrentTurret.GetTurretAimDir()) * 16384, CurrentTurret.GetPhysicalFireStartLoc( vect(0,0,0) ), 255, 128, 0);
			}
		}

		if(Category == 'allowedcoverlinks')
		{
			for(i=0;i<AllowedCoverLinks.length;i++)
			{
				DrawDebugLine(MyGearPawn.Location,AllowedCoverLinks[i].Location,255,255,0);
				for(j=0;j<AllowedCoverLinks[i].Slots.length;j++)
				{
					DrawDebugLine(AllowedCoverLinks[i].Location,AllowedCoverLinks[i].GetSlotLocation(j),255,255,0);
					DrawDebugCoordinateSystem(AllowedCoverLinks[i].GetSlotLocation(j),AllowedCoverLinks[i].GetSlotRotation(j),50.f);
				}
			}
		}

		if(Category == 'cover' && MyGearPawn.CurrentLink != none && MyGearPawn.CurrentSlotIdx != -1)
		{
			DrawDebugLine(MyGearPawn.Location,MyGearPawn.CurrentLink.GetSlotLocation(MyGearPawn.CurrentSlotIdx),255,255,0);
			DrawDebugCoordinateSystem(MyGearPawn.CurrentLink.GetSlotLocation(MyGearPawn.CurrentSlotIdx),MyGearPawn.CurrentLink.GetSlotRotation(MyGearPawn.CurrentSlotIdx),10.f);
			//MessagePlayer("Left:"@MyGearPawn.LeftSlotIdx@"Cur:"@MyGearPawn.CurrentSlotIdx@"Right:"@MyGearPawn.RightSlotIdx@"Pct:"@MyGearPawn.CurrentSlotPct);
		}
	}

}

simulated function DrawIconOverhead(GearHUD_Base HUD, Texture2D Icon)
{
	local Canvas	Canvas;
	local String	Str;

	Super.DrawIconOverhead( HUD, Icon );

	Canvas = HUD.Canvas;

	if( MyGearPawn != None )
	{
		if( HasValidCover() )
		{
			Canvas.SetDrawColor(255, 128, 255);
			Str = "("$GetRightMost(Cover.Link)$"/"$Cover.SlotIdx$"/"$GetRightMost(Cover.Link.Slots[Cover.SlotIdx].SlotMarker)$")";
			Canvas.DrawText( Str, TRUE );
		}

		if( MyGearPawn.IsInCover() )
		{
			Canvas.SetDrawColor(0,255,255);
			Str = "("$GetRightMost(MyGearPawn.CurrentLink)$"/"$MyGearPawn.CurrentSlotIdx$"/"$GetRightMost(MyGearPawn.CurrentLink.Slots[MyGearPawn.CurrentSlotIdx].SlotMarker)$")";
			Canvas.DrawText( Str, TRUE );
		}
	}
	Canvas.SetDrawColor(255,255,255);
}

function OnAIConsiderPickups(SeqAct_AIConsiderPickups Action)
{
	local class<GearWeapon> ItemClass;
	local int Index;

	if (Action.InputLinks[0].bHasImpulse)
	{
		// add to list
		foreach Action.ConsiderList(ItemClass)
		{
			//@note: we allow duplicates so that executing the "Start" and then "Stop" in sequence will always
			//	return the list to the exact state it was in prior to the action being used
			SPItemsToConsider.AddItem(ItemClass);
		}
	}
	if (Action.InputLinks[1].bHasImpulse)
	{
		// remove from list
		foreach Action.ConsiderList(ItemClass)
		{
			Index = SPItemsToConsider.Find(ItemClass);
			if (Index != INDEX_NONE)
			{
				SPItemsToConsider.Remove(Index, 1);
			}
		}
	}
}

function BanzaiAttack()
{
	if(MyGearPawn != none && (MyGearPawn.bCanBeForcedToRoadieRun || MyGearPawn.bCanRoadieRun))
	{
		MyGearPawn.bCanRoadieRun=true;
	}

	Super.BanzaiAttack();
}

event ForcePauseAndRepath(Actor InInstigator)
{
	if(MoveGoal != none && MoveGoal.IsA('CoverSlotMarker_Rockworm') &&
		InInstigator.IsA('Rockworm_TailSegment') &&
		VSizeSq(MoveGoal.Location - InInstigator.Location) < 150.f * 150.f
		)
	{
		`AILog(GetFuncName()@"Called for"@InInstigator@"but we're moving to a rockworm cover slot so I ignored it!");
	}
	else
	{
		Super.ForcePauseAndRepath(InInstigator);
	}
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
	Super.CreateCheckpointRecord(Record);

	Record.DrivenTurretPathName = PathName(CurrentTurret);
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	local Turret FoundTurret;

	Super.ApplyCheckpointRecord(Record);

	if (MyGearPawn != None && Record.DrivenTurretPathName != "")
	{
		FoundTurret = Turret(FindObject(Record.DrivenTurretPathName, class'Turret'));
		if (FoundTurret != None)
		{
			UseTurret(FoundTurret, true, false);
		}
	}
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_CoverAI'

	DefaultReactConditionClasses.Add(class'AIReactCond_DmgLeaveCover')

	Begin Object Class=AIReactCond_StoppingPowerThreshold Name=StoppingPowerTemplate
	End Object
	StoppingPowerReactionTemplate=StoppingPowerTemplate

	// defaultcover goal evaluation profiles
	// ---->Enemies constraint
	Begin Object Class=CovGoal_Enemies Name=CovGoal_Enemies0
	End Object
	// ---->Enemies too close
	Begin object Class=CovGoal_EnemyProximity Name=CovGoal_EnemyProx0
	End object
	// ---->Weapon range constraint
	Begin object Class=CovGoal_WithinWeaponRange Name=CovGoal_WeaponRange0
	End Object
	// ---->Movement distance
	Begin Object Class=CovGoal_MovementDistance Name=CovGoal_MovDist0
		BestCoverDist=768.0f
		MinCoverDist=256.0f
		MaxCoverDist=1024.0f
	End object
	// ---->Squad leader proximity
	Begin Object Class=CovGoal_SquadLeaderProximity name=CovGoal_SquadLeadProx0
	End Object
	// ---->Teammate Proximity
	Begin Object Class=CovGoal_TeammateProximity name=CovGoal_TeammateProx0
	End Object
	// ---->within combat zones
	Begin Object Class=CovGoal_WithinCombatZones name=CovGoal_WithinCombatZones0
	End Object

	// ** TOWARDS
	Begin Object Class=Goal_AtCover name=AtCov_Towards0
		CoverGoalConstraints.Add(CovGoal_Enemies0)
		CoverGoalConstraints.Add(CovGoal_EnemyProx0)
		CoverGoalConstraints.Add(CovGoal_WeaponRange0)
		CoverGoalConstraints.Add(CovGoal_MovDist0)
		CoverGoalConstraints.Add(CovGoal_SquadLeadProx0)
		CoverGoalConstraints.Add(CovGoal_TeammateProx0)
		CoverGoalConstraints.Add(CovGoal_WithinCombatZones0)
	End object
	AtCover_Toward=AtCov_Towards0

	// ** AWAY
	// ---->Goal proximity constraint
	Begin object Class=CovGoal_GoalProximity Name=CovGoal_Proximity1
		BestGoalDist=1024.0f
		MinGoalDist=400.0f
		MaxGoalDist=2048.0f
	End Object
	Begin Object Class=Goal_AtCover name=AtCov_Away0
		CoverGoalConstraints.Add(CovGoal_Enemies0)
		CoverGoalConstraints.Add(CovGoal_EnemyProx0)
		CoverGoalConstraints.Add(CovGoal_Proximity1)
		CoverGoalConstraints.Add(CovGoal_MovDist0)
		CoverGoalConstraints.Add(CovGoal_SquadLeadProx0)
		CoverGoalConstraints.Add(CovGoal_TeammateProx0)
		CoverGoalConstraints.Add(CovGoal_WithinCombatZones0)
	End object
	AtCover_Away=AtCov_Away0

	// ** NEAR
	// ---->Goal proximity constraint
	Begin Object Class=CovGoal_GoalProximity Name=CovGoal_Proximity2
		BestGoalDist=256.0f
		MinGoalDist=16.0f
		MaxGoalDist=768.0f
	End Object
	Begin Object Class=Goal_AtCover Name=AtCov_Near0
		CoverGoalConstraints.Add(CovGoal_Enemies0)
		CoverGoalConstraints.Add(CovGoal_EnemyProx0)
		CoverGoalConstraints.Add(CovGoal_Proximity2)
		CoverGoalConstraints.Add(CovGoal_MovDist0)
		CoverGoalConstraints.Add(CovGoal_SquadLeadProx0)
		CoverGoalConstraints.Add(CovGoal_TeammateProx0)
		CoverGoalConstraints.Add(CovGoal_WithinCombatZones0)
	End Object
	AtCover_Near=AtCov_Near0

	Begin object Class=CovGoal_GoalProximity Name=CovGoal_Proximity3
		BestGoalDist=0.f
		MinGoalDist=0.f
		MaxGoalDist=512.f
		bHardLimits=TRUE
	End Object

	Begin Object Class=Goal_AtCover Name=AtCov_Squad0
		CoverGoalConstraints.Add(CovGoal_Enemies0)
		CoverGoalConstraints.Add(CovGoal_SquadLeadProx0)
		CoverGoalConstraints.Add(CovGoal_TeammateProx0)
		CoverGoalConstraints.Add(CovGoal_Proximity3)
	End Object
	AtCover_Squad=AtCov_Squad0

	Begin Object Class=CovGoal_ProtectedByLocation Name=CovGoal_ProtectedByLocation0
		bOnlySlotsWithFireLinks=TRUE
	End Object

	Begin Object Class=CovGoal_GoalProximity Name=CovGoal_Proximity4
		BestGoalDist=256.0f
		MinGoalDist=16.0f
		MaxGoalDist=768.0f
	End Object

	Begin Object Class=Goal_AtCover Name=AtCov_Ambush0
		CoverGoalConstraints.Add(CovGoal_ProtectedByLocation0)
		CoverGoalConstraints.Add(CovGoal_Proximity4)
		CoverGoalConstraints.Add(CovGoal_TeammateProx0)
		CoverGoalConstraints.Add(CovGoal_SquadLeadProx0)
		CoverGoalConstraints.Add(CovGoal_WithinCombatZones0)
	End Object
	AtCover_Ambush=AtCov_Ambush0

	Begin Object Class=CovGoal_MovementDistance Name=CovGoal_MovDist1
		BestCoverDist=384.0f
		MinCoverDist=256.0f
		MaxCoverDist=512.0f
		bMoveTowardGoal=TRUE
		MinDistTowardGoal=256.f
	End object

	Begin Object Class=CovGoal_ProtectedByLocation Name=CovGoal_ProtectedByLocation1
		bForceMoveTowardGoal=TRUE
		bOnlySlotsWithFireLinks=TRUE
	End Object

	Begin Object Class=Goal_AtCover Name=AtCover_AlongPath0
		CoverGoalConstraints.Add(CovGoal_ProtectedByLocation1)
		CoverGoalConstraints.Add(CovGoal_MovDist1)
	End Object
	AtCover_AlongPath=AtCover_AlongPath0
}
