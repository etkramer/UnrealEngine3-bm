/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class CombatZone extends Volume
	placeable
	hidecategories(Object,Actor,Brush,Volume,Movement,Attachment,Display,Navigation)
	native;

cpptext
{
	static ACombatZone* GetCombatZoneForNavPoint( ANavigationPoint* Nav );
	static UBOOL IsNavWithin( ANavigationPoint* Nav, ACombatZone* CZ );

	UBOOL IsAdjacentZone( ACombatZone* OtherZone );

	void PostEditMove(UBOOL bFinished);
	void CheckForErrors();

	virtual void GetActorReferences( TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel );
	virtual FGuid* GetGuid();
};

/** Display name of the combat zone */
var() String			ZoneName;
/** GUID used for linking zone across levels */
var() editconst const duplicatetransient guid ZoneGuid;

/** Max number of AI occupants for this zone 
	- the most AI that can be in the zone at one time, even if just moving through 
	- cannot be less than max residents if both are used (code will adjust at load) */
var()	private Byte MaxOccupants;
/** Occupants that want to move through this zone but cannot at the movement
	- only used when bDelayMovesAtMaxOccupancy is TRUE
	- all bookkeeping for this array handled in CheckForMovementDelay */
var		private array<Pawn>	PendingOccupants;
/** Max number of AI residents for this zone
	- the most AI that can say in this zone for combat */
var()	private Byte MaxResidents;

/** List of current occupants */
var array<Pawn>	Occupants_COG, 
				Occupants_Locust;
/** List of occupants that plan on entering the zone */
var array<Pawn> Residents_COG,
				Residents_Locust;

/** Designer specified priority for this volume */
var() const private float Priority_COG;
var() const private float Priority_Locust;

/** Zone is enabled/disabled */
var() bool	bEnabled;
/** Zone should delay movement of AI if max occupancy is reached */
var() bool	bDelayMovesAtMaxOccupancy;

/** List of cover slots in combat zone */
var const array<ActorReference>		CoverSlotRefs;
/** List of all nav points in combat zone */
var const array<ActorReference>		MyNavRefs;
/** Node on combat zone path network */
var() const editconst editinline FauxPathNode	NetworkNode;

var() GearTeamInfo.EGearTeam DelayMovesForTeam;

/** Rough center of combat zone based on all the pathnodes within it */
var const Vector	ZoneCenter;

/** Type of combat zone to help define actions of AI within them */
enum ECombatZoneType
{
	CZT_Normal,
	CZT_Ambush,
};
var() ECombatZoneType ZoneType;

/// CZT_AMBUSH VARIABLES ///
var() array<Actor>	AmbushTargets;

enum EMoveOverride
{
	EMO_None,
	EMO_Fast,
};

/** if you want AIs to move in a specific fashion while moving throught his combat zone, you may override that here. */
var() EMoveOverride MovementModeOverride;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// Handle initial setup of network node
	SetEnabled( bEnabled );

	// Handle initial setup of Max* vars
	SetMaxResidents( MaxResidents );
}

/** Returns the current priority for the given controller */
final function float GetPriority( GearAI AI )
{
	local int TeamIdx; 

	TeamIdx = AI.Pawn.GetTeamNum();
	if( TeamIdx == class'GearGame'.const.COG_TEAM_INDEX )
	{
		return Priority_COG;
	}
	else
	if( TeamIdx == class'GearGame'.const.LOCUST_TEAM_INDEX )
	{
		return Priority_Locust;
	}	
}

native final function bool IsValidZoneFor( GearAI AI, bool bResidenceQuery );
native final function bool IsOccupant( Pawn P );
native final function bool IsResident( Pawn P );

final function SetMaxResidents( Byte NewMax )
{
	MaxResidents = NewMax;

	// Cannot allow more residents than occupants
	// If setup that way, adjust max occupants
	if( MaxOccupants > 0 && 
		MaxResidents > 0 && 
		MaxOccupants < MaxResidents )
	{
		MaxOccupants = MaxResidents;
	}
}

final event AddOccupant( Pawn NewOccupant )
{
	local int TeamIdx, Idx;
	local GearPawn GP;

	TeamIdx = NewOccupant.GetTeamNum();
	if( TeamIdx == class'GearGame'.const.COG_TEAM_INDEX )
	{
		Idx = Occupants_COG.Find( NewOccupant );
		if( Idx < 0 )
		{
			Occupants_COG[Occupants_COG.Length] = NewOccupant;
		}
	}
	else
	if( TeamIdx == class'GearGame'.const.LOCUST_TEAM_INDEX )
	{
		Idx = Occupants_Locust.Find( NewOccupant );
		if( Idx < 0 )
		{
			Occupants_Locust[Occupants_Locust.Length] = NewOccupant;
		}
	}	

	// set movement mode if we have an override
	GP = GearPawn(NewOccupant);
	if(GP != none)
	{
		if(GP.MyGearAI != none)
		{
			`AILog_Ext("Entered combat zone"@self@MovementModeOverride@"as occupant.",,GP.MyGearAI);
			if(MovementModeOverride == EMO_Fast)
			{
				GP.MyGearAI.bShouldRoadieRun = (GP.bCanRoadieRun||GP.bCanBeForcedToRoadieRun);
			}
			
		}
	}


	//debug
	`log( self@GetFuncName()@NewOccupant@TeamIdx@"Cog#"@Occupants_COG.Length@"Loc#"@Occupants_Locust.Length, bDebug );
}

final event RemoveOccupant( Pawn OldOccupant )
{
	local int TeamIdx, Idx;
	local GearPawn GP;

	TeamIdx = OldOccupant.GetTeamNum();
	if( TeamIdx == class'GearGame'.const.COG_TEAM_INDEX )
	{
		Idx = Occupants_COG.Find( OldOccupant );
		if( Idx >= 0 )
		{
			Occupants_COG.Remove( Idx, 1 );
		}
	}
	else
	if( TeamIdx == class'GearGame'.const.LOCUST_TEAM_INDEX )
	{
		Idx = Occupants_Locust.Find( OldOccupant );
		if( Idx >= 0 )
		{
			Occupants_Locust.Remove( Idx, 1 );
		}
	}

	// re-set movement mode if we set it to somethign else before
	GP = GearPawn(OldOccupant);
	if(GP != none)
	{
		if(GP.MyGearAI != none)
		{
			`AILog_Ext("Left combat zone"@self@MovementModeOverride@"as occupant.",,GP.MyGearAI);
			if(MovementModeOverride != EMO_None)
			{
				GP.MyGearAI.bShouldRoadieRun = GP.MyGearAI.ShouldRoadieRun();
			}
		}
	}

	//debug
	`log( self@GetFuncName()@OldOccupant@TeamIdx@"Cog#"@Occupants_COG.Length@"Loc#"@Occupants_Locust.Length, bDebug );
}

final event AddResident( Pawn NewResident)
{
	local int TeamIdx, Idx;

	TeamIdx = NewResident.GetTeamNum();
	if( TeamIdx == class'GearGame'.const.COG_TEAM_INDEX )
	{
		Idx = Residents_COG.Find( NewResident );
		if( Idx < 0 )
		{
			Residents_COG[Residents_COG.Length] = NewResident;
		}
	}
	else
		if( TeamIdx == class'GearGame'.const.LOCUST_TEAM_INDEX )
		{
			Idx = Residents_Locust.Find( NewResident );
			if( Idx < 0 )
			{
				Residents_Locust[Residents_Locust.Length] = NewResident;
			}
		}	

		//debug
		`log( self@GetFuncName()@NewResident@TeamIdx@"Cog#"@Residents_COG.Length@"Loc#"@Residents_Locust.Length, bDebug );
}

final event RemoveResident( Pawn OldResident)
{
	local int TeamIdx, Idx;

	TeamIdx = OldResident.GetTeamNum();
	if( TeamIdx == class'GearGame'.const.COG_TEAM_INDEX )
	{
		Idx = Residents_COG.Find( OldResident );
		if( Idx >= 0 )
		{
			Residents_COG.Remove( Idx, 1 );
		}
	}
	else
		if( TeamIdx == class'GearGame'.const.LOCUST_TEAM_INDEX )
		{
			Idx = Residents_Locust.Find( OldResident );
			if( Idx >= 0 )
			{
				Residents_Locust.Remove( Idx, 1 );
			}
		}

		//debug
		`log( self@GetFuncName()@OldResident@TeamIdx@"Cog#"@Residents_COG.Length@"Loc#"@Residents_Locust.Length, bDebug );
} 

simulated function OnToggle( SeqAct_Toggle inAction )
{
	if( inAction.InputLinks[0].bHasImpulse )
	{
		// Turn on 
		SetEnabled( TRUE );
	}
	else
	if( inAction.InputLinks[1].bHasImpulse )
	{
		// Turn off
		SetEnabled( FALSE );
	}
	else
	if( inAction.InputLinks[2].bHasImpulse )
	{
		// Toggle 
		SetEnabled( !bEnabled );
	}
}

/*
 *	Handle all bookkeeping for enabling the zone
 */
simulated function SetEnabled( bool bNewEnabled )
{
	local GearAI AI;
	local int	 Idx;

	bEnabled = bNewEnabled;
	if( NetworkNode != None )
	{
		NetworkNode.bBlocked = !bEnabled;
	}

	if( !bEnabled )
	{
		for( Idx = 0; Idx < Residents_COG.Length; Idx++ )
		{
			AI = GearAI(Residents_COG[Idx].Controller);
			if( AI != None )
			{
				AI.SetAcquireCover( ACT_Immediate, self@'SetEnabled' );
			}
		}
		for( Idx = 0; Idx < Residents_Locust.Length; Idx++ )
		{
			AI = GearAI(Residents_Locust[Idx].Controller);
			if( AI != None )
			{
				AI.SetAcquireCover( ACT_Immediate, self@'SetEnabled' );
			}
		}

	}
}

/**
 *	Checks if given AI should stop moving until occupancy allows him to move through the zone
 *	Returns TRUE if AI should delay, FALSE otherwise
 *	Also handles bookkeeping of PendingOccupants/Occupants_* changes so AI moves in an orderly fashion
 */
simulated native function bool CheckForMovementDelay( GearAI AI );

simulated function String GetHumanReadableName()
{
	if( ZoneName != "" )
	{
		return ZoneName;
	}
	return Super.GetHumanReadableName();
}

defaultproperties
{
	Begin Object Class=CombatZoneRenderingComponent Name=CombatZoneRenderer
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(CombatZoneRenderer)
	BrushColor=(R=255,G=0,B=255,A=255)
	bColored=TRUE
//	bSolidWhenSelected=TRUE
	bEnabled=TRUE
	bCollideActors=FALSE
	bBlockActors=FALSE

	DelayMovesForTeam=TEAM_EVERYONE
}
