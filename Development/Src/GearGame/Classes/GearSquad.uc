/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearSquad extends Info
	config(AI)
	native(AI)
	dependson(GameplayRouteNode);


/** Name of squad */
var() editconst  Name SquadName;

/** team this squad is on */
var GearTeamInfo Team;

/** Struct that info about each squad member */
struct native SquadMemberInfo
{
	var	Controller	Member;
};
/** Array of all squad members */
var array<SquadMemberInfo>	SquadMembers;
/** Controller for leader of the squad */
var() editconst Controller Leader;

/** Formation this squad should position themselves in */
var GearSquadFormation	Formation;

/** Gameplay route squad should move along when not in combat */
var	GameplayRoute SquadRoute;
/** Previous index in squad route */
var int	  SquadRouteIndex;

/** whether this squad has a human controlled player in it */
var bool bPlayerSquad;

/**
 *	Struct to track AI knowledge of EnemyList
 */
struct native PlayerInfo
{
	/** Last world time that info for this player was updated */
	var float		LastUpdateTime;

	/** Pawn associated with this info */
	var	Pawn		Pawn;
	/** precase of Pawn to a GearPawn */
	var GearPawn GearPawn;
	/** Last known location of the pawn
	 *  If pawn is still in the cover info same cover (matching PlayerInfo.Cover)
	 *  this is an OFFSET from the slot location (allows tracking of dynamic cover) */
	var Vector		KnownLocation;

	/** Last known cover */
	var CoverInfo	Cover;
	/** Last known Base */
	var Actor		Base;

	/** Initial world time when visual contact was made */
	var float		InitialSeenTime;
	/** Last world time that visual contact was made */
	var float		LastSeenTime;
	/** Last time a notification for info was put out */
	var float		LastNotifyTime;
};

/** List of known enemies */
var array<PlayerInfo>		EnemyList;

///// STIMULUS RESPONSE VARIABLES /////
enum EPerceptionType
{
	PT_Sight,		// AI sees enemy
	PT_Heard,		// AI heard enemy
	PT_HurtBy,		// AI hurt by enemy
	PT_Force,		// AI forced to know everything (used for initial acquisition)
	PT_SightPlayer, // AI sees player
};

/** Used to retrieve location of an enemy */
enum ELocationType
{
	LT_Known,				// Retrieve known location  (Last place we got an exact location)
	LT_Exact,				// Retreive exact location
	LT_InterpVisibility,	// Uses exact location when visible, and performs an interpolation from known location to exact location when enemy becomes visible.
};

/** Struct used to delay perception update */
struct native DelayUpdateInfo
{
	/** Type of perception to update */
	var EPerceptionType	Type;
	/** Enemy pawn to update */
	var Pawn			Pawn;
	/** Update enemy info after WorldTime > UpdateTime */
	var float			UpdateTime;
	/** Identifying event name -- ie NoiseType/Calling Event */
	var Name			EventName;
};

/** List of perceptors to update */
var array<DelayUpdateInfo>	DelayUpdateList;

/** if set, when this squad receives a stimulus, it sends it to all other squads on the same team that also have this flag set */
var bool bInterSquadCommunication;

/** Cover Decay
 * this system is designed to force the AI to eventually move to new cover, even if it thinks its current cover is better,
 * primarily for two reasons:
 * 1) Enemies whose positions are unknown to the AI are increasingly likely to flank it the longer it stays in one place
 * 2) If combat isn't getting anywhere in its current cover, then it should acquire new cover, even cover it thinks is not as good,
 * either to surprise the enemy, to force the enemy to respond, or simply as an intentional mistake to resolve combat by the AI
 * getting itself killed
 */
struct native DecayedCover
{
	/** marker to which extra weight should be applied. Adjacent slots are also affected to a lesser amount. */
	var CoverSlotMarker CoverMarker;
	/** extra cost to add to selecting this marker as cover */
	var int ExtraCoverCost;
};
var const array<DecayedCover> DecayedCoverList;
/** map of final cost values for quick lookup when pathfinding */
var native const Map_Mirror DecayedCoverMap{TMap<ACoverSlotMarker*, INT>};

/** percentage of extra cost that is passed on to adjacent slots */
var float AdjacentDecayMult;
/** amount of extra cost that is removed per second */
var int DecayRecoveryPerSecond;
/** time remaining until next decay recovery */
var float DecayRecoveryIntervalRemaining;

cpptext
{
private:
	void UpdateDecayedCoverMap();
public:
	virtual void TickSpecial(FLOAT DeltaTime);
}

/////// NATIVE FUNCTIONS /////////
/**
 *	Process a given type of stimulus and update the EnemyList appropriately.
 *	If update was successful, it will call BroadcastStimulus
 */
native final function bool ProcessStimulus(const GearAI AI, Pawn Enemy, EPerceptionType Type, Name EventName);

/**
 *	Broadcast a processed stimulus to all squad members
 */
native final function bool BroadcastStimulus( Pawn Enemy, EPerceptionType Type, Name EventName );

/**
 *	Update known enemy location AND cover
 *	Properly handles setting location as an offset for Enemies
 *		- In Cover
 *		- On movable objects (Base==InterpActor)
 */
final native function SetKnownEnemyInfo( int EnemyIdx, GearPawn E, Vector EnemyLoc );

/**
 *	Adds an enemy to the EnemyList if they aren't already there
 *	Returns the index into the EnemyList for given enemy
 */
final native function int AddEnemy(const GearAI AI, Pawn NewEnemy);

/**
 *	Removes enmey from the EnemyList
 */
final native function bool RemoveEnemy( Pawn DeadEnemy );

/**
 *	Searches EnemyList for the given pawn and returns the index
 */
final native function int	GetEnemyIndex( Pawn Enemy );

/**
 * Gets position of given pawn by LT_* type
 */
final native function vector GetEnemyLocation( Pawn TestPawn, optional ELocationType LT );

/**
* Gets position of pawn in given index of EnemyList by LT_* type
*/
final native function vector GetEnemyLocationByIndex( int Idx, optional ELocationType LT );

 /**
  * Gets the last seent ime for a given pawn
  */
final native function float GetEnemyLastSeenTime( Pawn TestPawn );

/**
 * Find the position the squad leader is at, or in the case of the AI, where they are attempting to go.
 */
final native function Actor GetSquadLeaderPosition();

/**
 * Find the actual location of the squad leader.
 */
final native function vector GetSquadLeaderLocation();

/**
 * Retrieves enemy cover info
 */
final native function CoverInfo GetEnemyCover( Pawn TestPawn );

/**
 *	Returns all enemies in the EnemyList that are of the specified class or a subclass
 *  (Note: this function is only useful on the server)
 */
native final iterator function AllEnemies( class<Pawn> BaseClass, out Pawn P, optional Controller Asker);

/**
 *	Returns all members in the squad of the specified class or a subclass
 *	(Note: this function is only useful on the server)
 */
native final iterator function AllMembers( class<Controller> BaseClass, out Controller C );

/** adds the specified extra decay cost to the given CoverSlotMarker */
native final function AddCoverDecay(CoverSlotMarker CoverMarker, int Amount);

/** calculates the centroid of the the squad members */
native final function vector GetSquadCentroid();

////// END NATIVE FUNCTIONS //////

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	if( Role == ROLE_Authority )
	{
		Formation = Spawn( class'GearSquadFormation' );
		Formation.Squad = self;
	}
}

/** Returns total number of enemies known by squad */
final function int GetNumEnemies()
{
	return EnemyList.Length;
}

/** returns whether the new guy can take over this squad */
final function bool AllowLeaderChange(Controller NewMember)
{
	local PlayerController PC;

	// if both are players, first player gets priority
	if (PlayerController(Leader) == None || PlayerController(NewMember) == None)
	{
		return true;
	}
	else
	{
		// ControllerList is in reverse order of joins, so the player we encounter first in the list loses
		foreach WorldInfo.AllControllers(class'PlayerController', PC)
		{
			if (PC == Leader)
			{
				return true;
			}
			else if (PC == NewMember)
			{
				return false;
			}
		}

		// should be impossible, but default to true just in case
		return true;
	}
}

/** Add squad member to this squad and do all the associated bookkeeping */
final function RegisterSquadMember( Controller NewMember, bool bLeader )
{
	local int		Idx;
	local GearAI	AI;
	local GearPC	PC;
	local GearPRI	PRI;

	if (NewMember != None)
	{
		if (SquadMembers.Find('Member', NewMember) != INDEX_NONE)
		{
			// already in squad
			if (bLeader && AllowLeaderChange(NewMember))
			{
				Leader = NewMember;
			}
		}
		else
		{
			// make sure they're not already a member
			UnregisterSquadMember(NewMember);

			// set them to the leader if necessary
			if( bLeader && AllowLeaderChange(NewMember) )
			{
				Leader = NewMember;
			}

			Idx = SquadMembers.Length;
			SquadMembers.Length = Idx + 1;
			SquadMembers[Idx].Member = NewMember;

			AI = GearAI(NewMember);
			// if it's an AI
			if( AI != None )
			{
				//debug
				`AILog_Ext( self@GetFuncName()@SquadName@"leader:"@Leader, 'None', AI );

				// set the squad reference
				AI.Squad = self;
				if( !bLeader && !AI.bIgnoreSquadPosition )
				{
					Idx = Formation.AssignPosition( AI );
				}
			}
			else
			{
				PC = GearPC(NewMember);
				if( PC != None )
				{
					bPlayerSquad=true;
					PC.Squad = self;
				}
				Idx = -1;
			}

			PRI = GearPRI(NewMember.PlayerReplicationInfo);
			if( PRI != None )
			{
				PRI.SquadName = SquadName;
			}

			if( AI == None || !AI.bIgnoreSquadPosition )
			{
				// Update the positions, all if a new leader was assigned
				Formation.UpdatePositions( bLeader ? -1 : Idx, TRUE );
			}
		}
	}
}

/** Remove squad member from this squad and clean up */
final function UnregisterSquadMember( Controller OldMember )
{
	local GearAI	AI;
	local GearPC	PC;
	local GearPRI	PRI;
	local int		Idx;

	if (OldMember != None)
	{
		Idx = SquadMembers.Find( 'Member', OldMember );
		if( Idx >= 0 )
		{
			AI = GearAI(OldMember);

			SquadMembers.Remove( Idx, 1 );

			// If they were the leader
			if( OldMember == Leader )
			{
				// If leader was AI, select a new leader
				if( AI != None && SquadMembers.Length > 0 )
				{
					Leader = SquadMembers[0].Member;
				}
				else
				{
					// Otherwise, clear leader
					Leader = None;
				}
			}

			// if it's an AI
			if (AI != None)
			{
				//debug
				`AILog_Ext( self@GetFuncName()@SquadName, 'None', AI );


				// remove the reference to this squad
				if( AI.Squad == self )
				{
					AI.Squad = None;
				}

				Formation.DismissPosition( AI );
			}
			else
			{
				PC = GearPC(OldMember);
				if( PC != None )
				{
					if( PC.Squad == self )
					{
						// if we're a player and we're leaving the squad do a sweep to see if there are any other players on this squad, if not reset bPlayerSquad
						PC.Squad = None;
						bPlayerSquad=false;
						foreach AllMembers(class'GearPC', PC)
						{
							bPlayerSquad=true;
							break;
						}
					}
				}
			}
			PRI = GearPRI(OldMember.PlayerReplicationInfo);
			if( PRI != None )
			{
				PRI.SquadName = '';
			}

			// If there is no one left in the squad, destroy the squad
			if( SquadMembers.Length == 0 )
			{
				//`log("deleting squad"@SquadName@"becuase it's empty!");
				Formation.Destroy();
				Destroy();
			}
		}
	}

	// If there is no one left in the squad, clear enemies
	if( SquadMembers.Length == 0 )
	{
		EnemyList.Length = 0;
	}
}

/** Grabs the list members in this squad. */
final function GetMembers(out array<Controller> out_Members)
{
	local Controller C;
	foreach AllMembers( class'Controller', C )
	{
		out_Members[out_Members.Length] = C;
	}
}

/**
 *	Squad notification for a DBNO squad member
 *	See if anyone is available in the squad to go revive them
 */
final function NotifyMemberDBNO( GearPawn DownMember )
{
	local GearAI	AI, BestHelp;
	local float		Rating, BestRating;

	if (DownMember == None || DownMember.IsInPainVolume() || IsTryingToRevive(DownMember))
	{
		return;
	}

	foreach AllMembers( class'GearAI', AI )
	{
		// notify reaction manager that a squad mate is DBNO
		if(AI.Pawn != DownMember)
		{
			AI.ReactionManager.NudgeChannel(DownMember,'SquadMateDBNO');
		}


		if( AI.Pawn == DownMember || !AI.CanRevivePawn( DownMember ) )
		{
			continue;
		}

		// send the guy with the best (lowest) rating
		Rating = AI.GetReviveRatingFor(DownMember);
		//`log(GetFuncName()@AI@Rating);
		if( BestRating == 0 || Rating < BestRating )
		{
			BestHelp	= AI;
			BestRating	= Rating;
		}
	}
	//`log(GetFuncName()@"Chose:"@BestHelp);

	if (BestHelp != None)
	{
		// if we're not a TDM AI, and the dude that's down is a player just go.. otherwise go if they're within 1200 units
		if((GearAI_TDM(BestHelp) == none && DownMember.IsHumanControlled()) || BestRating < 1200)
		{
			BestHelp.GoReviveTeammate( DownMember );
		}
	}
}

final function NotifyEnemyDBNO( GearPawn DownEnemy )
{
	local GearAI	AI, BestHelp;
	local float Dist, BestDist;

	if (DownEnemy == None || DownEnemy.IsInPainVolume() || IsTryingToExecute(DownEnemy))
	{
		return;
	}

	// increase range if we're playing a multiplayer gametype where we need to execute enemies to kill them
	if (GearGameMP_Base(WorldInfo.Game) != None && GearGameMP_Base(WorldInfo.Game).bExecutionRules)
	{
		BestDist = 1500.0;
	}
	else
	{
		BestDist = 1024.0;
	}
	foreach AllMembers( class'GearAI', AI )
	{
		if( AI.Pawn == DownEnemy || !AI.CanExecutePawn( DownEnemy ) )
		{
			continue;
		}

		// Just send the closest guy for now
		Dist = VSize(DownEnemy.Location - AI.Pawn.Location);
		if (Dist < BestDist)
		{
			BestHelp = AI;
			BestDist = Dist;
		}
	}

	if( BestHelp != None )
	{
		BestHelp.GoExecuteEnemy( DownEnemy );
	}
}

final function NotifyReactingToMeatShield( GearPawn ReactingPawn, GearPawn MSEnemy)
{
	local GearAI_Cover AI;


	// reset everyone's timers so we don't double up on reactions
	foreach AllMembers( class'GearAI_Cover', AI )
	{
		AI.LastMeatShieldReactionTime = WorldInfo.TimeSeconds;
	}
}

/** @return whether someone on this Squad is attempting to execute the given target */
final function bool IsTryingToExecute(GearPawn Target)
{
	local Controller C;
	local GearAI AI;
	local AICmd_Execute ExecuteCommand;

	if (Target.IsDBNO())
	{
		foreach AllMembers(class'Controller', C)
		{
			// if it's an AI, read the command stack to find out
			AI = GearAI(C);
			if (AI != None)
			{
				ExecuteCommand = AI.FindCommandOfClass(class'AICmd_Execute');
				if (ExecuteCommand != None && ExecuteCommand.ExecuteTarget == Target)
				{
					return true;
				}
			}
			// otherwise just assume if they're close they might do it
			else if (C.Pawn != None && VSize(C.Pawn.Location - Target.Location) < 1024.0)
			{
				return true;
			}
		}
	}

	return false;
}

/** @return whether someone on this Squad is attempting to revive the given target */
final function bool IsTryingToRevive(GearPawn Target)
{
	local GearAI AI;
	local AICmd_Revive ReviveCommand;

	if (Target.IsDBNO())
	{
		//@note: we don't assume a close by human is going to do it!
		foreach AllMembers(class'GearAI', AI)
		{
			// if it's an AI, read the command stack to find out
			ReviveCommand = AI.FindCommandOfClass(class'AICmd_Revive');
			if (ReviveCommand != None && ReviveCommand.ReviveTarget == Target)
			{
				return true;
			}
		}
	}

	return false;
}

/**
 *	Reverse Turtling - increases chance AI will get back into cover when we out number the other team
 *	Chance decreases as this squad starts to loose the advantage.
 */
final function native float GetChanceToDuckBackIntoCover( GearAI AI );

function OnAISquadController( SeqAct_AISquadController inAction )
{
	local class<GearSquadFormation> GSF;
	local GearAI AI;

	//`Log(GetFuncName()@inAction.FormationType@"SquadRote (len):"@inAction.SquadRoute.length@"[0]="@inAction.SquadRoute[0]@inAction.PerceptionMood@inAction.CombatMood@inAction.MoveMood);


	if( inAction.SquadRoute.Length > 0 )
	{
		SetSquadRoute( inAction.SquadRoute[0] );
	}
	else
	{
		SetSquadRoute( none );
	}

	if( inAction.FormationType != GSF_None )
	{
		switch( inAction.FormationType )
		{
			case GSF_Line:
				GSF = class'GearSquadFormation_Line';
				break;
			case GSF_Column:
			default:
				GSF = class'GearSquadFormation';
				break;
		}

		if( Formation == None || Formation.Class != GSF )
		{
			if( Formation != None )
			{
				foreach AllMembers( class'GearAI', AI )
				{
					if( AI != Leader )
					{
						Formation.DismissPosition( AI );
					}
					AI.LastSquadLeaderPosition = vect(0,0,0);
				}
			}

			Formation = Spawn( GSF );
			Formation.Squad = self;

			if( Formation != None )
			{
				foreach AllMembers( class'GearAI', AI )
				{
					if( AI != Leader && !AI.bIgnoreSquadPosition )
					{
						Formation.AssignPosition( AI );
					}
				}
				Formation.UpdatePositions( -1, TRUE );
			}

			//debug
			`log( self@SquadName@GetFuncName()@"Changed formation to"@GSF );
		}
	}

	if( inAction.PerceptionMood != AIPM_None )
	{
		foreach AllMembers( class'GearAI', AI )
		{
			AI.SetPerceptionMood( inAction.PerceptionMood );
		}
	}
	if( inAction.CombatMood != AICM_None )
	{
		foreach AllMembers( class'GearAI', AI )
		{
			AI.SetCombatMood( inAction.CombatMood );
		}
	}
	if( inAction.MoveMood != AIMM_None )
	{
		foreach AllMembers( class'GearAI', AI )
		{
			AI.SetMovementMood( inAction.MoveMood );
		}
	}

	if( inAction.bClearTethers )
	{
		foreach AllMembers( class'GearAI', AI )
		{
			AI.ClearMovementInfo();
		}
	}
}

final event SetSquadRoute( GameplayRoute RG )
{
	SetSquadRouteIndex( -1 );
	SquadRoute = RG;
}

final event SetSquadRouteIndex( int Idx )
{
	local GearAI AI;
	local GameplayRouteNode GRN;
	foreach AllMembers( class'GearAI', AI )
	{
		`AILog_Ext(GetFuncName()@"new squad idx:"@Idx@"OldIdx:"@SquadRouteIndex,,AI);
		AI.LastSquadLeaderPosition = vect(0,0,0);

		// first notify the GRN that this AI is leaving our old node if any
		if (SquadRouteIndex >= 0)
		{
			GRN = GameplayRouteNode(SquadRoute.RouteList[SquadRouteIndex].Actor);
			if(GRN != none)
			{
				GRN.OnUnAssignedFromThisNode(AI);
			}
		}

		if( Idx >= 0 )
		{
			GRN = GameplayRouteNode(SquadRoute.RouteList[Idx].Actor);
			if(GRN != none)
			{
				GRN.OnAssignedToThisNode(AI);
			}
		}
	}
	SquadRouteIndex = Idx;
}

defaultproperties
{
	SquadRouteIndex=-1
	AdjacentDecayMult=0.5
	DecayRecoveryPerSecond=40
}
