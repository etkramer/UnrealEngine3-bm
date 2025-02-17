/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearSquadFormation extends Info
	native(AI);

const MAXPOSITIONDIST	= 1024;
const MAXPOSITIONDISTSQ = 1048576; /*1024^2*/

enum EGearSquadFormationType
{
	GSF_None,
	GSF_Column,
	GSF_Line,
};

var GearSquad Squad;

struct native FormationPosition
{
	/** Name of position */
	var() Name	PosName;
	/** Position is relative to position index (-1 == Leader) */
	/** Note: must be an index less than this current position */
	var() int RelIdx;
	/** Ideal yaw offset to "foward" (as defined by leader) */
	var() int YawOffset;
	/** Ideal distance from leader position */
	var() float Distance;

	/** Last update time */
	var float	LastUpdateTime;
	/** Last update position */
	var Vector	LastUpdatePosition;
	/** Ideal position in world space */
	var Vector	IdealPosition;

	/** Current navigation point assigned to this position */
	var() transient NavigationPoint Nav;
	/** Current AI controller assigned to this position */
	var() transient GearAI		AI;

	var() Color DebugColor;

	structdefaultproperties
	{
		RelIdx=-1
	}
};

/** List of positions in the formation (leader is not included) */
var() editconst array<FormationPosition> Positions;

// old squad position to calc deltas for squad dir
var transient vector LastSquadPosition;
// used to determine what our last squad position was
var transient vector CurrentSquadPosition; 

native function int GetNumSquadMembersThatUsePositions();

function Controller GetSquadLeader()
{
	if( Squad != None )
	{
		return Squad.Leader;
	}
	return None;
}

function Actor GetSquadLeaderPosition()
{
	if( Squad != None )
	{
		return Squad.GetSquadLeaderPosition();
	}
	return None;
}

function Vector GetSquadLeaderLocation()
{
	if( Squad != None )
	{
		return Squad.GetSquadLeaderLocation();
	}
	return vect(0,0,0);
}

function int AssignPosition( GearAI AI )
{
	local int Idx;
	Idx = Positions.Find( 'AI', None );
	if( Idx >= 0 )
	{
		Positions[Idx].AI = AI;
	}
	return Idx;
}

function DismissPosition( GearAI AI )
{
	local int Idx;

	Idx = Positions.Find('AI',AI);
	if (Idx != -1)
	{
		Positions[Idx].AI = None;
	}
}

final function bool GetCurrentIdealPosition( GearAI AI, out Vector out_IdealPos )
{
	local int PosIdx;
	PosIdx = Positions.Find( 'AI', AI );
	if( PosIdx >= 0 )
	{
		out_IdealPos = Positions[PosIdx].IdealPosition;
		return TRUE;
	}
	return FALSE;
}

final function bool DoesIdealSquadPosOverlapAnother(GearAI AI)
{
	local int PosIdx;
	local int i;
	local float RadiusSq;
	PosIdx = Positions.Find( 'AI', AI );
	if( PosIdx >= 0 )
	{

		if(AI.Pawn != none)
		{
			RadiusSq = AI.Pawn.GetCollisionRadius();
			RadiusSq *= RadiusSq;
		}
		else
		{
			RadiusSq = 34.f * 34.f;
		}
		
		// assume the only positions we care about are at the front of the list
		for(i=0;i<Squad.SquadMembers.length;i++)
		{
			if(i != PosIdx)
			{
				if(VSizeSq(Positions[i].IdealPosition - Positions[PosIdx].IdealPosition) < RadiusSq)
				{
					return TRUE;
				}
			}
		}

	}
	return FALSE;
}

	

private final function Vector GetSquadLeaderDir( int Idx )
{
	local Vector LeaderDir, LeaderPos;
	local Pawn	 LeaderPawn;

	LeaderPos  = GetSquadLeaderLocation();
	if(GetSquadLeader() != none)
	{
		LeaderPawn = GetSquadLeader().Pawn;
		if(LeaderPawn != none)
		{
			LeaderPos = LeaderPawn.Location;
		}
	}	

	if( LeaderPawn != none &&  IsZero(LeaderPawn.Velocity) )
	{
		LeaderDir = vector(LeaderPawn.Rotation);
	}
	else
	{
		LeaderDir = Normal((LeaderPos - LastSquadPosition) * vect(1,1,0));
	}

	return LeaderDir;
}


/** Returns the ideal location for the specified formation position. */
private final function Vector GetIdealPosition( int Idx )
{
	local Vector IdealPosition, Pos, Start, LeaderDir;
	local Rotator Rot;
	local actor HitActor;
	local vector HitLocation,HitNormal;

	Start = (Positions[Idx].RelIdx < 0) ? GetSquadLeaderLocation() : GetIdealPosition( Positions[Idx].RelIdx );
	LeaderDir = GetSquadLeaderDir( Idx );

	Rot = rotator(LeaderDir * -1);
	Rot.Yaw += Positions[Idx].YawOffset;
	Pos.X = Positions[Idx].Distance;
	IdealPosition = Start + (Pos >> Rot);


	if(Idx < Squad.SquadMembers.length)
	{
		HitActor = Trace(HitLocation,HitNormal,IdealPosition,GetSquadLeaderLocation());
		if(HitActor != none)
		{
			IdealPosition = HitLocation + (HitNormal * GetSquadLeader().Pawn.GetCollisionRadius()*1.1f);
		}
	}

	return IdealPosition;
}

/** Finds new formation positions based on the leader's current position/orientation. */
final function UpdatePositions(optional int RequestedIdx = -1, optional bool bForce)
{
	local GearAI AI;
	local int Idx;

	// if there isn't a leader
	if( GetSquadLeader() == None || GetSquadLeader().Pawn == None )
	{
		// then no update is possible
		return;
	}

	// if this position doesn't need to be updated and this isn't forced
	if ( RequestedIdx >= 0 &&
		( TimeSince(Positions[RequestedIdx].LastUpdateTime) < 0.25f ||
		 (Positions[RequestedIdx].Nav != None && VSize(Positions[RequestedIdx].LastUpdatePosition - GetSquadLeaderLocation()) < 128.f) ) )
	{
		return;
	}

	for( Idx = 0; Idx < Positions.Length; Idx++ )
	{
		if( AI == None || AI.bDeleteMe )
		{
			AI = Positions[Idx].AI;
			
		}

		Positions[Idx].IdealPosition = GetIdealPosition( Idx );
	}

	LastSquadPosition = CurrentSquadPosition;
	if(GetSquadLeader() != none && GetSquadLeader().Pawn != none)
	{
		CurrentSquadPosition = GetSquadLeader().Pawn.Location;
	}
	else
	{
		CurrentSquadPosition = GetSquadLeaderLocation();
	}
	

	if( AI != None )
	{
		AI.FindSquadPosition( GetSquadLeader(), RequestedIdx );
	}
}

/**
 * Returns the navigation point assigned to the specified AI.
 */
final function NavigationPoint GetPosition( GearAI AI )
{
	local int Idx;

	//`AILog_Ext(GetFuncName()@AI.Pawn@AI.bIgnoreSquadPosition,,AI);
	if( AI.Pawn != None && !AI.bIgnoreSquadPosition )
	{
		//`AILog_Ext(GetFuncName()@GetSquadLeader()@GetSquadLeader().Pawn,,AI);
		if( GetSquadLeader() != None &&
			GetSquadLeader().Pawn != None )
		{
			Idx = Positions.Find( 'AI', AI );

			//`AILog_Ext(GetFuncName()@"Found index:"@Idx,,AI);
			if( Idx >= 0 )
			{
				UpdatePositions( Idx );
				//`AILog_Ext(GetFuncName()@"Returning Positions["$Idx$"].Nav"@Positions[Idx].Nav,,AI);
				return Positions[Idx].Nav;
			}
			else
			// Otherwise, if there is no position AI may be the leader
			if( GetSquadLeader() == AI )
			{
				// Return nav point near leaders position (probably just uses the anchor)
				//`AILog_Ext(GetFuncName()@"returning GetSquadLeaderPosition()"@GetSquadLeaderPosition(),,AI);
				return NavigationPoint(GetSquadLeaderPosition());
			}
		}
		else
		{
			//`AILog_Ext(GetFuncName()@"returning AI.Pawn.Anchor"@AI.Pawn.Anchor,,AI);
			return AI.Pawn.Anchor;
		}
	}

	`AILog_Ext(GetFuncName()@"failed and is returning none",,AI);
	return None;
}

final function ClearPositions( Actor LeaderPos, optional int PositionIdx = -1 )
{
	local int	PosIdx;

	// Go through each position
	for( PosIdx = 0; PosIdx < Positions.Length; PosIdx++ )
	{
		if( PositionIdx >= 0 && PositionIdx != PosIdx )
			continue;

		if( Positions[PosIdx].AI != None )
		{
			// Clear position info
			Positions[PosIdx].Nav = None;

			// Update 
			Positions[PosIdx].LastUpdateTime	 = WorldInfo.TimeSeconds;
			Positions[PosIdx].LastUpdatePosition = LeaderPos.Location;
		}
	}
}

`if(`notdefined(FINAL_RELEASE))
simulated function Tick( float DeltaTime )
{
//	local int Idx;

	super.Tick( DeltaTime );

///*
//	bDebug=TRUE;
//*/
//	if( GetSquadLeader() != None &&
//		GetSquadLeader().Pawn != None &&
//		!GetSquadLeader().Pawn.IsHumanControlled() )
//	{
////		bDebug = TRUE;
//	}
//	else
//	{
//		disable('Tick');
//	}

	//if(GetSquadLeader() != none)
	//{
	//	DrawDebugCylinder(GetSquadLeader().Pawn.Location,GetSquadLeader().Pawn.Location,25.f,3,255,255,0);
	//}
	//for( Idx = 0; Idx < Positions.Length; Idx++ )
	//{
	//	if(Positions[Idx].AI != none)
	//	{
	//		DrawDebugCylinder(Positions[Idx].IdealPosition,Positions[Idx].IdealPosition,25.f,3,255,255,255);
	//		DrawDebugLine(Positions[Idx].IdealPosition,Positions[Idx].AI.Pawn.Location,255,255,255);
	//		if(Positions[Idx].Nav != none)
	//		{
	//			DrawDebugLine(Positions[Idx].IdealPosition,Positions[Idx].Nav.Location,100,255,100);
	//		}
	//		
	//	}
	//	else
	//	{
	//		DrawDebugCylinder(Positions[Idx].IdealPosition,Positions[Idx].IdealPosition,25.f,3,100,255,100);
	//		if(Positions[Idx].Nav != none)
	//		{
	//			DrawDebugLine(Positions[Idx].IdealPosition,Positions[Idx].Nav.Location,100,255,100);
	//		}
	//	}
	//}

	if( bDebug )
	{
		//FlushPersistentDebugLines();
		//for( Idx = 0; Idx < Positions.Length; Idx++ )
		//{
		//	if( Positions[Idx].AI != None &&
		//		Positions[Idx].AI.Pawn != None )
		//	{
		//		if( GetSquadLeader() != None )
		//		{
		//			// Ideal position little box w/ line to leader location
		//			DrawDebugBox( Positions[Idx].IdealPosition + vect(0,0,50), vect(5,5,5), Positions[Idx].DebugColor.R/2, Positions[Idx].DebugColor.G/2, Positions[Idx].DebugColor.B/2, TRUE );
		//			DrawDebugLine( Positions[Idx].IdealPosition + vect(0,0,50), GetSquadLeaderLocation(), Positions[Idx].DebugColor.R/2, Positions[Idx].DebugColor.G/2, Positions[Idx].DebugColor.B/2, TRUE );
		//		}

		//		if( Positions[Idx].Nav != None )
		//		{
		//			// Actual position larger box w/ line to ideal and AI pawn
		//			DrawDebugBox( Positions[Idx].Nav.Location + vect(0,0,50), vect(10,10,10), Positions[Idx].DebugColor.R, Positions[Idx].DebugColor.G, Positions[Idx].DebugColor.B, TRUE );
		//			DrawDebugLine( Positions[Idx].Nav.Location + vect(0,0,50), Positions[Idx].IdealPosition + vect(0,0,50), Positions[Idx].DebugColor.R, Positions[Idx].DebugColor.G, Positions[Idx].DebugColor.B, TRUE );
		//			DrawDebugLine( Positions[Idx].Nav.Location + vect(0,0,50), Positions[Idx].AI.Pawn.Location, Positions[Idx].DebugColor.R, Positions[Idx].DebugColor.G, Positions[Idx].DebugColor.B, TRUE );
		//		}
		//	}
		//}
		//// White sphere around leader position w/ line to leader
		//DrawDebugCylinder(GetSquadLeaderLocation(),GetSquadLeaderLocation(),25.f,3,255,255,0,TRUE);
		//DrawDebugLine( GetSquadLeaderLocation(), GetSquadLeader().Pawn.Location, 255, 255, 255, TRUE );
		//DrawDebugCoordinateSystem( GetSquadLeaderLocation(), Rotator(GetSquadLeaderDir( -1 )), 64.f, TRUE );
	}
}
`endif

defaultproperties
{
	Positions(0)=(PosName="LeftFlank",RelIdx=-1,YawOffset=8348,Distance=256.f,DebugColor=(R=255,G=0,B=0))
	Positions(1)=(PosName="RightFlank",RelIdx=-1,YawOffset=-8384,Distance=256.f,DebugColor=(R=0,G=0,B=255))
	Positions(2)=(PosName="LeftFlankRear_0",RelIdx=0,YawOffset=0,Distance=256.f,DebugColor=(R=0,G=255,B=0))
	Positions(3)=(PosName="RightFlankRear_0",RelIdx=1,YawOffset=0,Distance=256.f,DebugColor=(R=255,G=255,B=0))
	Positions(4)=(PosName="LeftFlankRear_1",RelIdx=2,YawOffset=0,Distance=256.f,DebugColor=(R=0,G=255,B=0))
	Positions(5)=(PosName="RightFlankRear_1",RelIdx=3,YawOffset=0,Distance=256.f,DebugColor=(R=255,G=255,B=0))
	Positions(6)=(PosName="LeftFlankRear_2",RelIdx=4,YawOffset=0,Distance=256.f,DebugColor=(R=0,G=255,B=0))
	Positions(7)=(PosName="RightFlankRear_2",RelIdx=5,YawOffset=0,Distance=256.f,DebugColor=(R=255,G=255,B=0))
}
